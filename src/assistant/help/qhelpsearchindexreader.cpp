// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpsearchindexreader_p.h"
#include "qhelpenginecore.h"
#include "qhelpfilterengine.h"

#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace fulltextsearch {

class Reader
{
public:
    void setIndexPath(const QString &path)
    {
        m_indexPath = path;
        m_namespaceAttributes.clear();
        m_filterEngineNamespaceList.clear();
        m_useFilterEngine = false;
    }
    void addNamespaceAttributes(const QString &namespaceName, const QStringList &attributes)
    {
        m_namespaceAttributes.insert(namespaceName, attributes);
    }
    void setFilterEngineNamespaceList(const QStringList &namespaceList)
    {
        m_useFilterEngine = true;
        m_filterEngineNamespaceList = namespaceList;
    }

    void searchInDB(const QString &term);
    QList<QHelpSearchResult> searchResults() const { return m_searchResults; }

private:
    QList<QHelpSearchResult> queryTable(const QSqlDatabase &db, const QString &tableName,
                                        const QString &searchInput) const;

    QMultiMap<QString, QStringList> m_namespaceAttributes;
    QStringList m_filterEngineNamespaceList;
    QList<QHelpSearchResult> m_searchResults;
    QString m_indexPath;
    bool m_useFilterEngine = false;
};

static QString namespacePlaceholders(const QMultiMap<QString, QStringList> &namespaces)
{
    QString placeholders;
    const auto &namespaceList = namespaces.uniqueKeys();
    bool firstNS = true;
    for (const QString &ns : namespaceList) {
        if (firstNS)
            firstNS = false;
        else
            placeholders += " OR "_L1;
        placeholders += "(namespace = ?"_L1;

        const QList<QStringList> &attributeSets = namespaces.values(ns);
        bool firstAS = true;
        for (const QStringList &attributeSet : attributeSets) {
            if (!attributeSet.isEmpty()) {
                if (firstAS) {
                    firstAS = false;
                    placeholders += " AND ("_L1;
                } else {
                    placeholders += " OR "_L1;
                }
                placeholders += "attributes = ?"_L1;
            }
        }
        if (!firstAS)
            placeholders += u')'; // close "AND ("
        placeholders += u')';
    }
    return placeholders;
}

static void bindNamespacesAndAttributes(QSqlQuery *query,
                                        const QMultiMap<QString, QStringList> &namespaces)
{
    const auto &namespaceList = namespaces.uniqueKeys();
    for (const QString &ns : namespaceList) {
        query->addBindValue(ns);

        const QList<QStringList> &attributeSets = namespaces.values(ns);
        for (const QStringList &attributeSet : attributeSets) {
            if (!attributeSet.isEmpty())
                query->addBindValue(attributeSet.join(u'|'));
        }
    }
}

static QString namespacePlaceholders(const QStringList &namespaceList)
{
    QString placeholders;
    bool firstNS = true;
    for (int i = namespaceList.size(); i; --i) {
        if (firstNS)
            firstNS = false;
        else
            placeholders += " OR "_L1;
        placeholders += "namespace = ?"_L1;
    }
    return placeholders;
}

static void bindNamespacesAndAttributes(QSqlQuery *query, const QStringList &namespaceList)
{
    for (const QString &ns : namespaceList)
        query->addBindValue(ns);
}

QList<QHelpSearchResult> Reader::queryTable(const QSqlDatabase &db, const QString &tableName,
                                            const QString &searchInput) const
{
    const QString nsPlaceholders = m_useFilterEngine
            ? namespacePlaceholders(m_filterEngineNamespaceList)
            : namespacePlaceholders(m_namespaceAttributes);
    QSqlQuery query(db);
    query.prepare("SELECT url, title, snippet("_L1 + tableName +
                  ", -1, '<b>', '</b>', '...', '10') FROM "_L1 + tableName +
                  " WHERE ("_L1 + nsPlaceholders +
                  ") AND "_L1 + tableName +
                  " MATCH ? ORDER BY rank"_L1);
    m_useFilterEngine
            ? bindNamespacesAndAttributes(&query, m_filterEngineNamespaceList)
            : bindNamespacesAndAttributes(&query, m_namespaceAttributes);
    query.addBindValue(searchInput);
    query.exec();

    QList<QHelpSearchResult> results;

    while (query.next()) {
        const QString &url = query.value("url"_L1).toString();
        const QString &title = query.value("title"_L1).toString();
        const QString &snippet = query.value(2).toString();
        results.append(QHelpSearchResult(url, title, snippet));
    }
    return results;
}

void Reader::searchInDB(const QString &searchInput)
{
    const QString &uniqueId = QHelpGlobal::uniquifyConnectionName("QHelpReader"_L1, this);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"_L1, uniqueId);
        db.setConnectOptions("QSQLITE_OPEN_READONLY"_L1);
        db.setDatabaseName(m_indexPath + "/fts"_L1);

        if (db.open()) {
            const QList<QHelpSearchResult> titleResults = queryTable(db, "titles"_L1, searchInput);
            const QList<QHelpSearchResult> contentResults =
                    queryTable(db, "contents"_L1, searchInput);

            // merge results form title and contents searches
            m_searchResults.clear();
            QSet<QUrl> urls;
            for (const QHelpSearchResult &result : titleResults) {
                const auto size = urls.size();
                urls.insert(result.url());
                if (size != urls.size()) // insertion took place
                    m_searchResults.append(result);
            }
            for (const QHelpSearchResult &result : contentResults) {
                const auto size = urls.size();
                urls.insert(result.url());
                if (size != urls.size()) // insertion took place
                    m_searchResults.append(result);
            }
        }
    }
    QSqlDatabase::removeDatabase(uniqueId);
}

static bool attributesMatchFilter(const QStringList &attributes, const QStringList &filter)
{
    for (const QString &attribute : filter) {
        if (!attributes.contains(attribute, Qt::CaseInsensitive))
            return false;
    }
    return true;
}

QHelpSearchIndexReader::~QHelpSearchIndexReader()
{
    cancelSearching();
    wait();
}

void QHelpSearchIndexReader::cancelSearching()
{
    QMutexLocker lock(&m_mutex);
    m_cancel = true;
}

void QHelpSearchIndexReader::search(const QString &collectionFile, const QString &indexFilesFolder,
                                    const QString &searchInput, bool usesFilterEngine)
{
    wait();

    m_searchResults.clear();
    m_cancel = false;
    m_searchInput = searchInput;
    m_collectionFile = collectionFile;
    m_indexFilesFolder = indexFilesFolder;
    m_usesFilterEngine = usesFilterEngine;

    start(QThread::NormalPriority);
}

int QHelpSearchIndexReader::searchResultCount() const
{
    QMutexLocker lock(&m_mutex);
    return m_searchResults.size();
}

QList<QHelpSearchResult> QHelpSearchIndexReader::searchResults(int start, int end) const
{
    QMutexLocker lock(&m_mutex);
    return m_searchResults.mid(start, end - start);
}

void QHelpSearchIndexReader::run()
{
    QMutexLocker lock(&m_mutex);

    if (m_cancel)
        return;

    const QString searchInput = m_searchInput;
    const QString collectionFile = m_collectionFile;
    const QString indexPath = m_indexFilesFolder;
    const bool usesFilterEngine = m_usesFilterEngine;

    lock.unlock();

    QHelpEngineCore engine(collectionFile, nullptr);
    if (!engine.setupData())
        return;

    emit searchingStarted();

    // setup the reader
    Reader reader;
    reader.setIndexPath(indexPath);

    if (usesFilterEngine) {
        reader.setFilterEngineNamespaceList(
                engine.filterEngine()->namespacesForFilter(engine.filterEngine()->activeFilter()));
    } else {
        const QStringList &registeredDocs = engine.registeredDocumentations();
        const QStringList &currentFilter = engine.filterAttributes(engine.currentFilter());

        for (const QString &namespaceName : registeredDocs) {
            const QList<QStringList> &attributeSets =
                    engine.filterAttributeSets(namespaceName);

            for (const QStringList &attributes : attributeSets) {
                if (attributesMatchFilter(attributes, currentFilter))
                    reader.addNamespaceAttributes(namespaceName, attributes);
            }
        }
    }

    lock.relock();
    if (m_cancel) {
        lock.unlock();
        emit searchingFinished();
        return;
    }
    m_searchResults.clear();
    lock.unlock();

    reader.searchInDB(searchInput);    // TODO: should this be interruptible as well ???

    lock.relock();
    m_searchResults = reader.searchResults();
    lock.unlock();

    emit searchingFinished();
}

} // namespace fulltextsearch

QT_END_NAMESPACE
