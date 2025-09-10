// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpdbreader_p.h"
#include "qhelp_global.h"

#include <QtCore/qfile.h>
#include <QtCore/qmap.h>
#include <QtCore/qvariant.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlquery.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QHelpDBReader::QHelpDBReader(const QString &dbName)
    : m_dbName(dbName)
    , m_uniqueId(QHelpGlobal::uniquifyConnectionName("QHelpDBReader"_L1, this))
{}

QHelpDBReader::QHelpDBReader(const QString &dbName, const QString &uniqueId, QObject *parent)
    : QObject(parent)
    , m_dbName(dbName)
    , m_uniqueId(uniqueId)
{}

QHelpDBReader::~QHelpDBReader()
{
    if (m_initDone)
        QSqlDatabase::removeDatabase(m_uniqueId);
}

bool QHelpDBReader::init()
{
    if (m_initDone)
        return true;

    if (!QFile::exists(m_dbName))
        return false;

    if (!initDB()) {
        QSqlDatabase::removeDatabase(m_uniqueId);
        return false;
    }

    m_initDone = true;
    m_query.reset(new QSqlQuery(QSqlDatabase::database(m_uniqueId)));
    return true;
}

bool QHelpDBReader::initDB()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"_L1, m_uniqueId);
    db.setConnectOptions("QSQLITE_OPEN_READONLY"_L1);
    db.setDatabaseName(m_dbName);
    if (!db.open()) {
        /*: The placeholders are: %1 - The name of the database which cannot be opened
                                  %2 - The unique id for the connection
                                  %3 - The actual error string */
        m_error = tr("Cannot open database \"%1\" \"%2\": %3").arg(m_dbName, m_uniqueId, db.lastError().text());
        return false;
    }
    return true;
}

QString QHelpDBReader::namespaceName() const
{
    if (!m_namespace.isEmpty())
        return m_namespace;
    if (m_query) {
        m_query->exec("SELECT Name FROM NamespaceTable"_L1);
        if (m_query->next())
            m_namespace = m_query->value(0).toString();
    }
    return m_namespace;
}

QString QHelpDBReader::virtualFolder() const
{
    if (m_query) {
        m_query->exec("SELECT Name FROM FolderTable WHERE Id=1"_L1);
        if (m_query->next())
            return m_query->value(0).toString();
    }
    return {};
}

QString QHelpDBReader::version() const
{
    const QString versionString = metaData("version"_L1).toString();
    if (versionString.isEmpty())
        return qtVersionHeuristic();
    return versionString;
}

QString QHelpDBReader::qtVersionHeuristic() const
{
    const QString nameSpace = namespaceName();
    if (!nameSpace.startsWith("org.qt-project."_L1))
        return {};

    // We take the namespace tail, starting from the last letter in namespace name.
    // We drop any non digit characters.
    const QChar dot(u'.');
    QString tail;
    for (int i = nameSpace.size(); i > 0; --i) {
        const QChar c = nameSpace.at(i - 1);
        if (c.isDigit() || c == dot)
            tail.prepend(c);

        if (c.isLetter())
            break;
    }

    if (!tail.startsWith(dot) && tail.count(dot) == 1) {
        // The org.qt-project.qtquickcontrols2.5120 case,
        // tail = 2.5120 here. We need to cut "2." here.
        const int dotIndex = tail.indexOf(dot);
        if (dotIndex > 0)
            tail = tail.mid(dotIndex);
    }

    // Drop beginning dots
    while (tail.startsWith(dot))
        tail = tail.mid(1);

    // Drop ending dots
    while (tail.endsWith(dot))
        tail.chop(1);

    if (tail.count(dot) == 0) {
        if (tail.size() > 5)
            return tail;

        // When we have 3 digits, we split it like: ABC -> A.B.C
        // When we have 4 digits, we split it like: ABCD -> A.BC.D
        // When we have 5 digits, we split it like: ABCDE -> A.BC.DE
        const int major = tail.left(1).toInt();
        const int minor = tail.size() == 3
                ? tail.mid(1, 1).toInt() : tail.mid(1, 2).toInt();
        const int patch = tail.size() == 5
                ? tail.right(2).toInt() : tail.right(1).toInt();

        return QString::fromUtf8("%1.%2.%3").arg(major).arg(minor).arg(patch);
    }
    return tail;
}

static bool isAttributeUsed(QSqlQuery *query, const QString &tableName, int attributeId)
{
    query->prepare(QString::fromLatin1("SELECT FilterAttributeId "
                                       "FROM %1 "
                                       "WHERE FilterAttributeId = ? "
                                       "LIMIT 1").arg(tableName));
    query->bindValue(0, attributeId);
    query->exec();
    return query->next(); // if we got a result it means it was used
}

static int filterDataCount(QSqlQuery *query, const QString &tableName)
{
    query->exec(QString::fromLatin1("SELECT COUNT(*) FROM"
                                    "(SELECT DISTINCT * FROM %1)").arg(tableName));
    query->next();
    return query->value(0).toInt();
}

QHelpDBReader::IndexTable QHelpDBReader::indexTable() const
{
    IndexTable table;
    if (!m_query)
        return table;

    QMap<int, QString> attributeIds;
    m_query->exec("SELECT DISTINCT Id, Name FROM FilterAttributeTable ORDER BY Id"_L1);
    while (m_query->next())
        attributeIds.insert(m_query->value(0).toInt(), m_query->value(1).toString());

    // Maybe some are unused and specified erroneously in the named filter only,
    // like it was in case of qtlocation.qch <= qt 5.9
    QList<int> usedAttributeIds;
    for (auto it = attributeIds.cbegin(), end = attributeIds.cend(); it != end; ++it) {
        const int attributeId = it.key();
        if (isAttributeUsed(m_query.get(), "IndexFilterTable"_L1, attributeId)
            || isAttributeUsed(m_query.get(), "ContentsFilterTable"_L1, attributeId)
            || isAttributeUsed(m_query.get(), "FileFilterTable"_L1, attributeId)) {
            usedAttributeIds.append(attributeId);
        }
    }

    bool legacy = false;
    m_query->exec("SELECT * FROM pragma_table_info('IndexTable') WHERE name='ContextName'"_L1);
    if (m_query->next())
        legacy = true;

    const QString identifierColumnName = legacy ? "ContextName"_L1 : "Identifier"_L1;
    const int usedAttributeCount = usedAttributeIds.size();

    QMap<int, IndexItem> idToIndexItem;
    m_query->exec(QString::fromLatin1("SELECT Name, %1, FileId, Anchor, Id "
                                      "FROM IndexTable "
                                      "ORDER BY Id").arg(identifierColumnName));
    while (m_query->next()) {
        IndexItem indexItem;
        indexItem.name       = m_query->value(0).toString();
        indexItem.identifier = m_query->value(1).toString();
        indexItem.fileId     = m_query->value(2).toInt();
        indexItem.anchor     = m_query->value(3).toString();
        const int indexId    = m_query->value(4).toInt();

        idToIndexItem.insert(indexId, indexItem);
    }

    QMap<int, FileItem> idToFileItem;
    QMap<int, int> originalFileIdToNewFileId;

    int filesCount = 0;
    m_query->exec(
        "SELECT "
            "FileNameTable.FileId, "
            "FileNameTable.Name, "
            "FileNameTable.Title "
        "FROM FileNameTable, FolderTable "
        "WHERE FileNameTable.FolderId = FolderTable.Id "
        "ORDER BY FileId"_L1);
    while (m_query->next()) {
        const int fileId = m_query->value(0).toInt();
        FileItem fileItem;
        fileItem.name   = m_query->value(1).toString();
        fileItem.title  = m_query->value(2).toString();

        idToFileItem.insert(fileId, fileItem);
        originalFileIdToNewFileId.insert(fileId, filesCount);
        ++filesCount;
    }

    QMap<int, ContentsItem> idToContentsItem;

    m_query->exec("SELECT Data, Id FROM ContentsTable ORDER BY Id"_L1);
    while (m_query->next()) {
        ContentsItem contentsItem;
        contentsItem.data    = m_query->value(0).toByteArray();
        const int contentsId = m_query->value(1).toInt();

        idToContentsItem.insert(contentsId, contentsItem);
    }

    bool optimized = true;

    if (usedAttributeCount) {
        // May optimize only when all usedAttributes are attached to every
        // index and file. It means the number of rows in the
        // IndexTable multiplied by number of used attributes
        // must equal the number of rows inside IndexFilterTable
        // (yes, we have a combinatorial explosion of data in IndexFilterTable,
        // which we want to optimize). The same with FileNameTable and
        // FileFilterTable.

        const bool mayOptimizeIndexTable = filterDataCount(m_query.get(), "IndexFilterTable"_L1)
                == idToIndexItem.size() * usedAttributeCount;
        const bool mayOptimizeFileTable = filterDataCount(m_query.get(), "FileFilterTable"_L1)
                == idToFileItem.size() * usedAttributeCount;
        const bool mayOptimizeContentsTable =
                filterDataCount(m_query.get(), "ContentsFilterTable"_L1)
                == idToContentsItem.size() * usedAttributeCount;
        optimized = mayOptimizeIndexTable && mayOptimizeFileTable && mayOptimizeContentsTable;

        if (!optimized) {
            m_query->exec(
                "SELECT "
                    "IndexFilterTable.IndexId, "
                    "FilterAttributeTable.Name "
                "FROM "
                    "IndexFilterTable, "
                    "FilterAttributeTable "
                "WHERE "
                    "IndexFilterTable.FilterAttributeId = FilterAttributeTable.Id"_L1);
            while (m_query->next()) {
                const int indexId = m_query->value(0).toInt();
                auto it = idToIndexItem.find(indexId);
                if (it != idToIndexItem.end())
                    it.value().filterAttributes.append(m_query->value(1).toString());
            }

            m_query->exec(
                "SELECT "
                    "FileFilterTable.FileId, "
                    "FilterAttributeTable.Name "
                "FROM "
                    "FileFilterTable, "
                    "FilterAttributeTable "
                "WHERE "
                    "FileFilterTable.FilterAttributeId = FilterAttributeTable.Id"_L1);
            while (m_query->next()) {
                const int fileId = m_query->value(0).toInt();
                auto it = idToFileItem.find(fileId);
                if (it != idToFileItem.end())
                    it.value().filterAttributes.append(m_query->value(1).toString());
            }

            m_query->exec(
                "SELECT "
                    "ContentsFilterTable.ContentsId, "
                    "FilterAttributeTable.Name "
                "FROM "
                    "ContentsFilterTable, "
                    "FilterAttributeTable "
                "WHERE "
                    "ContentsFilterTable.FilterAttributeId = FilterAttributeTable.Id"_L1);
            while (m_query->next()) {
                const int contentsId = m_query->value(0).toInt();
                auto it = idToContentsItem.find(contentsId);
                if (it != idToContentsItem.end())
                    it.value().filterAttributes.append(m_query->value(1).toString());
            }
        }
    }

    // reindex fileId references
    for (auto it = idToIndexItem.cbegin(), end = idToIndexItem.cend(); it != end; ++it) {
        IndexItem item = it.value();
        item.fileId = originalFileIdToNewFileId.value(item.fileId);
        table.indexItems.append(item);
    }

    table.fileItems = idToFileItem.values();
    table.contentsItems = idToContentsItem.values();

    if (optimized) {
        for (int attributeId : usedAttributeIds)
            table.usedFilterAttributes.append(attributeIds.value(attributeId));
    }
    return table;
}

QList<QStringList> QHelpDBReader::filterAttributeSets() const
{
    QList<QStringList> result;
    if (m_query) {
        m_query->exec(
            "SELECT "
                "FileAttributeSetTable.Id, "
                "FilterAttributeTable.Name "
            "FROM "
                "FileAttributeSetTable, "
                "FilterAttributeTable "
            "WHERE FileAttributeSetTable.FilterAttributeId = FilterAttributeTable.Id "
            "ORDER BY FileAttributeSetTable.Id"_L1);
        int oldId = -1;
        while (m_query->next()) {
            const int id = m_query->value(0).toInt();
            if (id != oldId) {
                result.append(QStringList());
                oldId = id;
            }
            result.last().append(m_query->value(1).toString());
        }
    }
    return result;
}

QByteArray QHelpDBReader::fileData(const QString &virtualFolder,
                                   const QString &filePath) const
{
    QByteArray ba;
    if (virtualFolder.isEmpty() || filePath.isEmpty() || !m_query)
        return ba;

    namespaceName();
    m_query->prepare(
        "SELECT "
            "FileDataTable.Data "
        "FROM "
            "FileDataTable, "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE FileDataTable.Id = FileNameTable.FileId "
        "AND (FileNameTable.Name = ? OR FileNameTable.Name = ?) "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND FolderTable.Name = ? "
        "AND FolderTable.NamespaceId = NamespaceTable.Id "
        "AND NamespaceTable.Name = ?"_L1);
    m_query->bindValue(0, filePath);
    m_query->bindValue(1, QString("./"_L1 + filePath));
    m_query->bindValue(2, virtualFolder);
    m_query->bindValue(3, m_namespace);
    m_query->exec();
    if (m_query->next() && m_query->isValid())
        ba = qUncompress(m_query->value(0).toByteArray());
    return ba;
}

QStringList QHelpDBReader::customFilters() const
{
    QStringList lst;
    if (m_query) {
        m_query->exec("SELECT Name FROM FilterNameTable"_L1);
        while (m_query->next())
            lst.append(m_query->value(0).toString());
    }
    return lst;
}

QStringList QHelpDBReader::filterAttributes(const QString &filterName) const
{
    QStringList lst;
    if (m_query) {
        if (filterName.isEmpty()) {
            m_query->prepare("SELECT Name FROM FilterAttributeTable"_L1);
        } else {
            m_query->prepare(
                 "SELECT "
                     "FilterAttributeTable.Name "
                 "FROM "
                     "FilterAttributeTable, "
                     "FilterTable, "
                     "FilterNameTable "
                 "WHERE FilterNameTable.Name = ? "
                "AND FilterNameTable.Id = FilterTable.NameId "
                "AND FilterTable.FilterAttributeId = FilterAttributeTable.Id"_L1);
            m_query->bindValue(0, filterName);
        }
        m_query->exec();
        while (m_query->next())
            lst.append(m_query->value(0).toString());
    }
    return lst;
}

QMultiMap<QString, QByteArray> QHelpDBReader::filesData(const QStringList &filterAttributes,
                                                        const QString &extensionFilter) const
{
    if (!m_query)
        return {};

    QString query;
    QString extension;
    if (!extensionFilter.isEmpty())
        extension = "AND FileNameTable.Name LIKE \'%.%1\'"_L1.arg(extensionFilter);

    if (filterAttributes.isEmpty()) {
        query =
            "SELECT "
                "FileNameTable.Name, "
                "FileDataTable.Data "
            "FROM "
                "FolderTable, "
                "FileNameTable, "
                "FileDataTable "
            "WHERE FileDataTable.Id = FileNameTable.FileId "
            "AND FileNameTable.FolderId = FolderTable.Id %1"_L1.arg(extension);
    } else {
        for (int i = 0; i < filterAttributes.size(); ++i) {
            if (i > 0)
                query.append(" INTERSECT "_L1);
            query.append(
                "SELECT "
                    "FileNameTable.Name, "
                    "FileDataTable.Data "
                "FROM "
                    "FolderTable, "
                    "FileNameTable, "
                    "FileDataTable, "
                    "FileFilterTable, "
                    "FilterAttributeTable "
                "WHERE FileDataTable.Id = FileNameTable.FileId "
                "AND FileNameTable.FolderId = FolderTable.Id "
                "AND FileNameTable.FileId = FileFilterTable.FileId "
                "AND FileFilterTable.FilterAttributeId = FilterAttributeTable.Id "
                "AND FilterAttributeTable.Name = \'%1\' %2"_L1
                            .arg(quote(filterAttributes.at(i)), extension));
        }
    }
    m_query->exec(query);
    QMultiMap<QString, QByteArray> result;
    while (m_query->next())
        result.insert(m_query->value(0).toString(), qUncompress(m_query->value(1).toByteArray()));
    return result;
}

QVariant QHelpDBReader::metaData(const QString &name) const
{
    if (!m_query)
        return {};

    m_query->prepare("SELECT COUNT(Value), Value FROM MetaDataTable WHERE Name=?"_L1);
    m_query->bindValue(0, name);
    if (m_query->exec() && m_query->next() && m_query->value(0).toInt() == 1)
        return m_query->value(1);
    return {};
}

QString QHelpDBReader::quote(const QString &string) const
{
    QString s = string;
    s.replace(u'\'', "\'\'"_L1);
    return s;
}

QT_END_NAMESPACE
