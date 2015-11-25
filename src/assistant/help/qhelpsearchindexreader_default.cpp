/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qhelpenginecore.h"
#include "qhelpsearchindexreader_default_p.h"

#include <QtCore/QSet>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace qt {

void Reader::setIndexPath(const QString &path)
{
    m_indexPath = path;
    m_namespaces.clear();
}

void Reader::addNamespace(const QString &namespaceName, const QStringList &attributes)
{
    m_namespaces.insert(namespaceName, attributes);
}

static QString namespacePlaceholders(const QMultiMap<QString, QStringList> &namespaces)
{
    QString placeholders;
    const auto namespaceList = namespaces.uniqueKeys();
    bool firstNS = true;
    for (const QString &ns : namespaceList) {
        if (firstNS)
            firstNS = false;
        else
            placeholders += QLatin1String(" OR ");
        placeholders += QLatin1String("(namespace = ?");

        const QList<QStringList> &attributeSets = namespaces.values(ns);
        bool firstAS = true;
        for (const QStringList &attributeSet : attributeSets) {
            if (!attributeSet.isEmpty()) {
                if (firstAS) {
                    firstAS = false;
                    placeholders += QLatin1String(" AND (");
                } else {
                    placeholders += QLatin1String(" OR ");
                }
                placeholders += QLatin1String("attributes = ?");
            }
        }
        if (!firstAS)
            placeholders += QLatin1Char(')'); // close "AND ("
        placeholders += QLatin1Char(')');
    }
    return placeholders;
}

static void bindNamespacesAndAttributes(QSqlQuery *query, const QMultiMap<QString, QStringList> &namespaces)
{
    QString placeholders;
    const auto namespaceList = namespaces.uniqueKeys();
    for (const QString &ns : namespaceList) {
        query->addBindValue(ns);

        const QList<QStringList> &attributeSets = namespaces.values(ns);
        for (const QStringList &attributeSet : attributeSets) {
            if (!attributeSet.isEmpty())
                query->addBindValue(attributeSet.join(QLatin1Char('|')));
        }
    }
}

QVector<DocumentInfo> Reader::queryTable(const QSqlDatabase &db,
                                         const QString &tableName,
                                         const QString &term) const
{
    const QString nsPlaceholders = namespacePlaceholders(m_namespaces);
    QSqlQuery query(db);
    query.prepare(QLatin1String("SELECT url, title FROM ") + tableName +
                  QLatin1String(" WHERE (") + nsPlaceholders +
                  QLatin1String(") AND ") + tableName +
                  QLatin1String(" MATCH ? ORDER BY rank"));
    bindNamespacesAndAttributes(&query, m_namespaces);
    query.addBindValue(term);
    query.exec();

    QVector<DocumentInfo> documentsInfo;

    while (query.next()) {
        const QString url = query.value(QLatin1String("url")).toString();
        const QString title = query.value(QLatin1String("title")).toString();
        documentsInfo.append(DocumentInfo(title, url));
    }

    return documentsInfo;
}

void Reader::searchInDB(const QString &term)
{
    const QString &uniqueId = QHelpGlobal::uniquifyConnectionName(QLatin1String("QHelpReader"), this);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), uniqueId);
        db.setConnectOptions(QLatin1String("QSQLITE_OPEN_READONLY"));
        db.setDatabaseName(m_indexPath + QLatin1String("/fts"));
        if (db.open()) {
            const QVector<DocumentInfo> titlesInfo = queryTable(db,
                                        QLatin1String("titles"), term);
            const QVector<DocumentInfo> contentsInfo = queryTable(db,
                                        QLatin1String("contents"), term);

            // merge results form title and contents searches
            m_hits = QVector<DocumentInfo>();

            QSet<QString> urls;
            for (const DocumentInfo &info : titlesInfo) {
                const QString url = info.m_url;
                if (!urls.contains(url)) {
                    m_hits.append(info);
                    urls.insert(url);
                }
            }

            for (const DocumentInfo &info : contentsInfo) {
                const QString url = info.m_url;
                if (!urls.contains(url)) {
                    m_hits.append(info);
                    urls.insert(url);
                }
            }
        }
    }
    QSqlDatabase::removeDatabase(uniqueId);
}

QVector<DocumentInfo> Reader::hits() const
{
    return m_hits;
}

static bool attributesMatchFilter(const QStringList &attributes,
                                  const QStringList &filter)
{
    for (const QString &attribute : filter) {
        if (!attributes.contains(attribute, Qt::CaseInsensitive))
            return false;
    }

    return true;
}

void QHelpSearchIndexReaderDefault::run()
{
    QMutexLocker lock(&m_mutex);

    if (m_cancel)
        return;

    const QList<QHelpSearchQuery> queryList = m_query;
    const QString collectionFile = m_collectionFile;
    const QString indexPath = m_indexFilesFolder;

    lock.unlock();

    QString queryTerm;

    // TODO: we may want to translate the QHelpSearchQuery list into
    // simple phrase using AND, OR, NOT or quotes in order to
    // keep working the public interface of QHelpSearchQuery
    for (const QHelpSearchQuery &query : queryList) {
        if (query.fieldName == QHelpSearchQuery::DEFAULT) {
            queryTerm = query.wordList.at(0);
            break;
        }
    }

    if (queryTerm.isEmpty())
        return;

    QHelpEngineCore engine(collectionFile, 0);
    if (!engine.setupData())
        return;

    const QStringList &registeredDocs = engine.registeredDocumentations();

    emit searchingStarted();

    const QStringList &currentFilter = engine.filterAttributes(engine.currentFilter());

    // setup the reader
    m_reader.setIndexPath(indexPath);
    for (const QString &namespaceName : registeredDocs) {
        const QList<QStringList> &attributeSets =
            engine.filterAttributeSets(namespaceName);

        for (const QStringList &attributes : attributeSets) {
            if (attributesMatchFilter(attributes, currentFilter)) {
                m_reader.addNamespace(namespaceName, attributes);
            }
        }
    }


    lock.relock();
    if (m_cancel) {
        emit searchingFinished(0);   // TODO: check this, speed issue while locking???
        return;
    }
    lock.unlock();

    m_hitList.clear();
    m_reader.searchInDB(queryTerm);    // TODO: should this be interruptible as well ???

    for (const DocumentInfo &docInfo : m_reader.hits())
        m_hitList.append(qMakePair(docInfo.m_url, docInfo.m_title));

    emit searchingFinished(m_hitList.count());
}

}   // namespace std
}   // namespace fulltextsearch

QT_END_NAMESPACE
