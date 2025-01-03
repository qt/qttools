// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtGui/qaction.h>

#include "buddyeditor_plugin.h"
#include "buddyeditor_tool.h"

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/abstractformeditor.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

BuddyEditorPlugin::BuddyEditorPlugin() = default;

BuddyEditorPlugin::~BuddyEditorPlugin() = default;

bool BuddyEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void BuddyEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Buddies"), this);
    m_action->setObjectName(u"__qt_edit_buddies_action"_s);
    QIcon buddyIcon = QIcon::fromTheme(u"designer-edit-buddy"_s,
                                       QIcon(core->resourceLocation() + "/buddytool.png"_L1));
    m_action->setIcon(buddyIcon);
    m_action->setEnabled(false);

    setParent(core);
    m_core = core;
    m_initialized = true;

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowAdded,
            this, &BuddyEditorPlugin::addFormWindow);

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::formWindowRemoved,
            this, &BuddyEditorPlugin::removeFormWindow);

    connect(core->formWindowManager(), &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
                this, &BuddyEditorPlugin::activeFormWindowChanged);
}

QDesignerFormEditorInterface *BuddyEditorPlugin::core() const
{
    return m_core;
}

void BuddyEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != nullptr);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    BuddyEditorTool *tool = new BuddyEditorTool(formWindow, this);
    m_tools[formWindow] = tool;
    connect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
    formWindow->registerTool(tool);
}

void BuddyEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != nullptr);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    BuddyEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, &QAction::triggered, tool->action(), &QAction::trigger);
    // ### FIXME disable the tool

    delete tool;
}

QAction *BuddyEditorPlugin::action() const
{
    return m_action;
}

void BuddyEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    m_action->setEnabled(formWindow != nullptr);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
