// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TREEWIDGET_TASKMENU_H
#define TREEWIDGET_TASKMENU_H

#include <QtWidgets/qtreewidget.h>
#include <QtCore/qpointer.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TreeWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent = nullptr);
    ~TreeWidgetTaskMenu() override;

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void editItems();
    void updateSelection();

private:
    QTreeWidget *m_treeWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

using TreeWidgetTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QTreeWidget, TreeWidgetTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TREEWIDGET_TASKMENU_H
