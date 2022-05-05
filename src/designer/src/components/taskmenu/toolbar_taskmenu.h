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

#ifndef TOOLBAR_TASKMENU_H
#define TOOLBAR_TASKMENU_H

#include <QtDesigner/taskmenu.h>

#include <extensionfactory_p.h>

#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qstatusbar.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    class PromotionTaskMenu;

// ToolBarTaskMenu forwards the actions of ToolBarEventFilter
class ToolBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit ToolBarTaskMenu(QToolBar *tb, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private:
    QToolBar *m_toolBar;
};

// StatusBarTaskMenu provides promotion and deletion
class StatusBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit StatusBarTaskMenu(QStatusBar *tb, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void removeStatusBar();

private:
    QStatusBar  *m_statusBar;
    QAction *m_removeAction;
    PromotionTaskMenu *m_promotionTaskMenu;
};

using ToolBarTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QToolBar, ToolBarTaskMenu>;
using StatusBarTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QStatusBar, StatusBarTaskMenu>;

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TOOLBAR_TASKMENU_H
