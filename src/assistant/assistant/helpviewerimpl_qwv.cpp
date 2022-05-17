// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpviewerimpl.h"
#include "helpviewerimpl_p.h"

#include "centralwidget.h"
#include "helpenginewrapper.h"
#include "helpbrowsersupport.h"
#include "openpagesmanager.h"
#include "tracer.h"

#include <QClipboard>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <QtWidgets/QApplication>
#include <QtGui/QWheelEvent>

#include <QtNetwork/QNetworkReply>

QT_BEGIN_NAMESPACE

// -- HelpPage

class HelpPage : public QWebPage
{
public:
    HelpPage(QObject *parent);

protected:
    virtual QWebPage *createWindow(QWebPage::WebWindowType);
    virtual void triggerAction(WebAction action, bool checked = false);

    virtual bool acceptNavigationRequest(QWebFrame *frame,
        const QNetworkRequest &request, NavigationType type);

private:
    bool closeNewTabIfNeeded;

    friend class HelpViewerImpl;
    QUrl m_loadingUrl;
    Qt::MouseButtons m_pressedButtons;
    Qt::KeyboardModifiers m_keyboardModifiers;
};

HelpPage::HelpPage(QObject *parent)
    : QWebPage(parent)
    , closeNewTabIfNeeded(false)
    , m_pressedButtons(Qt::NoButton)
    , m_keyboardModifiers(Qt::NoModifier)
{
    TRACE_OBJ
}

QWebPage *HelpPage::createWindow(QWebPage::WebWindowType)
{
    TRACE_OBJ
    HelpPage *newPage = static_cast<HelpPage *>(OpenPagesManager::instance()
        ->createBlankPage()->page());
    newPage->closeNewTabIfNeeded = closeNewTabIfNeeded;
    closeNewTabIfNeeded = false;
    return newPage;
}

void HelpPage::triggerAction(WebAction action, bool checked)
{
    TRACE_OBJ
    switch (action) {
        case OpenLinkInNewWindow:
            closeNewTabIfNeeded = true;
            Q_FALLTHROUGH();
        default:
            QWebPage::triggerAction(action, checked);
            break;
    }

#if QT_CONFIG(clipboard)
    if (action == CopyLinkToClipboard || action == CopyImageUrlToClipboard) {
        const QString link = QApplication::clipboard()->text();
        QApplication::clipboard()->setText(HelpEngineWrapper::instance().findFile(link).toString());
    }
#endif
}

bool HelpPage::acceptNavigationRequest(QWebFrame *,
    const QNetworkRequest &request, QWebPage::NavigationType type)
{
    TRACE_OBJ
    const bool closeNewTab = closeNewTabIfNeeded;
    closeNewTabIfNeeded = false;

    const QUrl &url = request.url();
    if (HelpViewer::launchWithExternalApp(url)) {
        if (closeNewTab)
            QMetaObject::invokeMethod(OpenPagesManager::instance(), "closeCurrentPage");
        return false;
    }

    if (type == QWebPage::NavigationTypeLinkClicked
        && (m_keyboardModifiers & Qt::ControlModifier
            || m_pressedButtons == Qt::MiddleButton)) {
        m_pressedButtons = Qt::NoButton;
        m_keyboardModifiers = Qt::NoModifier;
        OpenPagesManager::instance()->createPage(url);
        return false;
    }

    m_loadingUrl = url; // because of async page loading, we will hit some kind
    // of race condition while using a remote command, like a combination of
    // SetSource; SyncContent. SetSource would be called and SyncContents shortly
    // afterwards, but the page might not have finished loading and the old url
    // would be returned.
    return true;
}

// -- HelpViewerImpl

HelpViewerImpl::HelpViewerImpl(qreal zoom, QWidget *parent)
    : QWebView(parent)
    , d(new HelpViewerImplPrivate)
{
    TRACE_OBJ
    setAcceptDrops(false);
    settings()->setAttribute(QWebSettings::JavaEnabled, false);
    settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    setPage(new HelpPage(this));
    page()->setNetworkAccessManager(HelpBrowserSupport::createNetworkAccessManager(this));

    QAction* action = pageAction(QWebPage::OpenLinkInNewWindow);
    action->setText(tr("Open Link in New Page"));

    pageAction(QWebPage::DownloadLinkToDisk)->setVisible(false);
    pageAction(QWebPage::DownloadImageToDisk)->setVisible(false);
    pageAction(QWebPage::OpenImageInNewWindow)->setVisible(false);

    connect(pageAction(QWebPage::Copy), SIGNAL(changed()), this,
        SLOT(actionChanged()));
    connect(pageAction(QWebPage::Back), SIGNAL(changed()), this,
        SLOT(actionChanged()));
    connect(pageAction(QWebPage::Forward), SIGNAL(changed()), this,
        SLOT(actionChanged()));
    connect(page(), &QWebPage::linkHovered, this,
            [this] (const QString &link, const QString &, const QString &) {
                emit this->highlighted(QUrl(link));
    });
    connect(this, SIGNAL(urlChanged(QUrl)), this, SIGNAL(sourceChanged(QUrl)));
    connect(this, SIGNAL(loadStarted()), this, SLOT(setLoadStarted()));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setLoadFinished(bool)));
    connect(this, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged()));
    connect(page(), SIGNAL(printRequested(QWebFrame*)), this, SIGNAL(printRequested()));

    setFont(viewerFont());
    setZoomFactor(d->webDpiRatio * (zoom == 0.0 ? 1.0 : zoom));
}

QFont HelpViewerImpl::viewerFont() const
{
    TRACE_OBJ
    if (HelpEngineWrapper::instance().usesBrowserFont())
        return HelpEngineWrapper::instance().browserFont();

    QWebSettings *webSettings = QWebSettings::globalSettings();
    return QFont(webSettings->fontFamily(QWebSettings::StandardFont),
        webSettings->fontSize(QWebSettings::DefaultFontSize));
}

void HelpViewerImpl::setViewerFont(const QFont &font)
{
    TRACE_OBJ
    QWebSettings *webSettings = settings();
    webSettings->setFontFamily(QWebSettings::StandardFont, font.family());
    webSettings->setFontSize(QWebSettings::DefaultFontSize, font.pointSize());
}

void HelpViewerImpl::scaleUp()
{
    TRACE_OBJ
    setZoomFactor(zoomFactor() + 0.1);
}

void HelpViewerImpl::scaleDown()
{
    TRACE_OBJ
    setZoomFactor(qMax(0.0, zoomFactor() - 0.1));
}

void HelpViewerImpl::resetScale()
{
    TRACE_OBJ
    setZoomFactor(d->webDpiRatio);
}

qreal HelpViewerImpl::scale() const
{
    TRACE_OBJ
    return zoomFactor() / d->webDpiRatio;
}

QString HelpViewerImpl::title() const
{
    TRACE_OBJ
    return QWebView::title();
}

void HelpViewerImpl::setTitle(const QString &title)
{
    TRACE_OBJ
    Q_UNUSED(title);
}

QUrl HelpViewerImpl::source() const
{
    TRACE_OBJ
    HelpPage *currentPage = static_cast<HelpPage*> (page());
    if (currentPage && !d->m_loadFinished) {
        // see HelpPage::acceptNavigationRequest(...)
        return currentPage->m_loadingUrl;
    }
    return url();
}

void HelpViewerImpl::setSource(const QUrl &url)
{
    TRACE_OBJ
    load(url.toString() == QLatin1String("help") ? LocalHelpFile : url);
}

QString HelpViewerImpl::selectedText() const
{
    TRACE_OBJ
    return QWebView::selectedText();
}

bool HelpViewerImpl::isForwardAvailable() const
{
    TRACE_OBJ
    return pageAction(QWebPage::Forward)->isEnabled();
}

bool HelpViewerImpl::isBackwardAvailable() const
{
    TRACE_OBJ
    return pageAction(QWebPage::Back)->isEnabled();
}

bool HelpViewerImpl::findText(const QString &text, HelpViewer::FindFlags flags, bool incremental,
    bool fromSearch)
{
    TRACE_OBJ
    Q_UNUSED(incremental); Q_UNUSED(fromSearch);
    QWebPage::FindFlags options = QWebPage::FindWrapsAroundDocument;
    if (flags & HelpViewer::FindBackward)
        options |= QWebPage::FindBackward;
    if (flags & HelpViewer::FindCaseSensitively)
        options |= QWebPage::FindCaseSensitively;

    bool found = QWebView::findText(text, options);
    options = QWebPage::HighlightAllOccurrences;
    QWebView::findText(QString(), options); // clear first
    QWebView::findText(text, options); // force highlighting of all other matches
    return found;
}

// -- public slots

#if QT_CONFIG(clipboard)
void HelpViewerImpl::copy()
{
    TRACE_OBJ
    triggerPageAction(QWebPage::Copy);
}
#endif

void HelpViewerImpl::forward()
{
    TRACE_OBJ
    QWebView::forward();
}

void HelpViewerImpl::backward()
{
    TRACE_OBJ
    back();
}

// -- protected

void HelpViewerImpl::keyPressEvent(QKeyEvent *e)
{
    TRACE_OBJ
    // TODO: remove this once we support multiple keysequences per command
#if QT_CONFIG(clipboard)
    if (e->key() == Qt::Key_Insert && e->modifiers() == Qt::CTRL) {
        if (!selectedText().isEmpty())
            copy();
    }
#endif
    QWebView::keyPressEvent(e);
}

void HelpViewerImpl::wheelEvent(QWheelEvent *event)
{
    TRACE_OBJ
    if (event->modifiers()& Qt::ControlModifier) {
        event->accept();
        event->delta() > 0 ? scaleUp() : scaleDown();
    } else {
        QWebView::wheelEvent(event);
    }
}

void HelpViewerImpl::mousePressEvent(QMouseEvent *event)
{
    TRACE_OBJ
#ifdef Q_OS_LINUX
    if (handleForwardBackwardMouseButtons(event))
        return;
#endif

    if (HelpPage *currentPage = static_cast<HelpPage*> (page())) {
        currentPage->m_pressedButtons = event->buttons();
        currentPage->m_keyboardModifiers = event->modifiers();
    }

    QWebView::mousePressEvent(event);
}

void HelpViewerImpl::mouseReleaseEvent(QMouseEvent *event)
{
    TRACE_OBJ
#ifndef Q_OS_LINUX
    if (handleForwardBackwardMouseButtons(event))
        return;
#endif

    QWebView::mouseReleaseEvent(event);
}

// -- private slots

void HelpViewerImpl::actionChanged()
{
    TRACE_OBJ
    QAction *a = qobject_cast<QAction *>(sender());
    if (a == pageAction(QWebPage::Copy))
        emit copyAvailable(a->isEnabled());
    else if (a == pageAction(QWebPage::Back))
        emit backwardAvailable(a->isEnabled());
    else if (a == pageAction(QWebPage::Forward))
        emit forwardAvailable(a->isEnabled());
}

// -- private

bool HelpViewerImpl::eventFilter(QObject *obj, QEvent *event)
{
    TRACE_OBJ
    return QWebView::eventFilter(obj, event);
}

void HelpViewerImpl::contextMenuEvent(QContextMenuEvent *event)
{
    TRACE_OBJ
    QWebView::contextMenuEvent(event);
}

QT_END_NAMESPACE
