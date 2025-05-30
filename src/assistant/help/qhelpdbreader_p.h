// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHELPDBREADER_H
#define QHELPDBREADER_H

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

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QByteArray>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

class QSqlQuery;

class QHelpDBReader : public QObject
{
    Q_OBJECT

public:
    class IndexItem
    {
    public:
        IndexItem() = default;
        QString name;
        QString identifier;
        int fileId = 0;
        QString anchor;
        QStringList filterAttributes;
    };

    class FileItem
    {
    public:
        FileItem() = default;
        QString name;
        QString title;
        QStringList filterAttributes;
    };

    class ContentsItem
    {
    public:
        ContentsItem() = default;
        QByteArray data;
        QStringList filterAttributes;
    };

    class IndexTable
    {
    public:
        QList<IndexItem> indexItems;
        QList<FileItem> fileItems;
        QList<ContentsItem> contentsItems;
        QStringList usedFilterAttributes;
    };

    QHelpDBReader(const QString &dbName);
    QHelpDBReader(const QString &dbName, const QString &uniqueId,
        QObject *parent);
    ~QHelpDBReader();

    bool init();

    QString namespaceName() const;
    QString virtualFolder() const;
    QString version() const;
    IndexTable indexTable() const;
    QList<QStringList> filterAttributeSets() const;
    QMultiMap<QString, QByteArray> filesData(const QStringList &filterAttributes,
                                             const QString &extensionFilter = QString()) const;
    QByteArray fileData(const QString &virtualFolder,
        const QString &filePath) const;

    QStringList customFilters() const;
    QStringList filterAttributes(const QString &filterName = QString()) const;

    QVariant metaData(const QString &name) const;

private:
    QString quote(const QString &string) const;
    bool initDB();
    QString qtVersionHeuristic() const;

    bool m_initDone = false;
    QString m_dbName;
    QString m_uniqueId;
    QString m_error;
    QSqlQuery *m_query = nullptr;
    mutable QString m_namespace;
};

QT_END_NAMESPACE

#endif
