// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "signalsloteditor_plugin.h"
#include "signalsloteditor_tool.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

SignalSlotEditorPlugin::SignalSlotEditorPlugin() = default;

SignalSlotEditorPlugin::~SignalSlotEditorPlugin() = default;

bool SignalSlotEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void SignalSlotEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Signals/Slots"), this);
    m_action->setObjectName(u"__qt_edit_signals_slots_action"_s);
    m_action->setShortcut(tr("F4"));
    QIcon icon = QIcon::fromTheme(u"designer-edit-signals"_s,
                                  QIcon(core->resourceLocation() + "/signalslottool.png"_L1));
    m_action->setIcon(icon);
    m_action->setEnabled(false);

    setParent(core);
    m_core = core;
    m_initialized = true;

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
            this, &SignalSlotEditorPlugin::addFormWindow);

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowRemoved,
            this, &SignalSlotEditorPlugin::removeFormWindow);

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
            this, &SignalSlotEditorPlugin::activeFormWindowChanged);
}

QDesignerFormEditorInterface *SignalSlotEditorPlugin::core() const
{
    return m_core;
}

void SignalSlotEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != nullptr);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    SignalSlotEditorTool *tool = new SignalSlotEditorTool(formWindow, this);
    connect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
    m_tools[formWindow] = tool;
    formWindow->registerTool(tool);
}

void SignalSlotEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != nullptr);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    SignalSlotEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
    // ### FIXME disable the tool

    delete tool;
}

QAction *SignalSlotEditorPlugin::action() const
{
    return m_action;
}

void SignalSlotEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    m_action->setEnabled(formWindow != nullptr);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

#include "moc_signalsloteditor_plugin.cpp"
