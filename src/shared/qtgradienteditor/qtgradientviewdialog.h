/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef GRADIENTVIEWDIALOG_H
#define GRADIENTVIEWDIALOG_H

#include <QtWidgets/QWidget>
#include <QtCore/QMap>
#include "ui_qtgradientviewdialog.h"

QT_BEGIN_NAMESPACE

class QtGradientManager;

class QtGradientViewDialog : public QDialog
{
    Q_OBJECT
public:
    QtGradientViewDialog(QWidget *parent = 0);

    void setGradientManager(QtGradientManager *manager);
    QtGradientManager *gradientManager() const;

    static QGradient getGradient(bool *ok, QtGradientManager *manager, QWidget *parent = 0, const QString &caption = tr("Select Gradient", 0));

private slots:
    void slotGradientSelected(const QString &id);
    void slotGradientActivated(const QString &id);

private:
    Ui::QtGradientViewDialog m_ui;
};

QT_END_NAMESPACE

#endif

