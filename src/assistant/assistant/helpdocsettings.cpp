// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "helpdocsettings.h"

#include <QtHelp/QCompressedHelpInfo>
#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpFilterEngine>

#include <QtCore/QVersionNumber>

#include <QtDebug>

QT_BEGIN_NAMESPACE

class HelpDocSettingsPrivate : public QSharedData
{
public:
    HelpDocSettingsPrivate() = default;
    HelpDocSettingsPrivate(const HelpDocSettingsPrivate &other) = default;
    ~HelpDocSettingsPrivate() = default;

    QMap<QString, QString>            m_namespaceToComponent;
    QMap<QString, QStringList>        m_componentToNamespace;

    QMap<QString, QVersionNumber>     m_namespaceToVersion;
    QMap<QVersionNumber, QStringList> m_versionToNamespace;

    QMap<QString, QString>            m_namespaceToFileName;
    QMap<QString, QString>            m_fileNameToNamespace;
};


HelpDocSettings::HelpDocSettings()
    : d(new HelpDocSettingsPrivate)
{
}

HelpDocSettings::HelpDocSettings(const HelpDocSettings &) = default;

HelpDocSettings::HelpDocSettings(HelpDocSettings &&) = default;

HelpDocSettings::~HelpDocSettings() = default;

HelpDocSettings &HelpDocSettings::operator=(const HelpDocSettings &) = default;

HelpDocSettings &HelpDocSettings::operator=(HelpDocSettings &&) = default;

bool HelpDocSettings::addDocumentation(const QString &fileName)
{
    const QCompressedHelpInfo info = QCompressedHelpInfo::fromCompressedHelpFile(fileName);

    if (info.isNull())
        return false;

    const QString namespaceName = info.namespaceName();

    if (d->m_namespaceToFileName.contains(namespaceName))
        return false;

    if (d->m_fileNameToNamespace.contains(fileName))
        return false;

    const QString component = info.component();
    const QVersionNumber version = info.version();

    d->m_namespaceToFileName.insert(namespaceName, fileName);
    d->m_fileNameToNamespace.insert(fileName, namespaceName);

    d->m_namespaceToComponent.insert(namespaceName, component);
    d->m_componentToNamespace[component].append(namespaceName);

    d->m_namespaceToVersion.insert(namespaceName, version);
    d->m_versionToNamespace[version].append(namespaceName);

    return true;
}

bool HelpDocSettings::removeDocumentation(const QString &namespaceName)
{
    if (namespaceName.isEmpty())
        return false;

    const QString fileName = d->m_namespaceToFileName.value(namespaceName);
    if (fileName.isEmpty())
        return false;

    const QString component = d->m_namespaceToComponent.value(namespaceName);
    const QVersionNumber version = d->m_namespaceToVersion.value(namespaceName);

    d->m_namespaceToComponent.remove(namespaceName);
    d->m_namespaceToVersion.remove(namespaceName);
    d->m_namespaceToFileName.remove(namespaceName);
    d->m_fileNameToNamespace.remove(fileName);
    d->m_componentToNamespace[component].removeOne(namespaceName);
    if (d->m_componentToNamespace[component].isEmpty())
        d->m_componentToNamespace.remove(component);
    d->m_versionToNamespace[version].removeOne(namespaceName);
    if (d->m_versionToNamespace[version].isEmpty())
        d->m_versionToNamespace.remove(version);

    return true;
}

QString HelpDocSettings::namespaceName(const QString &fileName) const
{
    return d->m_fileNameToNamespace.value(fileName);
}

QStringList HelpDocSettings::components() const
{
    return d->m_componentToNamespace.keys();
}

QList<QVersionNumber> HelpDocSettings::versions() const
{
    return d->m_versionToNamespace.keys();
}

QStringList HelpDocSettings::namespaces() const
{
    return d->m_namespaceToFileName.keys();
}

QMap<QString, QString> HelpDocSettings::namespaceToFileName() const
{
    return d->m_namespaceToFileName;
}

HelpDocSettings HelpDocSettings::readSettings(QHelpEngineCore *helpEngine)
{
    QHelpFilterEngine *filterEngine = helpEngine->filterEngine();

    HelpDocSettings docSettings;
    docSettings.d->m_namespaceToComponent = filterEngine->namespaceToComponent();
    docSettings.d->m_namespaceToVersion = filterEngine->namespaceToVersion();
    for (auto it = docSettings.d->m_namespaceToComponent.constBegin();
         it != docSettings.d->m_namespaceToComponent.constEnd(); ++it) {
        const QString namespaceName = it.key();
        const QString namespaceFileName = helpEngine->documentationFileName(namespaceName);
        docSettings.d->m_namespaceToFileName.insert(namespaceName, namespaceFileName);
        docSettings.d->m_fileNameToNamespace.insert(namespaceFileName, namespaceName);
        docSettings.d->m_componentToNamespace[it.value()].append(namespaceName);
    }
    for (auto it = docSettings.d->m_namespaceToVersion.constBegin();
         it != docSettings.d->m_namespaceToVersion.constEnd(); ++it) {
        docSettings.d->m_versionToNamespace[it.value()].append(it.key());
    }

    return docSettings;
}

static QMap<QString, QString> subtract(const QMap<QString, QString> &minuend,
                                       const QMap<QString, QString> &subtrahend)
{
    auto result = minuend;

    for (auto itSubtrahend = subtrahend.cbegin(); itSubtrahend != subtrahend.cend(); ++itSubtrahend) {
        auto itResult = result.find(itSubtrahend.key());
        if (itResult != result.end() && itSubtrahend.value() == itResult.value())
            result.erase(itResult);
    }

    return result;
}

bool HelpDocSettings::applySettings(QHelpEngineCore *helpEngine,
                                    const HelpDocSettings &settings)
{
    const HelpDocSettings oldSettings = readSettings(helpEngine);

    const QMap<QString, QString> docsToRemove = subtract(
                oldSettings.namespaceToFileName(),
                settings.namespaceToFileName());
    const QMap<QString, QString> docsToAdd = subtract(
                settings.namespaceToFileName(),
                oldSettings.namespaceToFileName());

    bool changed = false;
    for (const QString &namespaceName : docsToRemove.keys()) {
        if (!helpEngine->unregisterDocumentation(namespaceName))
            qWarning() << "Cannot unregister documentation:" << namespaceName;
        changed = true;
    }

    for (const QString &fileName : docsToAdd.values()) {
        if (!helpEngine->registerDocumentation(fileName))
            qWarning() << "Cannot register documentation file:" << fileName;
        changed = true;
    }

    return changed;
}

QT_END_NAMESPACE
