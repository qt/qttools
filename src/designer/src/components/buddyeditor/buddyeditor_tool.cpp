// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "buddyeditor_tool.h"
#include "buddyeditor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

BuddyEditorTool::BuddyEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
    : QDesignerFormWindowToolInterface(parent),
      m_formWindow(formWindow),
      m_action(new QAction(tr("Edit Buddies"), this))
{
}

BuddyEditorTool::~BuddyEditorTool() = default;

QDesignerFormEditorInterface *BuddyEditorTool::core() const
{
    return m_formWindow->core();
}

QDesignerFormWindowInterface *BuddyEditorTool::formWindow() const
{
    return m_formWindow;
}

bool BuddyEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);
    Q_UNUSED(event);

    return false;
}

QWidget *BuddyEditorTool::editor() const
{
    if (!m_editor) {
        Q_ASSERT(formWindow() != nullptr);
        m_editor = new BuddyEditor(formWindow(), nullptr);
        connect(formWindow(), &QDesignerFormWindowInterface::mainContainerChanged,
                m_editor.data(), &BuddyEditor::setBackground);
        connect(formWindow(), &QDesignerFormWindowInterface::changed,
                m_editor.data(), &BuddyEditor::updateBackground);
    }

    return m_editor;
}

void BuddyEditorTool::activated()
{
    m_editor->enableUpdateBackground(true);
}

void BuddyEditorTool::deactivated()
{
    m_editor->enableUpdateBackground(false);
}

QAction *BuddyEditorTool::action() const
{
    return m_action;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
