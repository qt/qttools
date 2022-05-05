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

#ifndef LAYOUTTASKMENU_H
#define LAYOUTTASKMENU_H

#include <QtDesigner/taskmenu.h>

#include <qlayout_widget_p.h>
#include <spacer_widget_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    class FormLayoutMenu;
    class MorphMenu;
}

// Morph menu for QLayoutWidget.
class LayoutWidgetTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit LayoutWidgetTaskMenu(QLayoutWidget *w, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

private:
    QLayoutWidget *m_widget;
    qdesigner_internal::MorphMenu *m_morphMenu;
    qdesigner_internal::FormLayoutMenu *m_formLayoutMenu;
};

// Empty task menu for spacers.
class SpacerTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit SpacerTaskMenu(Spacer *bar, QObject *parent = nullptr);

    QAction *preferredEditAction() const override;
    QList<QAction*> taskActions() const override;

};

using LayoutWidgetTaskMenuFactory = qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, QLayoutWidget, LayoutWidgetTaskMenu>;
using SpacerTaskMenuFactory = qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, Spacer, SpacerTaskMenu>;

QT_END_NAMESPACE

#endif // LAYOUTTASKMENU_H
