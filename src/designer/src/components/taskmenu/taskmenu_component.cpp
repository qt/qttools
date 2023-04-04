// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "taskmenu_component.h"
#include "button_taskmenu.h"
#include "groupbox_taskmenu.h"
#include "label_taskmenu.h"
#include "lineedit_taskmenu.h"
#include "listwidget_taskmenu.h"
#include "treewidget_taskmenu.h"
#include "tablewidget_taskmenu.h"
#include "containerwidget_taskmenu.h"
#include "combobox_taskmenu.h"
#include "textedit_taskmenu.h"
#include "menutaskmenu.h"
#include "toolbar_taskmenu.h"
#include "layouttaskmenu.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/qextensionmanager.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

TaskMenuComponent::TaskMenuComponent(QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    Q_ASSERT(m_core != nullptr);

    QExtensionManager *mgr = core->extensionManager();
    const QString taskMenuId =  u"QDesignerInternalTaskMenuExtension"_s;

    ButtonTaskMenuFactory::registerExtension(mgr, taskMenuId);
    CommandLinkButtonTaskMenuFactory::registerExtension(mgr, taskMenuId); // Order!
    ButtonGroupTaskMenuFactory::registerExtension(mgr, taskMenuId);

    GroupBoxTaskMenuFactory::registerExtension(mgr, taskMenuId);
    LabelTaskMenuFactory::registerExtension(mgr, taskMenuId);
    LineEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
    ListWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    TreeWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    TableWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    TextEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
    PlainTextEditTaskMenuFactory::registerExtension(mgr, taskMenuId);
    MenuTaskMenuFactory::registerExtension(mgr, taskMenuId);
    MenuBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
    ToolBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
    StatusBarTaskMenuFactory::registerExtension(mgr, taskMenuId);
    LayoutWidgetTaskMenuFactory::registerExtension(mgr, taskMenuId);
    SpacerTaskMenuFactory::registerExtension(mgr, taskMenuId);

    mgr->registerExtensions(new ContainerWidgetTaskMenuFactory(core, mgr), taskMenuId);
    mgr->registerExtensions(new ComboBoxTaskMenuFactory(taskMenuId, mgr), taskMenuId);
}

TaskMenuComponent::~TaskMenuComponent() = default;

QDesignerFormEditorInterface *TaskMenuComponent::core() const
{
    return m_core;

}

} // namespace qdesigner_internal

QT_END_NAMESPACE

