// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qtresourcemodel_p.h"
#include "rcc_p.h"

#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qmap.h>
#include <QtCore/qresource.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qdir.h>
#include <QtCore/qdebug.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qfilesystemwatcher.h>

QT_BEGIN_NAMESPACE

enum { debugResourceModel = 0 };

// ------------------- QtResourceSetPrivate
class QtResourceSetPrivate
{
    QtResourceSet *q_ptr;
    Q_DECLARE_PUBLIC(QtResourceSet)
public:
    QtResourceSetPrivate(QtResourceModel *model = nullptr);

    QtResourceModel *m_resourceModel;
};

QtResourceSetPrivate::QtResourceSetPrivate(QtResourceModel *model) :
   q_ptr(nullptr),
   m_resourceModel(model)
{
}

// -------------------- QtResourceModelPrivate
class QtResourceModelPrivate
{
    QtResourceModel *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(QtResourceModel)
    Q_DISABLE_COPY_MOVE(QtResourceModelPrivate)
public:
    QtResourceModelPrivate();
    void activate(QtResourceSet *resourceSet, const QStringList &newPaths, int *errorCount = nullptr, QString *errorMessages = nullptr);
    void removeOldPaths(QtResourceSet *resourceSet, const QStringList &newPaths);

    QMap<QString, bool> m_pathToModified;
    QHash<QtResourceSet *, QStringList> m_resourceSetToPaths;
    QHash<QtResourceSet *, bool> m_resourceSetToReload; // while path is recreated it needs to be reregistered
                                                        // (it is - in the new current resource set, but when the path was used in
                                                        // other resource set
                                                        // then later when that resource set is activated it needs to be reregistered)
    QHash<QtResourceSet *, bool> m_newlyCreated; // all created but not activated yet
                                                 // (if was active at some point and it's not now it will not be on that map)
    QMap<QString, QList<QtResourceSet *>> m_pathToResourceSet;
    QtResourceSet                         *m_currentResourceSet = nullptr;

    QMap<QString, const QByteArray *> m_pathToData;

    QMap<QString, QStringList> m_pathToContents; // qrc path to its contents.
    QMap<QString, QString>     m_fileToQrc; // this map contains the content of active resource set only.
                                            // Activating different resource set changes the contents.

    QFileSystemWatcher *m_fileWatcher = nullptr;
    bool m_fileWatcherEnabled = true;
    QMap<QString, bool> m_fileWatchedMap;
private:
    void registerResourceSet(QtResourceSet *resourceSet);
    void unregisterResourceSet(QtResourceSet *resourceSet);
    void setWatcherEnabled(const QString &path, bool enable);
    void addWatcher(const QString &path);
    void removeWatcher(const QString &path);

    void slotFileChanged(const QString &);

    const QByteArray *createResource(const QString &path, QStringList *contents, int *errorCount, QIODevice &errorDevice) const;
    void deleteResource(const QByteArray *data) const;
};

QtResourceModelPrivate::QtResourceModelPrivate() = default;

// --------------------- QtResourceSet
QtResourceSet::QtResourceSet() :
    d_ptr(new QtResourceSetPrivate)
{
    d_ptr->q_ptr = this;
}

QtResourceSet::QtResourceSet(QtResourceModel *model) :
     d_ptr(new QtResourceSetPrivate(model))
{
    d_ptr->q_ptr = this;
}

QtResourceSet::~QtResourceSet() = default;

QStringList QtResourceSet::activeResourceFilePaths() const
{
    QtResourceSet *that = const_cast<QtResourceSet *>(this);
    return d_ptr->m_resourceModel->d_ptr->m_resourceSetToPaths.value(that);
}

void QtResourceSet::activateResourceFilePaths(const QStringList &paths, int *errorCount, QString *errorMessages)
{
    d_ptr->m_resourceModel->d_ptr->activate(this, paths, errorCount, errorMessages);
}

bool QtResourceSet::isModified(const QString &path) const
{
    return d_ptr->m_resourceModel->isModified(path);
}

void QtResourceSet::setModified(const QString &path)
{
    d_ptr->m_resourceModel->setModified(path);
}

// ------------------- QtResourceModelPrivate
const QByteArray *QtResourceModelPrivate::createResource(const QString &path, QStringList *contents, int *errorCount, QIODevice &errorDevice) const
{
    using ResourceDataFileMap = RCCResourceLibrary::ResourceDataFileMap;
    const QByteArray *rc = nullptr;
    *errorCount = -1;
    contents->clear();
    do {
        // run RCC
        RCCResourceLibrary library(3);
        library.setVerbose(true);
        library.setInputFiles(QStringList(path));
        library.setFormat(RCCResourceLibrary::Binary);

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        if (!library.readFiles(/* ignore errors*/ true, errorDevice))
            break;
        // return code cannot be fully trusted, might still be empty
        const ResourceDataFileMap resMap = library.resourceDataFileMap();
        if (!library.output(buffer, buffer /* tempfile, unused */, errorDevice))
            break;

        *errorCount = library.failedResources().size();
        *contents = resMap.keys();

        if (resMap.isEmpty())
            break;

        buffer.close();
        rc = new QByteArray(buffer.data());
    } while (false);

    if (debugResourceModel)
        qDebug() << "createResource" << path << "returns data=" << rc << " hasWarnings=" << *errorCount;
    return rc;
}

void QtResourceModelPrivate::deleteResource(const QByteArray *data) const
{
    if (data) {
        if (debugResourceModel)
            qDebug() << "deleteResource";
        delete data;
    }
}

void QtResourceModelPrivate::registerResourceSet(QtResourceSet *resourceSet)
{
    if (!resourceSet)
        return;

    // unregister old paths (all because the order of registration is important), later it can be optimized a bit
    const QStringList toRegister = resourceSet->activeResourceFilePaths();
    for (const QString &path : toRegister) {
        if (debugResourceModel)
            qDebug() << "registerResourceSet " << path;
        const auto itRcc = m_pathToData.constFind(path);
        if (itRcc != m_pathToData.constEnd()) { // otherwise data was not created yet
            const QByteArray *data = itRcc.value();
            if (data) {
                if (!QResource::registerResource(reinterpret_cast<const uchar *>(data->constData()))) {
                    qWarning() << "** WARNING: Failed to register " << path << " (QResource failure).";
                } else {
                    const QStringList contents = m_pathToContents.value(path);
                    for (const QString &filePath : contents) {
                        if (!m_fileToQrc.contains(filePath)) // the first loaded resource has higher priority in qt resource system
                            m_fileToQrc.insert(filePath, path);
                    }
                }
            }
        }
    }
}

void QtResourceModelPrivate::unregisterResourceSet(QtResourceSet *resourceSet)
{
    if (!resourceSet)
        return;

    // unregister old paths (all because the order of registration is importans), later it can be optimized a bit
    const QStringList toUnregister = resourceSet->activeResourceFilePaths();
    for (const QString &path : toUnregister) {
        if (debugResourceModel)
            qDebug() << "unregisterResourceSet " << path;
        const auto itRcc = m_pathToData.constFind(path);
        if (itRcc != m_pathToData.constEnd()) { // otherwise data was not created yet
            const QByteArray *data = itRcc.value();
            if (data) {
                if (!QResource::unregisterResource(reinterpret_cast<const uchar *>(itRcc.value()->constData())))
                    qWarning() << "** WARNING: Failed to unregister " << path << " (QResource failure).";
            }
        }
    }
    m_fileToQrc.clear();
}

void QtResourceModelPrivate::activate(QtResourceSet *resourceSet, const QStringList &newPaths, int *errorCountPtr, QString *errorMessages)
{
    if (debugResourceModel)
        qDebug() << "activate " << resourceSet;
    if (errorCountPtr)
        *errorCountPtr = 0;
    if (errorMessages)
        errorMessages->clear();

    QBuffer errorStream;
    errorStream.open(QIODevice::WriteOnly);

    int errorCount = 0;
    int generatedCount = 0;
    bool newResourceSetChanged = false;

    if (resourceSet && resourceSet->activeResourceFilePaths() != newPaths && !m_newlyCreated.contains(resourceSet))
        newResourceSetChanged = true;

    auto newPathToData = m_pathToData;

    for (const QString &path : newPaths) {
        if (resourceSet && !m_pathToResourceSet[path].contains(resourceSet))
            m_pathToResourceSet[path].append(resourceSet);
        const auto itMod = m_pathToModified.find(path);
        if (itMod == m_pathToModified.end() || itMod.value()) { // new path or path is already created, but needs to be recreated
            QStringList contents;
            int qrcErrorCount;
            generatedCount++;
            const QByteArray *data = createResource(path, &contents, &qrcErrorCount, errorStream);

            newPathToData.insert(path, data);
            if (qrcErrorCount) // Count single failed files as sort of 1/2 error
                errorCount++;
            addWatcher(path);

            m_pathToModified.insert(path, false);
            m_pathToContents.insert(path, contents);
            newResourceSetChanged = true;
            const auto itReload = m_pathToResourceSet.find(path);
            if (itReload != m_pathToResourceSet.end()) {
                const auto resources = itReload.value();
                for (QtResourceSet *res : resources) {
                    if (res != resourceSet) {
                        m_resourceSetToReload[res] = true;
                    }
                }
            }
        } else { // path is already created, don't need to recreate
        }
    }

    const auto oldData = m_pathToData.values();
    const auto newData = newPathToData.values();

    QList<const QByteArray *> toDelete;
    for (const QByteArray *array : oldData) {
        if (array && !newData.contains(array))
            toDelete.append(array);
    }

    // Nothing can fail below here?
    if (generatedCount) {
        if (errorCountPtr)
            *errorCountPtr = errorCount;
        errorStream.close();
        const QString stderrOutput = QString::fromUtf8(errorStream.data());
        if (debugResourceModel)
            qDebug() << "Output: (" << errorCount << ")\n" << stderrOutput;
        if (errorMessages)
            *errorMessages = stderrOutput;
    }
    // register
    const auto itReload = m_resourceSetToReload.find(resourceSet);
    if (itReload != m_resourceSetToReload.end()) {
        if (itReload.value()) {
            newResourceSetChanged = true;
            m_resourceSetToReload.insert(resourceSet, false);
        }
    }

    QStringList oldActivePaths;
    if (m_currentResourceSet)
        oldActivePaths = m_currentResourceSet->activeResourceFilePaths();

    const bool needReregister = (oldActivePaths != newPaths) || newResourceSetChanged;

    const auto itNew = m_newlyCreated.find(resourceSet);
    if (itNew != m_newlyCreated.end()) {
        m_newlyCreated.remove(resourceSet);
        if (needReregister)
            newResourceSetChanged = true;
    }

    if (!newResourceSetChanged && !needReregister && (m_currentResourceSet == resourceSet)) {
        for (const QByteArray *data : std::as_const(toDelete))
            deleteResource(data);

        return; // nothing changed
    }

    if (needReregister)
        unregisterResourceSet(m_currentResourceSet);

    for (const QByteArray *data : std::as_const(toDelete))
        deleteResource(data);

    m_pathToData = newPathToData;
    m_currentResourceSet = resourceSet;

    if (resourceSet)
        removeOldPaths(resourceSet, newPaths);

    if (needReregister)
        registerResourceSet(m_currentResourceSet);

    emit q_ptr->resourceSetActivated(m_currentResourceSet, newResourceSetChanged);

    // deactivates the paths from old current resource set
    // add new paths to the new current resource set
    // reloads all paths which are marked as modified from the current resource set;
    // activates the paths from current resource set
    // emits resourceSetActivated() (don't emit only in case when old resource set is the same as new one
    // AND no path was reloaded AND the list of paths is exactly the same)
}

void QtResourceModelPrivate::removeOldPaths(QtResourceSet *resourceSet, const QStringList &newPaths)
{
    const QStringList oldPaths = m_resourceSetToPaths.value(resourceSet);
    if (oldPaths != newPaths) {
        // remove old
        for (const QString &oldPath : oldPaths) {
            if (!newPaths.contains(oldPath)) {
                const auto itRemove = m_pathToResourceSet.find(oldPath);
                if (itRemove != m_pathToResourceSet.end()) {
                    const int idx = itRemove.value().indexOf(resourceSet);
                    if (idx >= 0)
                        itRemove.value().removeAt(idx);
                    if (itRemove.value().isEmpty()) {
                        const auto it = m_pathToData.find(oldPath);
                        if (it != m_pathToData.end())
                            deleteResource(it.value());
                        m_pathToResourceSet.erase(itRemove);
                        m_pathToModified.remove(oldPath);
                        m_pathToContents.remove(oldPath);
                        m_pathToData.remove(oldPath);
                        removeWatcher(oldPath);
                    }
                }
            }
        }
        m_resourceSetToPaths[resourceSet] = newPaths;
    }
}

void QtResourceModelPrivate::setWatcherEnabled(const QString &path, bool enable)
{
    if (!enable) {
        m_fileWatcher->removePath(path);
        return;
    }

    QFileInfo fi(path);
    if (fi.exists())
        m_fileWatcher->addPath(path);
}

void QtResourceModelPrivate::addWatcher(const QString &path)
{
    const auto it = m_fileWatchedMap.constFind(path);
    if (it != m_fileWatchedMap.constEnd() && !it.value())
        return;

    m_fileWatchedMap.insert(path, true);
    if (!m_fileWatcherEnabled)
        return;
    setWatcherEnabled(path, true);
}

void QtResourceModelPrivate::removeWatcher(const QString &path)
{
    if (!m_fileWatchedMap.contains(path))
        return;

    m_fileWatchedMap.remove(path);
    if (!m_fileWatcherEnabled)
        return;
    setWatcherEnabled(path, false);
}

void QtResourceModelPrivate::slotFileChanged(const QString &path)
{
    setWatcherEnabled(path, false);
    emit q_ptr->qrcFileModifiedExternally(path);
    setWatcherEnabled(path, true); //readd
}

// ----------------------- QtResourceModel
QtResourceModel::QtResourceModel(QObject *parent) :
    QObject(parent),
    d_ptr(new QtResourceModelPrivate)
{
    d_ptr->q_ptr = this;

    d_ptr->m_fileWatcher = new QFileSystemWatcher(this);
    connect(d_ptr->m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString &fileName) { d_ptr->slotFileChanged(fileName); });
}

QtResourceModel::~QtResourceModel()
{
    blockSignals(true);
    const auto resourceList = resourceSets();
    for (QtResourceSet *rs : resourceList)
        removeResourceSet(rs);
    blockSignals(false);
}

QStringList QtResourceModel::loadedQrcFiles() const
{
    return d_ptr->m_pathToModified.keys();
}

bool QtResourceModel::isModified(const QString &path) const
{
    return d_ptr->m_pathToModified.value(path, true);
}

void QtResourceModel::setModified(const QString &path)
{
    if (!d_ptr->m_pathToModified.contains(path))
        return;

    d_ptr->m_pathToModified[path] = true;
    const auto it = d_ptr->m_pathToResourceSet.constFind(path);
    if (it == d_ptr->m_pathToResourceSet.constEnd())
        return;

    const auto resourceList = it.value();
    for (QtResourceSet *rs : resourceList)
        d_ptr->m_resourceSetToReload.insert(rs, true);
}

QList<QtResourceSet *> QtResourceModel::resourceSets() const
{
    return d_ptr->m_resourceSetToPaths.keys();
}

QtResourceSet *QtResourceModel::currentResourceSet() const
{
    return d_ptr->m_currentResourceSet;
}

void QtResourceModel::setCurrentResourceSet(QtResourceSet *resourceSet, int *errorCount, QString *errorMessages)
{
    d_ptr->activate(resourceSet, d_ptr->m_resourceSetToPaths.value(resourceSet), errorCount, errorMessages);
}

QtResourceSet *QtResourceModel::addResourceSet(const QStringList &paths)
{
    QtResourceSet *newResource = new QtResourceSet(this);
    d_ptr->m_resourceSetToPaths.insert(newResource, paths);
    d_ptr->m_resourceSetToReload.insert(newResource, false);
    d_ptr->m_newlyCreated.insert(newResource, true);
    for (const QString &path : paths)
        d_ptr->m_pathToResourceSet[path].append(newResource);
    return newResource;
}

// TODO
void QtResourceModel::removeResourceSet(QtResourceSet *resourceSet)
{
    if (!resourceSet)
        return;
    if (currentResourceSet() == resourceSet)
        setCurrentResourceSet(nullptr);

    // remove rcc files for those paths which are not used in any other resource set
    d_ptr->removeOldPaths(resourceSet, QStringList());

    d_ptr->m_resourceSetToPaths.remove(resourceSet);
    d_ptr->m_resourceSetToReload.remove(resourceSet);
    d_ptr->m_newlyCreated.remove(resourceSet);
    delete resourceSet;
}

void QtResourceModel::reload(const QString &path, int *errorCount, QString *errorMessages)
{
    setModified(path);

    d_ptr->activate(d_ptr->m_currentResourceSet, d_ptr->m_resourceSetToPaths.value(d_ptr->m_currentResourceSet), errorCount, errorMessages);
}

void QtResourceModel::reload(int *errorCount, QString *errorMessages)
{
    for (auto it = d_ptr->m_pathToModified.begin(), end = d_ptr->m_pathToModified.end(); it != end; ++it)
        it.value() = true;

    // empty resourceSets could be omitted here
    for (auto itReload = d_ptr->m_resourceSetToReload.begin(), end = d_ptr->m_resourceSetToReload.end(); itReload != end; ++itReload)
        itReload.value() = true;

    d_ptr->activate(d_ptr->m_currentResourceSet, d_ptr->m_resourceSetToPaths.value(d_ptr->m_currentResourceSet), errorCount, errorMessages);
}

QMap<QString, QString> QtResourceModel::contents() const
{
    return d_ptr->m_fileToQrc;
}

QString QtResourceModel::qrcPath(const QString &file) const
{
    return d_ptr->m_fileToQrc.value(file);
}

void QtResourceModel::setWatcherEnabled(bool enable)
{
    if (d_ptr->m_fileWatcherEnabled == enable)
        return;

    d_ptr->m_fileWatcherEnabled = enable;

    if (!d_ptr->m_fileWatchedMap.isEmpty())
        d_ptr->setWatcherEnabled(d_ptr->m_fileWatchedMap.firstKey(), d_ptr->m_fileWatcherEnabled);
}

bool QtResourceModel::isWatcherEnabled() const
{
    return d_ptr->m_fileWatcherEnabled;
}

void QtResourceModel::setWatcherEnabled(const QString &path, bool enable)
{
    const auto it = d_ptr->m_fileWatchedMap.find(path);
    if (it == d_ptr->m_fileWatchedMap.end())
        return;

    if (it.value() == enable)
        return;

    it.value() = enable;

    if (!d_ptr->m_fileWatcherEnabled)
        return;

    d_ptr->setWatcherEnabled(it.key(), enable);
}

bool QtResourceModel::isWatcherEnabled(const QString &path)
{
    return d_ptr->m_fileWatchedMap.value(path, false);
}

QT_END_NAMESPACE

#include "moc_qtresourcemodel_p.cpp"
