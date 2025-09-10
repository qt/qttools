// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "groupbox_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

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
      TaskMenuInlineEditor(w, ValidationSingleLine, u"title"_s, parent)
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
