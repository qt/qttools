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

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/QFileInfo>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace qt {

namespace {
    QStringList split( const QString &str )
    {
        QStringList lst;
        int j = 0;
        int i = str.indexOf(QLatin1Char('*'), j );

        if (str.startsWith(QLatin1String("*")))
            lst << QLatin1String("*");

        while ( i != -1 ) {
            if ( i > j && i <= (int)str.length() ) {
                lst << str.mid( j, i - j );
                lst << QLatin1String("*");
            }
            j = i + 1;
            i = str.indexOf(QLatin1Char('*'), j );
        }

        const int l = str.length() - 1;
        if ( str.mid( j, l - j + 1 ).length() > 0 )
            lst << str.mid( j, l - j + 1 );

        return lst;
    }
}


Reader::Reader()
    : indexPath(QString())
    , indexFile(QString())
    , documentFile(QString())
{
    termList.clear();
    indexTable.clear();
    searchIndexTable.clear();
}

Reader::~Reader()
{
    reset();
    searchIndexTable.clear();
}

bool Reader::readIndex()
{
    if (indexTable.contains(indexFile))
        return true;

    QFile idxFile(indexFile);
    if (!idxFile.open(QFile::ReadOnly))
        return false;

    QString key;
    int numOfDocs;
    EntryTable entryTable;
    QVector<Document> docs;
    QDataStream dictStream(&idxFile);
    while (!dictStream.atEnd()) {
        dictStream >> key;
        dictStream >> numOfDocs;
        docs.resize(numOfDocs);
        dictStream >> docs;
        entryTable.insert(key, new Entry(docs));
    }
    idxFile.close();

    if (entryTable.isEmpty())
        return false;

    QFile docFile(documentFile);
    if (!docFile.open(QFile::ReadOnly))
        return false;

    QString title, url;
    DocumentList documentList;
    QDataStream docStream(&docFile);
    while (!docStream.atEnd()) {
        docStream >> title;
        docStream >> url;
        documentList.append(QStringList(title) << url);
    }
    docFile.close();

    if (documentList.isEmpty()) {
        cleanupIndex(entryTable);
        return false;
    }

    indexTable.insert(indexFile, Index(entryTable, documentList));
    return true;
}

bool Reader::initCheck() const
{
    return !searchIndexTable.isEmpty();
}

void Reader::setIndexPath(const QString &path)
{
    indexPath = path;
}

void Reader::filterFilesForAttributes(const QStringList &attributes)
{
    searchIndexTable.clear();
    for (auto it = indexTable.cbegin(), end = indexTable.cend(); it != end; ++it) {
        const QString fileName = it.key();
        bool containsAll = true;
        const QStringList split = fileName.split(QLatin1String("@"));
        for (const QString &attribute : attributes) {
            if (!split.contains(attribute, Qt::CaseInsensitive)) {
                containsAll = false;
                break;
            }
        }

        if (containsAll)
            searchIndexTable.insert(fileName, it.value());
    }
}

void Reader::setIndexFile(const QString &namespaceName, const QString &attributes)
{
    const QString extension = namespaceName + QLatin1String("@") + attributes;
    indexFile = indexPath + QLatin1String("/indexdb40.") + extension;
    documentFile = indexPath + QLatin1String("/indexdoc40.") + extension;
}

bool Reader::splitSearchTerm(const QString &searchTerm, QStringList *terms,
                                  QStringList *termSeq, QStringList *seqWords)
{
    QString term = searchTerm;

    term = term.simplified();
    term = term.replace(QLatin1Char('\''), QLatin1Char('"'));
    term = term.replace(QLatin1Char('`'), QLatin1Char('"'));
    term.remove(QLatin1Char('-'));
    term = term.replace(QRegExp(QLatin1String("\\s[\\S]?\\s")), QLatin1String(" "));

    *terms = term.split(QLatin1Char(' '));
    for (QString &str : *terms) {
        str = str.simplified();
        str = str.toLower();
        str.remove(QLatin1Char('"'));
    }

    if (term.contains(QLatin1Char('"'))) {
        if ((term.count(QLatin1Char('"')))%2 == 0) {
            int beg = 0;
            int end = 0;
            QString s;
            beg = term.indexOf(QLatin1Char('"'), beg);
            while (beg != -1) {
                beg++;
                end = term.indexOf(QLatin1Char('"'), beg);
                s = term.mid(beg, end - beg);
                s = s.toLower();
                s = s.simplified();
                if (s.contains(QLatin1Char('*'))) {
                    qWarning("Full Text Search, using a wildcard within phrases is not allowed.");
                    return false;
                }
                *seqWords += s.split(QLatin1Char(' '));
                *termSeq << s;
                beg = term.indexOf(QLatin1Char('\"'), end + 1);
            }
        } else {
            qWarning("Full Text Search, the closing quotation mark is missing.");
            return false;
        }
    }

    return true;
}

void Reader::searchInIndex(const QStringList &terms)
{
    for (const QString &term : terms) {
        QVector<Document> documents;

        for (auto it = searchIndexTable.cbegin(), end = searchIndexTable.cend(); it != end; ++it) {
            const EntryTable &entryTable = it.value().first;
            const DocumentList &documentList = it.value().second;

            if (term.contains(QLatin1Char('*')))
                documents = setupDummyTerm(getWildcardTerms(term, entryTable), entryTable);
            else if (entryTable.value(term))
                documents = entryTable.value(term)->documents;
            else
                continue;

            if (!documents.isEmpty()) {
                DocumentInfo info;
                QVector<DocumentInfo> documentsInfo;
                for (const Document &doc : qAsConst(documents)) {
                    info.docNumber = doc.docNumber;
                    info.frequency = doc.frequency;
                    info.documentUrl = documentList.at(doc.docNumber).at(1);
                    info.documentTitle = documentList.at(doc.docNumber).at(0);
                    documentsInfo.append(info);
                }

                const auto tit = std::find_if(termList.begin(), termList.end(),
                                              [term] (TermInfo &info) { return info.term == term; } );
                if (tit != termList.end()) {
                    tit->documents += documentsInfo;
                    tit->frequency += documentsInfo.count();
                } else {
                    termList.append(TermInfo(term, documentsInfo.count(), documentsInfo));
                }
            }
        }
    }
    ::std::sort(termList.begin(), termList.end());
}

QVector<DocumentInfo> Reader::hits()
{
    QVector<DocumentInfo> documents;
    if (!termList.count())
        return documents;

    documents = termList.takeFirst().documents;
    for (const TermInfo &info : qAsConst(termList)) {
        const QVector<DocumentInfo> &docs = info.documents;

        for (auto minDoc_it = documents.begin(); minDoc_it != documents.end(); ) {
            const int docNumber = minDoc_it->docNumber;
            const auto doc_it = std::find_if(docs.cbegin(), docs.cend(),
                                             [docNumber] (const DocumentInfo &docInfo)
                                        { return docInfo.docNumber == docNumber; });
            if (doc_it == docs.cend()) {
                minDoc_it = documents.erase(minDoc_it);
            } else {
                minDoc_it->frequency = doc_it->frequency;
                ++minDoc_it;
            }
        }
    }

    ::std::sort(documents.begin(), documents.end());
    return documents;
}

bool Reader::searchForPattern(const QStringList &patterns, const QStringList &words,
                                   const QByteArray &data)
{
    if (data.isEmpty())
        return false;

    for (auto mit = miniIndex.cbegin(); mit != miniIndex.cend(); ++mit)
        delete mit.value();

    miniIndex.clear();

    wordNum = 3;
    for (const QString &word : words)
        miniIndex.insert(word, new PosEntry(0));

    QTextStream s(data);
    const QString text = s.readAll();
    bool valid = true;
    const QChar *buf = text.unicode();
    QChar str[64];
    QChar c = buf[0];
    int j = 0;
    int i = 0;
    while ( j < text.length() ) {
        if ( c == QLatin1Char('<') || c == QLatin1Char('&') ) {
            valid = false;
            if ( i > 1 )
                buildMiniIndex( QString(str,i) );
            i = 0;
            c = buf[++j];
            continue;
        }
        if ( ( c == QLatin1Char('>') || c == QLatin1Char(';') ) && !valid ) {
            valid = true;
            c = buf[++j];
            continue;
        }
        if ( !valid ) {
            c = buf[++j];
            continue;
        }
        if ( ( c.isLetterOrNumber() || c == QLatin1Char('_') ) && i < 63 ) {
            str[i] = c.toLower();
            ++i;
        } else {
            if ( i > 1 )
                buildMiniIndex( QString(str,i) );
            i = 0;
        }
        c = buf[++j];
    }
    if ( i > 1 )
        buildMiniIndex( QString(str,i) );

    QList<uint> a;
    for (const QString &pat : patterns) {
        const QStringList wordList = pat.split(QLatin1Char(' '));
        a = miniIndex.value(wordList.at(0))->positions;
        for (int j = 1; j < wordList.count(); ++j) {
            const QList<uint> &b = miniIndex.value(wordList.at(j))->positions;

            for (auto aIt = a.begin(); aIt != a.end(); ) {
                if (b.contains(*aIt + 1)) {
                    (*aIt)++;
                    ++aIt;
                } else {
                    aIt = a.erase(aIt);
                }
            }
        }
    }
    if ( a.count() )
        return true;
    return false;
}

QVector<Document> Reader::setupDummyTerm(const QStringList &terms,
                                              const EntryTable &entryTable)
{
    QList<Term> termList;
    for (const QString &term : terms) {
        Entry *e = entryTable.value(term);
        if (e)
            termList.append(Term(term, e->documents.count(), e->documents ) );
    }
    QVector<Document> maxList(0);
    if ( !termList.count() )
        return maxList;
    ::std::sort(termList.begin(), termList.end());

    maxList = termList.takeLast().documents;
    for (const Term &term : qAsConst(termList)) {
        for (const Document &doc : term.documents) {
            if (maxList.indexOf(doc) == -1)
                maxList.append(doc);
        }
    }
    return maxList;
}

QStringList Reader::getWildcardTerms(const QString &termStr,
                                     const EntryTable &entryTable)
{
    QStringList list;
    const QStringList terms = split(termStr);

    for (auto it = entryTable.cbegin(), end = entryTable.cend(); it != end; ++it) {
        int index = 0;
        bool found = false;
        const QString text(it.key());
        for (auto termIt = terms.cbegin(), termItEnd = terms.cend(); termIt != termItEnd; ++termIt) {
            const QString &term = *termIt;
            if (term == QLatin1String("*")) {
                found = true;
                continue;
            }
            if (termIt == terms.cbegin() && term.at(0) != text.at(0)) {
                found = false;
                break;
            }
            index = text.indexOf(term, index);
            if (term == terms.last() && index != text.length() - 1) {
                index = text.lastIndexOf(term);
                if (index != text.length() - term.length()) {
                    found = false;
                    break;
                }
            }
            if (index != -1) {
                found = true;
                index += term.length();
                continue;
            } else {
                found = false;
                break;
            }
        }
        if (found)
            list << text;
    }

    return list;
}

void Reader::buildMiniIndex(const QString &string)
{
    if (miniIndex[string])
        miniIndex[string]->positions.append(wordNum);
    ++wordNum;
}

void Reader::reset()
{
    for (auto it = indexTable.begin(), end = indexTable.end(); it != end; ++it) {
        cleanupIndex(it.value().first);
        it.value().second.clear();
    }
}

void Reader::cleanupIndex(EntryTable &entryTable)
{
    for (auto it = entryTable.cbegin(), end = entryTable.cend(); it != end; ++it)
            delete it.value();

    entryTable.clear();
}


QHelpSearchIndexReaderDefault::QHelpSearchIndexReaderDefault()
    : QHelpSearchIndexReader()
{
    // nothing todo
}

QHelpSearchIndexReaderDefault::~QHelpSearchIndexReaderDefault()
{
}

void QHelpSearchIndexReaderDefault::run()
{
    mutex.lock();

    if (m_cancel) {
        mutex.unlock();
        return;
    }

    const QList<QHelpSearchQuery> &queryList = this->m_query;
    const QLatin1String key("DefaultSearchNamespaces");
    const QString collectionFile(this->m_collectionFile);
    const QString indexPath = m_indexFilesFolder;

    mutex.unlock();

    QString queryTerm;
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

    const QStringList registeredDocs = engine.registeredDocumentations();
    const QStringList indexedNamespaces = engine.customValue(key).toString().
        split(QLatin1String("|"), QString::SkipEmptyParts);

    emit searchingStarted();

    // setup the reader
    m_reader.setIndexPath(indexPath);
    for (const QString &namespaceName : registeredDocs) {
        mutex.lock();
        if (m_cancel) {
            mutex.unlock();
            searchingFinished(0);   // TODO: check this ???
            return;
        }
        mutex.unlock();

        const QList<QStringList> attributeSets =
            engine.filterAttributeSets(namespaceName);

        for (const QStringList &attributes : attributeSets) {
            // read all index files
            m_reader.setIndexFile(namespaceName, attributes.join(QLatin1String("@")));
            if (!m_reader.readIndex()) {
                qWarning("Full Text Search, could not read file for namespace: %s.",
                    namespaceName.toUtf8().constData());
            }
        }
    }

    // get the current filter attributes and minimize the index files table
    m_reader.filterFilesForAttributes(engine.filterAttributes(engine.currentFilter()));

    hitList.clear();
    QStringList terms, termSeq, seqWords;
    if (m_reader.initCheck() && // check if we could read anything
        m_reader.splitSearchTerm(queryTerm, &terms, &termSeq, &seqWords) ) {

        // search for term(s)
        m_reader.searchInIndex(terms);    // TODO: should this be interruptible as well ???

        const QVector<DocumentInfo> hits = m_reader.hits();
        if (!hits.isEmpty()) {
            if (termSeq.isEmpty()) {
                for (const DocumentInfo &docInfo : hits) {
                    mutex.lock();
                    if (m_cancel) {
                        mutex.unlock();
                        searchingFinished(0);   // TODO: check this, speed issue while locking???
                        return;
                    }
                    mutex.unlock();
                    hitList.append(qMakePair(docInfo.documentTitle, docInfo.documentUrl));
                }
            } else {
                for (const DocumentInfo &docInfo : hits) {
                    mutex.lock();
                    if (m_cancel) {
                        mutex.unlock();
                        searchingFinished(0);   // TODO: check this, speed issue while locking???
                        return;
                    }
                    mutex.unlock();

                    if (m_reader.searchForPattern(termSeq, seqWords, engine.fileData(docInfo.documentUrl))) // TODO: should this be interruptible as well ???
                        hitList.append(qMakePair(docInfo.documentTitle, docInfo.documentUrl));
                }
            }
        }
    }

    emit searchingFinished(hitList.count());
}

}   // namespace std
}   // namespace fulltextsearch

QT_END_NAMESPACE
