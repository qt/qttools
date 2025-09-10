// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of Qt Designer. This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

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

