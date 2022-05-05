/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include "designer_enums.h"
#include <shared_settings_p.h>
#include <QtCore/qmap.h>
#include <QtCore/qrect.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerSettingsInterface;
struct ToolWindowFontSettings;

class QDesignerSettings : public qdesigner_internal::QDesignerSharedSettings
{
public:
    QDesignerSettings(QDesignerFormEditorInterface *core);

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    void restoreGeometry(QWidget *w, QRect fallBack = QRect()) const;
    void saveGeometryFor(const QWidget *w);

    QStringList recentFilesList() const;
    void setRecentFilesList(const QStringList &list);

    void setShowNewFormOnStartup(bool showIt);
    bool showNewFormOnStartup() const;

    void setUiMode(UIMode mode);
    UIMode uiMode() const;

    void setToolWindowFont(const ToolWindowFontSettings &fontSettings);
    ToolWindowFontSettings toolWindowFont() const;

    QByteArray mainWindowState(UIMode mode) const;
    void setMainWindowState(UIMode mode, const QByteArray &mainWindowState);

    QByteArray toolBarsState(UIMode mode) const;
    void setToolBarsState(UIMode mode, const QByteArray &mainWindowState);

    void clearBackup();
    void setBackup(const QMap<QString, QString> &map);
    QMap<QString, QString> backup() const;
};

QT_END_NAMESPACE

#endif // QDESIGNER_SETTINGS_H
