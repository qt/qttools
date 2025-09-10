// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpsearchengine.h"
#include "qhelpenginecore.h"
#include "qhelpsearchenginecore.h"
#include "qhelpsearchquerywidget.h"
#include "qhelpsearchresultwidget.h"

QT_BEGIN_NAMESPACE

class QHelpSearchEnginePrivate
{
public:
    QHelpSearchEngineCore m_searchEngine;
    QHelpSearchQueryWidget *queryWidget = nullptr;
    QHelpSearchResultWidget *resultWidget = nullptr;
};

/*!
    \class QHelpSearchQuery
    \deprecated
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpSearchQuery class contains the field name and the associated
    search term.

    The QHelpSearchQuery class contains the field name and the associated search
    term. Depending on the field the search term might get split up into separate
    terms to be parsed differently by the search engine.

    \note This class has been deprecated in favor of QString.

    \sa QHelpSearchQueryWidget
*/

/*!
    \fn QHelpSearchQuery::QHelpSearchQuery()

    Constructs a new empty QHelpSearchQuery.
*/

/*!
    \fn QHelpSearchQuery::QHelpSearchQuery(FieldName field, const QStringList &wordList)

    Constructs a new QHelpSearchQuery and initializes it with the given \a field and \a wordList.
*/

/*!
    \enum QHelpSearchQuery::FieldName
    This enum type specifies the field names that are handled by the search engine.

    \value DEFAULT  the default field provided by the search widget, several terms should be
                    split and stored in the word list except search terms enclosed in quotes.
    \value FUZZY    \deprecated Terms should be split in separate
                    words and passed to the search engine.
    \value WITHOUT  \deprecated  Terms should be split in separate
                    words and passed to the search engine.
    \value PHRASE   \deprecated  Terms should not be split in separate words.
    \value ALL      \deprecated  Terms should be split in separate
                    words and passed to the search engine
    \value ATLEAST  \deprecated  Terms should be split in separate
                    words and passed to the search engine
*/

/*!
    \class QHelpSearchEngine
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpSearchEngine class provides access to widgets reusable
    to integrate fulltext search as well as to index and search documentation.

    Before the search engine can be used, one has to instantiate at least a
    QHelpEngineCore object that needs to be passed to the search engines constructor.
    This is required as the search engine needs to be connected to the help
    engines setupFinished() signal to know when it can start to index documentation.

    After starting the indexing process the signal indexingStarted() is emitted and
    on the end of the indexing process the indexingFinished() is emitted. To stop
    the indexing one can call cancelIndexing().

    When the indexing process has finished, the search engine can be used to
    search through the index for a given term using the search() function. When
    the search input is passed to the search engine, the searchingStarted()
    signal is emitted. When the search finishes, the searchingFinished() signal
    is emitted. The search process can be stopped by calling cancelSearching().

    If the search succeeds, searchingFinished() is called with the search result
    count to fetch the search results from the search engine. Calling the
    searchResults() function with a range returns a list of QHelpSearchResult
    objects within the range. The results consist of the document title and URL,
    as well as a snippet from the document that contains the best match for the
    search input.

    To display the given search results use the QHelpSearchResultWidget or build up your own one if you need
    more advanced functionality. Note that the QHelpSearchResultWidget can not be instantiated
    directly, you must retrieve the widget from the search engine in use as all connections will be
    established for you by the widget itself.
*/

/*!
    \fn void QHelpSearchEngine::indexingStarted()

    This signal is emitted when indexing process is started.
*/

/*!
    \fn void QHelpSearchEngine::indexingFinished()

    This signal is emitted when the indexing process is complete.
*/

/*!
    \fn void QHelpSearchEngine::searchingStarted()

    This signal is emitted when the search process is started.
*/

/*!
    \fn void QHelpSearchEngine::searchingFinished(int searchResultCount)

    This signal is emitted when the search process is complete.
    The search result count is stored in \a searchResultCount.
*/

/*!
    Constructs a new search engine with the given \a parent. The search engine
    uses the given \a helpEngine to access the documentation that needs to be indexed.
    The QHelpEngine's setupFinished() signal is automatically connected to the
    QHelpSearchEngine's indexing function, so that new documentation will be indexed
    after the signal is emitted.
*/
QHelpSearchEngine::QHelpSearchEngine(QHelpEngineCore *helpEngine, QObject *parent)
    : QObject(parent)
      , d(new QHelpSearchEnginePrivate{QHelpSearchEngineCore(helpEngine)})
{
    connect(&d->m_searchEngine, &QHelpSearchEngineCore::indexingStarted,
            this, &QHelpSearchEngine::indexingStarted);
    connect(&d->m_searchEngine, &QHelpSearchEngineCore::indexingFinished,
            this, &QHelpSearchEngine::indexingFinished);
    connect(&d->m_searchEngine, &QHelpSearchEngineCore::searchingStarted,
            this, &QHelpSearchEngine::searchingStarted);
    connect(&d->m_searchEngine, &QHelpSearchEngineCore::searchingFinished,
            this, [this] { emit searchingFinished(d->m_searchEngine.searchResultCount()); });
}

/*!
    Destructs the search engine.
*/
QHelpSearchEngine::~QHelpSearchEngine()
{
    delete d;
}

/*!
    Returns a widget to use as input widget. Depending on your search engine
    configuration you will get a different widget with more or less subwidgets.
*/
QHelpSearchQueryWidget* QHelpSearchEngine::queryWidget()
{
    if (!d->queryWidget)
        d->queryWidget = new QHelpSearchQueryWidget();
    return d->queryWidget;
}

/*!
    Returns a widget that can hold and display the search results.
*/
QHelpSearchResultWidget* QHelpSearchEngine::resultWidget()
{
    if (!d->resultWidget)
        d->resultWidget = new QHelpSearchResultWidget(this);
    return d->resultWidget;
}

#if QT_DEPRECATED_SINCE(5, 9)
/*!
    \deprecated
    Use searchResultCount() instead.
*/
int QHelpSearchEngine::hitsCount() const
{
    return searchResultCount();
}

/*!
    \since 4.6
    \deprecated
    Use searchResultCount() instead.
*/
int QHelpSearchEngine::hitCount() const
{
    return searchResultCount();
}
#endif // QT_DEPRECATED_SINCE(5, 9)

/*!
    \since 5.9
    Returns the number of results the search engine found.
*/
int QHelpSearchEngine::searchResultCount() const
{
    return d->m_searchEngine.searchResultCount();
}

#if QT_DEPRECATED_SINCE(5, 9)
/*!
    \typedef QHelpSearchEngine::SearchHit
    \deprecated

    Use QHelpSearchResult instead.

    Typedef for QPair<QString, QString>.
    The values of that pair are the documentation file path and the page title.

    \sa hits()
*/

/*!
    \deprecated
    Use searchResults() instead.
*/
QList<QHelpSearchEngine::SearchHit> QHelpSearchEngine::hits(int start, int end) const
{
    QList<QHelpSearchEngine::SearchHit> hits;
    for (const QHelpSearchResult &result : searchResults(start, end))
        hits.append(qMakePair(result.url().toString(), result.title()));
    return hits;
}
#endif // QT_DEPRECATED_SINCE(5, 9)

/*!
    \since 5.9
    Returns a list of search results within the range from the index
    specified by \a start to the index specified by \a end.
*/
QList<QHelpSearchResult> QHelpSearchEngine::searchResults(int start, int end) const
{
    return d->m_searchEngine.searchResults(start, end);
}

/*!
    \since 5.9
    Returns the phrase that was last searched for.
*/
QString QHelpSearchEngine::searchInput() const
{
    return d->m_searchEngine.searchInput();
}

#if QT_DEPRECATED_SINCE(5, 9)
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
/*!
    \deprecated
    \since 4.5
    Use searchInput() instead.
*/
QList<QHelpSearchQuery> QHelpSearchEngine::query() const
{
    return {{QHelpSearchQuery::DEFAULT, searchInput().split(QChar::Space)}};
}
QT_WARNING_POP
#endif // QT_DEPRECATED_SINCE(5, 9)

/*!
    Forces the search engine to reindex all documentation files.
*/
void QHelpSearchEngine::reindexDocumentation()
{
    d->m_searchEngine.reindexDocumentation();
}

/*!
    Stops the indexing process.
*/
void QHelpSearchEngine::cancelIndexing()
{
    d->m_searchEngine.cancelIndexing();
}

/*!
    Stops the search process.
*/
void QHelpSearchEngine::cancelSearching()
{
    d->m_searchEngine.cancelSearching();
}

/*!
    \since 5.9
    Starts the search process using the given search phrase \a searchInput.

    The phrase may consist of several words. By default, the search engine returns
    the list of documents that contain all the specified words.
    The phrase may contain any combination of the logical operators AND, OR, and
    NOT. The operator must be written in all capital letters, otherwise it will
    be considered a part of the search phrase.

    If double quotation marks are used to group the words,
    the search engine will search for an exact match of the quoted phrase.

    For more information about the text query syntax,
    see \l {https://sqlite.org/fts5.html#full_text_query_syntax}
    {SQLite FTS5 Extension}.
*/
void QHelpSearchEngine::search(const QString &searchInput)
{
    d->m_searchEngine.search(searchInput);
}

#if QT_DEPRECATED_SINCE(5, 9)
/*!
    \deprecated
    Use search(const QString &searchInput) instead.
*/
void QHelpSearchEngine::search(const QList<QHelpSearchQuery> &queryList)
{
    if (queryList.isEmpty())
        return;

    d->m_searchEngine.search(queryList.first().wordList.join(QChar::Space));
}
#endif // QT_DEPRECATED_SINCE(5, 9)

/*!
    \internal
*/
void QHelpSearchEngine::scheduleIndexDocumentation()
{
    d->m_searchEngine.scheduleIndexDocumentation();
}

// TODO: Deprecate me (but it's private???)
void QHelpSearchEngine::indexDocumentation()
{}

QT_END_NAMESPACE
