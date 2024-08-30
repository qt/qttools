// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qtresourceeditordialog_p.h"
#include "ui_qtresourceeditordialog.h"
#include "qtresourcemodel_p.h"
#include "iconloader_p.h"

#include <abstractdialoggui_p.h>

#include <QtDesigner/abstractsettings.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtCore/qcompare.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qcoreapplication.h>
#include <QtXml/qdom.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtGui/qstandarditemmodel.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto rccRootTag = "RCC"_L1;
static constexpr auto rccTag = "qresource"_L1;
static constexpr auto rccFileTag = "file"_L1;
static constexpr auto rccAliasAttribute = "alias"_L1;
static constexpr auto rccPrefixAttribute = "prefix"_L1;
static constexpr auto rccLangAttribute = "lang"_L1;
static constexpr auto SplitterPosition = "SplitterPosition"_L1;
static constexpr auto ResourceEditorGeometry = "Geometry"_L1;
static constexpr auto QrcDialogC = "QrcDialog"_L1;

static QString msgOverwrite(const QString &fname)
{
    return QCoreApplication::translate("QtResourceEditorDialog", "%1 already exists.\nDo you want to replace it?").arg(fname);
}

static QString msgTagMismatch(const QString &got, const QString &expected)
{
    return QCoreApplication::translate("QtResourceEditorDialog", "The file does not appear to be a resource file; element '%1' was found where '%2' was expected.").arg(got, expected);
}

namespace qdesigner_internal {

// below 3 data classes should be derived from QSharedData and made implicit shared class
struct QtResourceFileData
{
    QString path;
    QString alias;

    friend bool comparesEqual(const QtResourceFileData &lhs,
                              const QtResourceFileData &rhs) noexcept
    {
        return lhs.path == rhs.path && lhs.alias == rhs.alias;
    }
    Q_DECLARE_EQUALITY_COMPARABLE(QtResourceFileData)
};

struct QtResourcePrefixData
{
    QString prefix;
    QString language;
    QList<QtResourceFileData> resourceFileList;

    friend bool comparesEqual(const QtResourcePrefixData &lhs,
                              const QtResourcePrefixData &rhs) noexcept
    {
        return lhs.prefix == rhs.prefix && lhs.language == rhs.language
                && lhs.resourceFileList == rhs.resourceFileList;
    }
    Q_DECLARE_EQUALITY_COMPARABLE(QtResourcePrefixData)
};

struct QtQrcFileData
{
    QString qrcPath;
    QList<QtResourcePrefixData> resourceList;

    friend bool comparesEqual(const QtQrcFileData &lhs,
                              const QtQrcFileData &rhs) noexcept
    {
        return lhs.qrcPath == rhs.qrcPath && lhs.resourceList == rhs.resourceList;
    }
    Q_DECLARE_EQUALITY_COMPARABLE(QtQrcFileData)
};

} // namespace qdesigner_internal

using QtResourcePrefixData = qdesigner_internal::QtResourcePrefixData;
using QtResourceFileData = qdesigner_internal::QtResourceFileData;
using QtQrcFileData = qdesigner_internal::QtQrcFileData;

static bool loadResourceFileData(const QDomElement &fileElem, QtResourceFileData *fileData,
                                 QString *errorMessage)
{
    if (!fileData)
        return false;

    if (fileElem.tagName() != rccFileTag) {
        *errorMessage = msgTagMismatch(fileElem.tagName(), rccFileTag);
        return false;
    }

    QtResourceFileData &data = *fileData;

    data.path = fileElem.text();
    data.alias = fileElem.attribute(rccAliasAttribute);

    return true;
}

static bool loadResourcePrefixData(const QDomElement &prefixElem, QtResourcePrefixData *prefixData, QString *errorMessage)
{
    if (!prefixData)
        return false;

    if (prefixElem.tagName() != rccTag) {
        *errorMessage = msgTagMismatch(prefixElem.tagName(), rccTag);
        return false;
    }

    QtResourcePrefixData &data = *prefixData;

    data.prefix = prefixElem.attribute(rccPrefixAttribute);
    data.language = prefixElem.attribute(rccLangAttribute);
    QDomElement fileElem = prefixElem.firstChildElement();
    while (!fileElem.isNull()) {
        QtResourceFileData fileData;
        if (!loadResourceFileData(fileElem, &fileData, errorMessage))
            return false;
        data.resourceFileList.append(fileData);
        fileElem = fileElem.nextSiblingElement();
    }
    return true;
}

static bool loadQrcFileData(const QDomDocument &doc, const QString &path, QtQrcFileData *qrcFileData, QString *errorMessage)
{
    if (!qrcFileData)
        return false;

    QtQrcFileData &data = *qrcFileData;

    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() != rccRootTag) {
        *errorMessage = msgTagMismatch(docElem.tagName(), rccRootTag);
        return false;
    }

    QDomElement prefixElem = docElem.firstChildElement();
    while (!prefixElem.isNull()) {
        QtResourcePrefixData prefixData;
        if (!loadResourcePrefixData(prefixElem, &prefixData, errorMessage))
            return false;
        data.resourceList.append(prefixData);
        prefixElem = prefixElem.nextSiblingElement();
    }

    data.qrcPath = path;

    return true;
}

static QDomElement saveResourceFileData(QDomDocument &doc, const QtResourceFileData &fileData)
{
    QDomElement fileElem = doc.createElement(rccFileTag);
    if (!fileData.alias.isEmpty())
        fileElem.setAttribute(rccAliasAttribute, fileData.alias);

    QDomText textElem = doc.createTextNode(fileData.path);
    fileElem.appendChild(textElem);

    return fileElem;
}

static QDomElement saveResourcePrefixData(QDomDocument &doc, const QtResourcePrefixData &prefixData)
{
    QDomElement prefixElem = doc.createElement(rccTag);
    if (!prefixData.prefix.isEmpty())
        prefixElem.setAttribute(rccPrefixAttribute, prefixData.prefix);
    if (!prefixData.language.isEmpty())
        prefixElem.setAttribute(rccLangAttribute, prefixData.language);

    for (const QtResourceFileData &rfd : prefixData.resourceFileList) {
        QDomElement fileElem = saveResourceFileData(doc, rfd);
        prefixElem.appendChild(fileElem);
    }

    return prefixElem;
}

static QDomDocument saveQrcFileData(const QtQrcFileData &qrcFileData)
{
    QDomDocument doc;
    QDomElement docElem = doc.createElement(rccRootTag);
    for (const QtResourcePrefixData &prefixData : qrcFileData.resourceList) {
        QDomElement prefixElem = saveResourcePrefixData(doc, prefixData);

        docElem.appendChild(prefixElem);
    }
    doc.appendChild(docElem);

    return doc;
}

namespace qdesigner_internal {

// --------------- QtResourceFile
class QtResourceFile {
public:
    friend class QtQrcManager;

    QString path() const { return m_path; }
    QString alias() const { return m_alias; }
    QString fullPath() const { return m_fullPath; }
private:
    QtResourceFile() = default;

    QString m_path;
    QString m_alias;
    QString m_fullPath;
};

class QtResourcePrefix {
public:
    friend class QtQrcManager;

    QString prefix() const { return m_prefix; }
    QString language() const { return m_language; }
    QList<QtResourceFile *> resourceFiles() const { return m_resourceFiles; }
private:
    QtResourcePrefix() = default;

    QString m_prefix;
    QString m_language;
    QList<QtResourceFile *> m_resourceFiles;

};
// ------------------- QtQrcFile
class QtQrcFile {
public:
    friend class QtQrcManager;

    QString path() const { return m_path; }
    QString fileName() const { return m_fileName; }
    QList<QtResourcePrefix *> resourcePrefixList() const { return m_resourcePrefixes; }
    QtQrcFileData initialState() const { return m_initialState; }

private:
    QtQrcFile() = default;

    void setPath(const QString &path) {
        m_path = path;
        QFileInfo fi(path);
        m_fileName = fi.fileName();
    }

    QString m_path;
    QString m_fileName;
    QList<QtResourcePrefix *> m_resourcePrefixes;
    QtQrcFileData m_initialState;
};

// ------------------ QtQrcManager
class QtQrcManager : public QObject
{
    Q_OBJECT
public:
    QtQrcManager(QObject *parent = nullptr);
    ~QtQrcManager() override;

    QList<QtQrcFile *> qrcFiles() const;

    // helpers
    QtQrcFile *qrcFileOf(const QString &path) const;
    QtQrcFile *qrcFileOf(QtResourcePrefix *resourcePrefix) const;
    QtResourcePrefix *resourcePrefixOf(QtResourceFile *resourceFile) const;

    QtQrcFile *importQrcFile(const QtQrcFileData &qrcFileData, QtQrcFile *beforeQrcFile = nullptr);
    void exportQrcFile(QtQrcFile *qrcFile, QtQrcFileData *qrcFileData) const;

    QIcon icon(const QString &resourceFullPath) const;
    bool exists(const QString &resourceFullPath) const;
    bool exists(QtQrcFile *qrcFile) const;

    QtQrcFile *prevQrcFile(QtQrcFile *qrcFile) const;
    QtQrcFile *nextQrcFile(QtQrcFile *qrcFile) const;
    QtResourcePrefix *prevResourcePrefix(QtResourcePrefix *resourcePrefix) const;
    QtResourcePrefix *nextResourcePrefix(QtResourcePrefix *resourcePrefix) const;
    QtResourceFile *prevResourceFile(QtResourceFile *resourceFile) const;
    QtResourceFile *nextResourceFile(QtResourceFile *resourceFile) const;

    void clear();

public slots:

    QtQrcFile *insertQrcFile(const QString &path, qdesigner_internal::QtQrcFile *beforeQrcFile = nullptr,
                             bool newFile = false);
    void moveQrcFile(QtQrcFile *qrcFile, qdesigner_internal::QtQrcFile *beforeQrcFile);
    void setInitialState(qdesigner_internal::QtQrcFile *qrcFile,
                         const QtQrcFileData &initialState);
    void removeQrcFile(qdesigner_internal::QtQrcFile *qrcFile);

    QtResourcePrefix *insertResourcePrefix(qdesigner_internal::QtQrcFile *qrcFile,
                                           const QString &prefix, const QString &language,
                                           QtResourcePrefix *beforeResourcePrefix = nullptr);
    void moveResourcePrefix(qdesigner_internal::QtResourcePrefix *resourcePrefix, QtResourcePrefix *beforeResourcePrefix); // the same qrc file???
    void changeResourcePrefix(qdesigner_internal::QtResourcePrefix *resourcePrefix, const QString &newPrefix);
    void changeResourceLanguage(qdesigner_internal::QtResourcePrefix *resourcePrefix, const QString &newLanguage);
    void removeResourcePrefix(qdesigner_internal::QtResourcePrefix *resourcePrefix);

    QtResourceFile *insertResourceFile(qdesigner_internal::QtResourcePrefix *resourcePrefix,
                                       const QString &path, const QString &alias,
                                       qdesigner_internal::QtResourceFile *beforeResourceFile = nullptr);
    void moveResourceFile(qdesigner_internal::QtResourceFile *resourceFile,
                          qdesigner_internal::QtResourceFile *beforeResourceFile); // the same prefix???
    void changeResourceAlias(qdesigner_internal::QtResourceFile *resourceFile, const QString &newAlias);
    void removeResourceFile(qdesigner_internal::QtResourceFile *resourceFile);

signals:
    void qrcFileInserted(qdesigner_internal::QtQrcFile *qrcFile);
    void qrcFileMoved(qdesigner_internal::QtQrcFile *qrcFile,
                      qdesigner_internal::QtQrcFile *oldBeforeQrcFile);
    void qrcFileRemoved(qdesigner_internal::QtQrcFile *qrcFile);

    void resourcePrefixInserted(qdesigner_internal::QtResourcePrefix *resourcePrefix);
    void resourcePrefixMoved(qdesigner_internal::QtResourcePrefix *resourcePrefix,
                             qdesigner_internal::QtResourcePrefix *oldBeforeResourcePrefix);
    void resourcePrefixChanged(qdesigner_internal::QtResourcePrefix *resourcePrefix,
                               const QString &oldPrefix);
    void resourceLanguageChanged(qdesigner_internal::QtResourcePrefix *resourcePrefix,
                                 const QString &oldLanguage);
    void resourcePrefixRemoved(qdesigner_internal::QtResourcePrefix *resourcePrefix);

    void resourceFileInserted(qdesigner_internal::QtResourceFile *resourceFile);
    void resourceFileMoved(qdesigner_internal::QtResourceFile *resourceFile,
                           qdesigner_internal::QtResourceFile *oldBeforeResourceFile);
    void resourceAliasChanged(qdesigner_internal::QtResourceFile *resourceFile,
                              const QString &oldAlias);
    void resourceFileRemoved(qdesigner_internal::QtResourceFile *resourceFile);
private:

    QList<QtQrcFile *> m_qrcFiles;
    QMap<QString, QtQrcFile *> m_pathToQrc;
    QHash<QtQrcFile *, bool> m_qrcFileToExists;
    QHash<QtResourcePrefix *, QtQrcFile *> m_prefixToQrc;
    QHash<QtResourceFile *, QtResourcePrefix *> m_fileToPrefix;
    QMap<QString, QList<QtResourceFile *> > m_fullPathToResourceFiles;
    QMap<QString, QIcon> m_fullPathToIcon;
    QMap<QString, bool> m_fullPathToExists;
};

QtQrcManager::QtQrcManager(QObject *parent)
    : QObject(parent)
{

}

QtQrcManager::~QtQrcManager()
{
    clear();
}

QList<QtQrcFile *> QtQrcManager::qrcFiles() const
{
    return m_qrcFiles;
}

QtQrcFile *QtQrcManager::qrcFileOf(const QString &path) const
{
    return m_pathToQrc.value(path);
}

QtQrcFile *QtQrcManager::qrcFileOf(QtResourcePrefix *resourcePrefix) const
{
    return m_prefixToQrc.value(resourcePrefix);
}

QtResourcePrefix *QtQrcManager::resourcePrefixOf(QtResourceFile *resourceFile) const
{
    return m_fileToPrefix.value(resourceFile);
}

QtQrcFile *QtQrcManager::importQrcFile(const QtQrcFileData &qrcFileData, QtQrcFile *beforeQrcFile)
{
    QtQrcFile *qrcFile = insertQrcFile(qrcFileData.qrcPath, beforeQrcFile);
    if (!qrcFile)
        return nullptr;
    for (const QtResourcePrefixData &prefixData : qrcFileData.resourceList) {
        QtResourcePrefix *resourcePrefix = insertResourcePrefix(qrcFile, prefixData.prefix, prefixData.language, nullptr);
        for (const QtResourceFileData &fileData : prefixData.resourceFileList)
            insertResourceFile(resourcePrefix, fileData.path, fileData.alias, nullptr);
    }
    setInitialState(qrcFile, qrcFileData);
    return qrcFile;
}

void QtQrcManager::exportQrcFile(QtQrcFile *qrcFile, QtQrcFileData *qrcFileData) const
{
    if (!qrcFileData)
        return;

    if (!qrcFile)
        return;

    QtQrcFileData &data = *qrcFileData;

    QList<QtResourcePrefixData> resourceList;

    const auto resourcePrefixes = qrcFile->resourcePrefixList();
    for (const QtResourcePrefix *prefix : resourcePrefixes) {
        QList<QtResourceFileData> resourceFileList;
        const auto resourceFiles = prefix->resourceFiles();
        for (QtResourceFile *file : resourceFiles) {
            QtResourceFileData fileData;
            fileData.path = file->path();
            fileData.alias = file->alias();
            resourceFileList << fileData;
        }
        QtResourcePrefixData prefixData;
        prefixData.prefix = prefix->prefix();
        prefixData.language = prefix->language();
        prefixData.resourceFileList = resourceFileList;

        resourceList << prefixData;
    }
    data = QtQrcFileData();
    data.qrcPath = qrcFile->path();
    data.resourceList = resourceList;
}

QIcon QtQrcManager::icon(const QString &resourceFullPath) const
{
    return m_fullPathToIcon.value(resourceFullPath);
}

bool QtQrcManager::exists(const QString &resourceFullPath) const
{
    return m_fullPathToExists.value(resourceFullPath, false);
}

bool QtQrcManager::exists(QtQrcFile *qrcFile) const
{
    return m_qrcFileToExists.value(qrcFile, false);
}

QtQrcFile *QtQrcManager::prevQrcFile(QtQrcFile *qrcFile) const
{
    if (!qrcFile)
        return nullptr;
    const int idx = m_qrcFiles.indexOf(qrcFile);
    if (idx <= 0)
        return nullptr;
    return m_qrcFiles.at(idx - 1);
}

QtQrcFile *QtQrcManager::nextQrcFile(QtQrcFile *qrcFile) const
{
    if (!qrcFile)
        return nullptr;
    const int idx = m_qrcFiles.indexOf(qrcFile);
    if (idx < 0 || idx == m_qrcFiles.size() - 1)
        return nullptr;
    return m_qrcFiles.at(idx + 1);
}

QtResourcePrefix *QtQrcManager::prevResourcePrefix(QtResourcePrefix *resourcePrefix) const
{
    if (!resourcePrefix)
        return nullptr;
    const auto prefixes = qrcFileOf(resourcePrefix)->resourcePrefixList();
    const int idx = prefixes.indexOf(resourcePrefix);
    if (idx <= 0)
        return nullptr;
    return prefixes.at(idx - 1);
}

QtResourcePrefix *QtQrcManager::nextResourcePrefix(QtResourcePrefix *resourcePrefix) const
{
    if (!resourcePrefix)
        return nullptr;
    const auto prefixes = qrcFileOf(resourcePrefix)->resourcePrefixList();
    const int idx = prefixes.indexOf(resourcePrefix);
    if (idx < 0 || idx == prefixes.size() - 1)
        return nullptr;
    return prefixes.at(idx + 1);
}

QtResourceFile *QtQrcManager::prevResourceFile(QtResourceFile *resourceFile) const
{
    if (!resourceFile)
        return nullptr;
    const auto files = resourcePrefixOf(resourceFile)->resourceFiles();
    const int idx = files.indexOf(resourceFile);
    if (idx <= 0)
        return nullptr;
    return files.at(idx - 1);
}

QtResourceFile *QtQrcManager::nextResourceFile(QtResourceFile *resourceFile) const
{
    if (!resourceFile)
        return nullptr;
    const auto files = resourcePrefixOf(resourceFile)->resourceFiles();
    const int idx = files.indexOf(resourceFile);
    if (idx < 0 || idx == files.size() - 1)
        return nullptr;
    return files.at(idx + 1);
}

void QtQrcManager::clear()
{
    const auto oldQrcFiles = qrcFiles();
    for (QtQrcFile *qf : oldQrcFiles)
        removeQrcFile(qf);
}

QtQrcFile *QtQrcManager::insertQrcFile(const QString &path, QtQrcFile *beforeQrcFile, bool newFile)
{
    if (m_pathToQrc.contains(path))
        return nullptr;

    int idx = m_qrcFiles.indexOf(beforeQrcFile);
    if (idx < 0)
        idx = m_qrcFiles.size();

    QtQrcFile *qrcFile = new QtQrcFile();
    qrcFile->setPath(path);

    m_qrcFiles.insert(idx, qrcFile);
    m_pathToQrc[path] = qrcFile;

    const QFileInfo fi(path);
    m_qrcFileToExists[qrcFile] = fi.exists() || newFile;

    emit qrcFileInserted(qrcFile);
    return qrcFile;
}

void QtQrcManager::moveQrcFile(QtQrcFile *qrcFile, QtQrcFile *beforeQrcFile)
{
    if (qrcFile == beforeQrcFile)
        return;

    const int idx = m_qrcFiles.indexOf(qrcFile);
    if (idx < 0)
        return;

    int beforeIdx = m_qrcFiles.indexOf(beforeQrcFile);
    if (beforeIdx < 0)
        beforeIdx = m_qrcFiles.size();

    if (idx == beforeIdx - 1) // the same position, nothing changes
        return;

    QtQrcFile *oldBefore = nullptr;
    if (idx < m_qrcFiles.size() - 1)
        oldBefore = m_qrcFiles.at(idx + 1);

    m_qrcFiles.removeAt(idx);
    if (idx < beforeIdx)
        beforeIdx -= 1;

    m_qrcFiles.insert(beforeIdx, qrcFile);

    emit qrcFileMoved(qrcFile, oldBefore);
}

void QtQrcManager::setInitialState(QtQrcFile *qrcFile, const QtQrcFileData &initialState)
{
    qrcFile->m_initialState = initialState;
}

void QtQrcManager::removeQrcFile(QtQrcFile *qrcFile)
{
    const int idx = m_qrcFiles.indexOf(qrcFile);
    if (idx < 0)
        return;

    const auto resourcePrefixes = qrcFile->resourcePrefixList();
    for (QtResourcePrefix *rp : resourcePrefixes)
        removeResourcePrefix(rp);

    emit qrcFileRemoved(qrcFile);

    m_qrcFiles.removeAt(idx);
    m_pathToQrc.remove(qrcFile->path());
    m_qrcFileToExists.remove(qrcFile);
    delete qrcFile;
}

QtResourcePrefix *QtQrcManager::insertResourcePrefix(QtQrcFile *qrcFile, const QString &prefix,
        const QString &language, QtResourcePrefix *beforeResourcePrefix)
{
    if (!qrcFile)
        return nullptr;

    int idx = qrcFile->m_resourcePrefixes.indexOf(beforeResourcePrefix);
    if (idx < 0)
        idx = qrcFile->m_resourcePrefixes.size();

    QtResourcePrefix *resourcePrefix = new QtResourcePrefix();
    resourcePrefix->m_prefix = prefix;
    resourcePrefix->m_language = language;

    qrcFile->m_resourcePrefixes.insert(idx, resourcePrefix);
    m_prefixToQrc[resourcePrefix] = qrcFile;

    emit resourcePrefixInserted(resourcePrefix);
    return resourcePrefix;
}

void QtQrcManager::moveResourcePrefix(QtResourcePrefix *resourcePrefix, QtResourcePrefix *beforeResourcePrefix)
{
    if (resourcePrefix == beforeResourcePrefix)
        return;

    QtQrcFile *qrcFile = qrcFileOf(resourcePrefix);
    if (!qrcFile)
        return;

    if (beforeResourcePrefix && qrcFileOf(beforeResourcePrefix) != qrcFile)
        return;

    const int idx = qrcFile->m_resourcePrefixes.indexOf(resourcePrefix);

    int beforeIdx = qrcFile->m_resourcePrefixes.indexOf(beforeResourcePrefix);
    if (beforeIdx < 0)
        beforeIdx = qrcFile->m_resourcePrefixes.size();

    if (idx == beforeIdx - 1) // the same position, nothing changes
        return;

    QtResourcePrefix *oldBefore = nullptr;
    if (idx < qrcFile->m_resourcePrefixes.size() - 1)
        oldBefore = qrcFile->m_resourcePrefixes.at(idx + 1);

    qrcFile->m_resourcePrefixes.removeAt(idx);
    if (idx < beforeIdx)
        beforeIdx -= 1;

    qrcFile->m_resourcePrefixes.insert(beforeIdx, resourcePrefix);

    emit resourcePrefixMoved(resourcePrefix, oldBefore);
}

void QtQrcManager::changeResourcePrefix(QtResourcePrefix *resourcePrefix, const QString &newPrefix)
{
    if (!resourcePrefix)
        return;

    const QString oldPrefix = resourcePrefix->m_prefix;
    if (oldPrefix == newPrefix)
        return;

    resourcePrefix->m_prefix = newPrefix;

    emit resourcePrefixChanged(resourcePrefix, oldPrefix);
}

void QtQrcManager::changeResourceLanguage(QtResourcePrefix *resourcePrefix, const QString &newLanguage)
{
    if (!resourcePrefix)
        return;

    const QString oldLanguage = resourcePrefix->m_language;
    if (oldLanguage == newLanguage)
        return;

    resourcePrefix->m_language = newLanguage;

    emit resourceLanguageChanged(resourcePrefix, oldLanguage);
}

void QtQrcManager::removeResourcePrefix(QtResourcePrefix *resourcePrefix)
{
    QtQrcFile *qrcFile = qrcFileOf(resourcePrefix);
    if (!qrcFile)
        return;

    const int idx = qrcFile->m_resourcePrefixes.indexOf(resourcePrefix);

    const auto resourceFiles = resourcePrefix->resourceFiles();
    for (QtResourceFile *rf : resourceFiles)
        removeResourceFile(rf);

    emit resourcePrefixRemoved(resourcePrefix);

    qrcFile->m_resourcePrefixes.removeAt(idx);
    m_prefixToQrc.remove(resourcePrefix);
    delete resourcePrefix;
}

QtResourceFile *QtQrcManager::insertResourceFile(QtResourcePrefix *resourcePrefix, const QString &path,
        const QString &alias, QtResourceFile *beforeResourceFile)
{
    if (!resourcePrefix)
        return nullptr;

    int idx = resourcePrefix->m_resourceFiles.indexOf(beforeResourceFile);
    if (idx < 0)
        idx = resourcePrefix->m_resourceFiles.size();

    QtResourceFile *resourceFile = new QtResourceFile();
    resourceFile->m_path = path;
    resourceFile->m_alias = alias;
    const QFileInfo fi(qrcFileOf(resourcePrefix)->path());
    const QDir dir(fi.absolutePath());
    const QString fullPath = dir.absoluteFilePath(path);
    resourceFile->m_fullPath = fullPath;

    resourcePrefix->m_resourceFiles.insert(idx, resourceFile);
    m_fileToPrefix[resourceFile] = resourcePrefix;
    m_fullPathToResourceFiles[fullPath].append(resourceFile);
    if (!m_fullPathToIcon.contains(fullPath)) {
        m_fullPathToIcon[fullPath] = QIcon(fullPath);
        const QFileInfo fullInfo(fullPath);
        m_fullPathToExists[fullPath] = fullInfo.exists();
    }

    emit resourceFileInserted(resourceFile);
    return resourceFile;
}

void QtQrcManager::moveResourceFile(QtResourceFile *resourceFile, QtResourceFile *beforeResourceFile)
{
    if (resourceFile == beforeResourceFile)
        return;

    QtResourcePrefix *resourcePrefix = resourcePrefixOf(resourceFile);
    if (!resourcePrefix)
        return;

    if (beforeResourceFile && resourcePrefixOf(beforeResourceFile) != resourcePrefix)
        return;

    const int idx = resourcePrefix->m_resourceFiles.indexOf(resourceFile);

    int beforeIdx = resourcePrefix->m_resourceFiles.indexOf(beforeResourceFile);
    if (beforeIdx < 0)
        beforeIdx = resourcePrefix->m_resourceFiles.size();

    if (idx == beforeIdx - 1) // the same position, nothing changes
        return;

    QtResourceFile *oldBefore = nullptr;
    if (idx < resourcePrefix->m_resourceFiles.size() - 1)
        oldBefore = resourcePrefix->m_resourceFiles.at(idx + 1);

    resourcePrefix->m_resourceFiles.removeAt(idx);
    if (idx < beforeIdx)
        beforeIdx -= 1;

    resourcePrefix->m_resourceFiles.insert(beforeIdx, resourceFile);

    emit resourceFileMoved(resourceFile, oldBefore);
}

void QtQrcManager::changeResourceAlias(QtResourceFile *resourceFile, const QString &newAlias)
{
    if (!resourceFile)
        return;

    const QString oldAlias = resourceFile->m_alias;
    if (oldAlias == newAlias)
        return;

    resourceFile->m_alias = newAlias;

    emit resourceAliasChanged(resourceFile, oldAlias);
}

void QtQrcManager::removeResourceFile(QtResourceFile *resourceFile)
{
    QtResourcePrefix *resourcePrefix = resourcePrefixOf(resourceFile);
    if (!resourcePrefix)
        return;

    const int idx = resourcePrefix->m_resourceFiles.indexOf(resourceFile);

    emit resourceFileRemoved(resourceFile);

    resourcePrefix->m_resourceFiles.removeAt(idx);
    m_fileToPrefix.remove(resourceFile);
    const QString fullPath = resourceFile->fullPath();
    m_fullPathToResourceFiles[fullPath].removeAll(resourceFile); // optimize me
    if (m_fullPathToResourceFiles[fullPath].isEmpty()) {
        m_fullPathToResourceFiles.remove(fullPath);
        m_fullPathToIcon.remove(fullPath);
        m_fullPathToExists.remove(fullPath);
    }
    delete resourceFile;
}

} // namespace qdesigner_internal

using QtResourceFile = qdesigner_internal::QtResourceFile;
using QtResourcePrefix = qdesigner_internal::QtResourcePrefix;
using QtQrcFile = qdesigner_internal::QtQrcFile;
using QtQrcManager = qdesigner_internal::QtQrcManager;

// ----------------- QtResourceEditorDialogPrivate
class QtResourceEditorDialogPrivate
{
    QtResourceEditorDialog *q_ptr{};
    Q_DECLARE_PUBLIC(QtResourceEditorDialog)
public:
    QtResourceEditorDialogPrivate() = default;

    void slotQrcFileInserted(QtQrcFile *qrcFile);
    void slotQrcFileMoved(QtQrcFile *qrcFile);
    void slotQrcFileRemoved(QtQrcFile *qrcFile);

    QStandardItem *insertResourcePrefix(QtResourcePrefix *resourcePrefix);

    void slotResourcePrefixInserted(QtResourcePrefix *resourcePrefix) { insertResourcePrefix(resourcePrefix); }
    void slotResourcePrefixMoved(QtResourcePrefix *resourcePrefix);
    void slotResourcePrefixChanged(QtResourcePrefix *resourcePrefix);
    void slotResourceLanguageChanged(QtResourcePrefix *resourcePrefix);
    void slotResourcePrefixRemoved(QtResourcePrefix *resourcePrefix);
    void slotResourceFileInserted(QtResourceFile *resourceFile);
    void slotResourceFileMoved(QtResourceFile *resourceFile);
    void slotResourceAliasChanged(QtResourceFile *resourceFile);
    void slotResourceFileRemoved(QtResourceFile *resourceFile);

    void slotCurrentQrcFileChanged(QListWidgetItem *item);
    void slotCurrentTreeViewItemChanged(const QModelIndex &index);
    void slotListWidgetContextMenuRequested(const QPoint &pos);
    void slotTreeViewContextMenuRequested(const QPoint &pos);
    void slotTreeViewItemChanged(QStandardItem *item);

    void slotNewQrcFile();
    void slotImportQrcFile();
    void slotRemoveQrcFile();
    void slotMoveUpQrcFile();
    void slotMoveDownQrcFile();

    void slotNewPrefix();
    void slotAddFiles();
    void slotChangePrefix();
    void slotChangeLanguage();
    void slotChangeAlias();
    void slotClonePrefix();
    void slotRemove();
    void slotMoveUp();
    void slotMoveDown();

    bool loadQrcFile(const QString &path, QtQrcFileData *qrcFileData, QString *errorMessage);
    bool loadQrcFile(const QString &path, QtQrcFileData *qrcFileData);
    bool saveQrcFile(const QtQrcFileData &qrcFileData);

    QString qrcFileText(QtQrcFile *qrcFile) const;

    QMessageBox::StandardButton warning(const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) const;

    QString browseForNewLocation(const QString &resourceFile, const QDir &rootDir) const;
    QString copyResourceFile(const QString &resourceFile, const QString &destPath) const;
    QtResourceFile *getCurrentResourceFile() const;
    QtResourcePrefix *getCurrentResourcePrefix() const;
    void selectTreeRow(QStandardItem *item);
    QString getSaveFileNameWithExtension(QWidget *parent,
            const QString &title, QString dir, const QString &filter, const QString &extension) const;
    QString qrcStartDirectory() const;

    Ui::QtResourceEditorDialog m_ui;
    QDesignerFormEditorInterface *m_core = nullptr;
    QtResourceModel *m_resourceModel = nullptr;
    QDesignerDialogGuiInterface *m_dlgGui = nullptr;
    QtQrcManager *m_qrcManager = nullptr;
    QList<QtQrcFileData> m_initialState;

    QHash<QtQrcFile *, QListWidgetItem *> m_qrcFileToItem;
    QHash<QListWidgetItem *, QtQrcFile *> m_itemToQrcFile;
    QHash<QtResourcePrefix *, QStandardItem *> m_resourcePrefixToPrefixItem;
    QHash<QtResourcePrefix *, QStandardItem *> m_resourcePrefixToLanguageItem;
    QHash<QStandardItem *, QtResourcePrefix *> m_prefixItemToResourcePrefix;
    QHash<QStandardItem *, QtResourcePrefix *> m_languageItemToResourcePrefix;
    QHash<QtResourceFile *, QStandardItem *> m_resourceFileToPathItem;
    QHash<QtResourceFile *, QStandardItem *> m_resourceFileToAliasItem;
    QHash<QStandardItem *, QtResourceFile *> m_pathItemToResourceFile;
    QHash<QStandardItem *, QtResourceFile *> m_aliasItemToResourceFile;

    bool m_ignoreCurrentChanged = false;
    bool m_firstQrcFileDialog = true;
    QtQrcFile *m_currentQrcFile = nullptr;

    QAction *m_newQrcFileAction = nullptr;
    QAction *m_importQrcFileAction = nullptr;
    QAction *m_removeQrcFileAction = nullptr;
    QAction *m_moveUpQrcFileAction = nullptr;
    QAction *m_moveDownQrcFileAction = nullptr;

    QAction *m_newPrefixAction = nullptr;
    QAction *m_addResourceFileAction = nullptr;
    QAction *m_changePrefixAction = nullptr;
    QAction *m_changeLanguageAction = nullptr;
    QAction *m_changeAliasAction = nullptr;
    QAction *m_clonePrefixAction = nullptr;
    QAction *m_moveUpAction = nullptr;
    QAction *m_moveDownAction = nullptr;
    QAction *m_removeAction = nullptr;

    QStandardItemModel *m_treeModel = nullptr;
    QItemSelectionModel *m_treeSelection = nullptr;
};

QMessageBox::StandardButton QtResourceEditorDialogPrivate::warning(const QString &title, const QString &text, QMessageBox::StandardButtons buttons,
                                                                   QMessageBox::StandardButton defaultButton) const
{
    return m_dlgGui->message(q_ptr, QDesignerDialogGuiInterface::ResourceEditorMessage, QMessageBox::Warning, title, text, buttons, defaultButton);
}

QString QtResourceEditorDialogPrivate::qrcFileText(QtQrcFile *qrcFile) const
{
    const QString path = qrcFile->path();
    const QString fileName = qrcFile->fileName();
    const QFileInfo fi(path);
    if (fi.exists() && !fi.isWritable())
        return QApplication::translate("QtResourceEditorDialog", "%1 [read-only]").arg(fileName);
    if (!m_qrcManager->exists(qrcFile))
        return QApplication::translate("QtResourceEditorDialog", "%1 [missing]").arg(fileName);
    return fileName;
}

void QtResourceEditorDialogPrivate::slotQrcFileInserted(QtQrcFile *qrcFile)
{
    QListWidgetItem *currentItem = m_ui.qrcFileList->currentItem();
    int idx = m_ui.qrcFileList->count();
    QtQrcFile *nextQrcFile = m_qrcManager->nextQrcFile(qrcFile);
    QListWidgetItem *nextItem = m_qrcFileToItem.value(nextQrcFile);
    if (nextItem) {
        const int row = m_ui.qrcFileList->row(nextItem);
        if (row >= 0)
            idx = row;
    }
    const QString path = qrcFile->path();
    QListWidgetItem *item = new QListWidgetItem(qrcFileText(qrcFile));
    item->setToolTip(path);
    m_ignoreCurrentChanged = true;
    m_ui.qrcFileList->insertItem(idx, item);
    m_ui.qrcFileList->setCurrentItem(currentItem);
    m_ignoreCurrentChanged = false;
    m_qrcFileToItem[qrcFile] = item;
    m_itemToQrcFile[item] = qrcFile;
    if (!m_qrcManager->exists(qrcFile))
        item->setForeground(QBrush(Qt::red));
}

void QtResourceEditorDialogPrivate::slotQrcFileMoved(QtQrcFile *qrcFile)
{
    QListWidgetItem *currentItem = m_ui.qrcFileList->currentItem();
    QListWidgetItem *item = m_qrcFileToItem.value(qrcFile);
    m_ignoreCurrentChanged = true;
    m_ui.qrcFileList->takeItem(m_ui.qrcFileList->row(item));

    int idx = m_ui.qrcFileList->count();
    QtQrcFile *nextQrcFile = m_qrcManager->nextQrcFile(qrcFile);
    QListWidgetItem *nextItem = m_qrcFileToItem.value(nextQrcFile);
    if (nextItem) {
        int row = m_ui.qrcFileList->row(nextItem);
        if (row >= 0)
            idx = row;
    }
    m_ui.qrcFileList->insertItem(idx, item);
    if (currentItem == item)
        m_ui.qrcFileList->setCurrentItem(item);
    m_ignoreCurrentChanged = false;
}

void QtResourceEditorDialogPrivate::slotQrcFileRemoved(QtQrcFile *qrcFile)
{
    QListWidgetItem *item = m_qrcFileToItem.value(qrcFile);
    if (item == m_ui.qrcFileList->currentItem())
        m_ui.qrcFileList->setCurrentItem(nullptr); // this should trigger list view signal currentItemChanged(0), and slot should set m_currentQrcFile to 0
    m_ignoreCurrentChanged = true;
    delete item;
    m_ignoreCurrentChanged = false;
    m_itemToQrcFile.remove(item);
    m_qrcFileToItem.remove(qrcFile);
}

QStandardItem *QtResourceEditorDialogPrivate::insertResourcePrefix(QtResourcePrefix *resourcePrefix)
{
    if (m_qrcManager->qrcFileOf(resourcePrefix) != m_currentQrcFile)
        return nullptr;

    QtResourcePrefix *prevResourcePrefix = m_qrcManager->prevResourcePrefix(resourcePrefix);
    QStandardItem *prevItem = m_resourcePrefixToPrefixItem.value(prevResourcePrefix);

    int row = 0;
    if (prevItem)
        row = m_treeModel->indexFromItem(prevItem).row() + 1;

    QStandardItem *prefixItem = new QStandardItem();
    QStandardItem *languageItem = new QStandardItem();
    QList<QStandardItem *> items;
    items << prefixItem;
    items << languageItem;
    m_treeModel->insertRow(row, items);
    const QModelIndex newIndex = m_treeModel->indexFromItem(prefixItem);
    m_ui.resourceTreeView->setExpanded(newIndex, true);
    prefixItem->setFlags(prefixItem->flags() | Qt::ItemIsEditable);
    languageItem->setFlags(languageItem->flags() | Qt::ItemIsEditable);
    m_resourcePrefixToPrefixItem[resourcePrefix] = prefixItem;
    m_resourcePrefixToLanguageItem[resourcePrefix] = languageItem;
    m_prefixItemToResourcePrefix[prefixItem] = resourcePrefix;
    m_languageItemToResourcePrefix[languageItem] = resourcePrefix;
    slotResourcePrefixChanged(resourcePrefix);
    slotResourceLanguageChanged(resourcePrefix);
    return prefixItem;
}

void QtResourceEditorDialogPrivate::slotResourcePrefixMoved(QtResourcePrefix *resourcePrefix)
{
    QStandardItem *prefixItem = m_resourcePrefixToPrefixItem.value(resourcePrefix);
    if (!prefixItem)
        return;

    QStandardItem *languageItem = m_resourcePrefixToLanguageItem.value(resourcePrefix);
    if (!languageItem)
        return;

    const QModelIndex index = m_treeModel->indexFromItem(prefixItem);
    const bool expanded = m_ui.resourceTreeView->isExpanded(index);
    m_ignoreCurrentChanged = true;
    const auto items = m_treeModel->takeRow(index.row());

    int row = m_treeModel->rowCount();
    QtResourcePrefix *nextResourcePrefix = m_qrcManager->nextResourcePrefix(resourcePrefix);
    QStandardItem *nextItem = m_resourcePrefixToPrefixItem.value(nextResourcePrefix);
    if (nextItem)
        row = m_treeModel->indexFromItem(nextItem).row();
    m_treeModel->insertRow(row, items);
    m_ignoreCurrentChanged = false;
    m_ui.resourceTreeView->setExpanded(m_treeModel->indexFromItem(items.at(0)), expanded);
}

void QtResourceEditorDialogPrivate::slotResourcePrefixChanged(QtResourcePrefix *resourcePrefix)
{
    QStandardItem *item = m_resourcePrefixToPrefixItem.value(resourcePrefix);
    if (!item)
        return;

    m_ignoreCurrentChanged = true;
    QString prefix = resourcePrefix->prefix();
    if (prefix.isEmpty())
        prefix = QCoreApplication::translate("QtResourceEditorDialog", "<no prefix>");
    item->setText(prefix);
    item->setToolTip(prefix);
    m_ignoreCurrentChanged = false;
}

void QtResourceEditorDialogPrivate::slotResourceLanguageChanged(QtResourcePrefix *resourcePrefix)
{
    QStandardItem *item = m_resourcePrefixToLanguageItem.value(resourcePrefix);
    if (!item)
        return;

    m_ignoreCurrentChanged = true;
    const QString language = resourcePrefix->language();
    item->setText(language);
    item->setToolTip(language);

    m_ignoreCurrentChanged = false;
}

void QtResourceEditorDialogPrivate::slotResourcePrefixRemoved(QtResourcePrefix *resourcePrefix)
{
    QStandardItem *prefixItem = m_resourcePrefixToPrefixItem.value(resourcePrefix);
    if (!prefixItem)
        return;

    QStandardItem *languageItem = m_resourcePrefixToLanguageItem.value(resourcePrefix);
    if (!languageItem)
        return;

    m_ignoreCurrentChanged = true;
    m_treeModel->takeRow(m_treeModel->indexFromItem(prefixItem).row());
    delete prefixItem;
    delete languageItem;
    m_ignoreCurrentChanged = false;
    m_prefixItemToResourcePrefix.remove(prefixItem);
    m_languageItemToResourcePrefix.remove(languageItem);
    m_resourcePrefixToPrefixItem.remove(resourcePrefix);
    m_resourcePrefixToLanguageItem.remove(resourcePrefix);
}

void QtResourceEditorDialogPrivate::slotResourceFileInserted(QtResourceFile *resourceFile)
{
    QtResourcePrefix *resourcePrefix = m_qrcManager->resourcePrefixOf(resourceFile);
    if (m_qrcManager->qrcFileOf(resourcePrefix) != m_currentQrcFile)
        return;

    QtResourceFile *prevResourceFile = m_qrcManager->prevResourceFile(resourceFile);
    QStandardItem *prevItem = m_resourceFileToPathItem.value(prevResourceFile);

    QStandardItem *pathItem = new QStandardItem(resourceFile->path());
    QStandardItem *aliasItem = new QStandardItem();
    QStandardItem *parentItem = m_resourcePrefixToPrefixItem.value(resourcePrefix);
    QList<QStandardItem *> items;
    items << pathItem;
    items << aliasItem;

    int row = 0;
    if (prevItem)
        row = m_treeModel->indexFromItem(prevItem).row() + 1;

    parentItem->insertRow(row, items);

    pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
    aliasItem->setFlags(aliasItem->flags() | Qt::ItemIsEditable);
    m_resourceFileToPathItem[resourceFile] = pathItem;
    m_resourceFileToAliasItem[resourceFile] = aliasItem;
    m_pathItemToResourceFile[pathItem] = resourceFile;
    m_aliasItemToResourceFile[aliasItem] = resourceFile;
    pathItem->setToolTip(resourceFile->path());
    pathItem->setIcon(m_qrcManager->icon(resourceFile->fullPath()));
    if (!m_qrcManager->exists(resourceFile->fullPath())) {
        pathItem->setText(QApplication::translate("QtResourceEditorDialog", "%1 [missing]").arg(resourceFile->path()));
        QBrush redBrush(Qt::red);
        pathItem->setForeground(redBrush);
        aliasItem->setForeground(redBrush);
    }
    slotResourceAliasChanged(resourceFile);
}

void QtResourceEditorDialogPrivate::slotResourceFileMoved(QtResourceFile *resourceFile)
{
    QStandardItem *pathItem = m_resourceFileToPathItem.value(resourceFile);
    if (!pathItem)
        return;

    QStandardItem *aliasItem = m_resourceFileToAliasItem.value(resourceFile);
    if (!aliasItem)
        return;

    QStandardItem *parentItem = pathItem->parent();
    m_ignoreCurrentChanged = true;
    const auto items = parentItem->takeRow(m_treeModel->indexFromItem(pathItem).row());

    int row = parentItem->rowCount();
    QtResourceFile *nextResourceFile = m_qrcManager->nextResourceFile(resourceFile);
    QStandardItem *nextItem = m_resourceFileToPathItem.value(nextResourceFile);
    if (nextItem)
        row = m_treeModel->indexFromItem(nextItem).row();
    parentItem->insertRow(row, items);
    m_ignoreCurrentChanged = false;
}

void QtResourceEditorDialogPrivate::slotResourceAliasChanged(QtResourceFile *resourceFile)
{
    QStandardItem *item = m_resourceFileToAliasItem.value(resourceFile);
    if (!item)
        return;

    m_ignoreCurrentChanged = true;
    const QString alias = resourceFile->alias();
    item->setText(alias);
    item->setToolTip(alias);

    m_ignoreCurrentChanged = false;
}

void QtResourceEditorDialogPrivate::slotResourceFileRemoved(QtResourceFile *resourceFile)
{
    QStandardItem *pathItem = m_resourceFileToPathItem.value(resourceFile);
    if (!pathItem)
        return;

    QStandardItem *aliasItem = m_resourceFileToAliasItem.value(resourceFile);
    if (!aliasItem)
        return;

    QStandardItem *parentItem = pathItem->parent();

    m_ignoreCurrentChanged = true;
    parentItem->takeRow(m_treeModel->indexFromItem(pathItem).row());
    delete pathItem;
    delete aliasItem;
    m_ignoreCurrentChanged = false;
    m_pathItemToResourceFile.remove(pathItem);
    m_aliasItemToResourceFile.remove(aliasItem);
    m_resourceFileToPathItem.remove(resourceFile);
    m_resourceFileToAliasItem.remove(resourceFile);
}


void QtResourceEditorDialogPrivate::slotCurrentQrcFileChanged(QListWidgetItem *item)
{
    if (m_ignoreCurrentChanged)
        return;

    QtQrcFile *newCurrentQrcFile = m_itemToQrcFile.value(item);

    if (newCurrentQrcFile == m_currentQrcFile)
        return;

    if (m_currentQrcFile) {
        QHash<QtResourcePrefix *, QStandardItem *> currentPrefixList = m_resourcePrefixToPrefixItem;
        for (auto it = currentPrefixList.cbegin(), end = currentPrefixList.cend(); it != end; ++it) {
            QtResourcePrefix *resourcePrefix = it.key();
            const auto currentResourceFiles = resourcePrefix->resourceFiles();
            for (QtResourceFile *rf : currentResourceFiles)
                slotResourceFileRemoved(rf);
            slotResourcePrefixRemoved(resourcePrefix);
        }
    }

    m_currentQrcFile = newCurrentQrcFile;
    slotCurrentTreeViewItemChanged(QModelIndex());
    QStandardItem *firstPrefix = nullptr; // select first prefix
    if (m_currentQrcFile) {
        const auto newPrefixList = m_currentQrcFile->resourcePrefixList();
        for (QtResourcePrefix *resourcePrefix : newPrefixList) {
            if (QStandardItem *newPrefixItem = insertResourcePrefix(resourcePrefix))
                if (!firstPrefix)
                    firstPrefix = newPrefixItem;
            const auto newResourceFiles = resourcePrefix->resourceFiles();
            for (QtResourceFile *rf : newResourceFiles)
                slotResourceFileInserted(rf);
        }
    }
    m_ui.resourceTreeView->setCurrentIndex(firstPrefix ? m_treeModel->indexFromItem(firstPrefix) : QModelIndex());

    m_removeQrcFileAction->setEnabled(m_currentQrcFile);
    m_moveUpQrcFileAction->setEnabled(m_currentQrcFile && m_qrcManager->prevQrcFile(m_currentQrcFile));
    m_moveDownQrcFileAction->setEnabled(m_currentQrcFile && m_qrcManager->nextQrcFile(m_currentQrcFile));
}

void QtResourceEditorDialogPrivate::slotCurrentTreeViewItemChanged(const QModelIndex &index)
{
    QStandardItem *item = m_treeModel->itemFromIndex(index);
    QtResourceFile *resourceFile = m_pathItemToResourceFile.value(item);
    if (!resourceFile)
        resourceFile = m_aliasItemToResourceFile.value(item);
    QtResourcePrefix *resourcePrefix = m_prefixItemToResourcePrefix.value(item);
    if (!resourcePrefix)
        resourcePrefix = m_languageItemToResourcePrefix.value(item);

    bool moveUpEnabled = false;
    bool moveDownEnabled = false;
    bool currentItem = resourceFile || resourcePrefix;

    if (resourceFile) {
        if (m_qrcManager->prevResourceFile(resourceFile))
            moveUpEnabled = true;
        if (m_qrcManager->nextResourceFile(resourceFile))
            moveDownEnabled = true;
    } else if (resourcePrefix) {
        if (m_qrcManager->prevResourcePrefix(resourcePrefix))
            moveUpEnabled = true;
        if (m_qrcManager->nextResourcePrefix(resourcePrefix))
            moveDownEnabled = true;
    }

    m_newPrefixAction->setEnabled(m_currentQrcFile);
    m_addResourceFileAction->setEnabled(currentItem);
    m_changePrefixAction->setEnabled(currentItem);
    m_changeLanguageAction->setEnabled(currentItem);
    m_changeAliasAction->setEnabled(resourceFile);
    m_removeAction->setEnabled(currentItem);
    m_moveUpAction->setEnabled(moveUpEnabled);
    m_moveDownAction->setEnabled(moveDownEnabled);
    m_clonePrefixAction->setEnabled(currentItem);
}

void QtResourceEditorDialogPrivate::slotListWidgetContextMenuRequested(const QPoint &pos)
{
    QMenu menu(q_ptr);
    menu.addAction(m_newQrcFileAction);
    menu.addAction(m_importQrcFileAction);
    menu.addAction(m_removeQrcFileAction);
    menu.addSeparator();
    menu.addAction(m_moveUpQrcFileAction);
    menu.addAction(m_moveDownQrcFileAction);
    menu.exec(m_ui.qrcFileList->mapToGlobal(pos));
}

void QtResourceEditorDialogPrivate::slotTreeViewContextMenuRequested(const QPoint &pos)
{
    QMenu menu(q_ptr);
    menu.addAction(m_newPrefixAction);
    menu.addAction(m_addResourceFileAction);
    menu.addAction(m_removeAction);
    menu.addSeparator();
    menu.addAction(m_changePrefixAction);
    menu.addAction(m_changeLanguageAction);
    menu.addAction(m_changeAliasAction);
    menu.addSeparator();
    menu.addAction(m_clonePrefixAction);
    menu.addSeparator();
    menu.addAction(m_moveUpAction);
    menu.addAction(m_moveDownAction);
    menu.exec(m_ui.resourceTreeView->mapToGlobal(pos));
}

void QtResourceEditorDialogPrivate::slotTreeViewItemChanged(QStandardItem *item)
{
    if (m_ignoreCurrentChanged)
        return;

    const QString newValue = item->text();
    QtResourceFile *resourceFile = m_aliasItemToResourceFile.value(item);
    if (resourceFile) {
        m_qrcManager->changeResourceAlias(resourceFile, newValue);
        return;
    }

    QtResourcePrefix *resourcePrefix = m_prefixItemToResourcePrefix.value(item);
    if (resourcePrefix) {
        m_qrcManager->changeResourcePrefix(resourcePrefix, newValue);
        return;
    }

    resourcePrefix = m_languageItemToResourcePrefix.value(item);
    if (resourcePrefix) {
        m_qrcManager->changeResourceLanguage(resourcePrefix, newValue);
        return;
    }
}

QString QtResourceEditorDialogPrivate::getSaveFileNameWithExtension(QWidget *parent,
            const QString &title, QString dir, const QString &filter, const QString &extension) const
{
    QString saveFile;
    while (true) {
        saveFile = m_dlgGui->getSaveFileName(parent, title, dir, filter, nullptr, QFileDialog::DontConfirmOverwrite);
        if (saveFile.isEmpty())
            return saveFile;

        const QFileInfo fInfo(saveFile);
        if (fInfo.suffix().isEmpty() && !fInfo.fileName().endsWith(u'.'))
            saveFile += u'.' + extension;

        const QFileInfo fi(saveFile);
        if (!fi.exists())
            break;

        if (warning(title, msgOverwrite(fi.fileName()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            break;

        dir = saveFile;
    }
    return saveFile;
}

QString QtResourceEditorDialogPrivate::qrcStartDirectory() const
{
    if (!m_currentQrcFile)
        return QString();
    const QDir dir = QFileInfo(m_currentQrcFile->path()).dir();
    return dir.exists() ? dir.absolutePath() : QString();
}

void QtResourceEditorDialogPrivate::slotNewQrcFile()
{
    const QString qrcPath = getSaveFileNameWithExtension(q_ptr,
                QCoreApplication::translate("QtResourceEditorDialog", "New Resource File"),
                m_firstQrcFileDialog ? qrcStartDirectory() : QString(),
                QCoreApplication::translate("QtResourceEditorDialog", "Resource files (*.qrc)"),
                u"qrc"_s);
    if (qrcPath.isEmpty())
        return;

    m_firstQrcFileDialog = false;
    if (QtQrcFile *sameQrcFile = m_qrcManager->qrcFileOf(qrcPath)) {
        // QMessageBox ???
        QListWidgetItem *item = m_qrcFileToItem.value(sameQrcFile);
        m_ui.qrcFileList->setCurrentItem(item);
        item->setSelected(true);
        return;
    }

    QtQrcFile *nextQrcFile = m_qrcManager->nextQrcFile(m_currentQrcFile);

    QtQrcFile *qrcFile = m_qrcManager->insertQrcFile(qrcPath, nextQrcFile, true);
    m_ui.qrcFileList->setCurrentItem(m_qrcFileToItem.value(qrcFile));
}

void QtResourceEditorDialogPrivate::slotImportQrcFile()
{
    const QString qrcPath = m_dlgGui->getOpenFileName(q_ptr,
                QCoreApplication::translate("QtResourceEditorDialog", "Import Resource File"),
                m_firstQrcFileDialog ? qrcStartDirectory() : QString(),
                QCoreApplication::translate("QtResourceEditorDialog", "Resource files (*.qrc)"));
    if (qrcPath.isEmpty())
        return;
    m_firstQrcFileDialog = false;
    if (QtQrcFile *sameQrcFile = m_qrcManager->qrcFileOf(qrcPath)) {
        // QMessageBox ???
        QListWidgetItem *item = m_qrcFileToItem.value(sameQrcFile);
        m_ui.qrcFileList->setCurrentItem(item);
        item->setSelected(true);
        return;
    }

    QtQrcFile *nextQrcFile = m_qrcManager->nextQrcFile(m_currentQrcFile);

    QtQrcFileData qrcFileData;
    loadQrcFile(qrcPath, &qrcFileData);
    QtQrcFile *qrcFile = m_qrcManager->importQrcFile(qrcFileData, nextQrcFile);
    m_ui.qrcFileList->setCurrentItem(m_qrcFileToItem.value(qrcFile));
}

void QtResourceEditorDialogPrivate::slotRemoveQrcFile()
{
    if (!m_currentQrcFile)
        return;

    QtQrcFile *currentQrcFile = m_qrcManager->nextQrcFile(m_currentQrcFile);
    if (!currentQrcFile)
        currentQrcFile = m_qrcManager->prevQrcFile(m_currentQrcFile);

    m_qrcManager->removeQrcFile(m_currentQrcFile);
    QListWidgetItem *item = m_qrcFileToItem.value(currentQrcFile);
    if (item) {
        m_ui.qrcFileList->setCurrentItem(item);
        item->setSelected(true);
    }
}

void QtResourceEditorDialogPrivate::slotMoveUpQrcFile()
{
    if (!m_currentQrcFile)
        return;

    QtQrcFile *prevQrcFile = m_qrcManager->prevQrcFile(m_currentQrcFile);
    if (!prevQrcFile)
        return;

    m_qrcManager->moveQrcFile(m_currentQrcFile, prevQrcFile);
}

void QtResourceEditorDialogPrivate::slotMoveDownQrcFile()
{
    if (!m_currentQrcFile)
        return;

    QtQrcFile *nextQrcFile = m_qrcManager->nextQrcFile(m_currentQrcFile);
    if (!nextQrcFile)
        return;
    nextQrcFile = m_qrcManager->nextQrcFile(nextQrcFile);

    m_qrcManager->moveQrcFile(m_currentQrcFile, nextQrcFile);
}

QtResourceFile *QtResourceEditorDialogPrivate::getCurrentResourceFile() const
{
    QStandardItem *currentItem = m_treeModel->itemFromIndex(m_treeSelection->currentIndex());


    QtResourceFile *currentResourceFile = nullptr;
    if (currentItem) {
        currentResourceFile = m_pathItemToResourceFile.value(currentItem);
        if (!currentResourceFile)
            currentResourceFile = m_aliasItemToResourceFile.value(currentItem);
    }
    return currentResourceFile;
}

QtResourcePrefix *QtResourceEditorDialogPrivate::getCurrentResourcePrefix() const
{
    QStandardItem *currentItem = m_treeModel->itemFromIndex(m_treeSelection->currentIndex());

    QtResourcePrefix *currentResourcePrefix = nullptr;
    if (currentItem) {
        currentResourcePrefix = m_prefixItemToResourcePrefix.value(currentItem);
        if (!currentResourcePrefix) {
            currentResourcePrefix = m_languageItemToResourcePrefix.value(currentItem);
            if (!currentResourcePrefix) {
                QtResourceFile *currentResourceFile = getCurrentResourceFile();
                if (currentResourceFile)
                    currentResourcePrefix = m_qrcManager->resourcePrefixOf(currentResourceFile);
            }
        }
    }
    return currentResourcePrefix;
}

void QtResourceEditorDialogPrivate::selectTreeRow(QStandardItem *item)
{
    const QModelIndex index = m_treeModel->indexFromItem(item);
    m_treeSelection->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_treeSelection->setCurrentIndex(index, QItemSelectionModel::Select);
}

void QtResourceEditorDialogPrivate::slotNewPrefix()
{
    if (!m_currentQrcFile)
        return;

    QtResourcePrefix *currentResourcePrefix = getCurrentResourcePrefix();
    QtResourcePrefix *nextResourcePrefix = m_qrcManager->nextResourcePrefix(currentResourcePrefix);
    QtResourcePrefix *newResourcePrefix = m_qrcManager->insertResourcePrefix(m_currentQrcFile,
                QCoreApplication::translate("QtResourceEditorDialog", "newPrefix"),
                QString(), nextResourcePrefix);
    if (!newResourcePrefix)
        return;

    QStandardItem *newItem = m_resourcePrefixToPrefixItem.value(newResourcePrefix);
    if (!newItem)
        return;

    const QModelIndex index = m_treeModel->indexFromItem(newItem);
    selectTreeRow(newItem);
    m_ui.resourceTreeView->edit(index);
}

static inline QString outOfPathWarning(const QString &fname)
{
    return QApplication::translate("QtResourceEditorDialog",
    "<p><b>Warning:</b> The file</p>"
    "<p>%1</p>"
    "<p>is outside of the current resource file's parent directory.</p>").arg(fname);
}

static inline QString outOfPathWarningInfo()
{
    return QApplication::translate("QtResourceEditorDialog",
    "<p>To resolve the issue, press:</p>"
    "<table>"
    "<tr><th align=\"left\">Copy</th><td>to copy the file to the resource file's parent directory.</td></tr>"
    "<tr><th align=\"left\">Copy As...</th><td>to copy the file into a subdirectory of the resource file's parent directory.</td></tr>"
    "<tr><th align=\"left\">Keep</th><td>to use its current location.</td></tr></table>");
}

void QtResourceEditorDialogPrivate::slotAddFiles()
{
    if (!m_currentQrcFile)
        return;

    QtResourcePrefix *currentResourcePrefix = getCurrentResourcePrefix();
    QtResourceFile *currentResourceFile = getCurrentResourceFile();
    if (!currentResourcePrefix)
        return;

    QString initialPath = m_currentQrcFile->path();
    if (currentResourceFile) {
        QFileInfo fi(currentResourceFile->fullPath());
        initialPath = fi.absolutePath();
    }

    const QStringList resourcePaths = m_dlgGui->getOpenImageFileNames(q_ptr,
                QCoreApplication::translate("QtResourceEditorDialog", "Add Files"),
                initialPath);
    if (resourcePaths.isEmpty())
        return;

    QtResourceFile *nextResourceFile = m_qrcManager->nextResourceFile(currentResourceFile);
    if (!currentResourceFile) {
        const auto resourceFiles = currentResourcePrefix->resourceFiles();
        if (!resourceFiles.isEmpty())
            nextResourceFile = resourceFiles.first();
    }

    const QFileInfo fi(m_currentQrcFile->path());
    const QString destDir = fi.absolutePath();
    const QDir dir(fi.absolutePath());
    for (QString resourcePath : resourcePaths) {
        QString relativePath = dir.relativeFilePath(resourcePath);
        if (relativePath.startsWith(".."_L1)) {
            QMessageBox msgBox(QMessageBox::Warning,
                    QCoreApplication::translate("QtResourceEditorDialog", "Incorrect Path"),
                    outOfPathWarning(relativePath), QMessageBox::Cancel);
            msgBox.setInformativeText(outOfPathWarningInfo());
            QPushButton *copyButton = msgBox.addButton(QCoreApplication::translate("QtResourceEditorDialog",
                        "Copy"), QMessageBox::ActionRole);
            QPushButton *copyAsButton = msgBox.addButton(QCoreApplication::translate("QtResourceEditorDialog",
                        "Copy As..."), QMessageBox::ActionRole);
            QPushButton *keepButton = msgBox.addButton(QCoreApplication::translate("QtResourceEditorDialog",
                        "Keep"), QMessageBox::ActionRole);
            QPushButton *skipButton = msgBox.addButton(QCoreApplication::translate("QtResourceEditorDialog",
                        "Skip"), QMessageBox::ActionRole);
            msgBox.setEscapeButton(QMessageBox::Cancel);
            msgBox.setDefaultButton(copyButton);
            msgBox.exec();
            QString destPath;
            if (msgBox.clickedButton() == keepButton) {
                // nothing
            } else if (msgBox.clickedButton() == copyButton) {
                QFileInfo resInfo(resourcePath);
                QDir dd(destDir);
                destPath = dd.absoluteFilePath(resInfo.fileName());
                if (dd.exists(resInfo.fileName())) {
                    if (warning(QCoreApplication::translate("QtResourceEditorDialog", "Copy"),
                                msgOverwrite(resInfo.fileName()),
                                QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Yes)
                        continue;
                }
                resourcePath = copyResourceFile(resourcePath, destPath); // returns empty string in case copy failed or was canceled
            } else if (msgBox.clickedButton() == copyAsButton) {
                destPath = browseForNewLocation(resourcePath, dir); // returns empty string in case browsing was canceled
                if (destPath.isEmpty())
                    continue;
                resourcePath = copyResourceFile(resourcePath, destPath); // returns empty string in case copy failed or was canceled
            } else if (msgBox.clickedButton() == skipButton) { // skipped
                continue;
            } else { // canceled
                return;
            }
            if (resourcePath.isEmpty())
                continue;
        }
        relativePath = dir.relativeFilePath(resourcePath);
        QtResourceFile *newResourceFile = m_qrcManager->insertResourceFile(currentResourcePrefix, relativePath, QString(), nextResourceFile);

        QStandardItem *newItem = m_resourceFileToPathItem.value(newResourceFile);
        if (newItem)
            selectTreeRow(newItem);
    }
}

void QtResourceEditorDialogPrivate::slotChangePrefix()
{
    QtResourcePrefix *currentResourcePrefix = getCurrentResourcePrefix();
    if (!currentResourcePrefix)
        return;

    QStandardItem *item = m_resourcePrefixToPrefixItem.value(currentResourcePrefix);
    QModelIndex index = m_treeModel->indexFromItem(item);
    selectTreeRow(item);
    m_ui.resourceTreeView->scrollTo(index);
    m_ui.resourceTreeView->edit(index);
}

void QtResourceEditorDialogPrivate::slotChangeLanguage()
{
    QtResourcePrefix *currentResourcePrefix = getCurrentResourcePrefix();
    if (!currentResourcePrefix)
        return;

    QStandardItem *item = m_resourcePrefixToLanguageItem.value(currentResourcePrefix);
    QModelIndex index = m_treeModel->indexFromItem(item);
    selectTreeRow(item);
    m_ui.resourceTreeView->scrollTo(index);
    m_ui.resourceTreeView->edit(index);
}

void QtResourceEditorDialogPrivate::slotChangeAlias()
{
    QtResourceFile *currentResourceFile = getCurrentResourceFile();
    if (!currentResourceFile)
        return;

    QStandardItem *item = m_resourceFileToAliasItem.value(currentResourceFile);
    QModelIndex index = m_treeModel->indexFromItem(item);
    selectTreeRow(item);
    m_ui.resourceTreeView->scrollTo(index);
    m_ui.resourceTreeView->edit(index);
}

void QtResourceEditorDialogPrivate::slotClonePrefix()
{
    QtResourcePrefix *currentResourcePrefix = getCurrentResourcePrefix();
    if (!currentResourcePrefix)
        return;

    bool ok;
    QString suffix = QInputDialog::getText(q_ptr, QApplication::translate("QtResourceEditorDialog", "Clone Prefix"),
            QCoreApplication::translate("QtResourceEditorDialog", "Enter the suffix which you want to add to the names of the cloned files.\n"
                "This could for example be a language extension like \"_de\"."),
            QLineEdit::Normal, QString(), &ok);
    if (!ok)
        return;

    QtResourcePrefix *newResourcePrefix = m_qrcManager->insertResourcePrefix(m_currentQrcFile, currentResourcePrefix->prefix(),
                                    currentResourcePrefix->language(), m_qrcManager->nextResourcePrefix(currentResourcePrefix));
    if (newResourcePrefix) {
        const auto files = currentResourcePrefix->resourceFiles();
        for (QtResourceFile *resourceFile : files) {
            QString path = resourceFile->path();
            QFileInfo fi(path);
            QDir dir(fi.dir());
            QString oldSuffix = fi.completeSuffix();
            if (!oldSuffix.isEmpty())
                oldSuffix = u'.' + oldSuffix;
            const QString newBaseName = fi.baseName() + suffix + oldSuffix;
            const QString newPath = QDir::cleanPath(dir.filePath(newBaseName));
            m_qrcManager->insertResourceFile(newResourcePrefix, newPath,
                    resourceFile->alias());
        }
    }
}

void QtResourceEditorDialogPrivate::slotRemove()
{
    QStandardItem *item = m_treeModel->itemFromIndex(m_treeSelection->currentIndex());
    if (!item)
        return;

    QtResourceFile *resourceFile = m_pathItemToResourceFile.value(item);
    if (!resourceFile)
        resourceFile = m_aliasItemToResourceFile.value(item);
    QtResourcePrefix *resourcePrefix = m_prefixItemToResourcePrefix.value(item);
    if (!resourcePrefix)
        resourcePrefix = m_languageItemToResourcePrefix.value(item);

    QStandardItem *newCurrentItem = nullptr;

    if (resourceFile) {
        QtResourceFile *nextFile = m_qrcManager->nextResourceFile(resourceFile);
        if (!nextFile)
            nextFile = m_qrcManager->prevResourceFile(resourceFile);
        newCurrentItem = m_resourceFileToPathItem.value(nextFile);
        if (!newCurrentItem)
            newCurrentItem = m_resourcePrefixToPrefixItem.value(m_qrcManager->resourcePrefixOf(resourceFile));
    }
    if (!newCurrentItem) {
        QtResourcePrefix *nextPrefix = m_qrcManager->nextResourcePrefix(resourcePrefix);
        if (!nextPrefix)
            nextPrefix = m_qrcManager->prevResourcePrefix(resourcePrefix);
        newCurrentItem = m_resourcePrefixToPrefixItem.value(nextPrefix);
    }

    selectTreeRow(newCurrentItem);

    if (resourcePrefix)
        m_qrcManager->removeResourcePrefix(resourcePrefix);
    else if (resourceFile)
        m_qrcManager->removeResourceFile(resourceFile);
}

void QtResourceEditorDialogPrivate::slotMoveUp()
{
    if (QtResourceFile *resourceFile = getCurrentResourceFile()) {
        QtResourceFile *prevFile = m_qrcManager->prevResourceFile(resourceFile);

        if (!prevFile)
            return;

        m_qrcManager->moveResourceFile(resourceFile, prevFile);
        selectTreeRow(m_resourceFileToPathItem.value(resourceFile));
    } else if (QtResourcePrefix *resourcePrefix = getCurrentResourcePrefix()) {
        QtResourcePrefix *prevPrefix = m_qrcManager->prevResourcePrefix(resourcePrefix);

        if (!prevPrefix)
            return;

        m_qrcManager->moveResourcePrefix(resourcePrefix, prevPrefix);
        selectTreeRow(m_resourcePrefixToPrefixItem.value(resourcePrefix));
    }
}

void QtResourceEditorDialogPrivate::slotMoveDown()
{
    if (QtResourceFile *resourceFile = getCurrentResourceFile()) {
        QtResourceFile *nextFile = m_qrcManager->nextResourceFile(resourceFile);

        if (!nextFile)
            return;

        m_qrcManager->moveResourceFile(resourceFile, m_qrcManager->nextResourceFile(nextFile));
        selectTreeRow(m_resourceFileToPathItem.value(resourceFile));
    } else if (QtResourcePrefix *resourcePrefix = getCurrentResourcePrefix()) {
        QtResourcePrefix *nextPrefix = m_qrcManager->nextResourcePrefix(resourcePrefix);

        if (!nextPrefix)
            return;

        m_qrcManager->moveResourcePrefix(resourcePrefix, m_qrcManager->nextResourcePrefix(nextPrefix));
        selectTreeRow(m_resourcePrefixToPrefixItem.value(resourcePrefix));
    }
}

QString QtResourceEditorDialogPrivate::browseForNewLocation(const QString &resourceFile, const QDir &rootDir) const
{
    QFileInfo fi(resourceFile);
    const QString initialPath = rootDir.absoluteFilePath(fi.fileName());
    while (true) {
        QString newPath = m_dlgGui->getSaveFileName(q_ptr,
                    QCoreApplication::translate("QtResourceEditorDialog", "Copy As"),
                    initialPath);
        QString relativePath = rootDir.relativeFilePath(newPath);
        if (relativePath.startsWith(".."_L1)) {
            if (warning(QCoreApplication::translate("QtResourceEditorDialog", "Copy As"),
                        QCoreApplication::translate("QtResourceEditorDialog", "<p>The selected file:</p>"
                                        "<p>%1</p><p>is outside of the current resource file's directory:</p><p>%2</p>"
                                        "<p>Please select another path within this directory.<p>").
                        arg(relativePath, rootDir.absolutePath()),
                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)
                return QString();
        } else {
            return newPath;
        }
    }

    return QString();
}

QString QtResourceEditorDialogPrivate::copyResourceFile(const QString &resourceFile, const QString &destPath) const
{
    QFileInfo fi(destPath);
    if (fi.exists()) {
        while (fi.exists() && !QFile::remove(destPath)) {
            if (warning(QCoreApplication::translate("QtResourceEditorDialog", "Copy"),
                        QCoreApplication::translate("QtResourceEditorDialog", "Could not overwrite %1.").arg(fi.fileName()),
                        QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Retry)
                return QString();
        }
    }
    while (!QFile::copy(resourceFile, destPath)) {
        if (warning(QCoreApplication::translate("QtResourceEditorDialog", "Copy"),
                    QCoreApplication::translate("QtResourceEditorDialog", "Could not copy\n%1\nto\n%2")
                    .arg(resourceFile, destPath),
                    QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Retry)
            return QString();
    }
    return destPath;
}
bool QtResourceEditorDialogPrivate::loadQrcFile(const QString &path, QtQrcFileData *qrcFileData)
{
    QString errorMessage;
    const bool rc = loadQrcFile(path, qrcFileData, &errorMessage);
//    if (!rc)
//        warning(QApplication::translate("QtResourceEditorDialog", "Resource File Load Error"), errorMessage);
    return rc;
}
bool QtResourceEditorDialogPrivate::loadQrcFile(const QString &path, QtQrcFileData *qrcFileData, QString *errorMessage)
{
    if (!qrcFileData)
        return false;

    qrcFileData->qrcPath = path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        // there is sufficient hint while loading a form and after opening the editor (qrc marked marked with red and with [missing] text)
        //*errorMessage =  QApplication::translate("QtResourceEditorDialog", "Unable to open %1 for reading: %2").arg(path).arg(file.errorString());
        return false;
    }

    QByteArray dataArray = file.readAll();
    file.close();

    QDomDocument doc;
    if (QDomDocument::ParseResult result = doc.setContent(dataArray)) {
        return loadQrcFileData(doc, path, qrcFileData, errorMessage);
    } else {
        *errorMessage =
            QCoreApplication::translate("QtResourceEditorDialog",
                                        "A parse error occurred at line %1, column %2 of %3:\n%4")
                .arg(result.errorLine).arg(result.errorColumn).arg(path, result.errorMessage);
        return false;
    }
}

bool QtResourceEditorDialogPrivate::saveQrcFile(const QtQrcFileData &qrcFileData)
{
    QFile file(qrcFileData.qrcPath);
    while (!file.open(QIODevice::WriteOnly)) {
        QMessageBox msgBox(QMessageBox::Warning,
                QCoreApplication::translate("QtResourceEditorDialog",
                                            "Save Resource File"),
                QCoreApplication::translate("QtResourceEditorDialog",
                                            "Could not write %1: %2")
                                           .arg(qrcFileData.qrcPath,
                                                file.errorString()),
                QMessageBox::Cancel|QMessageBox::Ignore|QMessageBox::Retry);
        msgBox.setEscapeButton(QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ignore);
        switch (msgBox.exec()) {
        case QMessageBox::Retry:
            break; // nothing
        case QMessageBox::Ignore:
            return true;
        default:
            return false;
        }
    }

    QDomDocument doc = saveQrcFileData(qrcFileData);

    QByteArray dataArray = doc.toByteArray(2);
    file.write(dataArray);

    file.close();
    return true;
}

QtResourceEditorDialog::QtResourceEditorDialog(QDesignerFormEditorInterface *core, QDesignerDialogGuiInterface *dlgGui, QWidget *parent)
    : QDialog(parent), d_ptr(new QtResourceEditorDialogPrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);
    d_ptr->m_qrcManager = new QtQrcManager(this);
    d_ptr->m_dlgGui = dlgGui;
    d_ptr->m_core = core;

    setWindowTitle(tr("Edit Resources"));

    connect(d_ptr->m_qrcManager, &QtQrcManager::qrcFileInserted,
            this, [this](QtQrcFile *file) { d_ptr->slotQrcFileInserted(file); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::qrcFileMoved,
            this, [this](QtQrcFile *file) { d_ptr->slotQrcFileMoved(file); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::qrcFileRemoved,
            this, [this](QtQrcFile *file) { d_ptr->slotQrcFileRemoved(file); });

    connect(d_ptr->m_qrcManager, &QtQrcManager::resourcePrefixInserted,
            this, [this](QtResourcePrefix *prefix) { d_ptr->slotResourcePrefixInserted(prefix); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourcePrefixMoved,
            this, [this](QtResourcePrefix *prefix) { d_ptr->slotResourcePrefixMoved(prefix); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourcePrefixChanged,
            this, [this](QtResourcePrefix *prefix) { d_ptr->slotResourcePrefixChanged(prefix); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourceLanguageChanged,
            this, [this](QtResourcePrefix *prefix) { d_ptr->slotResourceLanguageChanged(prefix); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourcePrefixRemoved,
            this, [this](QtResourcePrefix *prefix) { d_ptr->slotResourcePrefixRemoved(prefix); });

    connect(d_ptr->m_qrcManager, &QtQrcManager::resourceFileInserted,
            this, [this](QtResourceFile *file) { d_ptr->slotResourceFileInserted(file); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourceFileMoved,
            this, [this](QtResourceFile *file) { d_ptr->slotResourceFileMoved(file); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourceAliasChanged,
            this, [this](QtResourceFile *file) { d_ptr->slotResourceAliasChanged(file); });
    connect(d_ptr->m_qrcManager, &QtQrcManager::resourceFileRemoved,
            this, [this](QtResourceFile *file) { d_ptr->slotResourceFileRemoved(file); });

    QIcon upIcon = qdesigner_internal::createIconSet("up.png"_L1);
    QIcon downIcon = qdesigner_internal::createIconSet("down.png"_L1);
    QIcon minusIcon = qdesigner_internal::createIconSet("minus-16.png"_L1);
    QIcon newIcon = qdesigner_internal::createIconSet("filenew-16.png"_L1);
    QIcon openIcon = qdesigner_internal::createIconSet("fileopen-16.png"_L1);
    QIcon removeIcon = qdesigner_internal::createIconSet("editdelete-16.png"_L1);
    QIcon addPrefixIcon = qdesigner_internal::createIconSet("prefix-add.png"_L1);

    d_ptr->m_newQrcFileAction = new QAction(newIcon, tr("New..."), this);
    d_ptr->m_newQrcFileAction->setToolTip(tr("New Resource File"));
    d_ptr->m_importQrcFileAction = new QAction(openIcon, tr("Open..."), this);
    d_ptr->m_importQrcFileAction->setToolTip(tr("Open Resource File"));
    d_ptr->m_removeQrcFileAction = new QAction(removeIcon, tr("Remove"), this);
    d_ptr->m_moveUpQrcFileAction = new QAction(upIcon, tr("Move Up"), this);
    d_ptr->m_moveDownQrcFileAction = new QAction(downIcon, tr("Move Down"), this);

    d_ptr->m_newPrefixAction = new QAction(addPrefixIcon, tr("Add Prefix"), this);
    d_ptr->m_newPrefixAction->setToolTip(tr("Add Prefix"));
    d_ptr->m_addResourceFileAction = new QAction(openIcon, tr("Add Files..."), this);
    d_ptr->m_changePrefixAction = new QAction(tr("Change Prefix"), this);
    d_ptr->m_changeLanguageAction = new QAction(tr("Change Language"), this);
    d_ptr->m_changeAliasAction = new QAction(tr("Change Alias"), this);
    d_ptr->m_clonePrefixAction = new QAction(tr("Clone Prefix..."), this);
    d_ptr->m_removeAction = new QAction(minusIcon, tr("Remove"), this);
    d_ptr->m_moveUpAction = new QAction(upIcon, tr("Move Up"), this);
    d_ptr->m_moveDownAction = new QAction(downIcon, tr("Move Down"), this);

    d_ptr->m_ui.newQrcButton->setDefaultAction(d_ptr->m_newQrcFileAction);
    d_ptr->m_ui.importQrcButton->setDefaultAction(d_ptr->m_importQrcFileAction);
    d_ptr->m_ui.removeQrcButton->setDefaultAction(d_ptr->m_removeQrcFileAction);

    d_ptr->m_ui.newResourceButton->setDefaultAction(d_ptr->m_newPrefixAction);
    d_ptr->m_ui.addResourceButton->setDefaultAction(d_ptr->m_addResourceFileAction);
    d_ptr->m_ui.removeResourceButton->setDefaultAction(d_ptr->m_removeAction);

    connect(d_ptr->m_newQrcFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotNewQrcFile(); });
    connect(d_ptr->m_importQrcFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotImportQrcFile(); });
    connect(d_ptr->m_removeQrcFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotRemoveQrcFile(); });
    connect(d_ptr->m_moveUpQrcFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotMoveUpQrcFile(); });
    connect(d_ptr->m_moveDownQrcFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotMoveDownQrcFile(); });

    connect(d_ptr->m_newPrefixAction, &QAction::triggered,
            this, [this] { d_ptr->slotNewPrefix(); });
    connect(d_ptr->m_addResourceFileAction, &QAction::triggered,
            this, [this] { d_ptr->slotAddFiles(); });
    connect(d_ptr->m_changePrefixAction, &QAction::triggered,
            this, [this] { d_ptr->slotChangePrefix(); });
    connect(d_ptr->m_changeLanguageAction, &QAction::triggered,
            this, [this] { d_ptr->slotChangeLanguage(); });
    connect(d_ptr->m_changeAliasAction, &QAction::triggered,
            this, [this] { d_ptr->slotChangeAlias(); });
    connect(d_ptr->m_clonePrefixAction, &QAction::triggered,
            this, [this] { d_ptr->slotClonePrefix(); });
    connect(d_ptr->m_removeAction, &QAction::triggered,
            this, [this] { d_ptr->slotRemove(); });
    connect(d_ptr->m_moveUpAction, &QAction::triggered,
            this, [this] { d_ptr->slotMoveUp(); });
    connect(d_ptr->m_moveDownAction, &QAction::triggered,
            this, [this] { d_ptr->slotMoveDown(); });

    d_ptr->m_ui.qrcFileList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d_ptr->m_ui.qrcFileList, &QListWidget::customContextMenuRequested,
            this, [this](const QPoint &point) { d_ptr->slotListWidgetContextMenuRequested(point); });
    connect(d_ptr->m_ui.qrcFileList, &QListWidget::currentItemChanged,
            this, [this](QListWidgetItem *item) { d_ptr->slotCurrentQrcFileChanged(item); });

    d_ptr->m_treeModel = new QStandardItemModel(this);
    d_ptr->m_treeModel->setColumnCount(2);
    d_ptr->m_treeModel->setHorizontalHeaderItem(0, new QStandardItem(tr("Prefix / Path")));
    d_ptr->m_treeModel->setHorizontalHeaderItem(1, new QStandardItem(tr("Language / Alias")));
    d_ptr->m_ui.resourceTreeView->setModel(d_ptr->m_treeModel);
    d_ptr->m_ui.resourceTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    d_ptr->m_treeSelection = d_ptr->m_ui.resourceTreeView->selectionModel();
    connect(d_ptr->m_ui.resourceTreeView->header(), &QHeaderView::sectionDoubleClicked,
            d_ptr->m_ui.resourceTreeView, &QTreeView::resizeColumnToContents);
    d_ptr->m_ui.resourceTreeView->setTextElideMode(Qt::ElideLeft);

    connect(d_ptr->m_ui.resourceTreeView, &QTreeView::customContextMenuRequested,
            this, [this](const QPoint &point) { d_ptr->slotTreeViewContextMenuRequested(point); });
    connect(d_ptr->m_treeModel, &QStandardItemModel::itemChanged,
            this, [this](QStandardItem *item) { d_ptr->slotTreeViewItemChanged(item); });
    connect(d_ptr->m_treeSelection, &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &index) { d_ptr->slotCurrentTreeViewItemChanged(index); });

    d_ptr->m_ui.resourceTreeView->setColumnWidth(0, 200);

    d_ptr->slotCurrentTreeViewItemChanged(QModelIndex());
    d_ptr->m_removeQrcFileAction->setEnabled(false);
    d_ptr->m_moveUpQrcFileAction->setEnabled(false);
    d_ptr->m_moveDownQrcFileAction->setEnabled(false);

    QDesignerSettingsInterface *settings = core->settingsManager();
    settings->beginGroup(QrcDialogC);

    d_ptr->m_ui.splitter->restoreState(settings->value(SplitterPosition).toByteArray());
    const QVariant geometry = settings->value(ResourceEditorGeometry);
    if (geometry.metaType().id() == QMetaType::QByteArray) // Used to be a QRect up until 5.4.0, QTBUG-43374
        restoreGeometry(geometry.toByteArray());

    settings->endGroup();
}

QtResourceEditorDialog::~QtResourceEditorDialog()
{
    QDesignerSettingsInterface *settings = d_ptr->m_core->settingsManager();
    settings->beginGroup(QrcDialogC);

    settings->setValue(SplitterPosition, d_ptr->m_ui.splitter->saveState());
    settings->setValue(ResourceEditorGeometry, saveGeometry());
    settings->endGroup();

    disconnect(d_ptr->m_qrcManager, nullptr, this, nullptr);
}

QtResourceModel *QtResourceEditorDialog::model() const
{
    return d_ptr->m_resourceModel;
}

void QtResourceEditorDialog::setResourceModel(QtResourceModel *model)
{
    d_ptr->m_resourceModel = model;

    QtResourceSet *resourceSet = d_ptr->m_resourceModel->currentResourceSet();
    if (!resourceSet) {
        // disable everything but cancel button
        return;
    }

    d_ptr->m_initialState.clear();

    // enable qrcBox

    const QStringList paths = resourceSet->activeResourceFilePaths();
    for (const QString &path : paths) {
        QtQrcFileData qrcFileData;
        d_ptr->loadQrcFile(path, &qrcFileData);
        d_ptr->m_initialState << qrcFileData;
        d_ptr->m_qrcManager->importQrcFile(qrcFileData);
    }
    if (d_ptr->m_ui.qrcFileList->count() > 0) {
        d_ptr->m_ui.qrcFileList->item(0)->setSelected(true);
    }
}

QString QtResourceEditorDialog::selectedResource() const
{
    //QtResourcePrefix *currentResourcePrefix = d_ptr->m_qrcManager->resourcePrefixOf(currentResourceFile);
    QtResourcePrefix *currentResourcePrefix = d_ptr->getCurrentResourcePrefix();
    if (!currentResourcePrefix)
        return QString();

    const QChar slash(u'/');
    QString resource = currentResourcePrefix->prefix();
    if (!resource.startsWith(slash))
        resource.prepend(slash);
    if (!resource.endsWith(slash))
        resource.append(slash);
    resource.prepend(u':');

    QtResourceFile *currentResourceFile = d_ptr->getCurrentResourceFile();
    if (!currentResourceFile)
        return resource;

    QString resourceEnding = currentResourceFile->path();
    if (!currentResourceFile->alias().isEmpty())
        resourceEnding = currentResourceFile->alias();

    const auto dotSlash = "./"_L1;
    const auto dotDotSlash = "../"_L1;
    while (true) {
        if (resourceEnding.startsWith(slash))
            resourceEnding = resourceEnding.mid(1);
        else if (resourceEnding.startsWith(dotSlash))
            resourceEnding = resourceEnding.mid(dotSlash.size());
        else if (resourceEnding.startsWith(dotDotSlash))
            resourceEnding = resourceEnding.mid(dotDotSlash.size());
        else
            break;
    }

    resource.append(resourceEnding);

    return resource;
}

void QtResourceEditorDialog::displayResourceFailures(const QString &logOutput, QDesignerDialogGuiInterface *dlgGui, QWidget *parent)
{
    const QString msg = tr("<html><p><b>Warning:</b> There have been problems while reloading the resources:</p><pre>%1</pre></html>").arg(logOutput);
    dlgGui->message(parent, QDesignerDialogGuiInterface::ResourceEditorMessage, QMessageBox::Warning,
                    tr("Resource Warning"), msg);
}

void QtResourceEditorDialog::accept()
{
    QStringList newQrcPaths;
    QList<QtQrcFileData> currentState;

    const auto qrcFiles = d_ptr->m_qrcManager->qrcFiles();
    for (QtQrcFile *qrcFile : qrcFiles) {
        QtQrcFileData qrcFileData;
        d_ptr->m_qrcManager->exportQrcFile(qrcFile, &qrcFileData);
        currentState << qrcFileData;
        if (qrcFileData == qrcFile->initialState()) {
            // nothing
        } else {
            d_ptr->m_resourceModel->setWatcherEnabled(qrcFileData.qrcPath, false);
            bool ok = d_ptr->saveQrcFile(qrcFileData);
            d_ptr->m_resourceModel->setWatcherEnabled(qrcFileData.qrcPath, true);
            if (!ok)
                return;

            d_ptr->m_resourceModel->setModified(qrcFileData.qrcPath);
        }
        newQrcPaths << qrcFileData.qrcPath;
    }

    if (currentState == d_ptr->m_initialState) {
        // nothing
    } else {
        int errorCount;
        QString errorMessages;
        d_ptr->m_resourceModel->currentResourceSet()->activateResourceFilePaths(newQrcPaths, &errorCount, &errorMessages);
        if (errorCount)
            displayResourceFailures(errorMessages, d_ptr->m_dlgGui, this);
    }
    QDialog::accept();
}

QString QtResourceEditorDialog::editResources(QDesignerFormEditorInterface *core,
                                              QtResourceModel *model,
                                              QDesignerDialogGuiInterface *dlgGui,
                                              QWidget *parent)
{
    QtResourceEditorDialog dialog(core, dlgGui, parent);
    dialog.setResourceModel(model);
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedResource();
    return QString();
}

QT_END_NAMESPACE

#include "moc_qtresourceeditordialog_p.cpp"
#include "qtresourceeditordialog.moc"
