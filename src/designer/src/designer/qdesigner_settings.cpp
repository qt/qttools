// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner.h"
#include "qdesigner_settings.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_workbench.h"

#include <QtDesigner/abstractsettings.h>

#include <abstractformeditor.h>
#include <qdesigner_utils_p.h>
#include <previewmanager_p.h>

#include <QtCore/qvariant.h>
#include <QtCore/qdir.h>

#include <QtWidgets/qstyle.h>
#include <QtWidgets/qlistview.h>

#include <QtCore/qdebug.h>

enum { debugSettings = 0 };

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto newFormShowKey = "newFormDialog/ShowOnStartup"_L1;

// Change the version whenever the arrangement changes significantly.
static constexpr auto mainWindowStateKey = "MainWindowState45"_L1;
static constexpr auto toolBarsStateKey = "ToolBarsState45"_L1;

static constexpr auto backupOrgListKey = "backup/fileListOrg"_L1;
static constexpr auto backupBakListKey = "backup/fileListBak"_L1;
static constexpr auto recentFilesListKey = "recentFilesList"_L1;

QDesignerSettings::QDesignerSettings(QDesignerFormEditorInterface *core) :
    qdesigner_internal::QDesignerSharedSettings(core)
{
}

void QDesignerSettings::setValue(const QString &key, const QVariant &value)
{
    settings()->setValue(key, value);
}

QVariant QDesignerSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return settings()->value(key, defaultValue);
}

static inline QChar modeChar(UIMode mode)
{
    return QLatin1Char(static_cast<char>(mode) + '0');
}

void QDesignerSettings::saveGeometryFor(const QWidget *w)
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    QDesignerSettingsInterface *s = settings();
    const bool visible = w->isVisible();
    if (debugSettings)
        qDebug() << Q_FUNC_INFO << w << "visible=" << visible;
    s->beginGroup(w->objectName());
    s->setValue(u"visible"_s, visible);
    s->setValue(u"geometry"_s, w->saveGeometry());
    s->endGroup();
}

void QDesignerSettings::restoreGeometry(QWidget *w, QRect fallBack) const
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    const QString key = w->objectName();
    const QByteArray ba(settings()->value(key + "/geometry"_L1).toByteArray());
    const bool visible = settings()->value(key + "/visible"_L1, true).toBool();

    if (debugSettings)
        qDebug() << Q_FUNC_INFO << w << fallBack << "visible=" << visible;
    if (ba.isEmpty()) {
        /// Apply default geometry, check for null and maximal size
        if (fallBack.isNull())
            fallBack = QRect(QPoint(0, 0), w->sizeHint());
        if (fallBack.size() == QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
            w->setWindowState(w->windowState() | Qt::WindowMaximized);
        } else {
            w->move(fallBack.topLeft());
            w->resize(fallBack.size());
        }
    } else {
        w->restoreGeometry(ba);
    }

    if (visible)
        w->show();
}

QStringList QDesignerSettings::recentFilesList() const
{
    return settings()->value(recentFilesListKey).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    settings()->setValue(recentFilesListKey, sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    settings()->setValue(newFormShowKey, showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return settings()->value(newFormShowKey, true).toBool();
}

QByteArray QDesignerSettings::mainWindowState(UIMode mode) const
{
    return settings()->value(mainWindowStateKey + modeChar(mode)).toByteArray();
}

void QDesignerSettings::setMainWindowState(UIMode mode, const QByteArray &mainWindowState)
{
    settings()->setValue(mainWindowStateKey + modeChar(mode), mainWindowState);
}

QByteArray QDesignerSettings::toolBarsState(UIMode mode) const
{
    QString key = toolBarsStateKey;
    key += modeChar(mode);
    return settings()->value(key).toByteArray();
}

void QDesignerSettings::setToolBarsState(UIMode mode, const QByteArray &toolBarsState)
{
    QString key = toolBarsStateKey;
    key += modeChar(mode);
    settings()->setValue(key, toolBarsState);
}

void QDesignerSettings::clearBackup()
{
    QDesignerSettingsInterface *s = settings();
    s->remove(backupOrgListKey);
    s->remove(backupBakListKey);
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
    const QStringList org = map.keys();
    const QStringList bak = map.values();

    QDesignerSettingsInterface *s = settings();
    s->setValue(backupOrgListKey, org);
    s->setValue(backupBakListKey, bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
    const QStringList org = settings()->value(backupOrgListKey, QStringList()).toStringList();
    const QStringList bak = settings()->value(backupBakListKey, QStringList()).toStringList();

    QMap<QString, QString> map;
    const qsizetype orgCount = org.size();
    for (qsizetype i = 0; i < orgCount; ++i)
        map.insert(org.at(i), bak.at(i));

    return map;
}

void QDesignerSettings::setUiMode(UIMode mode)
{
    QDesignerSettingsInterface *s = settings();
    s->beginGroup(u"UI"_s);
    s->setValue(u"currentMode"_s, mode);
    s->endGroup();
}

UIMode QDesignerSettings::uiMode() const
{
    constexpr UIMode defaultMode = DockedMode;
    UIMode uiMode = static_cast<UIMode>(value(u"UI/currentMode"_s, defaultMode).toInt());
    return uiMode;
}

void QDesignerSettings::setToolWindowFont(const ToolWindowFontSettings &fontSettings)
{
    QDesignerSettingsInterface *s = settings();
    s->beginGroup(u"UI"_s);
    s->setValue(u"font"_s, fontSettings.m_font);
    s->setValue(u"useFont"_s, fontSettings.m_useFont);
    s->setValue(u"writingSystem"_s, fontSettings.m_writingSystem);
    s->endGroup();
}

ToolWindowFontSettings QDesignerSettings::toolWindowFont() const
{
    ToolWindowFontSettings fontSettings;
    fontSettings.m_writingSystem =
            static_cast<QFontDatabase::WritingSystem>(value(u"UI/writingSystem"_s,
                                                            QFontDatabase::Any).toInt());
    fontSettings.m_font = qvariant_cast<QFont>(value(u"UI/font"_s));
    fontSettings.m_useFont =
            settings()->value(u"UI/useFont"_s, QVariant(false)).toBool();
    return fontSettings;
}

QT_END_NAMESPACE
