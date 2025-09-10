// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "label_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

#include <QtGui/qaction.h>
#include <QtGui/qtextdocument.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto textPropertyC = "text"_L1;

namespace qdesigner_internal {

// -------- LabelTaskMenuInlineEditor
class LabelTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    LabelTaskMenuInlineEditor(QLabel *button, QObject *parent);

protected:
    QRect editRectangle() const override;
};

LabelTaskMenuInlineEditor::LabelTaskMenuInlineEditor(QLabel *w, QObject *parent) :
      TaskMenuInlineEditor(w, ValidationRichText, textPropertyC, parent)
{
}

QRect LabelTaskMenuInlineEditor::editRectangle() const
{
    QStyleOptionButton opt;
    opt.initFrom(widget());
    return opt.rect;
}

// --------------- LabelTaskMenu

LabelTaskMenu::LabelTaskMenu(QLabel *label, QObject *parent)
    : QDesignerTaskMenu(label, parent),
      m_label(label),
      m_editRichTextAction(new QAction(tr("Change rich text..."), this)),
      m_editPlainTextAction(new QAction(tr("Change plain text..."), this))
{
    LabelTaskMenuInlineEditor *editor = new LabelTaskMenuInlineEditor(label, this);
    connect(m_editPlainTextAction, &QAction::triggered, editor, &LabelTaskMenuInlineEditor::editText);
    m_taskActions.append(m_editPlainTextAction);

    connect(m_editRichTextAction, &QAction::triggered, this, &LabelTaskMenu::editRichText);
    m_taskActions.append(m_editRichTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

QAction *LabelTaskMenu::preferredEditAction() const
{
    if (m_label->textFormat () == Qt::PlainText) return m_editPlainTextAction;
    return Qt::mightBeRichText(m_label->text()) ? m_editRichTextAction : m_editPlainTextAction;
}

QList<QAction*> LabelTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void LabelTaskMenu::editRichText()
{
    changeTextProperty(textPropertyC, QString(), MultiSelectionMode, m_label->textFormat());
}

}
QT_END_NAMESPACE
