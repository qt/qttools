// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "view3d_plugin.h"
#include "view3d_tool.h"

#include <QtDesigner/qdesignerformeditorinterface.h>
#include <QtDesigner/qdesignerformwindowmanagerinterface.h>

#include <QtGui/qaction.h>

#include <QtCore/qdebug.h>
#include <QtCore/qtplugin.h>

QView3DPlugin::QView3DPlugin()
{
    m_core = 0;
    m_action = 0;
}

bool QView3DPlugin::isInitialized() const
{
    return m_core != 0;
}

void QView3DPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("3D View"), this);
    m_core = core;
    setParent(core);

    connect(core->formWindowManager(), SIGNAL(formWindowAdded(QDesignerFormWindowInterface*)),
            this, SLOT(addFormWindow(QDesignerFormWindowInterface*)));

    connect(core->formWindowManager(), SIGNAL(formWindowRemoved(QDesignerFormWindowInterface*)),
            this, SLOT(removeFormWindow(QDesignerFormWindowInterface*)));

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
                this, SLOT(activeFormWindowChanged(QDesignerFormWindowInterface*)));
}

QAction *QView3DPlugin::action() const
{
    return m_action;
}

QDesignerFormEditorInterface *QView3DPlugin::core() const
{
    return m_core;
}

void QView3DPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    m_action->setEnabled(formWindow != 0);
}

void QView3DPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tool_list.contains(formWindow) == false);

    QView3DTool *tool = new QView3DTool(formWindow, this);
    m_tool_list[formWindow] = tool;
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    formWindow->registerTool(tool);
}

void QView3DPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tool_list.contains(formWindow));

    QView3DTool *tool = m_tool_list.value(formWindow);
    m_tool_list.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));

    delete tool;
}
