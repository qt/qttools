// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lineedit_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

// -------- LineEditTaskMenuInlineEditor
class LineEditTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    LineEditTaskMenuInlineEditor(QLineEdit *button, QObject *parent);

protected:
    QRect editRectangle() const override;
};

LineEditTaskMenuInlineEditor::LineEditTaskMenuInlineEditor(QLineEdit *w, QObject *parent) :
      TaskMenuInlineEditor(w, ValidationSingleLine, u"text"_s, parent)
{
}

QRect LineEditTaskMenuInlineEditor::editRectangle() const
{
    QStyleOption opt;
    opt.initFrom(widget());
    return opt.rect;
}

// --------------- LineEditTaskMenu
LineEditTaskMenu::LineEditTaskMenu(QLineEdit *lineEdit, QObject *parent) :
    QDesignerTaskMenu(lineEdit, parent),
    m_editTextAction(new QAction(tr("Change text..."), this))
{
    TaskMenuInlineEditor *editor = new LineEditTaskMenuInlineEditor(lineEdit, this);
    connect(m_editTextAction, &QAction::triggered, editor, &LineEditTaskMenuInlineEditor::editText);
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

QAction *LineEditTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> LineEditTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

}

QT_END_NAMESPACE
