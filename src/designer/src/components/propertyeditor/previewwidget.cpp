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

#include "previewwidget.h"

#include <QtWidgets/qmenu.h>

#include <QtGui/qaction.h>

#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.treeWidget->expandAll();
    auto model = ui.treeWidget->model();
    ui.treeWidget->setCurrentIndex(model->index(0, 0, model->index(0, 0)));
    auto toolButtonMenu = new QMenu(ui.menuToolButton);
    toolButtonMenu->addAction(tr("Option 1"));
    toolButtonMenu->addSeparator();
    auto checkable = toolButtonMenu->addAction(tr("Checkable"));
    checkable->setCheckable(true);
    ui.menuToolButton->setMenu(toolButtonMenu);
    ui.menuToolButton->setPopupMode(QToolButton::InstantPopup);
}

PreviewWidget::~PreviewWidget() = default;


QT_END_NAMESPACE
