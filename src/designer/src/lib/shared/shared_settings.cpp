// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "shared_settings_p.h"
#include "grid_p.h"
#include "previewmanager_p.h"
#include "qdesigner_utils_p.h"

#include <actioneditor_p.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractsettings.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qdir.h>
#include <QtCore/qvariant.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qsize.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static const char defaultGridKey[] = "defaultGrid";
static const char previewKey[] = "Preview";
static const char enabledKey[] = "Enabled";
static const char *userDeviceSkinsKey= "UserDeviceSkins";
static const char zoomKey[] = "zoom";
static const char zoomEnabledKey[] = "zoomEnabled";
static const char deviceProfileIndexKey[] = "DeviceProfileIndex";
static const char deviceProfilesKey[] = "DeviceProfiles";
static const char formTemplatePathsKey[] = "FormTemplatePaths";
static const char formTemplateKey[] = "FormTemplate";
static const char newFormSizeKey[] = "NewFormSize";
static inline QString namingModeKey() { return u"naming"_s; }
static inline QString underScoreNamingMode() { return u"underscore"_s; }
static inline QString camelCaseNamingMode() { return u"camelcase"_s; }

static bool checkTemplatePath(const QString &path, bool create)
{
    QDir current(QDir::current());
    if (current.exists(path))
        return true;

    if (!create)
        return false;

    if (current.mkpath(path))
        return true;

    qdesigner_internal::designerWarning(QCoreApplication::translate("QDesignerSharedSettings", "The template path %1 could not be created.").arg(path));
    return false;
}

namespace qdesigner_internal {

QDesignerSharedSettings::QDesignerSharedSettings(QDesignerFormEditorInterface *core)
        : m_settings(core->settingsManager())
{
}

Grid QDesignerSharedSettings::defaultGrid() const
{
    Grid grid;
    const QVariantMap defaultGridMap
            = m_settings->value(QLatin1StringView(defaultGridKey), QVariantMap()).toMap();
    if (!defaultGridMap.isEmpty())
        grid.fromVariantMap(defaultGridMap);
    return grid;
}

void QDesignerSharedSettings::setDefaultGrid(const Grid &grid)
{
    m_settings->setValue(QLatin1StringView(defaultGridKey), grid.toVariantMap());
}

const QStringList &QDesignerSharedSettings::defaultFormTemplatePaths()
{
    static QStringList rc;
    if (rc.isEmpty()) {
        // Ensure default form template paths
        const auto templatePath = "/templates"_L1;
        // home
        QString path = dataDirectory() + templatePath;
        if (checkTemplatePath(path, true))
            rc += path;

        // designer/bin: Might be owned by root in some installations, do not force it.
        path = qApp->applicationDirPath();
        path += templatePath;
        if (checkTemplatePath(path, false))
            rc += path;
    }
    return rc;
}

// Migrate templates from $HOME/.designer to standard paths in Qt 7
// ### FIXME Qt 8: Remove (QTBUG-96005)
void QDesignerSharedSettings::migrateTemplates()
{
    const QString templatePath = u"/templates"_s;
    QString path = dataDirectory() + templatePath;
    if (QFileInfo::exists(path))
        return;
    if (!QDir().mkpath(path))
        return;
    QString legacyPath = legacyDataDirectory() + templatePath;
    if (!QFileInfo::exists(path))
        return;
    const auto &files = QDir(legacyPath).entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
    for (const auto &file : files) {
        const QString newPath = path + u'/' + file.fileName();
        QFile::copy(file.absoluteFilePath(), newPath);
    }
}

QStringList QDesignerSharedSettings::formTemplatePaths() const
{
    return m_settings->value(QLatin1StringView(formTemplatePathsKey),
                            defaultFormTemplatePaths()).toStringList();
}

void QDesignerSharedSettings::setFormTemplatePaths(const QStringList &paths)
{
    m_settings->setValue(QLatin1StringView(formTemplatePathsKey), paths);
}

QString  QDesignerSharedSettings::formTemplate() const
{
    return m_settings->value(QLatin1StringView(formTemplateKey)).toString();
}

void QDesignerSharedSettings::setFormTemplate(const QString &t)
{
    m_settings->setValue(QLatin1StringView(formTemplateKey), t);
}

void QDesignerSharedSettings::setAdditionalFormTemplatePaths(const QStringList &additionalPaths)
{
    // merge template paths
    QStringList templatePaths = defaultFormTemplatePaths();
    templatePaths += additionalPaths;
    setFormTemplatePaths(templatePaths);
}

QStringList QDesignerSharedSettings::additionalFormTemplatePaths() const
{
    // get template paths excluding internal ones
    QStringList rc = formTemplatePaths();
    for (const QString &internalTemplatePath : defaultFormTemplatePaths()) {
        const int index = rc.indexOf(internalTemplatePath);
        if (index != -1)
            rc.removeAt(index);
    }
    return rc;
}

QSize QDesignerSharedSettings::newFormSize() const
{
    return m_settings->value(QLatin1StringView(newFormSizeKey), QSize(0, 0)).toSize();
}

void  QDesignerSharedSettings::setNewFormSize(const QSize &s)
{
    if (s.isNull()) {
        m_settings->remove(QLatin1StringView(newFormSizeKey));
    } else {
        m_settings->setValue(QLatin1StringView(newFormSizeKey), s);
    }
}


PreviewConfiguration QDesignerSharedSettings::customPreviewConfiguration() const
{
    PreviewConfiguration configuration;
    configuration.fromSettings(QLatin1StringView(previewKey), m_settings);
    return configuration;
}

void QDesignerSharedSettings::setCustomPreviewConfiguration(const PreviewConfiguration &configuration)
{
    configuration.toSettings(QLatin1StringView(previewKey), m_settings);
}

bool QDesignerSharedSettings::isCustomPreviewConfigurationEnabled() const
{
    m_settings->beginGroup(QLatin1StringView(previewKey));
    bool isEnabled = m_settings->value(QLatin1StringView(enabledKey), false).toBool();
    m_settings->endGroup();
    return isEnabled;
}

void QDesignerSharedSettings::setCustomPreviewConfigurationEnabled(bool enabled)
{
    m_settings->beginGroup(QLatin1StringView(previewKey));
    m_settings->setValue(QLatin1StringView(enabledKey), enabled);
    m_settings->endGroup();
}

QStringList QDesignerSharedSettings::userDeviceSkins() const
{
    m_settings->beginGroup(QLatin1StringView(previewKey));
    QStringList userDeviceSkins
            = m_settings->value(QLatin1StringView(userDeviceSkinsKey), QStringList()).toStringList();
    m_settings->endGroup();
    return userDeviceSkins;
}

void QDesignerSharedSettings::setUserDeviceSkins(const QStringList &userDeviceSkins)
{
    m_settings->beginGroup(QLatin1StringView(previewKey));
    m_settings->setValue(QLatin1StringView(userDeviceSkinsKey), userDeviceSkins);
    m_settings->endGroup();
}

int QDesignerSharedSettings::zoom() const
{
    return m_settings->value(QLatin1StringView(zoomKey), 100).toInt();
}

void QDesignerSharedSettings::setZoom(int z)
{
    m_settings->setValue(QLatin1StringView(zoomKey), QVariant(z));
}

ObjectNamingMode QDesignerSharedSettings::objectNamingMode() const
{
    const QString value = m_settings->value(namingModeKey()).toString();
    return value == camelCaseNamingMode()
        ? qdesigner_internal::CamelCase : qdesigner_internal::Underscore;
}

void QDesignerSharedSettings::setObjectNamingMode(ObjectNamingMode n)
{
    const QString value = n == qdesigner_internal::CamelCase
        ? camelCaseNamingMode() : underScoreNamingMode();
    m_settings->setValue(namingModeKey(), QVariant(value));
}

bool QDesignerSharedSettings::zoomEnabled() const
{
    return m_settings->value(QLatin1StringView(zoomEnabledKey), false).toBool();
}

void QDesignerSharedSettings::setZoomEnabled(bool v)
{
     m_settings->setValue(QLatin1StringView(zoomEnabledKey), v);
}

DeviceProfile QDesignerSharedSettings::currentDeviceProfile() const
{
    return deviceProfileAt(currentDeviceProfileIndex());
}

void QDesignerSharedSettings::setCurrentDeviceProfileIndex(int i)
{
    m_settings->setValue(QLatin1StringView(deviceProfileIndexKey), i);
}

int QDesignerSharedSettings::currentDeviceProfileIndex() const
{
     return m_settings->value(QLatin1StringView(deviceProfileIndexKey), -1).toInt();
}

static inline QString msgWarnDeviceProfileXml(const QString &msg)
{
    return QCoreApplication::translate("QDesignerSharedSettings", "An error has been encountered while parsing device profile XML: %1").arg(msg);
}

DeviceProfile QDesignerSharedSettings::deviceProfileAt(int idx) const
{
    DeviceProfile rc;
    if (idx < 0)
        return rc;
    const QStringList xmls = deviceProfileXml();
    if (idx >= xmls.size())
        return rc;
    QString errorMessage;
    if (!rc.fromXml(xmls.at(idx), &errorMessage)) {
        rc.clear();
        designerWarning(msgWarnDeviceProfileXml(errorMessage));
    }
    return rc;
}

QStringList QDesignerSharedSettings::deviceProfileXml() const
{
    return m_settings->value(QLatin1StringView(deviceProfilesKey), QStringList()).toStringList();
}

QDesignerSharedSettings::DeviceProfileList QDesignerSharedSettings::deviceProfiles() const
{
    DeviceProfileList rc;
    const QStringList xmls = deviceProfileXml();
    if (xmls.isEmpty())
        return rc;
    // De-serialize
    QString errorMessage;
    DeviceProfile dp;
    for (const auto &xml : xmls) {
        if (dp.fromXml(xml, &errorMessage)) {
            rc.push_back(dp);
        } else {
            designerWarning(msgWarnDeviceProfileXml(errorMessage));
        }
    }
    return rc;
}

void QDesignerSharedSettings::setDeviceProfiles(const DeviceProfileList &dpl)
{
    QStringList l;
    for (const auto &dp : dpl)
        l.push_back(dp.toXml());
    m_settings->setValue(QLatin1StringView(deviceProfilesKey), l);
}
}

QT_END_NAMESPACE
