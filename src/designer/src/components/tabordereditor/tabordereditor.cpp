// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tabordereditor.h"

#include <metadatabase_p.h>
#include <qdesigner_command_p.h>
#include <qdesigner_utils_p.h>
#include <qlayout_widget_p.h>
#include <orderdialog_p.h>

#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformwindowcursor.h>
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractwidgetfactory.h>
#include <QtDesigner/propertysheet.h>

#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qapplication.h>

Q_DECLARE_METATYPE(QWidgetList)

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
    enum { VBOX_MARGIN = 1, HBOX_MARGIN = 4, BG_ALPHA = 32 };
}

static QRect fixRect(QRect r)
{
    return QRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

namespace qdesigner_internal {

TabOrderEditor::TabOrderEditor(QDesignerFormWindowInterface *form, QWidget *parent) :
    QWidget(parent),
    m_form_window(form),
    m_bg_widget(nullptr),
    m_undo_stack(form->commandHistory()),
    m_font_metrics(font()),
    m_current_index(0),
    m_beginning(true)
{
    connect(form, &QDesignerFormWindowInterface::widgetRemoved, this, &TabOrderEditor::widgetRemoved);

    QFont tabFont = font();
    tabFont.setPointSize(tabFont.pointSize()*2);
    tabFont.setBold(true);
    setFont(tabFont);
    m_font_metrics = QFontMetrics(tabFont);
    setAttribute(Qt::WA_MouseTracking, true);
}

QDesignerFormWindowInterface *TabOrderEditor::formWindow() const
{
    return m_form_window;
}

void TabOrderEditor::setBackground(QWidget *background)
{
    if (background == m_bg_widget) {
        return;
    }

    m_bg_widget = background;
    updateBackground();
}

void TabOrderEditor::updateBackground()
{
    if (m_bg_widget == nullptr) {
        // nothing to do
        return;
    }

    initTabOrder();
    update();
}

void TabOrderEditor::widgetRemoved(QWidget*)
{
    initTabOrder();
}

void TabOrderEditor::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    updateBackground();
}

QRect TabOrderEditor::indicatorRect(int index) const
{
    if (index < 0 || index >= m_tab_order_list.size())
        return QRect();

    const QWidget *w = m_tab_order_list.at(index);
    const QString text = QString::number(index + 1);

    const QPoint tl = mapFromGlobal(w->mapToGlobal(w->rect().topLeft()));
    const QSize size = m_font_metrics.size(Qt::TextSingleLine, text);
    QRect r(tl - QPoint(size.width(), size.height())/2, size);
    r = QRect(r.left() - HBOX_MARGIN, r.top() - VBOX_MARGIN,
                r.width() + HBOX_MARGIN*2, r.height() + VBOX_MARGIN*2);

    return r;
}

static bool isWidgetVisible(QWidget *widget)
{
    while (widget && widget->parentWidget()) {
        if (!widget->isVisibleTo(widget->parentWidget()))
            return false;

        widget = widget->parentWidget();
    }

    return true;
}

void TabOrderEditor::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRegion(e->region());

    int cur = m_current_index - 1;
    if (!m_beginning && cur < 0)
        cur = m_tab_order_list.size() - 1;

    for (qsizetype i = 0; i < m_tab_order_list.size(); ++i) {
        QWidget *widget = m_tab_order_list.at(i);
        if (!isWidgetVisible(widget))
            continue;

        const QRect r = indicatorRect(i);

        QColor c = Qt::darkGreen;
        if (i == cur)
            c = Qt::red;
        else if (i > cur)
            c = Qt::blue;
        p.setPen(c);
        c.setAlpha(BG_ALPHA);
        p.setBrush(c);
        p.drawRect(fixRect(r));

        p.setPen(Qt::white);
        p.drawText(r, QString::number(i + 1), QTextOption(Qt::AlignCenter));
    }
}

bool TabOrderEditor::skipWidget(QWidget *w) const
{
    if (qobject_cast<QLayoutWidget*>(w)
            || w == formWindow()->mainContainer()
            || w->isHidden())
        return true;

    if (!formWindow()->isManaged(w)) {
        return true;
    }

    QExtensionManager *ext = formWindow()->core()->extensionManager();
    if (const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(ext, w)) {
        const int index = sheet->indexOf(u"focusPolicy"_s);
        if (index != -1) {
            bool ok = false;
            Qt::FocusPolicy q = (Qt::FocusPolicy) Utils::valueOf(sheet->property(index), &ok);
            return !ok || !(q & Qt::TabFocus);
        }
    }

    return true;
}

void TabOrderEditor::initTabOrder()
{
    m_tab_order_list.clear();

    QDesignerFormEditorInterface *core = formWindow()->core();

    if (const QDesignerMetaDataBaseItemInterface *item = core->metaDataBase()->item(formWindow())) {
        m_tab_order_list = item->tabOrder();
    }

    // Remove any widgets that have been removed form the form
    for (qsizetype i = 0; i < m_tab_order_list.size(); ) {
        QWidget *w = m_tab_order_list.at(i);
        if (!formWindow()->mainContainer()->isAncestorOf(w) || skipWidget(w))
            m_tab_order_list.removeAt(i);
        else
            ++i;
    }

    // Append any widgets that are in the form but are not in the tab order
    QWidgetList childQueue;
    childQueue.append(formWindow()->mainContainer());
    while (!childQueue.isEmpty()) {
        QWidget *child = childQueue.takeFirst();
        childQueue += qvariant_cast<QWidgetList>(child->property("_q_widgetOrder"));

        if (skipWidget(child))
            continue;

        if (!m_tab_order_list.contains(child))
            m_tab_order_list.append(child);
    }

    // Just in case we missed some widgets
    QDesignerFormWindowCursorInterface *cursor = formWindow()->cursor();
    for (int i = 0; i < cursor->widgetCount(); ++i) {

        QWidget *widget = cursor->widget(i);
        if (skipWidget(widget))
            continue;

        if (!m_tab_order_list.contains(widget))
            m_tab_order_list.append(widget);
    }

    m_indicator_region = QRegion();
    for (qsizetype i = 0; i < m_tab_order_list.size(); ++i) {
        if (m_tab_order_list.at(i)->isVisible())
            m_indicator_region |= indicatorRect(i);
    }

    if (m_current_index >= m_tab_order_list.size())
        m_current_index = m_tab_order_list.size() - 1;
    if (m_current_index < 0)
        m_current_index = 0;
}

void TabOrderEditor::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
#if QT_CONFIG(cursor)
    if (m_indicator_region.contains(e->position().toPoint()))
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(QCursor());
#endif
}

int TabOrderEditor::widgetIndexAt(QPoint pos) const
{
    int target_index = -1;
    for (qsizetype i = 0; i < m_tab_order_list.size(); ++i) {
        if (!m_tab_order_list.at(i)->isVisible())
            continue;
        if (indicatorRect(i).contains(pos)) {
            target_index = i;
            break;
        }
    }

    return target_index;
}

void TabOrderEditor::mousePressEvent(QMouseEvent *e)
{
    e->accept();

    if (!m_indicator_region.contains(e->position().toPoint())) {
        if (QWidget *child = m_bg_widget->childAt(e->position().toPoint())) {
            QDesignerFormEditorInterface *core = m_form_window->core();
            if (core->widgetFactory()->isPassiveInteractor(child)) {

                QMouseEvent event(QEvent::MouseButtonPress,
                                  child->mapFromGlobal(e->globalPosition().toPoint()),
                                  e->globalPosition().toPoint(), e->button(), e->buttons(),
                                  e->modifiers());

                qApp->sendEvent(child, &event);

                QMouseEvent event2(QEvent::MouseButtonRelease,
                                   child->mapFromGlobal(e->globalPosition().toPoint()),
                                   e->globalPosition().toPoint(), e->button(), e->buttons(),
                                   e->modifiers());

                qApp->sendEvent(child, &event2);

                updateBackground();
            }
        }
        return;
    }

    if (e->button() != Qt::LeftButton)
        return;

    const int target_index = widgetIndexAt(e->position().toPoint());
    if (target_index == -1)
        return;

    m_beginning = false;

    if (e->modifiers() & Qt::ControlModifier) {
        m_current_index = target_index + 1;
        if (m_current_index >= m_tab_order_list.size())
            m_current_index = 0;
        update();
        return;
    }

    if (m_current_index == -1)
        return;

    m_tab_order_list.swapItemsAt(target_index, m_current_index);

    ++m_current_index;
    if (m_current_index == m_tab_order_list.size())
        m_current_index = 0;

    TabOrderCommand *cmd = new TabOrderCommand(formWindow());
    cmd->init(m_tab_order_list);
    formWindow()->commandHistory()->push(cmd);
}

void TabOrderEditor::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    const int target_index = widgetIndexAt(e->pos());
    QAction *setIndex = menu.addAction(tr("Start from Here"));
    setIndex->setEnabled(target_index >= 0);

    QAction *resetIndex = menu.addAction(tr("Restart"));
    menu.addSeparator();
    QAction *showDialog = menu.addAction(tr("Tab Order List..."));
    showDialog->setEnabled(m_tab_order_list.size() > 1);

    QAction *result = menu.exec(e->globalPos());
    if (result == resetIndex) {
        m_current_index = 0;
        m_beginning = true;
        update();
    } else if (result == setIndex) {
        m_beginning = false;
        m_current_index = target_index + 1;
        if (m_current_index >= m_tab_order_list.size())
            m_current_index = 0;
        update();
    } else if (result == showDialog) {
        showTabOrderDialog();
    }
}

void TabOrderEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    const int target_index = widgetIndexAt(e->position().toPoint());
    if (target_index >= 0)
        return;

    m_beginning = true;
    m_current_index = 0;
    update();
}

void TabOrderEditor::resizeEvent(QResizeEvent *e)
{
    updateBackground();
    QWidget::resizeEvent(e);
}

void TabOrderEditor::showTabOrderDialog()
{
    if (m_tab_order_list.size() < 2)
        return;
    OrderDialog dlg(this);
    dlg.setWindowTitle(tr("Tab Order List"));
    dlg.setDescription(tr("Tab Order"));
    dlg.setFormat(OrderDialog::TabOrderFormat);
    dlg.setPageList(m_tab_order_list);

    if (dlg.exec() == QDialog::Rejected)
        return;

    const QWidgetList newOrder = dlg.pageList();
    if (newOrder == m_tab_order_list)
        return;

    m_tab_order_list = newOrder;
    TabOrderCommand *cmd = new TabOrderCommand(formWindow());
    cmd->init(m_tab_order_list);
    formWindow()->commandHistory()->push(cmd);
    update();
}

}

QT_END_NAMESPACE
