// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tool_widgeteditor.h"
#include "formwindow.h"

// sdk
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractwidgetfactory.h>
#include <QtDesigner/abstractwidgetbox.h>

#include <layoutinfo_p.h>
#include <qdesigner_dnditem_p.h>
#include <qdesigner_resource.h>

#include <QtWidgets/qmainwindow.h>

#include <QtGui/qaction.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

WidgetEditorTool::WidgetEditorTool(FormWindow *formWindow)
    : QDesignerFormWindowToolInterface(formWindow),
      m_formWindow(formWindow),
      m_action(new QAction(tr("Edit Widgets"), this)),
      m_specialDockDrag(false)
{
}

QAction *WidgetEditorTool::action() const
{
    return m_action;
}

WidgetEditorTool::~WidgetEditorTool() = default;

QDesignerFormEditorInterface *WidgetEditorTool::core() const
{
    return m_formWindow->core();
}

QDesignerFormWindowInterface *WidgetEditorTool::formWindow() const
{
    return m_formWindow;
}

// separators in QMainWindow are no longer widgets
bool WidgetEditorTool::mainWindowSeparatorEvent(QWidget *widget, QEvent *event)
{
    QMainWindow *mw = qobject_cast<QMainWindow*>(widget);
    if (mw == nullptr)
        return false;

    if (event->type() != QEvent::MouseButtonPress
            && event->type() != QEvent::MouseMove
            && event->type() != QEvent::MouseButtonRelease)
        return false;

    QMouseEvent *e = static_cast<QMouseEvent*>(event);

    if (event->type() == QEvent::MouseButtonPress) {
        if (mw->isSeparator(e->position().toPoint())) {
            m_separator_drag_mw = mw;
            return true;
        }
        return false;
    }

    if (event->type() == QEvent::MouseMove)
        return m_separator_drag_mw == mw;

    if (event->type() == QEvent::MouseButtonRelease) {
        if (m_separator_drag_mw != mw)
            return false;
        m_separator_drag_mw = nullptr;
        return true;
    }

    return false;
}

bool WidgetEditorTool::isPassiveInteractor(QWidget *widget, QEvent *event)
{
    auto *widgetFactory = core()->widgetFactory();
    return widgetFactory->isPassiveInteractor(widget) || mainWindowSeparatorEvent(widget, event);
}

bool WidgetEditorTool::handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move:
        m_formWindow->updateSelection(widget);
        break;

    case QEvent::FocusOut:
    case QEvent::FocusIn: // Popup cancelled over a form widget: Reset its focus frame
        return widget != m_formWindow && widget != m_formWindow->mainContainer()
            && !isPassiveInteractor(widget, event);

    case QEvent::Wheel: // Prevent spinboxes and combos from reacting
        if (widget == m_formWindow->formContainer() || widget == m_formWindow
            || widget == m_formWindow->mainContainer()) { // Allow scrolling the form with wheel.
            return false;
        }
        return !isPassiveInteractor(widget, event);

    case QEvent::KeyPress:
        return !isPassiveInteractor(widget, event)
            && handleKeyPressEvent(widget, managedWidget, static_cast<QKeyEvent*>(event));

    case QEvent::KeyRelease:
        return !isPassiveInteractor(widget, event)
            && handleKeyReleaseEvent(widget, managedWidget, static_cast<QKeyEvent*>(event));

    case QEvent::MouseMove:
        return !isPassiveInteractor(widget, event)
            && handleMouseMoveEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonPress:
        return !isPassiveInteractor(widget, event)
            && handleMousePressEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonRelease:
        return !isPassiveInteractor(widget, event)
            && handleMouseReleaseEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::MouseButtonDblClick:
        return !isPassiveInteractor(widget, event)
            && handleMouseButtonDblClickEvent(widget, managedWidget, static_cast<QMouseEvent*>(event));

    case QEvent::ContextMenu:
        return !isPassiveInteractor(widget, event)
            && handleContextMenu(widget, managedWidget, static_cast<QContextMenuEvent*>(event));

    case QEvent::DragEnter:
        return handleDragEnterMoveEvent(widget, managedWidget, static_cast<QDragEnterEvent *>(event), true);
    case QEvent::DragMove:
        return handleDragEnterMoveEvent(widget, managedWidget, static_cast<QDragMoveEvent *>(event), false);
    case QEvent::DragLeave:
        return handleDragLeaveEvent(widget, managedWidget, static_cast<QDragLeaveEvent *>(event));
    case QEvent::Drop:
        return handleDropEvent(widget, managedWidget, static_cast<QDropEvent *>(event));
    default:
        break;

    } // end switch

    return false;
}

// ### remove me

bool WidgetEditorTool::handleContextMenu(QWidget *widget, QWidget *managedWidget, QContextMenuEvent *e)
{
    return m_formWindow->handleContextMenu(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMouseButtonDblClickEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMouseButtonDblClickEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMousePressEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMousePressEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMouseMoveEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMouseMoveEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleMouseReleaseEvent(QWidget *widget, QWidget *managedWidget, QMouseEvent *e)
{
    return m_formWindow->handleMouseReleaseEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleKeyPressEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e)
{
    return m_formWindow->handleKeyPressEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handleKeyReleaseEvent(QWidget *widget, QWidget *managedWidget, QKeyEvent *e)
{
    return m_formWindow->handleKeyReleaseEvent(widget, managedWidget, e);
}

bool WidgetEditorTool::handlePaintEvent(QWidget *widget, QWidget *managedWidget, QPaintEvent *e)
{
    Q_UNUSED(widget);
    Q_UNUSED(managedWidget);
    Q_UNUSED(e);

    return false;
}

void WidgetEditorTool::detectDockDrag(const QDesignerMimeData *mimeData)
{
    m_specialDockDrag = false;
    if (!mimeData)
        return;

    QMainWindow *mw = qobject_cast<QMainWindow*>(m_formWindow->mainContainer());
    if (!mw)
        return;

    const auto item_list = mimeData->items();

    for (QDesignerDnDItemInterface *item : item_list) {
        if (item->decoration() && item->decoration()->property("_q_dockDrag").toBool())
            m_specialDockDrag = true;

    }
}

bool WidgetEditorTool::handleDragEnterMoveEvent(QWidget *widget, QWidget * /*managedWidget*/, QDragMoveEvent *e, bool isEnter)
{
    const QDesignerMimeData *mimeData = qobject_cast<const QDesignerMimeData *>(e->mimeData());
    if (!mimeData)
        return false;

    if (!m_formWindow->hasFeature(QDesignerFormWindowInterface::EditFeature)) {
        e->ignore();
        return true;
    }

    if (isEnter)
        detectDockDrag(mimeData);


    QPoint globalPos = QPoint(0, 0);
    if (m_specialDockDrag) {
        m_lastDropTarget = nullptr;
        QMainWindow *mw = qobject_cast<QMainWindow*>(m_formWindow->mainContainer());
        if (mw)
            m_lastDropTarget = mw->centralWidget();
    } else {
        // If custom widgets have acceptDrops=true, the event occurs for them
        const QPoint formPos = widget != m_formWindow ? widget->mapTo(m_formWindow, e->position().toPoint()) : e->position().toPoint();
        globalPos = m_formWindow->mapToGlobal(formPos);
        const FormWindowBase::WidgetUnderMouseMode wum = mimeData->items().size() == 1 ? FormWindowBase::FindSingleSelectionDropTarget : FormWindowBase::FindMultiSelectionDropTarget;
        QWidget *dropTarget = m_formWindow->widgetUnderMouse(formPos, wum);
        if (m_lastDropTarget && dropTarget != m_lastDropTarget)
            m_formWindow->highlightWidget(m_lastDropTarget, m_lastDropTarget->mapFromGlobal(globalPos), FormWindow::Restore);
        m_lastDropTarget = dropTarget;
    }

    if (m_lastDropTarget)
        m_formWindow->highlightWidget(m_lastDropTarget, m_lastDropTarget->mapFromGlobal(globalPos), FormWindow::Highlight);

    if (isEnter || m_lastDropTarget)
        mimeData->acceptEvent(e);
    else
        e->ignore();
    return true;
}

bool WidgetEditorTool::handleDropEvent(QWidget *widget, QWidget *, QDropEvent *e)
{
    const QDesignerMimeData *mimeData = qobject_cast<const QDesignerMimeData *>(e->mimeData());
    if (!mimeData)
        return false;

    if (!m_lastDropTarget ||
        !m_formWindow->hasFeature(QDesignerFormWindowInterface::EditFeature)) {
        e->ignore();
        return true;
    }
    // FormWindow determines the position from the decoration.
    const QPoint globalPos = widget->mapToGlobal(e->position().toPoint());
    mimeData->moveDecoration(globalPos);
    if (m_specialDockDrag) {
        if (!m_formWindow->dropDockWidget(mimeData->items().at(0), globalPos)) {
            e->ignore();
            return true;
        }
    } else if (!m_formWindow->dropWidgets(mimeData->items(), m_lastDropTarget, globalPos)) {
        e->ignore();
        return true;
    }
    mimeData->acceptEvent(e);
    return true;
}

bool WidgetEditorTool::restoreDropHighlighting()
{
    if (!m_lastDropTarget)
        return false;

    m_formWindow->highlightWidget(m_lastDropTarget, m_lastDropTarget->mapFromGlobal(QCursor::pos()), FormWindow::Restore);
    m_lastDropTarget = nullptr;
    return true;
}

bool WidgetEditorTool::handleDragLeaveEvent(QWidget *, QWidget *, QDragLeaveEvent *event)
{
    if (restoreDropHighlighting()) {
        event->accept();
        return true;
    }
    return false;
}

QWidget *WidgetEditorTool::editor() const
{
    Q_ASSERT(formWindow() != nullptr);
    return formWindow()->mainContainer();
}

void WidgetEditorTool::activated()
{
    if (core()->widgetBox())
        core()->widgetBox()->setEnabled(true);

    if (m_formWindow == nullptr)
        return;

    const QWidgetList &sel = m_formWindow->selectedWidgets();
    for (QWidget *w : sel)
        m_formWindow->raiseSelection(w);
}

void WidgetEditorTool::deactivated()
{
    if (core()->widgetBox())
        core()->widgetBox()->setEnabled(false);

    if (m_formWindow == nullptr)
        return;

    m_formWindow->clearSelection();
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
