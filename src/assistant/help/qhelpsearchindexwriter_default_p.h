// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPSEARCHINDEXWRITERDEFAULT_H
#define QHELPSEARCHINDEXWRITERDEFAULT_H

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

#include <QtCore/QMutex>
#include <QtCore/QThread>

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace qt {

class Writer
{
public:
    Q_DISABLE_COPY_MOVE(Writer);

    Writer(const QString &path);
    ~Writer();

    bool tryInit(bool reindex);
    void flush();

    void removeNamespace(const QString &namespaceName);
    bool hasNamespace(const QString &namespaceName);
    void insertDoc(const QString &namespaceName,
                   const QString &attributes,
                   const QString &url,
                   const QString &title,
                   const QString &contents);
    void startTransaction();
    void endTransaction();
private:
    void init(bool reindex);
    bool hasDB();
    void clearLegacyIndex();

    const QString m_dbDir;
    QString m_uniqueId;

    bool m_needOptimize = false;
    QSqlDatabase *m_db = nullptr;
    QVariantList m_namespaces;
    QVariantList m_attributes;
    QVariantList m_urls;
    QVariantList m_titles;
    QVariantList m_contents;
};


class QHelpSearchIndexWriter : public QThread
{
    Q_OBJECT

public:
    QHelpSearchIndexWriter();
    ~QHelpSearchIndexWriter() override;

    void cancelIndexing();
    void updateIndex(const QString &collectionFile,
        const QString &indexFilesFolder, bool reindex);

signals:
    void indexingStarted();
    void indexingFinished();

private:
    void run() override;

private:
    QMutex m_mutex;

    bool m_cancel;
    bool m_reindex;
    QString m_collectionFile;
    QString m_indexFilesFolder;
};

}   // namespace std
}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXWRITERDEFAULT_H
