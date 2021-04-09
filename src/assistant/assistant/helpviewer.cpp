/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "helpviewer.h"
#include "helpviewerimpl.h"

#include "helpenginewrapper.h"
#include "tracer.h"

#include <QtCore/QFileInfo>
#include <QtCore/QStringBuilder>
#include <QtCore/QTemporaryFile>

#include <QtGui/QDesktopServices>

#include <QtWidgets/QVBoxLayout>

#include <QtHelp/QHelpEngineCore>

QT_BEGIN_NAMESPACE

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

class HelpViewerPrivate
{
public:
    HelpViewerImpl *m_viewer;
};

HelpViewer::HelpViewer(qreal zoom, QWidget *parent)
    : QWidget(parent)
    , d(new HelpViewerPrivate)
{
    auto layout = new QVBoxLayout;
    d->m_viewer = new HelpViewerImpl(zoom, this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->m_viewer, 10);

    connect(d->m_viewer, &HelpViewerImpl::titleChanged,      this, &HelpViewer::titleChanged);
#if QT_CONFIG(clipboard)
    connect(d->m_viewer, &HelpViewerImpl::copyAvailable,     this, &HelpViewer::copyAvailable);
#endif
    connect(d->m_viewer, &HelpViewerImpl::sourceChanged,     this, &HelpViewer::sourceChanged);
    connect(d->m_viewer, &HelpViewerImpl::forwardAvailable,  this, &HelpViewer::forwardAvailable);
    connect(d->m_viewer, &HelpViewerImpl::backwardAvailable, this, &HelpViewer::backwardAvailable);
    connect(d->m_viewer, &HelpViewerImpl::highlighted,       this, &HelpViewer::highlighted);
#if defined(BROWSER_QTWEBKIT)
    connect(d->m_viewer, &HelpViewerImpl::printRequested,    this, &HelpViewer::printRequested);
#endif
    connect(d->m_viewer, &HelpViewerImpl::loadFinished,      this, &HelpViewer::loadFinished);
}

HelpViewer::~HelpViewer()
{
    delete d;
}

QFont HelpViewer::viewerFont() const
{
    return d->m_viewer->viewerFont();
}

void HelpViewer::setViewerFont(const QFont &font)
{
    d->m_viewer->setViewerFont(font);
}

void HelpViewer::scaleUp()
{
    d->m_viewer->scaleUp();
}

void HelpViewer::scaleDown()
{
    d->m_viewer->scaleDown();
}

void HelpViewer::resetScale()
{
    d->m_viewer->resetScale();
}

qreal HelpViewer::scale() const
{
    return d->m_viewer->scale();
}

QString HelpViewer::title() const
{
    return d->m_viewer->title();
}

QUrl HelpViewer::source() const
{
    return d->m_viewer->source();
}

void HelpViewer::reload()
{
    d->m_viewer->reload();
}

void HelpViewer::setSource(const QUrl &url)
{
    d->m_viewer->setSource(url);
}

void HelpViewer::print(QPagedPaintDevice *printer)
{
    d->m_viewer->print(printer);
}

QString HelpViewer::selectedText() const
{
    return d->m_viewer->selectedText();
}

bool HelpViewer::isForwardAvailable() const
{
    return d->m_viewer->isForwardAvailable();
}

bool HelpViewer::isBackwardAvailable() const
{
    return d->m_viewer->isBackwardAvailable();
}

bool HelpViewer::findText(const QString &text, FindFlags flags, bool incremental, bool fromSearch)
{
    return d->m_viewer->findText(text, flags, incremental, fromSearch);
}

#if QT_CONFIG(clipboard)
void HelpViewer::copy()
{
    d->m_viewer->copy();
}
#endif

void HelpViewer::home()
{
    d->m_viewer->home();
}

void HelpViewer::forward()
{
    d->m_viewer->forward();
}

void HelpViewer::backward()
{
    d->m_viewer->backward();
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
