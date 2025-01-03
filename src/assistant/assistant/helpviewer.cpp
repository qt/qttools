// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpviewer.h"
#include "helpviewerimpl.h"

#include "helpenginewrapper.h"
#include "tracer.h"

#include <QtCore/QFileInfo>
#include <QtCore/QStringBuilder>
#include <QtCore/QTemporaryFile>

#include <QtGui/QDesktopServices>
#if QT_CONFIG(clipboard)
#include <QtGui/QClipboard>
#endif
#include <QtGui/QGuiApplication>
#include <QtGui/QWheelEvent>

#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>

#include <QtHelp/QHelpEngineCore>

#include <qlitehtmlwidget.h>

QT_BEGIN_NAMESPACE

const int kMaxHistoryItems = 20;

struct ExtensionMap {
    const char *extension;
    const char *mimeType;
} extensionMap[] = {
    { ".bmp", "image/bmp" },
    { ".css", "text/css" },
    { ".gif", "image/gif" },
    { ".html", "text/html" },
    { ".htm", "text/html" },
    { ".ico", "image/x-icon" },
    { ".jpeg", "image/jpeg" },
    { ".jpg", "image/jpeg" },
    { ".js", "application/x-javascript" },
    { ".mng", "video/x-mng" },
    { ".pbm", "image/x-portable-bitmap" },
    { ".pgm", "image/x-portable-graymap" },
    { ".pdf", nullptr },
    { ".png", "image/png" },
    { ".ppm", "image/x-portable-pixmap" },
    { ".rss", "application/rss+xml" },
    { ".svg", "image/svg+xml" },
    { ".svgz", "image/svg+xml" },
    { ".text", "text/plain" },
    { ".tif", "image/tiff" },
    { ".tiff", "image/tiff" },
    { ".txt", "text/plain" },
    { ".xbm", "image/x-xbitmap" },
    { ".xml", "text/xml" },
    { ".xpm", "image/x-xpm" },
    { ".xsl", "text/xsl" },
    { ".xhtml", "application/xhtml+xml" },
    { ".wml", "text/vnd.wap.wml" },
    { ".wmlc", "application/vnd.wap.wmlc" },
    { "about:blank", nullptr },
    { nullptr, nullptr }
};

static QByteArray getData(const QUrl &url)
{
    // TODO: this is just a hack for Qt documentation
    // which decides to use a simpler CSS if the viewer does not have JavaScript
    // which was a hack to decide if we are viewing in QTextBrowser or QtWebEngine et al
    QUrl actualUrl = url;
    QString path = url.path(QUrl::FullyEncoded);
    static const char simpleCss[] = "/offline-simple.css";
    if (path.endsWith(simpleCss)) {
        path.replace(simpleCss, "/offline.css");
        actualUrl.setPath(path);
    }

    if (actualUrl.isValid())
        return HelpEngineWrapper::instance().fileData(actualUrl);

    const bool isAbout = (actualUrl.toString() == QLatin1String("about:blank"));
    return isAbout ? HelpViewerImpl::AboutBlank.toUtf8()
                   : HelpViewerImpl::PageNotFoundMessage.arg(url.toString()).toUtf8();
}

class HelpViewerPrivate
{
public:
    struct HistoryItem
    {
        QUrl url;
        QString title;
        int vscroll;
    };
    HistoryItem currentHistoryItem() const;
    void setSourceInternal(const QUrl &url, int *vscroll = nullptr, bool reload = false);
    void incrementZoom(int steps);
    void applyZoom(int percentage);

    HelpViewer *q = nullptr;
    QLiteHtmlWidget *m_viewer = nullptr;
    std::vector<HistoryItem> m_backItems;
    std::vector<HistoryItem> m_forwardItems;
    int m_fontZoom = 100; // zoom percentage
};

HelpViewerPrivate::HistoryItem HelpViewerPrivate::currentHistoryItem() const
{
    return { m_viewer->url(), m_viewer->title(), m_viewer->verticalScrollBar()->value() };
}

void HelpViewerPrivate::setSourceInternal(const QUrl &url, int *vscroll, bool reload)
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    const bool isHelp = (url.toString() == QLatin1String("help"));
    const QUrl resolvedUrl = (isHelp ? HelpViewerImpl::LocalHelpFile
                                     : HelpEngineWrapper::instance().findFile(url));

    QUrl currentUrlWithoutFragment = m_viewer->url();
    currentUrlWithoutFragment.setFragment({});
    QUrl newUrlWithoutFragment = resolvedUrl;
    newUrlWithoutFragment.setFragment({});

    m_viewer->setUrl(resolvedUrl);
    if (currentUrlWithoutFragment != newUrlWithoutFragment || reload)
        m_viewer->setHtml(QString::fromUtf8(getData(resolvedUrl)));
    if (vscroll)
        m_viewer->verticalScrollBar()->setValue(*vscroll);
    else
        m_viewer->scrollToAnchor(resolvedUrl.fragment(QUrl::FullyEncoded));

    QGuiApplication::restoreOverrideCursor();

    emit q->sourceChanged(q->source());
    emit q->loadFinished();
    emit q->titleChanged();
}

void HelpViewerPrivate::incrementZoom(int steps)
{
    const int incrementPercentage = 10 * steps; // 10 percent increase by single step
    const int previousZoom = m_fontZoom;
    applyZoom(previousZoom + incrementPercentage);
}

void HelpViewerPrivate::applyZoom(int percentage)
{
    const int newZoom = qBound(10, percentage, 300);
    if (newZoom == m_fontZoom)
        return;
    m_fontZoom = newZoom;
    m_viewer->setZoomFactor(newZoom / 100.0);
}

HelpViewer::HelpViewer(qreal zoom, QWidget *parent)
    : QWidget(parent)
    , d(new HelpViewerPrivate)
{
    auto layout = new QVBoxLayout;
    d->q = this;
    d->m_viewer = new QLiteHtmlWidget(this);
    d->m_viewer->setResourceHandler([](const QUrl &url) { return getData(url); });
    d->m_viewer->viewport()->installEventFilter(this);
    const int zoomPercentage = zoom == 0 ? 100 : zoom * 100;
    d->applyZoom(zoomPercentage);
    connect(d->m_viewer, &QLiteHtmlWidget::linkClicked, this, &HelpViewer::setSource);
    connect(d->m_viewer, &QLiteHtmlWidget::linkHighlighted, this, &HelpViewer::highlighted);
#if QT_CONFIG(clipboard)
    connect(d->m_viewer, &QLiteHtmlWidget::copyAvailable, this, &HelpViewer::copyAvailable);
#endif
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->m_viewer, 10);

    // Make docs' contents visible in dark theme
    QPalette p = palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight,
        p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
        p.color(QPalette::Active, QPalette::HighlightedText));
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Text, Qt::black);
    setPalette(p);
}

HelpViewer::~HelpViewer()
{
    delete d;
}

QFont HelpViewer::viewerFont() const
{
    return d->m_viewer->defaultFont();
}

void HelpViewer::setViewerFont(const QFont &font)
{
    d->m_viewer->setDefaultFont(font);
}

void HelpViewer::scaleUp()
{
    d->incrementZoom(1);
}

void HelpViewer::scaleDown()
{
    d->incrementZoom(-1);
}

void HelpViewer::resetScale()
{
    d->applyZoom(100);
}

qreal HelpViewer::scale() const
{
    return d->m_viewer->zoomFactor();
}

QString HelpViewer::title() const
{
    return d->m_viewer->title();
}

QUrl HelpViewer::source() const
{
    return d->m_viewer->url();
}

void HelpViewer::reload()
{
    doSetSource(source(), true);
}

void HelpViewer::setSource(const QUrl &url)
{
    doSetSource(url, false);
}

void HelpViewer::doSetSource(const QUrl &url, bool reload)
{
    if (launchWithExternalApp(url))
        return;

    d->m_forwardItems.clear();
    emit forwardAvailable(false);
    if (d->m_viewer->url().isValid()) {
        d->m_backItems.push_back(d->currentHistoryItem());
        while (d->m_backItems.size() > kMaxHistoryItems) // this should trigger only once anyhow
            d->m_backItems.erase(d->m_backItems.begin());
        emit backwardAvailable(true);
    }

    d->setSourceInternal(url, nullptr, reload);
}

void HelpViewer::print(QPagedPaintDevice *printer)
{
    Q_UNUSED(printer);
    // TODO: implement me
}

QString HelpViewer::selectedText() const
{
    return d->m_viewer->selectedText();
}

bool HelpViewer::isForwardAvailable() const
{
    return !d->m_forwardItems.empty();
}

bool HelpViewer::isBackwardAvailable() const
{
    return !d->m_backItems.empty();
}

static QTextDocument::FindFlags textDocumentFlagsForFindFlags(HelpViewer::FindFlags flags)
{
    QTextDocument::FindFlags textDocFlags;
    if (flags & HelpViewer::FindBackward)
        textDocFlags |= QTextDocument::FindBackward;
    if (flags & HelpViewer::FindCaseSensitively)
        textDocFlags |= QTextDocument::FindCaseSensitively;
    return textDocFlags;
}

bool HelpViewer::findText(const QString &text, FindFlags flags, bool incremental, bool fromSearch)
{
    Q_UNUSED(fromSearch);
    return d->m_viewer->findText(text, textDocumentFlagsForFindFlags(flags), incremental);
}

#if QT_CONFIG(clipboard)
void HelpViewer::copy()
{
    QGuiApplication::clipboard()->setText(selectedText());
}
#endif

void HelpViewer::home()
{
    setSource(HelpEngineWrapper::instance().homePage());
}

void HelpViewer::forward()
{
    HelpViewerPrivate::HistoryItem nextItem = d->currentHistoryItem();
    if (d->m_forwardItems.empty())
        return;
    d->m_backItems.push_back(nextItem);
    nextItem = d->m_forwardItems.front();
    d->m_forwardItems.erase(d->m_forwardItems.begin());

    emit backwardAvailable(isBackwardAvailable());
    emit forwardAvailable(isForwardAvailable());
    d->setSourceInternal(nextItem.url, &nextItem.vscroll);
}

void HelpViewer::backward()
{
    HelpViewerPrivate::HistoryItem previousItem = d->currentHistoryItem();
    if (d->m_backItems.empty())
        return;
    d->m_forwardItems.insert(d->m_forwardItems.begin(), previousItem);
    previousItem = d->m_backItems.back();
    d->m_backItems.pop_back();

    emit backwardAvailable(isBackwardAvailable());
    emit forwardAvailable(isForwardAvailable());
    d->setSourceInternal(previousItem.url, &previousItem.vscroll);
}

bool HelpViewer::eventFilter(QObject *src, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        auto we = static_cast<QWheelEvent *>(event);
        if (we->modifiers() == Qt::ControlModifier) {
            we->accept();
            const int deltaY = we->angleDelta().y();
            if (deltaY != 0)
                d->incrementZoom(deltaY / 120);
            return true;
        }
    }
    return QWidget::eventFilter(src, event);
}

bool HelpViewer::isLocalUrl(const QUrl &url)
{
    TRACE_OBJ
    const QString &scheme = url.scheme();
    return scheme.isEmpty()
        || scheme == QLatin1String("file")
        || scheme == QLatin1String("qrc")
        || scheme == QLatin1String("data")
        || scheme == QLatin1String("qthelp")
        || scheme == QLatin1String("about");
}

bool HelpViewer::canOpenPage(const QString &path)
{
    TRACE_OBJ
    return !mimeFromUrl(QUrl::fromLocalFile(path)).isEmpty();
}

QString HelpViewer::mimeFromUrl(const QUrl &url)
{
    TRACE_OBJ
    const QString &path = url.path();
    const int index = path.lastIndexOf(QLatin1Char('.'));
    const QByteArray &ext = path.mid(index).toUtf8().toLower();

    const ExtensionMap *e = extensionMap;
    while (e->extension) {
        if (ext == e->extension)
            return QLatin1String(e->mimeType);
        ++e;
    }
    return QLatin1String("application/octet-stream");
}

bool HelpViewer::launchWithExternalApp(const QUrl &url)
{
    TRACE_OBJ
    if (isLocalUrl(url)) {
        const HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
        const QUrl &resolvedUrl = helpEngine.findFile(url);
        if (!resolvedUrl.isValid())
            return false;

        const QString& path = resolvedUrl.toLocalFile();
        if (!canOpenPage(path)) {
            QTemporaryFile tmpTmpFile;
            if (!tmpTmpFile.open())
                return false;

            const QString &extension = QFileInfo(path).completeSuffix();
            QFile actualTmpFile(tmpTmpFile.fileName() % QLatin1String(".")
                % extension);
            if (!actualTmpFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
                return false;

            actualTmpFile.write(helpEngine.fileData(resolvedUrl));
            actualTmpFile.close();
            return QDesktopServices::openUrl(QUrl::fromLocalFile(actualTmpFile.fileName()));
        }
        return false;
    }
    return QDesktopServices::openUrl(url);
}

QT_END_NAMESPACE
