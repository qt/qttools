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

#ifndef QHELPSEARCHINDEXREADER_H
#define QHELPSEARCHINDEXREADER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include "qhelpsearchengine.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

class QHelpEngineCore;

namespace fulltextsearch {

class QHelpSearchIndexReader : public QThread
{
    Q_OBJECT

public:
    ~QHelpSearchIndexReader() override;

    void cancelSearching();
    void search(const QString &collectionFile,
                const QString &indexFilesFolder,
                const QString &searchInput,
                bool usesFilterEngine = false);
    int searchResultCount() const;
    QList<QHelpSearchResult> searchResults(int start, int end) const;

signals:
    void searchingStarted();
    void searchingFinished(int searchResultCount);

protected:
    mutable QMutex m_mutex;
    QList<QHelpSearchResult> m_searchResults;
    bool m_cancel = false;
    QString m_collectionFile;
    QString m_searchInput;
    QString m_indexFilesFolder;
    bool m_usesFilterEngine = false;

private:
    void run() override = 0;
};

}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADER_H
