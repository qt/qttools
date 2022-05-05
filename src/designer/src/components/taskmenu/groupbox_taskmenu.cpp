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

#include "groupbox_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// -------- GroupBoxTaskMenuInlineEditor
class GroupBoxTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    GroupBoxTaskMenuInlineEditor(QGroupBox *button, QObject *parent);

protected:
    QRect editRectangle() const override;
};

GroupBoxTaskMenuInlineEditor::GroupBoxTaskMenuInlineEditor(QGroupBox *w, QObject *parent) :
      TaskMenuInlineEditor(w, ValidationSingleLine, QStringLiteral("title"), parent)
{
}

QRect GroupBoxTaskMenuInlineEditor::editRectangle() const
{
    QWidget *w = widget();
    QStyleOption opt; // ## QStyleOptionGroupBox
    opt.initFrom(w);
    return QRect(QPoint(), QSize(w->width(),20));
}

// --------------- GroupBoxTaskMenu

GroupBoxTaskMenu::GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent)
    : QDesignerTaskMenu(groupbox, parent),
      m_editTitleAction(new QAction(tr("Change title..."), this))

{
    TaskMenuInlineEditor *editor = new GroupBoxTaskMenuInlineEditor(groupbox, this);
    connect(m_editTitleAction, &QAction::triggered, editor, &TaskMenuInlineEditor::editText);
    m_taskActions.append(m_editTitleAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

QList<QAction*> GroupBoxTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

QAction *GroupBoxTaskMenu::preferredEditAction() const
{
    return m_editTitleAction;
}

}
QT_END_NAMESPACE
