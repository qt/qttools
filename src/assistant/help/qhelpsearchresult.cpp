// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpsearchresult.h"

#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QHelpSearchResultData : public QSharedData
{
public:
    QUrl m_url;
    QString m_title;
    QString m_snippet;
};

/*!
    \class QHelpSearchResult
    \since 5.9
    \inmodule QtHelp
    \brief The QHelpSearchResult class provides the data associated with the
    search result.

    The QHelpSearchResult object is a data object that describes a single search result.
    The vector of search result objects is returned by QHelpSearchEngine::searchResults().
    The description of the search result contains the document title and URL
    that the search input matched. It also contains the snippet from
    the document content containing the best match of the search input.
    \sa QHelpSearchEngine
*/

/*!
    Constructs a new empty QHelpSearchResult.
*/
QHelpSearchResult::QHelpSearchResult() : d(new QHelpSearchResultData) { }

/*!
    Constructs a copy of \a other.
*/
QHelpSearchResult::QHelpSearchResult(const QHelpSearchResult &other) = default;

/*!
    Constructs the search result containing \a url, \a title and \a snippet
    as the description of the result.
*/
QHelpSearchResult::QHelpSearchResult(const QUrl &url, const QString &title, const QString &snippet)
    : d(new QHelpSearchResultData)
{
    d->m_url = url;
    d->m_title = title;
    d->m_snippet = snippet;
}

/*!
    Destroys the search result.
*/
QHelpSearchResult::~QHelpSearchResult() = default;

/*!
    Assigns \a other to this search result and returns a reference to this search result.
*/
QHelpSearchResult &QHelpSearchResult::operator=(const QHelpSearchResult &other) = default;

/*!
    Returns the document title of the search result.
*/
QString QHelpSearchResult::title() const
{
    return d->m_title;
}

/*!
    Returns the document URL of the search result.
*/
QUrl QHelpSearchResult::url() const
{
    return d->m_url;
}

/*!
    Returns the document snippet containing the search phrase of the search result.
*/
QString QHelpSearchResult::snippet() const
{
    return d->m_snippet;
}

QT_END_NAMESPACE
