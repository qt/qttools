// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef HELPDOCSETTINGSWIDGET_H
#define HELPDOCSETTINGSWIDGET_H

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class HelpDocSettings;
class HelpDocSettingsWidgetPrivate;

class HelpDocSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    HelpDocSettingsWidget(QWidget *parent = nullptr);

    ~HelpDocSettingsWidget();

    void setDocSettings(const HelpDocSettings &settings);
    HelpDocSettings docSettings() const;

Q_SIGNALS:
    void docSettingsChanged(const HelpDocSettings &settings);

private:
    QScopedPointer<class HelpDocSettingsWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(HelpDocSettingsWidget)
    Q_DISABLE_COPY_MOVE(HelpDocSettingsWidget)
};

QT_END_NAMESPACE

#endif

