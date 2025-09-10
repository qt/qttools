// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HELPVIEWERIMPLPRIVATE_H
#define HELPVIEWERIMPLPRIVATE_H

#include "centralwidget.h"
#include "helpviewer.h"
#include "openpagesmanager.h"

#include <QtCore/QObject>
#if defined(BROWSER_QTEXTBROWSER)
#  include <QtWidgets/QTextBrowser>
#elif defined(BROWSER_QTWEBKIT)
#  include <QtGui/QGuiApplication>
#  include <QtGui/QScreen>
#endif // BROWSER_QTWEBKIT

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class HelpViewerImpl::HelpViewerImplPrivate : public QObject
{
    Q_OBJECT

public:
#if defined(BROWSER_QTEXTBROWSER)
    HelpViewerImplPrivate(int zoom)
        : zoomCount(zoom)
    { }
#elif defined(BROWSER_QTWEBKIT)
    HelpViewerImplPrivate()
    {
        // The web uses 96dpi by default on the web to preserve the font size across platforms, but
        // since we control the content for the documentation, we want the system DPI to be used.
        // - OS X reports 72dpi but doesn't allow changing the DPI, ignore anything below a 1.0 ratio to handle this.
        // - On Windows and Linux don't zoom the default web 96dpi below a 1.25 ratio to avoid
        //   filtered images in the doc unless the font readability difference is considerable.
        webDpiRatio = QGuiApplication::primaryScreen()->logicalDotsPerInch() / 96.;
        if (webDpiRatio < 1.25)
            webDpiRatio = 1.0;
    }
#endif // BROWSER_QTWEBKIT

#if defined(BROWSER_QTEXTBROWSER)
    bool hasAnchorAt(QTextBrowser *browser, const QPoint& pos)
    {
        lastAnchor = browser->anchorAt(pos);
        if (lastAnchor.isEmpty())
            return false;

        lastAnchor = browser->source().resolved(lastAnchor).toString();
        if (lastAnchor.at(0) == u'#') {
            QString src = browser->source().toString();
            int hsh = src.indexOf(u'#');
            lastAnchor = (hsh >= 0 ? src.left(hsh) : src) + lastAnchor;
        }
        return true;
    }

public slots:
    void openLink()
    {
        doOpenLink(false);
    }

    void openLinkInNewPage()
    {
        doOpenLink(true);
    }

public:
    QString lastAnchor;
    int zoomCount;
    bool forceFont = false;

private:

    void doOpenLink(bool newPage)
    {
        if (lastAnchor.isEmpty())
            return;
        if (newPage)
            OpenPagesManager::instance()->createPage(lastAnchor);
        else
            CentralWidget::instance()->setSource(lastAnchor);
        lastAnchor.clear();
    }

#elif defined(BROWSER_QTWEBKIT)
    qreal webDpiRatio;
#endif // BROWSER_QTWEBKIT
};

QT_END_NAMESPACE

#endif  // HELPVIEWERIMPLPRIVATE_H
