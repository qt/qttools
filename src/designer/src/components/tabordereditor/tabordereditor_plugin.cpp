// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtGui/qaction.h>

#include "tabordereditor_plugin.h"
#include "tabordereditor_tool.h"

#include <iconloader_p.h>

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowmanager.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

TabOrderEditorPlugin::TabOrderEditorPlugin() = default;

TabOrderEditorPlugin::~TabOrderEditorPlugin() = default;

bool TabOrderEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void TabOrderEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Tab Order"), this);
    m_action->setObjectName(u"_qt_edit_tab_order_action"_s);
    m_action->setIcon(createIconSet("tabordertool.png"_L1));
    m_action->setEnabled(false);

    setParent(core);
    m_core = core;
    m_initialized = true;

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
            this, &TabOrderEditorPlugin::addFormWindow);

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowRemoved,
            this, &TabOrderEditorPlugin::removeFormWindow);

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
                this, &TabOrderEditorPlugin::activeFormWindowChanged);
}

void TabOrderEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    m_action->setEnabled(formWindow != nullptr);
}

QDesignerFormEditorInterface *TabOrderEditorPlugin::core() const
{
    return m_core;
}

void TabOrderEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != nullptr);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    TabOrderEditorTool *tool = new TabOrderEditorTool(formWindow, this);
    m_tools[formWindow] = tool;
    connect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
    formWindow->registerTool(tool);
}

void TabOrderEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != nullptr);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    TabOrderEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
    // ### FIXME disable the tool

    delete tool;
}

QAction *TabOrderEditorPlugin::action() const
{
    return m_action;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

#include "moc_tabordereditor_plugin.cpp"
