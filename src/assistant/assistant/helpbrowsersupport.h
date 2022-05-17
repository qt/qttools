// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef HELPBROWSERSUPPORT_H
#define HELPBROWSERSUPPORT_H

#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

class QNetworkAccessManager;
class QObject;
class QString;
class QByteArray;
class QUrl;

// Provide helper functions for feeding the QtHelp data stored in the help database
// into various browsers.
class HelpBrowserSupport
{
public:
    enum ResolveUrlResult {
        UrlRedirect,
        UrlLocalData,
        UrlResolveError
    };

    static QString msgError404();
    static QString msgPageNotFound();
    static QString msgAllDocumentationSets();
    static QString msgLoadError(const QUrl &url);
    static QString msgHtmlErrorPage(const QUrl &url);

    static ResolveUrlResult resolveUrl(const QUrl &url, QUrl *targetUrl,
                                       QByteArray *data);
    static QByteArray fileDataForLocalUrl(const QUrl &url);

    // Create an instance of QNetworkAccessManager for WebKit-type browsers.
    static QNetworkAccessManager *createNetworkAccessManager(QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // HELPBROWSERSUPPORT_H
