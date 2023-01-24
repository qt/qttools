// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tabordereditor_tool.h"
#include "tabordereditor.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtGui/qaction.h>

#include <QtCore/qcoreevent.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

TabOrderEditorTool::TabOrderEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent)
    : QDesignerFormWindowToolInterface(parent),
      m_formWindow(formWindow),
      m_action(new QAction(tr("Edit Tab Order"), this))
{
}

TabOrderEditorTool::~TabOrderEditorTool() = default;

QDesignerFormEditorInterface *TabOrderEditorTool::core() const
{
    return m_formWindow->core();
}

QDesignerFormWindowInterface *TabOrderEditorTool::formWindow() const
{
    return m_formWindow;
}

bool TabOrderEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);

    return event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease;
}

QWidget *TabOrderEditorTool::editor() const
{
    if (!m_editor) {
        Q_ASSERT(formWindow() != nullptr);
        m_editor = new TabOrderEditor(formWindow(), nullptr);
        connect(formWindow(), &QDesignerFormWindowInterface::mainContainerChanged,
                m_editor.data(), &TabOrderEditor::setBackground);
    }

    return m_editor;
}

void TabOrderEditorTool::activated()
{
    connect(formWindow(), &QDesignerFormWindowInterface::changed,
                m_editor.data(), &TabOrderEditor::updateBackground);
}

void TabOrderEditorTool::deactivated()
{
    disconnect(formWindow(), &QDesignerFormWindowInterface::changed,
                m_editor.data(), &TabOrderEditor::updateBackground);
}

QAction *TabOrderEditorTool::action() const
{
    return m_action;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
