/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

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
    return m_searchResults.count();
}

QList<QHelpSearchResult> QHelpSearchIndexReader::searchResults(int start,
                                                                 int end) const
{
    QMutexLocker lock(&m_mutex);
    return m_searchResults.mid(start, end - start);
}


}   // namespace fulltextsearch

QT_END_NAMESPACE
