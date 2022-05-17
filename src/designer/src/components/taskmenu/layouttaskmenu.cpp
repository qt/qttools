// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "layouttaskmenu.h"
#include <formlayoutmenu_p.h>
#include <morphmenu_p.h>

#include <QtDesigner/abstractformwindow.h>

#include <QtGui/qaction.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

// ------------ LayoutWidgetTaskMenu
LayoutWidgetTaskMenu::LayoutWidgetTaskMenu(QLayoutWidget *lw, QObject *parent) :
   QObject(parent),
   m_widget(lw),
   m_morphMenu(new qdesigner_internal::MorphMenu(this)),
   m_formLayoutMenu(new qdesigner_internal::FormLayoutMenu(this))
{
}

QAction *LayoutWidgetTaskMenu::preferredEditAction() const
{
    return m_formLayoutMenu->preferredEditAction(m_widget, m_widget->formWindow());
}

QList<QAction*> LayoutWidgetTaskMenu::taskActions() const
{
    QList<QAction*> rc;
    QDesignerFormWindowInterface *fw = m_widget->formWindow();
    m_morphMenu->populate(m_widget, fw, rc);
    m_formLayoutMenu->populate(m_widget, fw, rc);
    return rc;
}

// ------------- SpacerTaskMenu
SpacerTaskMenu::SpacerTaskMenu(Spacer *, QObject *parent) :
    QObject(parent)
{
}

QAction *SpacerTaskMenu::preferredEditAction() const
{
    return nullptr;
}

QList<QAction*> SpacerTaskMenu::taskActions() const
{
    return {};
}

QT_END_NAMESPACE

