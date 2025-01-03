// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmainwindow_container.h"
#include "qdesigner_toolbar_p.h"
#include "formwindow.h"

#include <QtCore/qdebug.h>

#include <QtWidgets/qlayout.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qdockwidget.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QMainWindowContainer::QMainWindowContainer(QMainWindow *widget, QObject *parent)
    : QObject(parent),
      m_mainWindow(widget)
{
}

int QMainWindowContainer::count() const
{
    return m_widgets.size();
}

QWidget *QMainWindowContainer::widget(int index) const
{
    return m_widgets.value(index, nullptr);
}

int QMainWindowContainer::currentIndex() const
{
    // QTBUG-111603, handle plugins with unmanaged central widgets
    auto *cw = m_mainWindow->centralWidget();
    return cw != nullptr && m_widgets.contains(cw) ? 0 : -1;
}

void QMainWindowContainer::setCurrentIndex(int index)
{
    Q_UNUSED(index);
}


namespace {
    // Pair of <area,break_before>
    using ToolBarData = QPair<Qt::ToolBarArea,bool> ;

    ToolBarData toolBarData(QToolBar *me) {
        const QMainWindow *mw = qobject_cast<const QMainWindow*>(me->parentWidget());
        if (!mw || !mw->layout() ||  mw->layout()->indexOf(me) == -1) {
            const QVariant desiredAreaV = me->property("_q_desiredArea");
            const Qt::ToolBarArea desiredArea = desiredAreaV.canConvert<Qt::ToolBarArea>()
                ? desiredAreaV.value<Qt::ToolBarArea>() : Qt::TopToolBarArea;
            return {desiredArea, false};
        }
        return ToolBarData(mw->toolBarArea(me), mw->toolBarBreak(me));
    }

Qt::DockWidgetArea dockWidgetArea(QDockWidget *me)
{
    if (const QMainWindow *mw = qobject_cast<const QMainWindow*>(me->parentWidget())) {
        // Make sure that me is actually managed by mw, otherwise
        // QMainWindow::dockWidgetArea() will be VERY upset
        QList<QLayout*> candidates;
        if (mw->layout()) {
            candidates.append(mw->layout());
            candidates += mw->layout()->findChildren<QLayout*>();
        }
        for (QLayout *l : std::as_const(candidates)) {
            if (l->indexOf(me) != -1)
                return mw->dockWidgetArea(me);
        }
    }
    return Qt::LeftDockWidgetArea;
}
}

// In QMainWindowContainer::remove(), remember the dock area in a dynamic
// property so that it can used in addWidget() if that is called by undo().
static const char dockAreaPropertyName[] = "_q_dockArea";

void QMainWindowContainer::addWidget(QWidget *widget)
{
    // remove all the occurrences of widget
    m_widgets.removeAll(widget);

    // the
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        m_widgets.append(widget);
        const ToolBarData data = toolBarData(toolBar);
        m_mainWindow->addToolBar(data.first, toolBar);
        if (data.second) m_mainWindow->insertToolBarBreak(toolBar);
        toolBar->show();
    }

    else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
        if (menuBar != m_mainWindow->menuBar())
            m_mainWindow->setMenuBar(menuBar);

        m_widgets.append(widget);
        menuBar->show();
    }

    else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
        if (statusBar != m_mainWindow->statusBar())
            m_mainWindow->setStatusBar(statusBar);

        m_widgets.append(widget);
        statusBar->show();
    }

    else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(widget)) {
        m_widgets.append(widget);

        Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;
        const auto areaProperty = widget->property(dockAreaPropertyName);
        if (areaProperty.canConvert<Qt::DockWidgetArea>()) {
            area = areaProperty.value<Qt::DockWidgetArea>();
            widget->setProperty(dockAreaPropertyName, {});
        } else {
            area = dockWidgetArea(dockWidget);
        }

        m_mainWindow->addDockWidget(area, dockWidget);
        dockWidget->show();

        if (FormWindow *fw = FormWindow::findFormWindow(m_mainWindow)) {
            fw->manageWidget(widget);
        }
    }

    else if (widget) {
        m_widgets.prepend(widget);

        if (widget != m_mainWindow->centralWidget()) {
            // note that qmainwindow will delete the current central widget if you
            // call setCentralWidget(), we end up with dangeling pointers in m_widgets list
            m_widgets.removeAll(m_mainWindow->centralWidget());

            widget->setParent(m_mainWindow);
            m_mainWindow->setCentralWidget(widget);
        }
    }
}

void QMainWindowContainer::insertWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);

    addWidget(widget);
}

void QMainWindowContainer::remove(int index)
{
    QWidget *widget = m_widgets.at(index);
    if (QToolBar *toolBar = qobject_cast<QToolBar*>(widget)) {
        m_mainWindow->removeToolBar(toolBar);
    } else if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(widget)) {
        menuBar->hide();
        menuBar->setParent(nullptr);
        m_mainWindow->setMenuBar(nullptr);
    } else if (QStatusBar *statusBar = qobject_cast<QStatusBar*>(widget)) {
        statusBar->hide();
        statusBar->setParent(nullptr);
        m_mainWindow->setStatusBar(nullptr);
    } else if (QDockWidget *dockWidget = qobject_cast<QDockWidget*>(widget)) {
        const auto area = m_mainWindow->dockWidgetArea(dockWidget);
        dockWidget->setProperty(dockAreaPropertyName, QVariant::fromValue(area));
        m_mainWindow->removeDockWidget(dockWidget);
    }
    m_widgets.removeAt(index);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
