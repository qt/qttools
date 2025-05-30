// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpsearchindexreader_p.h"

QT_BEGIN_NAMESPACE

namespace fulltextsearch {

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

QList<QHelpSearchResult> QHelpSearchIndexReader::searchResults(int start,
                                                                 int end) const
{
    QMutexLocker lock(&m_mutex);
    return m_searchResults.mid(start, end - start);
}


}   // namespace fulltextsearch

QT_END_NAMESPACE
