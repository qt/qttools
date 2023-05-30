// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_dnditem_p.h"
#include "formwindowbase_p.h"
#include <QtDesigner/private/ui4_p.h>

#include <QtGui/qpainter.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qimage.h>
#include <QtWidgets/qlabel.h>
#include <QtGui/qdrag.h>
#include <QtGui/qcursor.h>
#include <QtGui/qevent.h>
#include <QtGui/qrgb.h>

#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDesignerDnDItem::QDesignerDnDItem(DropType type, QWidget *source) :
    m_source(source),
    m_type(type),
    m_dom_ui(nullptr),
    m_widget(nullptr),
    m_decoration(nullptr)
{
}

void QDesignerDnDItem::init(DomUI *ui, QWidget *widget, QWidget *decoration,
                                    const QPoint &global_mouse_pos)
{
    Q_ASSERT(widget != nullptr || ui != nullptr);
    Q_ASSERT(decoration != nullptr);

    m_dom_ui = ui;
    m_widget = widget;
    m_decoration = decoration;

    m_hot_spot = global_mouse_pos - m_decoration->geometry().topLeft();
}

QDesignerDnDItem::~QDesignerDnDItem()
{
    if (m_decoration != nullptr)
        m_decoration->deleteLater();
    delete m_dom_ui;
}

DomUI *QDesignerDnDItem::domUi() const
{
    return m_dom_ui;
}

QWidget *QDesignerDnDItem::decoration() const
{
    return m_decoration;
}

QPoint QDesignerDnDItem::hotSpot() const
{
    return m_hot_spot;
}

QWidget *QDesignerDnDItem::widget() const
{
    return m_widget;
}

QDesignerDnDItem::DropType QDesignerDnDItem::type() const
{
    return m_type;
}

QWidget *QDesignerDnDItem::source() const
{
    return m_source;
}

void QDesignerDnDItem::setDomUi(DomUI *dom_ui)
{
    delete m_dom_ui;
    m_dom_ui = dom_ui;
}

// ---------- QDesignerMimeData

// Make pixmap transparent on Windows only. Mac is transparent by default, Unix usually does not work.
#ifdef Q_OS_WIN
#  define TRANSPARENT_DRAG_PIXMAP
#endif

QDesignerMimeData::QDesignerMimeData(const QDesignerDnDItems &items, QDrag *drag) :
    m_items(items)
{
    enum { Alpha = 200 };
    QPoint decorationTopLeft;
    switch (m_items.size()) {
    case 0:
        break;
    case 1: {
        QWidget *deco = m_items.first()->decoration();
        decorationTopLeft = deco->pos();
        const QPixmap widgetPixmap = deco->grab(QRect(0, 0, -1, -1));
#ifdef TRANSPARENT_DRAG_PIXMAP
        QImage image(widgetPixmap.size(), QImage::Format_ARGB32);
        image.setDevicePixelRatio(widgetPixmap.devicePixelRatio());
        image.fill(QColor(Qt::transparent).rgba());
        QPainter painter(&image);
        painter.drawPixmap(QPoint(0, 0), widgetPixmap);
        painter.end();
        setImageTransparency(image, Alpha);
        drag->setPixmap(QPixmap::fromImage(image));
#else
        drag->setPixmap(widgetPixmap);
#endif
    }
        break;
    default: {
        // determine size of drag decoration by uniting all geometries
        const auto cend = m_items.cend();
        auto it = m_items.cbegin();
        QRect unitedGeometry = (*it)->decoration()->geometry();
        const qreal devicePixelRatio = (*it)->decoration()->devicePixelRatioF();
        for (++it; it != cend; ++it )
            unitedGeometry  = unitedGeometry .united((*it)->decoration()->geometry());

        // paint with offset. At the same time, create a mask bitmap, containing widget rectangles.
        const QSize imageSize = (QSizeF(unitedGeometry.size()) * devicePixelRatio).toSize();
        QImage image(imageSize, QImage::Format_ARGB32);
        image.setDevicePixelRatio(devicePixelRatio);
        image.fill(QColor(Qt::transparent).rgba());
        QBitmap mask(imageSize);
        mask.setDevicePixelRatio(devicePixelRatio);
        mask.clear();
        // paint with offset, determine action
        QPainter painter(&image);
        QPainter maskPainter(&mask);
        decorationTopLeft = unitedGeometry.topLeft();
        for (auto *item : std::as_const(m_items)) {
            QWidget *w = item->decoration();
            const QPixmap wp = w->grab(QRect(0, 0, -1, -1));
            const QPoint pos = w->pos() - decorationTopLeft;
            painter.drawPixmap(pos, wp);
            maskPainter.fillRect(QRect(pos, w->size()), Qt::color1);
        }
        painter.end();
        maskPainter.end();
#ifdef TRANSPARENT_DRAG_PIXMAP
        setImageTransparency(image, Alpha);
#endif
        QPixmap pixmap = QPixmap::fromImage(image);
        pixmap.setMask(mask);
        drag->setPixmap(pixmap);
    }
        break;
    }
    // determine hot spot and reconstruct the exact starting position as form window
    // introduces some offset when detecting DnD
    m_globalStartPos =  m_items.first()->decoration()->pos() +  m_items.first()->hotSpot();
    m_hotSpot = m_globalStartPos - decorationTopLeft;
    drag->setHotSpot(m_hotSpot);

    drag->setMimeData(this);
}

QDesignerMimeData::~QDesignerMimeData()
{
    qDeleteAll(m_items);
}

Qt::DropAction QDesignerMimeData::proposedDropAction() const
{
   return m_items.first()->type() == QDesignerDnDItemInterface::CopyDrop ? Qt::CopyAction : Qt::MoveAction;
}

Qt::DropAction QDesignerMimeData::execDrag(const QDesignerDnDItems &items, QWidget * dragSource)
{
    if (items.isEmpty())
        return Qt::IgnoreAction;

    QDrag *drag = new QDrag(dragSource);
    QDesignerMimeData *mimeData = new QDesignerMimeData(items, drag);

    // Store pointers to widgets that are to be re-shown if a move operation is canceled
    QWidgetList reshowWidgets;
    for (auto *item : items) {
        if (QWidget *w = item->widget()) {
            if (item->type() == QDesignerDnDItemInterface::MoveDrop)
                reshowWidgets.push_back(w);
        }
    }

    const Qt::DropAction executedAction = drag->exec(Qt::CopyAction|Qt::MoveAction, mimeData->proposedDropAction());

    if (executedAction == Qt::IgnoreAction) {
        for (QWidget *w : std::as_const(reshowWidgets))
            w->show();
    }

    return executedAction;
}


void QDesignerMimeData::moveDecoration(const QPoint &globalPos) const
{
    const QPoint relativeDistance = globalPos - m_globalStartPos;
    for (auto *item : m_items) {
        QWidget *w = item->decoration();
        w->move(w->pos() + relativeDistance);
    }
}

void QDesignerMimeData::removeMovedWidgetsFromSourceForm(const QDesignerDnDItems &items)
{
    QMultiMap<FormWindowBase *, QWidget *> formWidgetMap;
    // Find moved widgets per form
    for (auto *item : items) {
        if (item->type() == QDesignerDnDItemInterface::MoveDrop) {
            if (QWidget *w = item->widget()) {
                if (FormWindowBase *fb = qobject_cast<FormWindowBase *>(item->source()))
                    formWidgetMap.insert(fb, w);
            }
        }
    }

    const auto &formWindows = formWidgetMap.uniqueKeys();
    for (FormWindowBase *fb : formWindows)
        fb->deleteWidgetList(formWidgetMap.values(fb));
}

void QDesignerMimeData::acceptEventWithAction(Qt::DropAction desiredAction, QDropEvent *e)
{
    if (e->proposedAction() == desiredAction) {
        e->acceptProposedAction();
    } else {
        e->setDropAction(desiredAction);
        e->accept();
    }
}

void QDesignerMimeData::acceptEvent(QDropEvent *e) const
{
    acceptEventWithAction(proposedDropAction(), e);
}

void QDesignerMimeData::setImageTransparency(QImage &image, int alpha)
{
    const int height = image.height();
    for (int l = 0; l < height; l++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(l));
        QRgb *lineEnd = line + image.width();
        for ( ; line < lineEnd; line++) {
            const QRgb rgba = *line;
            *line = qRgba(qRed(rgba), qGreen(rgba), qBlue(rgba), alpha);
        }
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
