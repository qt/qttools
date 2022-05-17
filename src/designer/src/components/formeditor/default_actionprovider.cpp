// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "default_actionprovider.h"
#include "invisible_widget_p.h"
#include "qdesigner_toolbar_p.h"

#include <QtWidgets/qapplication.h>

#include <QtGui/qaction.h>

#include <QtCore/qrect.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ------------ ActionProviderBase:
// Draws the drag indicator when dragging an action over a widget
// that receives action Dnd, such as ToolBar, Menu or MenuBar.
ActionProviderBase::ActionProviderBase(QWidget *widget) :
    m_indicator(new InvisibleWidget(widget))
{
    Q_ASSERT(widget != nullptr);

    m_indicator->setAutoFillBackground(true);
    m_indicator->setBackgroundRole(QPalette::Window);

    QPalette p;
    p.setColor(m_indicator->backgroundRole(), Qt::red);
    m_indicator->setPalette(p);
    m_indicator->hide();
}

enum { indicatorSize = 2 };

// Position an indicator horizontally over the rectangle, indicating
// 'Insert before' (left or right according to layout direction)
static inline QRect horizontalIndicatorRect(const QRect &rect, Qt::LayoutDirection layoutDirection)
{
    // Position right?
    QRect rc = QRect(rect.x(), 0, indicatorSize, rect.height() - 1);
    if (layoutDirection == Qt::RightToLeft)
        rc.moveLeft(rc.x() + rect.width() -  indicatorSize);
    return rc;
}

// Position an indicator vertically over the rectangle, indicating 'Insert before' (top)
static inline QRect verticalIndicatorRect(const QRect &rect)
{
    return QRect(0, rect.top(), rect.width() - 1, indicatorSize);
}

// Determine the geometry of the indicator by retrieving
// the action under mouse and positioning the bar within its geometry.
QRect ActionProviderBase::indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const
{
    QAction *action = actionAt(pos);
    if (!action)
        return QRect();
    QRect rc = actionGeometry(action);
    return orientation() == Qt::Horizontal ? horizontalIndicatorRect(rc, layoutDirection) : verticalIndicatorRect(rc);
}

// Adjust the indicator while dragging. (-1,1) is called to finish a DND operation
void ActionProviderBase::adjustIndicator(const QPoint &pos)
{
    if (pos == QPoint(-1, -1)) {
        m_indicator->hide();
        return;
    }
    const QRect ig = indicatorGeometry(pos, m_indicator->layoutDirection());
    if (ig.isValid()) {
        m_indicator->setGeometry(ig);
        QPalette p = m_indicator->palette();
        if (p.color(m_indicator->backgroundRole()) != Qt::red) {
            p.setColor(m_indicator->backgroundRole(), Qt::red);
            m_indicator->setPalette(p);
        }
        m_indicator->show();
        m_indicator->raise();
    } else {
        m_indicator->hide();
    }
}

// ------------- QToolBarActionProvider
QToolBarActionProvider::QToolBarActionProvider(QToolBar *widget, QObject *parent) :
    QObject(parent),
    ActionProviderBase(widget),
    m_widget(widget)
{
}

QRect QToolBarActionProvider::actionGeometry(QAction *action) const
{
     return m_widget->actionGeometry(action);
}

QAction *QToolBarActionProvider::actionAt(const QPoint &pos) const
{
     return ToolBarEventFilter::actionAt(m_widget, pos);
}

Qt::Orientation QToolBarActionProvider::orientation() const
{
    return m_widget->orientation();
}

QRect QToolBarActionProvider::indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const
{
    const QRect actionRect = ActionProviderBase::indicatorGeometry(pos, layoutDirection);
    if (actionRect.isValid())
        return actionRect;
    // Toolbar differs in that is has no dummy placeholder to 'insert before'
    // when intending to append. Check the free area.
    const QRect freeArea = ToolBarEventFilter::freeArea(m_widget);
    if (!freeArea.contains(pos))
        return QRect();
    return orientation() == Qt::Horizontal ? horizontalIndicatorRect(freeArea, layoutDirection) : verticalIndicatorRect(freeArea);
}

// ------------- QMenuBarActionProvider
QMenuBarActionProvider::QMenuBarActionProvider(QMenuBar *widget, QObject *parent) :
    QObject(parent),
    ActionProviderBase(widget),
    m_widget(widget)
{
}

QRect QMenuBarActionProvider::actionGeometry(QAction *action) const
{
     return m_widget->actionGeometry(action);
}

QAction *QMenuBarActionProvider::actionAt(const QPoint &pos) const
{
     return m_widget->actionAt(pos);
}

Qt::Orientation QMenuBarActionProvider::orientation() const
{
    return  Qt::Horizontal;
}

// ------------- QMenuActionProvider
QMenuActionProvider::QMenuActionProvider(QMenu *widget, QObject *parent) :
    QObject(parent),
    ActionProviderBase(widget),
    m_widget(widget)
{
}

QRect QMenuActionProvider::actionGeometry(QAction *action) const
{
     return m_widget->actionGeometry(action);
}

QAction *QMenuActionProvider::actionAt(const QPoint &pos) const
{
     return m_widget->actionAt(pos);
}

Qt::Orientation QMenuActionProvider::orientation() const
{
    return Qt::Vertical;
}
}

QT_END_NAMESPACE
