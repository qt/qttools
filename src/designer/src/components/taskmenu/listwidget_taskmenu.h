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

#ifndef LISTWIDGET_TASKMENU_H
#define LISTWIDGET_TASKMENU_H

#include <QtWidgets/qlistwidget.h>
#include <QtCore/qpointer.h>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ListWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit ListWidgetTaskMenu(QListWidget *button, QObject *parent = nullptr);
    ~ListWidgetTaskMenu() override;

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void editItems();
    void updateSelection();

private:
    QListWidget *m_listWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

using ListWidgetTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QListWidget, ListWidgetTaskMenu>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LISTWIDGET_TASKMENU_H
