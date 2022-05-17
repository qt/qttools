// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WATERINGCONFIGDIALOG_H
#define WATERINGCONFIGDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_wateringconfigdialog.h"

class WateringConfigDialog : public QDialog
{
    Q_OBJECT
public:
    WateringConfigDialog();

private slots:
    void focusChanged(QWidget *old, QWidget *now);

private:
    Ui::WateringConfigDialog m_ui;
    QMap<QWidget*, QString> m_widgetInfo;
};

#endif
