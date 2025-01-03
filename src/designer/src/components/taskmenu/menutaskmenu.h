// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MENUTASKMENU_H
#define MENUTASKMENU_H

#include <QtDesigner/taskmenu.h>

#include <qdesigner_menu_p.h>
#include <qdesigner_menubar_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

    class PromotionTaskMenu;

// The QMenu task menu provides promotion and a remove option. The actual
// menu context options are not forwarded since they make only sense
// when a menu is being edited/visible.

class MenuTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit MenuTaskMenu(QDesignerMenu *menu, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private slots:
    void removeMenu();

private:
    QDesignerMenu *m_menu;
    QAction *m_removeAction;
    PromotionTaskMenu *m_promotionTaskMenu;
};

// The QMenuBar task menu forwards the actions of QDesignerMenuBar,
// making them available in the object inspector.

class MenuBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit MenuBarTaskMenu(QDesignerMenuBar *bar, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private:
    QDesignerMenuBar *m_bar;
};

using MenuTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QDesignerMenu, MenuTaskMenu>;
using MenuBarTaskMenuFactory = ExtensionFactory<QDesignerTaskMenuExtension, QDesignerMenuBar, MenuBarTaskMenu>;

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // MENUTASKMENU_H
