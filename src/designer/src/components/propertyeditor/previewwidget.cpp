/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "previewwidget.h"

#include <QtWidgets/qaction.h>
#include <QtWidgets/qmenu.h>

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
