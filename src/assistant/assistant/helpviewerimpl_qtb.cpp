// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpviewerimpl.h"
#include "helpviewerimpl_p.h"

#include "globalactions.h"
#include "helpenginewrapper.h"
#include "openpagesmanager.h"
#include "tracer.h"

#include <QtCore/QStringBuilder>

#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QMenu>
#include <QtWidgets/QScrollBar>
#if QT_CONFIG(clipboard)
#include <QtGui/QClipboard>
#endif
#include <QtWidgets/QApplication>

QT_BEGIN_NAMESPACE

HelpViewerImpl::HelpViewerImpl(qreal zoom, QWidget *parent)
    : QTextBrowser(parent)
    , d(new HelpViewerImplPrivate(zoom))
{
    TRACE_OBJ
    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight,
        p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
        p.color(QPalette::Active, QPalette::HighlightedText));
    setPalette(p);

    installEventFilter(this);
    document()->setDocumentMargin(8);

    QFont font = viewerFont();
    font.setPointSize(int(font.pointSize() + zoom));
    setViewerFont(font);

    connect(this, &QTextBrowser::sourceChanged, this, &HelpViewerImpl::titleChanged);
    connect(this, &HelpViewerImpl::loadFinished, this, &HelpViewerImpl::setLoadFinished);
}

QFont HelpViewerImpl::viewerFont() const
{
    TRACE_OBJ
    if (HelpEngineWrapper::instance().usesBrowserFont())
        return HelpEngineWrapper::instance().browserFont();
    return qApp->font();
}

void HelpViewerImpl::setViewerFont(const QFont &newFont)
{
    TRACE_OBJ
    if (font() != newFont) {
        d->forceFont = true;
        setFont(newFont);
        d->forceFont = false;
    }
}

void HelpViewerImpl::scaleUp()
{
    TRACE_OBJ
    if (d->zoomCount < 10) {
        d->zoomCount++;
        d->forceFont = true;
        zoomIn();
        d->forceFont = false;
    }
}

void HelpViewerImpl::scaleDown()
{
    TRACE_OBJ
    if (d->zoomCount > -5) {
        d->zoomCount--;
        d->forceFont = true;
        zoomOut();
        d->forceFont = false;
    }
}

void HelpViewerImpl::resetScale()
{
    TRACE_OBJ
    if (d->zoomCount != 0) {
        d->forceFont = true;
        zoomOut(d->zoomCount);
        d->forceFont = false;
    }
    d->zoomCount = 0;
}

qreal HelpViewerImpl::scale() const
{
    TRACE_OBJ
    return d->zoomCount;
}

QString HelpViewerImpl::title() const
{
    TRACE_OBJ
    return documentTitle();
}

QUrl HelpViewerImpl::source() const
{
    TRACE_OBJ
    return QTextBrowser::source();
}

void HelpViewerImpl::doSetSource(const QUrl &url, QTextDocument::ResourceType type)
{
    TRACE_OBJ
    Q_UNUSED(type);
    if (HelpViewer::launchWithExternalApp(url))
        return;

    bool helpOrAbout = (url.toString() == QLatin1String("help"));
    const QUrl resolvedUrl = (helpOrAbout ? LocalHelpFile : HelpEngineWrapper::instance().findFile(url));

    QTextBrowser::doSetSource(resolvedUrl, type);

    if (!resolvedUrl.isValid()) {
        helpOrAbout = (url.toString() == QLatin1String("about:blank"));
        setHtml(helpOrAbout ? AboutBlank : PageNotFoundMessage.arg(url.toString()));
    }
    emit loadFinished(true);
}

QString HelpViewerImpl::selectedText() const
{
    TRACE_OBJ
    return textCursor().selectedText();
}

bool HelpViewerImpl::isForwardAvailable() const
{
    TRACE_OBJ
    return QTextBrowser::isForwardAvailable();
}

bool HelpViewerImpl::isBackwardAvailable() const
{
    TRACE_OBJ
    return QTextBrowser::isBackwardAvailable();
}

bool HelpViewerImpl::findText(const QString &text, HelpViewer::FindFlags flags, bool incremental,
    bool fromSearch)
{
    TRACE_OBJ
    QTextDocument *doc = document();
    QTextCursor cursor = textCursor();
    if (!doc || cursor.isNull())
        return false;

    const int position = cursor.selectionStart();
    if (incremental)
        cursor.setPosition(position);

    QTextDocument::FindFlags textDocFlags;
    if (flags & HelpViewer::FindBackward)
        textDocFlags |= QTextDocument::FindBackward;
    if (flags & HelpViewer::FindCaseSensitively)
        textDocFlags |= QTextDocument::FindCaseSensitively;

    QTextCursor found = doc->find(text, cursor, textDocFlags);
    if (found.isNull()) {
        if ((flags & HelpViewer::FindBackward) == 0)
            cursor.movePosition(QTextCursor::Start);
        else
            cursor.movePosition(QTextCursor::End);
        found = doc->find(text, cursor, textDocFlags);
    }

    if (fromSearch) {
        cursor.beginEditBlock();
        viewport()->setUpdatesEnabled(false);

        QTextCharFormat marker;
        marker.setForeground(Qt::red);
        cursor.movePosition(QTextCursor::Start);
        setTextCursor(cursor);

        while (find(text)) {
            QTextCursor hit = textCursor();
            hit.mergeCharFormat(marker);
        }

        viewport()->setUpdatesEnabled(true);
        cursor.endEditBlock();
    }

    bool cursorIsNull = found.isNull();
    if (cursorIsNull) {
        found = textCursor();
        found.setPosition(position);
    }
    setTextCursor(found);
    return !cursorIsNull;
}

// -- public slots

#if QT_CONFIG(clipboard)
void HelpViewerImpl::copy()
{
    TRACE_OBJ
    QTextBrowser::copy();
}
#endif

void HelpViewerImpl::forward()
{
    TRACE_OBJ
    QTextBrowser::forward();
}

void HelpViewerImpl::backward()
{
    TRACE_OBJ
    QTextBrowser::backward();
}

// -- protected

void HelpViewerImpl::keyPressEvent(QKeyEvent *e)
{
    TRACE_OBJ
    if ((e->key() == Qt::Key_Home && e->modifiers() != Qt::NoModifier)
        || (e->key() == Qt::Key_End && e->modifiers() != Qt::NoModifier)) {
        QKeyEvent* event = new QKeyEvent(e->type(), e->key(), Qt::NoModifier,
            e->text(), e->isAutoRepeat(), e->count());
        e = event;
    }
    QTextBrowser::keyPressEvent(e);
}

void HelpViewerImpl::wheelEvent(QWheelEvent *e)
{
    TRACE_OBJ
    if (e->modifiers() == Qt::ControlModifier) {
        e->accept();
        e->angleDelta().y() > 0 ? scaleUp() : scaleDown();
    } else {
        QTextBrowser::wheelEvent(e);
    }
}

void HelpViewerImpl::mousePressEvent(QMouseEvent *e)
{
    TRACE_OBJ
#ifdef Q_OS_LINUX
    if (handleForwardBackwardMouseButtons(e))
        return;
#endif

    QTextBrowser::mousePressEvent(e);
}

void HelpViewerImpl::mouseReleaseEvent(QMouseEvent *e)
{
    TRACE_OBJ
#ifndef Q_OS_LINUX
    if (handleForwardBackwardMouseButtons(e))
        return;
#endif

    bool controlPressed = e->modifiers() & Qt::ControlModifier;
    if ((controlPressed && d->hasAnchorAt(this, e->pos())) ||
        (e->button() == Qt::MiddleButton && d->hasAnchorAt(this, e->pos()))) {
        d->openLinkInNewPage();
        return;
    }

    QTextBrowser::mouseReleaseEvent(e);
}


void HelpViewerImpl::resizeEvent(QResizeEvent *e)
{
    const int topTextPosition = cursorForPosition({width() / 2, 0}).position();
    QTextBrowser::resizeEvent(e);
    scrollToTextPosition(topTextPosition);
}

// -- private slots

void HelpViewerImpl::actionChanged()
{
    // stub
    TRACE_OBJ
}

// -- private

bool HelpViewerImpl::eventFilter(QObject *obj, QEvent *event)
{
    TRACE_OBJ
    if (event->type() == QEvent::FontChange && !d->forceFont)
        return true;
    return QTextBrowser::eventFilter(obj, event);
}

void HelpViewerImpl::contextMenuEvent(QContextMenuEvent *event)
{
    TRACE_OBJ

    QMenu menu(QString(), nullptr);
    QUrl link;
#if QT_CONFIG(clipboard)
    QAction *copyAnchorAction = nullptr;
#endif
    if (d->hasAnchorAt(this, event->pos())) {
        link = anchorAt(event->pos());
        if (link.isRelative())
            link = source().resolved(link);
        menu.addAction(tr("Open Link"), d, &HelpViewerImplPrivate::openLink);
        menu.addAction(tr("Open Link in New Tab\tCtrl+LMB"), d, &HelpViewerImplPrivate::openLinkInNewPage);

#if QT_CONFIG(clipboard)
        if (!link.isEmpty() && link.isValid())
            copyAnchorAction = menu.addAction(tr("Copy &Link Location"));
#endif
    } else if (!selectedText().isEmpty()) {
#if QT_CONFIG(clipboard)
        menu.addAction(tr("Copy"), this, &HelpViewerImpl::copy);
#endif
    } else {
        menu.addAction(tr("Reload"), this, &HelpViewerImpl::reload);
    }

#if QT_CONFIG(clipboard)
    if (copyAnchorAction == menu.exec(event->globalPos()))
        QApplication::clipboard()->setText(link.toString());
#endif
}

QVariant HelpViewerImpl::loadResource(int type, const QUrl &name)
{
    TRACE_OBJ
    QByteArray ba;
    if (type < 4) {
        const QUrl url = HelpEngineWrapper::instance().findFile(name);
        ba = HelpEngineWrapper::instance().fileData(url);
        if (url.toString().endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
            QImage image;
            image.loadFromData(ba, "svg");
            if (!image.isNull())
                return image;
        }
    }
    return ba;
}


void HelpViewerImpl::scrollToTextPosition(int position)
{
    QTextCursor tc(document());
    tc.setPosition(position);
    const int dy = cursorRect(tc).top();
    if (verticalScrollBar()) {
        verticalScrollBar()->setValue(
                    std::min(verticalScrollBar()->value() + dy, verticalScrollBar()->maximum()));
    }
}

QT_END_NAMESPACE
