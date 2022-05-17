// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "menutaskmenu.h"

#include <promotiontaskmenu_p.h>

#include <QtGui/qaction.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    // ------------ MenuTaskMenu
    MenuTaskMenu::MenuTaskMenu(QDesignerMenu *menu, QObject *parent) :
       QObject(parent),
       m_menu(menu),
       m_removeAction(new QAction(tr("Remove"), this)),
       m_promotionTaskMenu(new PromotionTaskMenu(menu, PromotionTaskMenu::ModeSingleWidget, this))
    {
        connect(m_removeAction, &QAction::triggered, this, &MenuTaskMenu::removeMenu);
    }

    QAction *MenuTaskMenu::preferredEditAction() const
    {
        return nullptr;
    }

    QList<QAction*> MenuTaskMenu::taskActions() const
    {
        QList<QAction*> rc;
        rc.push_back(m_removeAction);
        m_promotionTaskMenu->addActions(PromotionTaskMenu::LeadingSeparator, rc);
        return rc;
    }

    void MenuTaskMenu::removeMenu()
    {
        // Are we on a menu bar or on a menu?
        QWidget *pw = m_menu->parentWidget();
        if (QDesignerMenuBar *mb = qobject_cast<QDesignerMenuBar *>(pw)) {
            mb->deleteMenuAction(m_menu->menuAction());
            return;
        }
        if (QDesignerMenu *m = qobject_cast<QDesignerMenu *>(pw)) {
            m->deleteAction(m_menu->menuAction());
        }
    }

    // ------------- MenuBarTaskMenu
    MenuBarTaskMenu::MenuBarTaskMenu(QDesignerMenuBar *bar, QObject *parent) :
        QObject(parent),
        m_bar(bar)
    {
    }

    QAction *MenuBarTaskMenu::preferredEditAction() const
    {
        return nullptr;
    }

    QList<QAction*> MenuBarTaskMenu::taskActions() const
    {
        return m_bar->contextMenuActions();
    }
}

QT_END_NAMESPACE

