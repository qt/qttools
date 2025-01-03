// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
