/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QHELPFILTERSETTINGSWIDGET_H
#define QHELPFILTERSETTINGSWIDGET_H

#include <QtHelp/qhelp_global.h>

#include <QtWidgets/qwidget.h>

QT_BEGIN_NAMESPACE

class QVersionNumber;

class QHelpFilterEngine;
class QHelpFilterSettingsWidgetPrivate;

class QHELP_EXPORT QHelpFilterSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QHelpFilterSettingsWidget(QWidget *parent = nullptr);

    ~QHelpFilterSettingsWidget();

    void setAvailableComponents(const QStringList &components);
    void setAvailableVersions(const QList<QVersionNumber> &versions);

    // TODO: filterEngine may be moved to c'tor or to setFilterEngine() setter
    void readSettings(const QHelpFilterEngine *filterEngine);
    bool applySettings(QHelpFilterEngine *filterEngine) const;

private:
    QScopedPointer<class QHelpFilterSettingsWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QHelpFilterSettingsWidget)
    Q_DISABLE_COPY_MOVE(QHelpFilterSettingsWidget)
};

QT_END_NAMESPACE

#endif // QHELPFILTERSETTINGSWIDGET_H
