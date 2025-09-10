// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TABLEWIDGET_TASKMENU_H
#define TABLEWIDGET_TASKMENU_H

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

#include <QtWidgets/qtablewidget.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TableWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit TableWidgetTaskMenu(QTableWidget *button, QObject *parent = nullptr);
    ~TableWidgetTaskMenu() override;

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void editItems();
    void updateSelection();

private:
    QTableWidget *m_tableWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

using TableWidgetTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QTableWidget, TableWidgetTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TABLEWIDGET_TASKMENU_H
