// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"

#include "mainwindow.h"
#include "searchwidget.h"

#include <QtCore/QMap>
#include <QtCore/QMimeData>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtWidgets/QMenu>
#include <QtWidgets/QLayout>
#include <QtGui/QKeyEvent>
#if QT_CONFIG(clipboard)
#include <QtGui/QClipboard>
#endif
#include <QtWidgets/QApplication>
#include <QtWidgets/QTextBrowser>

#include <QtHelp/QHelpSearchEngine>
#include <QtHelp/QHelpSearchQueryWidget>
#include <QtHelp/QHelpSearchResultWidget>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

SearchWidget::SearchWidget(QHelpSearchEngine *engine, QWidget *parent)
    : QWidget(parent)
    , zoomCount(0)
    , searchEngine(engine)
{
    TRACE_OBJ
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    resultWidget = searchEngine->resultWidget();
    QHelpSearchQueryWidget *queryWidget = searchEngine->queryWidget();

    vLayout->addWidget(queryWidget);
    vLayout->addWidget(resultWidget);

    setFocusProxy(queryWidget);

    connect(queryWidget, &QHelpSearchQueryWidget::search,
            this, &SearchWidget::search);
    connect(resultWidget, &QHelpSearchResultWidget::requestShowLink,
            this, &SearchWidget::requestShowLink);

    connect(searchEngine, &QHelpSearchEngine::searchingStarted,
            this, &SearchWidget::searchingStarted);
    connect(searchEngine, &QHelpSearchEngine::searchingFinished,
            this, &SearchWidget::searchingFinished);

    QTextBrowser* browser = resultWidget->findChild<QTextBrowser*>();
    if (browser)
        browser->viewport()->installEventFilter(this);
}

SearchWidget::~SearchWidget()
{
    TRACE_OBJ
    // nothing todo
}

void SearchWidget::zoomIn()
{
    TRACE_OBJ
    QTextBrowser* browser = resultWidget->findChild<QTextBrowser*>();
    if (browser && zoomCount != 10) {
        zoomCount++;
        browser->zoomIn();
    }
}

void SearchWidget::zoomOut()
{
    TRACE_OBJ
    QTextBrowser* browser = resultWidget->findChild<QTextBrowser*>();
    if (browser && zoomCount != -5) {
        zoomCount--;
        browser->zoomOut();
    }
}

void SearchWidget::resetZoom()
{
    TRACE_OBJ
    if (zoomCount == 0)
        return;

    QTextBrowser* browser = resultWidget->findChild<QTextBrowser*>();
    if (browser) {
        browser->zoomOut(zoomCount);
        zoomCount = 0;
    }
}

void SearchWidget::search() const
{
    TRACE_OBJ
    searchEngine->search(searchEngine->queryWidget()->searchInput());
}

void SearchWidget::searchingStarted()
{
    TRACE_OBJ
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
}

void SearchWidget::searchingFinished(int searchResultCount)
{
    TRACE_OBJ
    Q_UNUSED(searchResultCount);
    qApp->restoreOverrideCursor();
}

bool SearchWidget::eventFilter(QObject* o, QEvent *e)
{
    TRACE_OBJ
    QTextBrowser* browser = resultWidget->findChild<QTextBrowser*>();
    if (browser && o == browser->viewport()
        && e->type() == QEvent::MouseButtonRelease){
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        QUrl link = resultWidget->linkAt(me->pos());
        if (!link.isEmpty() || link.isValid()) {
            bool controlPressed = me->modifiers() & Qt::ControlModifier;
            if ((me->button() == Qt::LeftButton && controlPressed)
                || (me->button() == Qt::MiddleButton)) {
                    emit requestShowLinkInNewTab(link);
            }
        }
    }
    return QWidget::eventFilter(o,e);
}

void SearchWidget::keyPressEvent(QKeyEvent *keyEvent)
{
    TRACE_OBJ
    if (keyEvent->key() == Qt::Key_Escape)
        MainWindow::activateCurrentBrowser();
    else
        keyEvent->ignore();
}

void SearchWidget::contextMenuEvent(QContextMenuEvent *contextMenuEvent)
{
    TRACE_OBJ
    QMenu menu;
    QPoint point = contextMenuEvent->globalPos();

    QTextBrowser* browser = resultWidget->findChild<QTextBrowser*>();
    if (!browser)
        return;

    point = browser->mapFromGlobal(point);
    if (!browser->rect().contains(point, true))
        return;

    QUrl link = browser->anchorAt(point);

    QKeySequence keySeq;
#if QT_CONFIG(clipboard)
    keySeq = QKeySequence::Copy;
    QAction *copyAction = menu.addAction(tr("&Copy") + u'\t' +
        keySeq.toString(QKeySequence::NativeText));
    copyAction->setEnabled(QTextCursor(browser->textCursor()).hasSelection());

    QAction *copyAnchorAction = menu.addAction(tr("Copy &Link Location"));
    copyAnchorAction->setEnabled(!link.isEmpty() && link.isValid());
#endif

    keySeq = QKeySequence(Qt::CTRL);
    QAction *newTabAction = menu.addAction(tr("Open Link in New Tab") + u'\t'
                                           + keySeq.toString(QKeySequence::NativeText) + "LMB"_L1);
    newTabAction->setEnabled(!link.isEmpty() && link.isValid());

    menu.addSeparator();

    keySeq = QKeySequence::SelectAll;
    QAction *selectAllAction =
            menu.addAction(tr("Select All") + u'\t' + keySeq.toString(QKeySequence::NativeText));

    QAction *usedAction = menu.exec(mapToGlobal(contextMenuEvent->pos()));
#if QT_CONFIG(clipboard)
    if (usedAction == copyAction) {
        QTextCursor cursor = browser->textCursor();
        if (!cursor.isNull() && cursor.hasSelection()) {
            QString selectedText = cursor.selectedText();
            QMimeData *data = new QMimeData();
            data->setText(selectedText);
            QApplication::clipboard()->setMimeData(data);
        }
    }
    else if (usedAction == copyAnchorAction) {
        QApplication::clipboard()->setText(link.toString());
    }
    else
#endif
        if (usedAction == newTabAction) {
        emit requestShowLinkInNewTab(link);
    }
    else if (usedAction == selectAllAction) {
        browser->selectAll();
    }
}

QT_END_NAMESPACE
