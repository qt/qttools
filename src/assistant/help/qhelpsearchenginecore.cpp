// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpsearchenginecore.h"
#include "qhelpenginecore.h"

#include "qhelpsearchindexreader_p.h"
#include "qhelpsearchindexwriter_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpointer.h>
#include <QtCore/qtimer.h>

QT_BEGIN_NAMESPACE

using namespace fulltextsearch;
using namespace Qt::StringLiterals;

class QHelpSearchEngineCorePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QHelpSearchEngineCore)

public:
    QString indexFilesFolder() const
    {
        QString indexFilesFolder = ".fulltextsearch"_L1;
        if (m_helpEngine && !m_helpEngine->collectionFile().isEmpty()) {
            const QFileInfo fi(m_helpEngine->collectionFile());
            indexFilesFolder = fi.absolutePath() + QDir::separator() + u'.'
                    + fi.fileName().left(fi.fileName().lastIndexOf(".qhc"_L1));
        }
        return indexFilesFolder;
    }

    void updateIndex(bool reindex)
    {
        Q_Q(QHelpSearchEngineCore);
        if (m_helpEngine.isNull())
            return;

        if (!QFile::exists(QFileInfo(m_helpEngine->collectionFile()).path()))
            return;

        if (!m_indexWriter) {
            m_indexWriter.reset(new QHelpSearchIndexWriter);

            QObject::connect(m_indexWriter.get(), &QHelpSearchIndexWriter::indexingStarted,
                             q, &QHelpSearchEngineCore::indexingStarted);
            QObject::connect(m_indexWriter.get(), &QHelpSearchIndexWriter::indexingFinished,
                             q, &QHelpSearchEngineCore::indexingFinished);
        }

        m_indexWriter->cancelIndexing();
        m_indexWriter->updateIndex(m_helpEngine->collectionFile(), indexFilesFolder(), reindex);
    }

    void search(const QString &searchInput)
    {
        Q_Q(QHelpSearchEngineCore);
        if (m_helpEngine.isNull())
            return;

        if (!QFile::exists(QFileInfo(m_helpEngine->collectionFile()).path()))
            return;

        if (!m_indexReader) {
            m_indexReader.reset(new QHelpSearchIndexReader);
            QObject::connect(m_indexReader.get(), &QHelpSearchIndexReader::searchingStarted,
                             q, &QHelpSearchEngineCore::searchingStarted);
            QObject::connect(m_indexReader.get(), &QHelpSearchIndexReader::searchingFinished,
                             q, &QHelpSearchEngineCore::searchingFinished);
        }

        m_searchInput = searchInput;
        m_indexReader->cancelSearching();
        m_indexReader->search(m_helpEngine->collectionFile(), indexFilesFolder(), searchInput,
                              m_helpEngine->usesFilterEngine());
    }

    bool m_isIndexingScheduled = false;

    std::unique_ptr<QHelpSearchIndexReader> m_indexReader;
    std::unique_ptr<QHelpSearchIndexWriter> m_indexWriter;

    QPointer<QHelpEngineCore> m_helpEngine;
    QString m_searchInput;
};

/*!
    \class QHelpSearchEngineCore
    \since 6.8
    \inmodule QtHelp
    \brief The QHelpSearchEngineCore class provides access to index and
           search documentation.

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
*/

/*!
    \fn void QHelpSearchEngineCore::indexingStarted()

    This signal is emitted when indexing process is started.
*/

/*!
    \fn void QHelpSearchEngineCore::indexingFinished()

    This signal is emitted when the indexing process is complete.
*/

/*!
    \fn void QHelpSearchEngineCore::searchingStarted()

    This signal is emitted when the search process is started.
*/

/*!
    \fn void QHelpSearchEngineCore::searchingFinished()

    This signal is emitted when the search process is complete.
*/

/*!
    Constructs a new search engine with the given \a parent. The search engine
    uses the given \a helpEngine to access the documentation that needs to be indexed.
    The QHelpEngineCore's setupFinished() signal is automatically connected to the
    QHelpSearchEngineCore's indexing function, so that new documentation will
    be indexed after the signal is emitted.
*/
QHelpSearchEngineCore::QHelpSearchEngineCore(QHelpEngineCore *helpEngine, QObject *parent)
    : QObject(*new QHelpSearchEngineCorePrivate, parent)
{
    Q_D(QHelpSearchEngineCore);
    d->m_helpEngine = helpEngine;
    connect(helpEngine, &QHelpEngineCore::setupFinished,
            this, &QHelpSearchEngineCore::scheduleIndexDocumentation);
}

/*!
    Destructs the search engine.
*/
QHelpSearchEngineCore::~QHelpSearchEngineCore() = default;

/*!
    Returns the number of results the search engine found.
*/
int QHelpSearchEngineCore::searchResultCount() const
{
    Q_D(const QHelpSearchEngineCore);
    return d->m_indexReader ? d->m_indexReader->searchResultCount() : 0;;
}

/*!
    Returns a list of search results within the range from the index
    specified by \a start to the index specified by \a end.
*/
QList<QHelpSearchResult> QHelpSearchEngineCore::searchResults(int start, int end) const
{
    Q_D(const QHelpSearchEngineCore);
    return d->m_indexReader ? d->m_indexReader->searchResults(start, end)
                            : QList<QHelpSearchResult>();
}

/*!
    Returns the phrase that was last searched for.
*/
QString QHelpSearchEngineCore::searchInput() const
{
    Q_D(const QHelpSearchEngineCore);
    return d->m_searchInput;
}

/*!
    Forces the search engine to reindex all documentation files.
*/
void QHelpSearchEngineCore::reindexDocumentation()
{
    Q_D(QHelpSearchEngineCore);
    d->updateIndex(true);
}

/*!
    Stops the indexing process.
*/
void QHelpSearchEngineCore::cancelIndexing()
{
    Q_D(QHelpSearchEngineCore);
    if (d->m_indexWriter)
        d->m_indexWriter->cancelIndexing();
}

/*!
    Stops the search process.
*/
void QHelpSearchEngineCore::cancelSearching()
{
    Q_D(QHelpSearchEngineCore);
    if (d->m_indexReader)
        d->m_indexReader->cancelSearching();
}

/*!
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
void QHelpSearchEngineCore::search(const QString &searchInput)
{
    Q_D(QHelpSearchEngineCore);
    d->search(searchInput);
}

/*!
    \internal
*/
void QHelpSearchEngineCore::scheduleIndexDocumentation()
{
    Q_D(QHelpSearchEngineCore);
    if (d->m_isIndexingScheduled)
        return;

    d->m_isIndexingScheduled = true;
    QTimer::singleShot(0, this, [this] {
        Q_D(QHelpSearchEngineCore);
        d->m_isIndexingScheduled = false;
        d->updateIndex(false);
    });
}

QT_END_NAMESPACE
