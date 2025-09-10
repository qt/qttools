// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "previewwidget.h"

#include <QtWidgets/qmenu.h>

#include <QtGui/qaction.h>

#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

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

} // namespace qdesigner_internal

QT_END_NAMESPACE
