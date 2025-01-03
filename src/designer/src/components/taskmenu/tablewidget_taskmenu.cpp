// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tablewidget_taskmenu.h"
#include "tablewidgeteditor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtWidgets/qtablewidget.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

#include <QtGui/qaction.h>

#include <QtCore/qcoreevent.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

TableWidgetTaskMenu::TableWidgetTaskMenu(QTableWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_tableWidget(button),
      m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
    connect(m_editItemsAction, &QAction::triggered, this, &TableWidgetTaskMenu::editItems);
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}


TableWidgetTaskMenu::~TableWidgetTaskMenu() = default;

QAction *TableWidgetTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> TableWidgetTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TableWidgetTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_tableWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_tableWidget != nullptr);

    TableWidgetEditorDialog dlg(m_formWindow, m_tableWidget->window());
    TableWidgetContents oldCont = dlg.fillContentsFromTableWidget(m_tableWidget);
    if (dlg.exec() == QDialog::Accepted) {
        TableWidgetContents newCont = dlg.contents();
        if (newCont != oldCont) {
            ChangeTableContentsCommand *cmd = new ChangeTableContentsCommand(m_formWindow);
            cmd->init(m_tableWidget, oldCont, newCont);
            m_formWindow->commandHistory()->push(cmd);
        }
    }
}

void TableWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
