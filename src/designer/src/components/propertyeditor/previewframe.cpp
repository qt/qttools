// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "previewframe.h"
#include "previewwidget.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qmdiarea.h>
#include <QtWidgets/qmdisubwindow.h>
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

    class PreviewMdiArea: public QMdiArea {
    public:
        PreviewMdiArea(QWidget *parent = nullptr) : QMdiArea(parent) {}
    protected:
        bool viewportEvent(QEvent *event) override;
    };

    bool PreviewMdiArea::viewportEvent (QEvent * event) {
        if (event->type() != QEvent::Paint)
            return QMdiArea::viewportEvent (event);
        QWidget *paintWidget = viewport();
        QPainter p(paintWidget);
        p.fillRect(rect(), paintWidget->palette().color(backgroundRole()).darker());
        p.setPen(QPen(Qt::white));
        //: Palette editor background
        p.drawText(0, height() / 2,  width(), height(), Qt::AlignHCenter,
                   QCoreApplication::translate("qdesigner_internal::PreviewMdiArea", "The moose in the noose\nate the goose who was loose."));
        return true;
    }

PreviewFrame::PreviewFrame(QWidget *parent) :
    QFrame(parent),
    m_mdiArea(new PreviewMdiArea(this))
{
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(QMargins());
    vbox->addWidget(m_mdiArea);

    setMinimumSize(ensureMdiSubWindow()->minimumSizeHint());
}

void PreviewFrame::setPreviewPalette(const QPalette &pal)
{
    ensureMdiSubWindow()->setPalette(pal);
}

void PreviewFrame::setSubWindowActive(bool active)
{
    m_mdiArea->setActiveSubWindow (active ? ensureMdiSubWindow() : nullptr);
}

QMdiSubWindow *PreviewFrame::ensureMdiSubWindow()
{
    if (!m_mdiSubWindow) {
        PreviewWidget *previewWidget = new PreviewWidget(m_mdiArea);
        m_mdiSubWindow = m_mdiArea->addSubWindow(previewWidget, Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
        m_mdiSubWindow->move(10,10);
        m_mdiSubWindow->showMaximized();
    }

    const Qt::WindowStates state = m_mdiSubWindow->windowState();
    if (state & Qt::WindowMinimized)
        m_mdiSubWindow->setWindowState(state & ~Qt::WindowMinimized);

    return m_mdiSubWindow;
}
}

QT_END_NAMESPACE
