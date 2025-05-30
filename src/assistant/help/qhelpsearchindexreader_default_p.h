// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPSEARCHINDEXREADERDEFAULT_H
#define QHELPSEARCHINDEXREADERDEFAULT_H

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

#include "qhelpsearchindexreader_p.h"

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace qt {

class Reader
{
public:
    void setIndexPath(const QString &path);
    void addNamespaceAttributes(const QString &namespaceName, const QStringList &attributes);
    void setFilterEngineNamespaceList(const QStringList &namespaceList);

    void searchInDB(const QString &term);
    QList<QHelpSearchResult> searchResults() const;

private:
    QList<QHelpSearchResult> queryTable(const QSqlDatabase &db,
                                          const QString &tableName,
                                          const QString &searchInput) const;

    QMultiMap<QString, QStringList> m_namespaceAttributes;
    QStringList m_filterEngineNamespaceList;
    QList<QHelpSearchResult> m_searchResults;
    QString m_indexPath;
    bool m_useFilterEngine = false;
};


class QHelpSearchIndexReaderDefault : public QHelpSearchIndexReader
{
    Q_OBJECT

private:
    void run() override;

private:
    Reader m_reader;
};

}   // namespace std
}   // namespace fulltextsearch

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXREADERDEFAULT_H
