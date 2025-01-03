// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
