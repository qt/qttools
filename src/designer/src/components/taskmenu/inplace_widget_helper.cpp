// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "abstractformwindow.h"
#include "inplace_widget_helper.h"


#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtoolbutton.h>

#include <QtGui/qevent.h>
#include <QtGui/qshortcut.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    InPlaceWidgetHelper::InPlaceWidgetHelper(QWidget *editorWidget, QWidget *parentWidget, QDesignerFormWindowInterface *fw)
        : QObject(nullptr),
    m_editorWidget(editorWidget),
    m_parentWidget(parentWidget),
    m_noChildEvent(m_parentWidget->testAttribute(Qt::WA_NoChildEventsForParent))
    {
        m_editorWidget->setAttribute(Qt::WA_DeleteOnClose);
        m_editorWidget->setParent(m_parentWidget->window());
        m_parentWidget->installEventFilter(this);
        m_editorWidget->installEventFilter(this);
        connect(m_editorWidget, &QObject::destroyed,
                fw->mainContainer(), QOverload<>::of(&QWidget::setFocus));
    }

    InPlaceWidgetHelper::~InPlaceWidgetHelper()
    {
        if (m_parentWidget)
            m_parentWidget->setAttribute(Qt::WA_NoChildEventsForParent, m_noChildEvent);
    }

    Qt::Alignment InPlaceWidgetHelper::alignment() const {
         if (m_parentWidget->metaObject()->indexOfProperty("alignment") != -1)
             return Qt::Alignment(m_parentWidget->property("alignment").toInt());

         if (qobject_cast<const QPushButton *>(m_parentWidget)
             || qobject_cast<const QToolButton *>(m_parentWidget) /* tool needs to be more complex */)
             return Qt::AlignHCenter;

         return Qt::AlignJustify;
     }


    bool InPlaceWidgetHelper::eventFilter(QObject *object, QEvent *e)
    {
        if (object == m_parentWidget) {
            if (e->type() == QEvent::Resize) {
                const QResizeEvent *event = static_cast<const QResizeEvent*>(e);
                const QPoint localPos = m_parentWidget->geometry().topLeft();
                const QPoint globalPos = m_parentWidget->parentWidget() ? m_parentWidget->parentWidget()->mapToGlobal(localPos) : localPos;
                const QPoint newPos = (m_editorWidget->parentWidget() ? m_editorWidget->parentWidget()->mapFromGlobal(globalPos) : globalPos)
                    + m_posOffset;
                const QSize newSize = event->size() + m_sizeOffset;
                m_editorWidget->setGeometry(QRect(newPos, newSize));
            }
        } else if (object == m_editorWidget) {
            if (e->type() == QEvent::ShortcutOverride) {
                if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
                    e->accept();
                    return false;
                }
            } else if (e->type() == QEvent::KeyPress) {
                if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Escape) {
                    e->accept();
                    m_editorWidget->close();
                    return true;
                }
            } else if (e->type() == QEvent::Show) {
                const QPoint localPos = m_parentWidget->geometry().topLeft();
                const QPoint globalPos = m_parentWidget->parentWidget() ? m_parentWidget->parentWidget()->mapToGlobal(localPos) : localPos;
                const QPoint newPos = m_editorWidget->parentWidget() ? m_editorWidget->parentWidget()->mapFromGlobal(globalPos) : globalPos;
                m_posOffset = m_editorWidget->geometry().topLeft() - newPos;
                m_sizeOffset = m_editorWidget->size() - m_parentWidget->size();
            }
        }

        return QObject::eventFilter(object, e);
    }
}

QT_END_NAMESPACE
