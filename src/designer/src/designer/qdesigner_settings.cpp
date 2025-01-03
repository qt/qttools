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

static const char newFormShowKey[] = "newFormDialog/ShowOnStartup";

// Change the version whenever the arrangement changes significantly.
static const char mainWindowStateKey[] = "MainWindowState45";
static const char toolBarsStateKey[] = "ToolBarsState45";

static const char backupOrgListKey[] = "backup/fileListOrg";
static const char backupBakListKey[] = "backup/fileListBak";
static const char recentFilesListKey[] = "recentFilesList";

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
    return settings()->value(QLatin1StringView(recentFilesListKey)).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    settings()->setValue(QLatin1StringView(recentFilesListKey), sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    settings()->setValue(QLatin1StringView(newFormShowKey), showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return settings()->value(QLatin1StringView(newFormShowKey), true).toBool();
}

QByteArray QDesignerSettings::mainWindowState(UIMode mode) const
{
    return settings()->value(QLatin1StringView(mainWindowStateKey) + modeChar(mode)).toByteArray();
}

void QDesignerSettings::setMainWindowState(UIMode mode, const QByteArray &mainWindowState)
{
    settings()->setValue(QLatin1StringView(mainWindowStateKey) + modeChar(mode), mainWindowState);
}

QByteArray QDesignerSettings::toolBarsState(UIMode mode) const
{
    QString key = QLatin1StringView(toolBarsStateKey);
    key += modeChar(mode);
    return settings()->value(key).toByteArray();
}

void QDesignerSettings::setToolBarsState(UIMode mode, const QByteArray &toolBarsState)
{
    QString key = QLatin1StringView(toolBarsStateKey);
    key += modeChar(mode);
    settings()->setValue(key, toolBarsState);
}

void QDesignerSettings::clearBackup()
{
    QDesignerSettingsInterface *s = settings();
    s->remove(QLatin1StringView(backupOrgListKey));
    s->remove(QLatin1StringView(backupBakListKey));
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
    const QStringList org = map.keys();
    const QStringList bak = map.values();

    QDesignerSettingsInterface *s = settings();
    s->setValue(QLatin1StringView(backupOrgListKey), org);
    s->setValue(QLatin1StringView(backupBakListKey), bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
    const QStringList org = settings()->value(QLatin1StringView(backupOrgListKey), QStringList()).toStringList();
    const QStringList bak = settings()->value(QLatin1StringView(backupBakListKey), QStringList()).toStringList();

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
#ifdef Q_OS_MACOS
    const UIMode defaultMode = TopLevelMode;
#else
    const UIMode defaultMode = DockedMode;
#endif
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
