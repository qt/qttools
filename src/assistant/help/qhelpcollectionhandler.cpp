// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpcollectionhandler_p.h"
#include "qhelp_global.h"
#include "qhelpdbreader_p.h"
#include "qhelpfilterdata.h"
#include "qhelplink.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qmap.h>
#include <QtCore/qtimer.h>
#include <QtCore/qversionnumber.h>
#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlerror.h>
#include <QtSql/qsqlquery.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class Transaction
{
public:
    Q_DISABLE_COPY_MOVE(Transaction);

    Transaction(const QString &connectionName)
        : m_db(QSqlDatabase::database(connectionName)),
          m_inTransaction(m_db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        if (m_inTransaction)
            m_inTransaction = m_db.transaction();
    }

    ~Transaction()
    {
        if (m_inTransaction)
            m_db.rollback();
    }

    void commit()
    {
        if (!m_inTransaction)
            return;

        m_db.commit();
        m_inTransaction = false;
    }

private:
    QSqlDatabase m_db;
    bool m_inTransaction;
};

QHelpCollectionHandler::QHelpCollectionHandler(const QString &collectionFile, QObject *parent)
    : QObject(parent)
    , m_collectionFile(collectionFile)
{
    const QFileInfo fi(m_collectionFile);
    if (!fi.isAbsolute())
        m_collectionFile = fi.absoluteFilePath();
}

QHelpCollectionHandler::~QHelpCollectionHandler()
{
    closeDB();
}

bool QHelpCollectionHandler::isDBOpened() const
{
    if (m_query)
        return true;
    auto *that = const_cast<QHelpCollectionHandler *>(this);
    emit that->error(tr("The collection file \"%1\" is not set up yet.").arg(m_collectionFile));
    return false;
}

void QHelpCollectionHandler::closeDB()
{
    if (!m_query)
        return;

    m_query.reset();
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName.clear();
}

bool QHelpCollectionHandler::openCollectionFile()
{
    if (m_query)
        return true;

    m_connectionName = QHelpGlobal::uniquifyConnectionName("QHelpCollectionHandler"_L1, this);
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"_L1, m_connectionName);
        if (db.driver()
            && db.driver()->lastError().type() == QSqlError::ConnectionError) {
            emit error(tr("Cannot load sqlite database driver."));
            return false;
        }

        db.setDatabaseName(collectionFile());
        if (db.open())
            m_query.reset(new QSqlQuery(db));

        if (!m_query) {
            QSqlDatabase::removeDatabase(m_connectionName);
            emit error(tr("Cannot open collection file: %1").arg(collectionFile()));
            return false;
        }
    }

    if (m_readOnly)
        return true;

    m_query->exec("PRAGMA synchronous=OFF"_L1);
    m_query->exec("PRAGMA cache_size=3000"_L1);

    m_query->exec("SELECT COUNT(*) FROM sqlite_master WHERE TYPE=\'table\' "
                                                           "AND Name=\'NamespaceTable\'"_L1);
    m_query->next();

    const bool tablesExist = m_query->value(0).toInt() > 0;
    if (!tablesExist) {
        if (!createTables(m_query.get())) {
            closeDB();
            emit error(tr("Cannot create tables in file %1.").arg(collectionFile()));
            return false;
        }
    }

    bool indexAndNamespaceFilterTablesMissing = false;

    const QStringList newTables = {
        "IndexTable"_L1,
        "FileNameTable"_L1,
        "ContentsTable"_L1,
        "FileFilterTable"_L1,
        "IndexFilterTable"_L1,
        "ContentsFilterTable"_L1,
        "FileAttributeSetTable"_L1,
        "OptimizedFilterTable"_L1,
        "TimeStampTable"_L1,
        "VersionTable"_L1,
        "Filter"_L1,
        "ComponentTable"_L1,
        "ComponentMapping"_L1,
        "ComponentFilter"_L1,
        "VersionFilter"_L1
    };

    QString queryString = "SELECT COUNT(*) FROM sqlite_master WHERE TYPE=\'table\'"_L1;
    queryString.append(" AND (Name=\'"_L1);
    queryString.append(newTables.join("\' OR Name=\'"_L1));
    queryString.append("\')"_L1);

    m_query->exec(queryString);
    m_query->next();
    if (m_query->value(0).toInt() != newTables.size()) {
        if (!recreateIndexAndNamespaceFilterTables(m_query.get())) {
            emit error(tr("Cannot create index tables in file %1.").arg(collectionFile()));
            return false;
        }

        // Old tables exist, index tables didn't, recreate index tables only in this case
        indexAndNamespaceFilterTablesMissing = tablesExist;
    }

    const FileInfoList &docList = registeredDocumentations();
    if (indexAndNamespaceFilterTablesMissing) {
        for (const QHelpCollectionHandler::FileInfo &info : docList) {
            if (!registerIndexAndNamespaceFilterTables(info.namespaceName, true)) {
                emit error(tr("Cannot register index tables in file %1.").arg(collectionFile()));
                return false;
            }
        }
        return true;
    }

    QList<TimeStamp> timeStamps;
    m_query->exec("SELECT NamespaceId, FolderId, FilePath, Size, TimeStamp FROM TimeStampTable"_L1);
    while (m_query->next()) {
        TimeStamp timeStamp;
        timeStamp.namespaceId = m_query->value(0).toInt();
        timeStamp.folderId    = m_query->value(1).toInt();
        timeStamp.fileName    = m_query->value(2).toString();
        timeStamp.size        = m_query->value(3).toInt();
        timeStamp.timeStamp   = m_query->value(4).toDateTime();
        timeStamps.append(timeStamp);
    }

    QList<TimeStamp> toRemove;
    for (const TimeStamp &timeStamp : timeStamps) {
        if (!isTimeStampCorrect(timeStamp))
            toRemove.append(timeStamp);
    }

    // TODO: we may optimize when toRemove.size() == timeStamps.size().
    // In this case we remove all records from tables.
    Transaction transaction(m_connectionName);
    for (const TimeStamp &timeStamp : toRemove) {
        if (!unregisterIndexTable(timeStamp.namespaceId, timeStamp.folderId)) {
            emit error(tr("Cannot unregister index tables in file %1.").arg(collectionFile()));
            return false;
        }
    }
    transaction.commit();

    for (const QHelpCollectionHandler::FileInfo &info : docList) {
        if (!hasTimeStampInfo(info.namespaceName)
                && !registerIndexAndNamespaceFilterTables(info.namespaceName)) {
            // we may have a doc registered without a timestamp
            // and the doc may be missing currently
            unregisterDocumentation(info.namespaceName);
        }
    }
    return true;
}

QString QHelpCollectionHandler::absoluteDocPath(const QString &fileName) const
{
    const QFileInfo fi(collectionFile());
    return QDir::isAbsolutePath(fileName)
            ? fileName
            : QFileInfo(fi.absolutePath() + u'/' + fileName).absoluteFilePath();
}

bool QHelpCollectionHandler::isTimeStampCorrect(const TimeStamp &timeStamp) const
{
    const QFileInfo fi(absoluteDocPath(timeStamp.fileName));

    if (!fi.exists())
        return false;

    if (fi.size() != timeStamp.size)
        return false;

    if (fi.lastModified(QTimeZone::UTC) != timeStamp.timeStamp)
        return false;

    m_query->prepare("SELECT FilePath FROM NamespaceTable WHERE Id = ?"_L1);
    m_query->bindValue(0, timeStamp.namespaceId);
    if (!m_query->exec() || !m_query->next())
        return false;

    const QString oldFileName = m_query->value(0).toString();
    m_query->clear();
    if (oldFileName != timeStamp.fileName)
        return false;

    return true;
}

bool QHelpCollectionHandler::hasTimeStampInfo(const QString &nameSpace) const
{
    m_query->prepare(
        "SELECT "
            "TimeStampTable.NamespaceId "
        "FROM "
            "NamespaceTable, "
            "TimeStampTable "
        "WHERE NamespaceTable.Id = TimeStampTable.NamespaceId "
        "AND NamespaceTable.Name = ? LIMIT 1"_L1);
    m_query->bindValue(0, nameSpace);
    if (!m_query->exec())
        return false;

    if (!m_query->next())
        return false;

    m_query->clear();
    return true;
}

void QHelpCollectionHandler::scheduleVacuum()
{
    if (m_vacuumScheduled)
        return;

    m_vacuumScheduled = true;
    QTimer::singleShot(0, this, &QHelpCollectionHandler::execVacuum);
}

void QHelpCollectionHandler::execVacuum()
{
    if (!m_query)
        return;

    m_query->exec("VACUUM"_L1);
    m_vacuumScheduled = false;
}

bool QHelpCollectionHandler::copyCollectionFile(const QString &fileName)
{
    if (!m_query)
        return false;

    const QFileInfo fi(fileName);
    if (fi.exists()) {
        emit error(tr("The collection file \"%1\" already exists.").arg(fileName));
        return false;
    }

    if (!fi.absoluteDir().exists() && !QDir().mkpath(fi.absolutePath())) {
        emit error(tr("Cannot create directory: %1").arg(fi.absolutePath()));
        return false;
    }

    const QString &colFile = fi.absoluteFilePath();
    const QString &connectionName =
            QHelpGlobal::uniquifyConnectionName("QHelpCollectionHandlerCopy"_L1, this);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"_L1, connectionName);
    db.setDatabaseName(colFile);
    if (!db.open()) {
        emit error(tr("Cannot open collection file: %1").arg(colFile));
        return false;
    }

    QSqlQuery copyQuery(db);
    copyQuery.exec("PRAGMA synchronous=OFF"_L1);
    copyQuery.exec("PRAGMA cache_size=3000"_L1);

    if (!createTables(&copyQuery) || !recreateIndexAndNamespaceFilterTables(&copyQuery)) {
        emit error(tr("Cannot copy collection file: %1").arg(colFile));
        return false;
    }

    const QString &oldBaseDir = QFileInfo(collectionFile()).absolutePath();
    const QFileInfo newColFi(colFile);
    m_query->exec("SELECT Name, FilePath FROM NamespaceTable"_L1);
    while (m_query->next()) {
        copyQuery.prepare("INSERT INTO NamespaceTable VALUES(NULL, ?, ?)"_L1);
        copyQuery.bindValue(0, m_query->value(0).toString());
        QString oldFilePath = m_query->value(1).toString();
        if (!QDir::isAbsolutePath(oldFilePath))
            oldFilePath = oldBaseDir + u'/' + oldFilePath;
        copyQuery.bindValue(1, newColFi.absoluteDir().relativeFilePath(oldFilePath));
        copyQuery.exec();
    }

    m_query->exec("SELECT NamespaceId, Name FROM FolderTable"_L1);
    while (m_query->next()) {
        copyQuery.prepare("INSERT INTO FolderTable VALUES(NULL, ?, ?)"_L1);
        copyQuery.bindValue(0, m_query->value(0).toString());
        copyQuery.bindValue(1, m_query->value(1).toString());
        copyQuery.exec();
    }

    m_query->exec("SELECT Name FROM FilterAttributeTable"_L1);
    while (m_query->next()) {
        copyQuery.prepare("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"_L1);
        copyQuery.bindValue(0, m_query->value(0).toString());
        copyQuery.exec();
    }

    m_query->exec("SELECT Name FROM FilterNameTable"_L1);
    while (m_query->next()) {
        copyQuery.prepare("INSERT INTO FilterNameTable VALUES(NULL, ?)"_L1);
        copyQuery.bindValue(0, m_query->value(0).toString());
        copyQuery.exec();
    }

    m_query->exec("SELECT NameId, FilterAttributeId FROM FilterTable"_L1);
    while (m_query->next()) {
        copyQuery.prepare("INSERT INTO FilterTable VALUES(?, ?)"_L1);
        copyQuery.bindValue(0, m_query->value(0).toInt());
        copyQuery.bindValue(1, m_query->value(1).toInt());
        copyQuery.exec();
    }

    m_query->exec("SELECT Key, Value FROM SettingsTable"_L1);
    while (m_query->next()) {
        if (m_query->value(0).toString() == "FTS5IndexedNamespaces"_L1)
            continue;
        copyQuery.prepare("INSERT INTO SettingsTable VALUES(?, ?)"_L1);
        copyQuery.bindValue(0, m_query->value(0).toString());
        copyQuery.bindValue(1, m_query->value(1));
        copyQuery.exec();
    }

    copyQuery.clear();
    QSqlDatabase::removeDatabase(connectionName);
    return true;
}

bool QHelpCollectionHandler::createTables(QSqlQuery *query)
{
    const QStringList tables = {
        "CREATE TABLE NamespaceTable ("
            "Id INTEGER PRIMARY KEY, "
            "Name TEXT, "
            "FilePath TEXT )"_L1,
        "CREATE TABLE FolderTable ("
            "Id INTEGER PRIMARY KEY, "
            "NamespaceId INTEGER, "
            "Name TEXT )"_L1,
        "CREATE TABLE FilterAttributeTable ("
            "Id INTEGER PRIMARY KEY, "
            "Name TEXT )"_L1,
        "CREATE TABLE FilterNameTable ("
            "Id INTEGER PRIMARY KEY, "
            "Name TEXT )"_L1,
        "CREATE TABLE FilterTable ("
            "NameId INTEGER, "
            "FilterAttributeId INTEGER )"_L1,
        "CREATE TABLE SettingsTable ("
            "Key TEXT PRIMARY KEY, "
            "Value BLOB )"_L1
    };

    for (const QString &q : tables) {
        if (!query->exec(q))
            return false;
    }
    return true;
}

bool QHelpCollectionHandler::recreateIndexAndNamespaceFilterTables(QSqlQuery *query)
{
    const QStringList tables = {
        "DROP TABLE IF EXISTS FileNameTable"_L1,
        "DROP TABLE IF EXISTS IndexTable"_L1,
        "DROP TABLE IF EXISTS ContentsTable"_L1,
        "DROP TABLE IF EXISTS FileFilterTable"_L1, // legacy
        "DROP TABLE IF EXISTS IndexFilterTable"_L1, // legacy
        "DROP TABLE IF EXISTS ContentsFilterTable"_L1, // legacy
        "DROP TABLE IF EXISTS FileAttributeSetTable"_L1, // legacy
        "DROP TABLE IF EXISTS OptimizedFilterTable"_L1, // legacy
        "DROP TABLE IF EXISTS TimeStampTable"_L1,
        "DROP TABLE IF EXISTS VersionTable"_L1,
        "DROP TABLE IF EXISTS Filter"_L1,
        "DROP TABLE IF EXISTS ComponentTable"_L1,
        "DROP TABLE IF EXISTS ComponentMapping"_L1,
        "DROP TABLE IF EXISTS ComponentFilter"_L1,
        "DROP TABLE IF EXISTS VersionFilter"_L1,
        "CREATE TABLE FileNameTable ("
            "FolderId INTEGER, "
            "Name TEXT, "
            "FileId INTEGER PRIMARY KEY, "
            "Title TEXT)"_L1,
        "CREATE TABLE IndexTable ("
            "Id INTEGER PRIMARY KEY, "
            "Name TEXT, "
            "Identifier TEXT, "
            "NamespaceId INTEGER, "
            "FileId INTEGER, "
            "Anchor TEXT)"_L1,
        "CREATE TABLE ContentsTable ("
            "Id INTEGER PRIMARY KEY, "
            "NamespaceId INTEGER, "
            "Data BLOB)"_L1,
        "CREATE TABLE FileFilterTable ("
            "FilterAttributeId INTEGER, "
            "FileId INTEGER)"_L1,
        "CREATE TABLE IndexFilterTable ("
            "FilterAttributeId INTEGER, "
            "IndexId INTEGER)"_L1,
        "CREATE TABLE ContentsFilterTable ("
            "FilterAttributeId INTEGER, "
            "ContentsId INTEGER )"_L1,
        "CREATE TABLE FileAttributeSetTable ("
            "NamespaceId INTEGER, "
            "FilterAttributeSetId INTEGER, "
            "FilterAttributeId INTEGER)"_L1,
        "CREATE TABLE OptimizedFilterTable ("
            "NamespaceId INTEGER, "
            "FilterAttributeId INTEGER)"_L1,
        "CREATE TABLE TimeStampTable ("
            "NamespaceId INTEGER, "
            "FolderId INTEGER, "
            "FilePath TEXT, "
            "Size INTEGER, "
            "TimeStamp TEXT)"_L1,
        "CREATE TABLE VersionTable ("
            "NamespaceId INTEGER, "
            "Version TEXT)"_L1,
        "CREATE TABLE Filter ("
            "FilterId INTEGER PRIMARY KEY, "
            "Name TEXT)"_L1,
        "CREATE TABLE ComponentTable ("
            "ComponentId INTEGER PRIMARY KEY, "
            "Name TEXT)"_L1,
        "CREATE TABLE ComponentMapping ("
            "ComponentId INTEGER, "
            "NamespaceId INTEGER)"_L1,
        "CREATE TABLE ComponentFilter ("
            "ComponentName TEXT, "
            "FilterId INTEGER)"_L1,
        "CREATE TABLE VersionFilter ("
            "Version TEXT, "
            "FilterId INTEGER)"_L1
    };

    for (const QString &q : tables) {
        if (!query->exec(q))
            return false;
    }
    return true;
}

QStringList QHelpCollectionHandler::customFilters() const
{
    QStringList list;
    if (m_query) {
        m_query->exec("SELECT Name FROM FilterNameTable"_L1);
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

QStringList QHelpCollectionHandler::filters() const
{
    QStringList list;
    if (m_query) {
        m_query->exec("SELECT Name FROM Filter ORDER BY Name"_L1);
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

QStringList QHelpCollectionHandler::availableComponents() const
{
    QStringList list;
    if (m_query) {
        m_query->exec("SELECT DISTINCT Name FROM ComponentTable ORDER BY Name"_L1);
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

QList<QVersionNumber> QHelpCollectionHandler::availableVersions() const
{
    QList<QVersionNumber> list;
    if (m_query) {
        m_query->exec("SELECT DISTINCT Version FROM VersionTable ORDER BY Version"_L1);
        while (m_query->next())
            list.append(QVersionNumber::fromString(m_query->value(0).toString()));
    }
    return list;
}

QMap<QString, QString> QHelpCollectionHandler::namespaceToComponent() const
{
    QMap<QString, QString> result;
    if (m_query) {
        m_query->exec(
            "SELECT "
                "NamespaceTable.Name, "
                "ComponentTable.Name "
            "FROM NamespaceTable, "
                "ComponentTable, "
                "ComponentMapping "
            "WHERE NamespaceTable.Id = ComponentMapping.NamespaceId "
            "AND ComponentMapping.ComponentId = ComponentTable.ComponentId"_L1);
        while (m_query->next())
            result.insert(m_query->value(0).toString(), m_query->value(1).toString());
    }
    return result;
}

QMap<QString, QVersionNumber> QHelpCollectionHandler::namespaceToVersion() const
{
    QMap<QString, QVersionNumber> result;
    if (m_query) {
        m_query->exec(
            "SELECT "
                "NamespaceTable.Name, "
                "VersionTable.Version "
            "FROM NamespaceTable, "
                "VersionTable "
            "WHERE NamespaceTable.Id = VersionTable.NamespaceId"_L1);
        while (m_query->next()) {
            result.insert(m_query->value(0).toString(),
                QVersionNumber::fromString(m_query->value(1).toString()));
        }
    }
    return result;
}

QHelpFilterData QHelpCollectionHandler::filterData(const QString &filterName) const
{
    QStringList components;
    QList<QVersionNumber> versions;
    if (m_query) {
        m_query->prepare(
            "SELECT ComponentFilter.ComponentName "
            "FROM ComponentFilter, Filter "
            "WHERE ComponentFilter.FilterId = Filter.FilterId "
            "AND Filter.Name = ? "
            "ORDER BY ComponentFilter.ComponentName"_L1);
        m_query->bindValue(0, filterName);
        m_query->exec();
        while (m_query->next())
            components.append(m_query->value(0).toString());

        m_query->prepare(
            "SELECT VersionFilter.Version "
            "FROM VersionFilter, Filter "
            "WHERE VersionFilter.FilterId = Filter.FilterId "
            "AND Filter.Name = ? "
            "ORDER BY VersionFilter.Version"_L1);
        m_query->bindValue(0, filterName);
        m_query->exec();
        while (m_query->next())
            versions.append(QVersionNumber::fromString(m_query->value(0).toString()));

    }
    QHelpFilterData data;
    data.setComponents(components);
    data.setVersions(versions);
    return data;
}

bool QHelpCollectionHandler::setFilterData(const QString &filterName,
                                           const QHelpFilterData &filterData)
{
    if (!removeFilter(filterName))
        return false;

    m_query->prepare("INSERT INTO Filter VALUES (NULL, ?)"_L1);
    m_query->bindValue(0, filterName);
    if (!m_query->exec())
        return false;

    const int filterId = m_query->lastInsertId().toInt();

    QVariantList componentList;
    QVariantList versionList;
    QVariantList filterIdList;

    for (const QString &component : filterData.components()) {
        componentList.append(component);
        filterIdList.append(filterId);
    }

    m_query->prepare("INSERT INTO ComponentFilter VALUES (?, ?)"_L1);
    m_query->addBindValue(componentList);
    m_query->addBindValue(filterIdList);
    if (!m_query->execBatch())
        return false;

    filterIdList.clear();
    for (const QVersionNumber &version : filterData.versions()) {
        versionList.append(version.isNull() ? QString() : version.toString());
        filterIdList.append(filterId);
    }

    m_query->prepare("INSERT INTO VersionFilter VALUES (?, ?)"_L1);
    m_query->addBindValue(versionList);
    m_query->addBindValue(filterIdList);
    if (!m_query->execBatch())
        return false;

    return true;
}

bool QHelpCollectionHandler::removeFilter(const QString &filterName)
{
    m_query->prepare("SELECT FilterId FROM Filter WHERE Name = ?"_L1);
    m_query->bindValue(0, filterName);
    if (!m_query->exec())
        return false;

    if (!m_query->next())
        return true; // no filter in DB

    const int filterId = m_query->value(0).toInt();

    m_query->prepare("DELETE FROM Filter WHERE Filter.Name = ?"_L1);
    m_query->bindValue(0, filterName);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM ComponentFilter WHERE ComponentFilter.FilterId = ?"_L1);
    m_query->bindValue(0, filterId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM VersionFilter WHERE VersionFilter.FilterId = ?"_L1);
    m_query->bindValue(0, filterId);
    if (!m_query->exec())
        return false;

    return true;
}

bool QHelpCollectionHandler::removeCustomFilter(const QString &filterName)
{
    if (!isDBOpened() || filterName.isEmpty())
        return false;

    int filterNameId = -1;
    m_query->prepare("SELECT Id FROM FilterNameTable WHERE Name=?"_L1);
    m_query->bindValue(0, filterName);
    m_query->exec();
    if (m_query->next())
        filterNameId = m_query->value(0).toInt();

    if (filterNameId < 0) {
        emit error(tr("Unknown filter \"%1\".").arg(filterName));
        return false;
    }

    m_query->prepare("DELETE FROM FilterTable WHERE NameId=?"_L1);
    m_query->bindValue(0, filterNameId);
    m_query->exec();

    m_query->prepare("DELETE FROM FilterNameTable WHERE Id=?"_L1);
    m_query->bindValue(0, filterNameId);
    m_query->exec();

    return true;
}

bool QHelpCollectionHandler::addCustomFilter(const QString &filterName,
                                             const QStringList &attributes)
{
    if (!isDBOpened() || filterName.isEmpty())
        return false;

    int nameId = -1;
    m_query->prepare("SELECT Id FROM FilterNameTable WHERE Name=?"_L1);
    m_query->bindValue(0, filterName);
    m_query->exec();
    if (m_query->next())
        nameId = m_query->value(0).toInt();

    m_query->exec("SELECT Id, Name FROM FilterAttributeTable"_L1);
    QStringList idsToInsert = attributes;
    QMap<QString, int> attributeMap;
    while (m_query->next()) {
        // all old attributes
        const QString attributeName = m_query->value(1).toString();
        attributeMap.insert(attributeName, m_query->value(0).toInt());
        idsToInsert.removeAll(attributeName);
    }

    for (const QString &id : std::as_const(idsToInsert)) {
        m_query->prepare("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"_L1);
        m_query->bindValue(0, id);
        m_query->exec();
        attributeMap.insert(id, m_query->lastInsertId().toInt());
    }

    if (nameId < 0) {
        m_query->prepare("INSERT INTO FilterNameTable VALUES(NULL, ?)"_L1);
        m_query->bindValue(0, filterName);
        if (m_query->exec())
            nameId = m_query->lastInsertId().toInt();
    }

    if (nameId < 0) {
        emit error(tr("Cannot register filter %1.").arg(filterName));
        return false;
    }

    m_query->prepare("DELETE FROM FilterTable WHERE NameId=?"_L1);
    m_query->bindValue(0, nameId);
    m_query->exec();

    for (const QString &att : attributes) {
        m_query->prepare("INSERT INTO FilterTable VALUES(?, ?)"_L1);
        m_query->bindValue(0, nameId);
        m_query->bindValue(1, attributeMap[att]);
        if (!m_query->exec())
            return false;
    }
    return true;
}

QHelpCollectionHandler::FileInfo QHelpCollectionHandler::registeredDocumentation(
        const QString &namespaceName) const
{
    FileInfo fileInfo;

    if (!m_query)
        return fileInfo;

    m_query->prepare(
        "SELECT "
            "NamespaceTable.Name, "
            "NamespaceTable.FilePath, "
            "FolderTable.Name "
        "FROM "
            "NamespaceTable, "
            "FolderTable "
        "WHERE NamespaceTable.Id = FolderTable.NamespaceId "
        "AND NamespaceTable.Name = ? LIMIT 1"_L1);
    m_query->bindValue(0, namespaceName);
    if (!m_query->exec() || !m_query->next())
        return fileInfo;

    fileInfo.namespaceName = m_query->value(0).toString();
    fileInfo.fileName = m_query->value(1).toString();
    fileInfo.folderName = m_query->value(2).toString();

    m_query->clear();

    return fileInfo;
}

QHelpCollectionHandler::FileInfoList QHelpCollectionHandler::registeredDocumentations() const
{
    FileInfoList list;
    if (!m_query)
        return list;

    m_query->exec(
        "SELECT "
            "NamespaceTable.Name, "
            "NamespaceTable.FilePath, "
            "FolderTable.Name "
        "FROM "
            "NamespaceTable, "
            "FolderTable "
        "WHERE NamespaceTable.Id = FolderTable.NamespaceId"_L1);

    while (m_query->next()) {
        FileInfo fileInfo;
        fileInfo.namespaceName = m_query->value(0).toString();
        fileInfo.fileName = m_query->value(1).toString();
        fileInfo.folderName = m_query->value(2).toString();
        list.append(fileInfo);
    }

    return list;
}

bool QHelpCollectionHandler::registerDocumentation(const QString &fileName)
{
    if (!isDBOpened())
        return false;

    QHelpDBReader reader(fileName, QHelpGlobal::uniquifyConnectionName(
        "QHelpCollectionHandler"_L1, this), nullptr);
    if (!reader.init()) {
        emit error(tr("Cannot open documentation file %1.").arg(fileName));
        return false;
    }

    const QString &ns = reader.namespaceName();
    if (ns.isEmpty()) {
        emit error(tr("Invalid documentation file \"%1\".").arg(fileName));
        return false;
    }

    const int nsId = registerNamespace(ns, fileName);
    if (nsId < 1)
        return false;

    const int vfId = registerVirtualFolder(reader.virtualFolder(), nsId);
    if (vfId < 1)
        return false;

    registerVersion(reader.version(), nsId);
    registerFilterAttributes(reader.filterAttributeSets(), nsId); // qset, what happens when removing documentation?
    for (const QString &filterName : reader.customFilters())
        addCustomFilter(filterName, reader.filterAttributes(filterName));

    if (!registerIndexTable(reader.indexTable(), nsId, vfId, registeredDocumentation(ns).fileName))
        return false;

    return true;
}

bool QHelpCollectionHandler::unregisterDocumentation(const QString &namespaceName)
{
    if (!isDBOpened())
        return false;

    m_query->prepare("SELECT Id FROM NamespaceTable WHERE Name = ?"_L1);
    m_query->bindValue(0, namespaceName);
    m_query->exec();

    if (!m_query->next()) {
        emit error(tr("The namespace %1 was not registered.").arg(namespaceName));
        return false;
    }

    const int nsId = m_query->value(0).toInt();

    m_query->prepare("DELETE FROM NamespaceTable WHERE Id = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("SELECT Id FROM FolderTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    m_query->exec();

    if (!m_query->next()) {
        emit error(tr("The namespace %1 was not registered.").arg(namespaceName));
        return false;
    }

    const int vfId = m_query->value(0).toInt();

    m_query->prepare("DELETE FROM NamespaceTable WHERE Id = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM FolderTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    if (!unregisterIndexTable(nsId, vfId))
        return false;

    scheduleVacuum();

    return true;
}

static QHelpCollectionHandler::FileInfo extractFileInfo(const QUrl &url)
{
    QHelpCollectionHandler::FileInfo fileInfo;

    if (!url.isValid() || url.toString().count(u'/') < 4
        || url.scheme() != "qthelp"_L1) {
        return fileInfo;
    }

    fileInfo.namespaceName = url.authority();
    fileInfo.fileName = url.path();
    if (fileInfo.fileName.startsWith(u'/'))
        fileInfo.fileName = fileInfo.fileName.mid(1);
    fileInfo.folderName = fileInfo.fileName.mid(0, fileInfo.fileName.indexOf(u'/', 1));
    fileInfo.fileName.remove(0, fileInfo.folderName.size() + 1);

    return fileInfo;
}

bool QHelpCollectionHandler::fileExists(const QUrl &url) const
{
    if (!isDBOpened())
        return false;

    const FileInfo fileInfo = extractFileInfo(url);
    if (fileInfo.namespaceName.isEmpty())
        return false;

    m_query->prepare(
        "SELECT COUNT (DISTINCT NamespaceTable.Id) "
        "FROM "
            "FileNameTable, "
            "NamespaceTable, "
            "FolderTable "
        "WHERE FolderTable.Name = ? "
        "AND FileNameTable.Name = ? "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND FolderTable.NamespaceId = NamespaceTable.Id"_L1);
    m_query->bindValue(0, fileInfo.folderName);
    m_query->bindValue(1, fileInfo.fileName);
    if (!m_query->exec() || !m_query->next())
        return false;

    const int count = m_query->value(0).toInt();
    m_query->clear();

    return count;
}

static QString prepareFilterQuery(const QString &filterName)
{
    if (filterName.isEmpty())
        return {};

    return " AND EXISTS(SELECT * FROM Filter WHERE Filter.Name = ?) "
        "AND ("
        "(NOT EXISTS(" // 1. filter by component
        "SELECT * FROM "
            "ComponentFilter, "
            "Filter "
        "WHERE ComponentFilter.FilterId = Filter.FilterId "
            "AND Filter.Name = ?) "
        "OR NamespaceTable.Id IN ("
        "SELECT "
            "NamespaceTable.Id "
        "FROM "
            "NamespaceTable, "
            "ComponentTable, "
            "ComponentMapping, "
            "ComponentFilter, "
            "Filter "
        "WHERE ComponentMapping.NamespaceId = NamespaceTable.Id "
            "AND ComponentTable.ComponentId = ComponentMapping.ComponentId "
            "AND ((ComponentTable.Name = ComponentFilter.ComponentName) "
                "OR (ComponentTable.Name IS NULL AND ComponentFilter.ComponentName IS NULL)) "
            "AND ComponentFilter.FilterId = Filter.FilterId "
            "AND Filter.Name = ?))"
        " AND "
        "(NOT EXISTS(" // 2. filter by version
        "SELECT * FROM "
            "VersionFilter, "
            "Filter "
        "WHERE VersionFilter.FilterId = Filter.FilterId "
            "AND Filter.Name = ?) "
        "OR NamespaceTable.Id IN ("
        "SELECT "
            "NamespaceTable.Id "
        "FROM "
            "NamespaceTable, "
            "VersionFilter, "
            "VersionTable, "
            "Filter "
        "WHERE VersionFilter.FilterId = Filter.FilterId "
            "AND ((VersionFilter.Version = VersionTable.Version) "
                "OR (VersionFilter.Version IS NULL AND VersionTable.Version IS NULL)) "
            "AND VersionTable.NamespaceId = NamespaceTable.Id "
            "AND Filter.Name = ?))"
        ")"_L1;
}

static void bindFilterQuery(QSqlQuery *query, int bindStart, const QString &filterName)
{
    if (filterName.isEmpty())
        return;

    query->bindValue(bindStart, filterName);
    query->bindValue(bindStart + 1, filterName);
    query->bindValue(bindStart + 2, filterName);
    query->bindValue(bindStart + 3, filterName);
    query->bindValue(bindStart + 4, filterName);
}

static QString prepareFilterQuery(int attributesCount,
                                  const QString &idTableName,
                                  const QString &idColumnName,
                                  const QString &filterTableName,
                                  const QString &filterColumnName)
{
    if (!attributesCount)
        return {};

    QString filterQuery = " AND (%1.%2 IN ("_L1.arg(idTableName, idColumnName);

    const QString filterQueryTemplate =
        "SELECT %1.%2 "
        "FROM %1, FilterAttributeTable "
        "WHERE %1.FilterAttributeId = FilterAttributeTable.Id "
        "AND FilterAttributeTable.Name = ?"_L1.arg(filterTableName, filterColumnName);

    for (int i = 0; i < attributesCount; ++i) {
        if (i > 0)
            filterQuery.append(" INTERSECT "_L1);
        filterQuery.append(filterQueryTemplate);
    }

    filterQuery.append(") OR NamespaceTable.Id IN ("_L1);

    const QString optimizedFilterQueryTemplate =
        "SELECT OptimizedFilterTable.NamespaceId "
        "FROM OptimizedFilterTable, FilterAttributeTable "
        "WHERE OptimizedFilterTable.FilterAttributeId = FilterAttributeTable.Id "
        "AND FilterAttributeTable.Name = ?"_L1;

    for (int i = 0; i < attributesCount; ++i) {
        if (i > 0)
            filterQuery.append(" INTERSECT "_L1);
        filterQuery.append(optimizedFilterQueryTemplate);
    }

    filterQuery.append("))"_L1);

    return filterQuery;
}

static void bindFilterQuery(QSqlQuery *query, int startingBindPos, const QStringList &filterAttributes)
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < filterAttributes.size(); j++) {
            query->bindValue(i * filterAttributes.size() + j + startingBindPos,
                             filterAttributes.at(j));
        }
    }
}

QString QHelpCollectionHandler::namespaceForFile(const QUrl &url,
                                                 const QStringList &filterAttributes) const
{
    if (!isDBOpened())
        return {};

    const FileInfo fileInfo = extractFileInfo(url);
    if (fileInfo.namespaceName.isEmpty())
        return {};

    const QString filterlessQuery =
        "SELECT DISTINCT "
            "NamespaceTable.Name "
        "FROM "
            "FileNameTable, "
            "NamespaceTable, "
            "FolderTable "
        "WHERE FolderTable.Name = ? "
        "AND FileNameTable.Name = ? "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND FolderTable.NamespaceId = NamespaceTable.Id"_L1;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterAttributes.size(), "FileNameTable"_L1, "FileId"_L1,
                                 "FileFilterTable"_L1, "FileId"_L1);

    m_query->prepare(filterQuery);
    m_query->bindValue(0, fileInfo.folderName);
    m_query->bindValue(1, fileInfo.fileName);
    bindFilterQuery(m_query.get(), 2, filterAttributes);

    if (!m_query->exec())
        return {};

    QStringList namespaceList;
    while (m_query->next())
        namespaceList.append(m_query->value(0).toString());

    if (namespaceList.isEmpty())
        return {};

    if (namespaceList.contains(fileInfo.namespaceName))
        return fileInfo.namespaceName;

    const QString originalVersion = namespaceVersion(fileInfo.namespaceName);

    for (const QString &ns : namespaceList) {
        const QString nsVersion = namespaceVersion(ns);
        if (originalVersion == nsVersion)
            return ns;
    }

    // TODO: still, we may like to return the ns for the highest available version
    return namespaceList.first();
}

QString QHelpCollectionHandler::namespaceForFile(const QUrl &url,
                                                 const QString &filterName) const
{
    if (!isDBOpened())
        return {};

    const FileInfo fileInfo = extractFileInfo(url);
    if (fileInfo.namespaceName.isEmpty())
        return {};

    const QString filterlessQuery =
        "SELECT DISTINCT "
            "NamespaceTable.Name "
        "FROM "
            "FileNameTable, "
            "NamespaceTable, "
            "FolderTable "
        "WHERE FolderTable.Name = ? "
        "AND FileNameTable.Name = ? "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND FolderTable.NamespaceId = NamespaceTable.Id"_L1;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterName);

    m_query->prepare(filterQuery);
    m_query->bindValue(0, fileInfo.folderName);
    m_query->bindValue(1, fileInfo.fileName);
    bindFilterQuery(m_query.get(), 2, filterName);

    if (!m_query->exec())
        return {};

    QStringList namespaceList;
    while (m_query->next())
        namespaceList.append(m_query->value(0).toString());

    if (namespaceList.isEmpty())
        return {};

    if (namespaceList.contains(fileInfo.namespaceName))
        return fileInfo.namespaceName;

    const QString originalVersion = namespaceVersion(fileInfo.namespaceName);

    for (const QString &ns : namespaceList) {
        const QString nsVersion = namespaceVersion(ns);
        if (originalVersion == nsVersion)
            return ns;
    }

    // TODO: still, we may like to return the ns for the highest available version
    return namespaceList.first();
}

QStringList QHelpCollectionHandler::files(const QString &namespaceName,
                                          const QStringList &filterAttributes,
                                          const QString &extensionFilter) const
{
    if (!isDBOpened())
        return {};

    const QString extensionQuery = extensionFilter.isEmpty()
            ? QString() : " AND FileNameTable.Name LIKE ?"_L1;
    const QString filterlessQuery =
        "SELECT "
            "FolderTable.Name, "
            "FileNameTable.Name "
        "FROM "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE FileNameTable.FolderId = FolderTable.Id "
        "AND FolderTable.NamespaceId = NamespaceTable.Id "
        "AND NamespaceTable.Name = ?"_L1 + extensionQuery;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterAttributes.size(), "FileNameTable"_L1, "FileId"_L1,
                                 "FileFilterTable"_L1, "FileId"_L1);

    m_query->prepare(filterQuery);
    m_query->bindValue(0, namespaceName);
    int bindCount = 1;
    if (!extensionFilter.isEmpty()) {
        m_query->bindValue(bindCount, "%.%1"_L1.arg(extensionFilter));
        ++bindCount;
    }
    bindFilterQuery(m_query.get(), bindCount, filterAttributes);

    if (!m_query->exec())
        return {};

    QStringList fileNames;
    while (m_query->next())
        fileNames.append(m_query->value(0).toString() + u'/' + m_query->value(1).toString());
    return fileNames;
}

QStringList QHelpCollectionHandler::files(const QString &namespaceName,
                                          const QString &filterName,
                                          const QString &extensionFilter) const
{
    if (!isDBOpened())
        return {};

    const QString extensionQuery = extensionFilter.isEmpty()
            ? QString() : " AND FileNameTable.Name LIKE ?"_L1;
    const QString filterlessQuery =
        "SELECT "
            "FolderTable.Name, "
            "FileNameTable.Name "
        "FROM "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE FileNameTable.FolderId = FolderTable.Id "
        "AND FolderTable.NamespaceId = NamespaceTable.Id "
        "AND NamespaceTable.Name = ?"_L1 + extensionQuery;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterName);

    m_query->prepare(filterQuery);
    m_query->bindValue(0, namespaceName);
    int bindCount = 1;
    if (!extensionFilter.isEmpty()) {
        m_query->bindValue(bindCount, "%.%1"_L1.arg(extensionFilter));
        ++bindCount;
    }

    bindFilterQuery(m_query.get(), bindCount, filterName);

    if (!m_query->exec())
        return{};

    QStringList fileNames;
    while (m_query->next())
        fileNames.append(m_query->value(0).toString() + u'/' + m_query->value(1).toString());
    return fileNames;
}

QUrl QHelpCollectionHandler::findFile(const QUrl &url, const QStringList &filterAttributes) const
{
    if (!isDBOpened())
        return {};

    const QString namespaceName = namespaceForFile(url, filterAttributes);
    if (namespaceName.isEmpty())
        return {};

    QUrl result = url;
    result.setAuthority(namespaceName);
    return result;
}

QUrl QHelpCollectionHandler::findFile(const QUrl &url, const QString &filterName) const
{
    if (!isDBOpened())
        return {};

    const QString namespaceName = namespaceForFile(url, filterName);
    if (namespaceName.isEmpty())
        return {};

    QUrl result = url;
    result.setAuthority(namespaceName);
    return result;
}

QByteArray QHelpCollectionHandler::fileData(const QUrl &url) const
{
    if (!isDBOpened())
        return {};

    const QString namespaceName = namespaceForFile(url, QString());
    if (namespaceName.isEmpty())
        return {};

    const FileInfo fileInfo = extractFileInfo(url);

    const FileInfo docInfo = registeredDocumentation(namespaceName);
    const QString absFileName = absoluteDocPath(docInfo.fileName);

    QHelpDBReader reader(absFileName, QHelpGlobal::uniquifyConnectionName(
                         docInfo.fileName, const_cast<QHelpCollectionHandler *>(this)), nullptr);
    if (!reader.init())
        return {};

    return reader.fileData(fileInfo.folderName, fileInfo.fileName);
}

QStringList QHelpCollectionHandler::indicesForFilter(const QStringList &filterAttributes) const
{
    QStringList indices;

    if (!isDBOpened())
        return indices;

    const QString filterlessQuery =
        "SELECT DISTINCT "
            "IndexTable.Name "
        "FROM "
            "IndexTable, "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE IndexTable.FileId = FileNameTable.FileId "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND IndexTable.NamespaceId = NamespaceTable.Id"_L1;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterAttributes.size(), "IndexTable"_L1, "Id"_L1,
                                 "IndexFilterTable"_L1, "IndexId"_L1)
            + " ORDER BY LOWER(IndexTable.Name), IndexTable.Name"_L1;
    //  this doesn't work: ASC COLLATE NOCASE

    m_query->prepare(filterQuery);
    bindFilterQuery(m_query.get(), 0, filterAttributes);

    m_query->exec();

    while (m_query->next())
        indices.append(m_query->value(0).toString());

    return indices;
}

QStringList QHelpCollectionHandler::indicesForFilter(const QString &filterName) const
{
    QStringList indices;

    if (!isDBOpened())
        return indices;

    const QString filterlessQuery =
        "SELECT DISTINCT "
            "IndexTable.Name "
        "FROM "
            "IndexTable, "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE IndexTable.FileId = FileNameTable.FileId "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND IndexTable.NamespaceId = NamespaceTable.Id"_L1;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterName)
            + " ORDER BY LOWER(IndexTable.Name), IndexTable.Name"_L1;

    m_query->prepare(filterQuery);
    bindFilterQuery(m_query.get(), 0, filterName);

    m_query->exec();

    while (m_query->next())
        indices.append(m_query->value(0).toString());

    return indices;
}

static QString getTitle(const QByteArray &contents)
{
    if (!contents.size())
        return {};

    int depth = 0;
    QString link;
    QString title;

    QDataStream s(contents);
    s >> depth;
    s >> link;
    s >> title;

    return title;
}

QList<QHelpCollectionHandler::ContentsData> QHelpCollectionHandler::contentsForFilter(
        const QStringList &filterAttributes) const
{
    if (!isDBOpened())
        return {};

    const QString filterlessQuery =
        "SELECT DISTINCT "
            "NamespaceTable.Name, "
            "FolderTable.Name, "
            "ContentsTable.Data, "
            "VersionTable.Version "
        "FROM "
            "FolderTable, "
            "NamespaceTable, "
            "ContentsTable, "
            "VersionTable "
        "WHERE ContentsTable.NamespaceId = NamespaceTable.Id "
        "AND NamespaceTable.Id = FolderTable.NamespaceId "
        "AND ContentsTable.NamespaceId = NamespaceTable.Id "
        "AND VersionTable.NamespaceId = NamespaceTable.Id"_L1;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterAttributes.size(), "ContentsTable"_L1, "Id"_L1,
                                 "ContentsFilterTable"_L1, "ContentsId"_L1);

    m_query->prepare(filterQuery);
    bindFilterQuery(m_query.get(), 0, filterAttributes);

    m_query->exec();

    QMap<QString, QMap<QVersionNumber, ContentsData>> contentsMap;

    while (m_query->next()) {
        const QString namespaceName = m_query->value(0).toString();
        const QByteArray contents = m_query->value(2).toByteArray();
        const QString versionString = m_query->value(3).toString();

        const QString title = getTitle(contents);
        const QVersionNumber version = QVersionNumber::fromString(versionString);
        // get existing or insert a new one otherwise
        ContentsData &contentsData = contentsMap[title][version];
        contentsData.namespaceName = namespaceName;
        contentsData.folderName = m_query->value(1).toString();
        contentsData.contentsList.append(contents);
    }

    QList<QHelpCollectionHandler::ContentsData> result;
    for (const auto &versionContents : std::as_const(contentsMap)) {
        // insert items in the reverse order of version number
        const auto itBegin = versionContents.constBegin();
        auto it = versionContents.constEnd();
        while (it != itBegin) {
            --it;
            result.append(it.value());
        }
    }
    return result;
}

QList<QHelpCollectionHandler::ContentsData> QHelpCollectionHandler::contentsForFilter(const QString &filterName) const
{
    if (!isDBOpened())
        return {};

    const QString filterlessQuery =
        "SELECT DISTINCT "
            "NamespaceTable.Name, "
            "FolderTable.Name, "
            "ContentsTable.Data, "
            "VersionTable.Version "
        "FROM "
            "FolderTable, "
            "NamespaceTable, "
            "ContentsTable, "
            "VersionTable "
        "WHERE ContentsTable.NamespaceId = NamespaceTable.Id "
        "AND NamespaceTable.Id = FolderTable.NamespaceId "
        "AND ContentsTable.NamespaceId = NamespaceTable.Id "
        "AND VersionTable.NamespaceId = NamespaceTable.Id"_L1;

    const QString filterQuery = filterlessQuery + prepareFilterQuery(filterName);

    m_query->prepare(filterQuery);
    bindFilterQuery(m_query.get(), 0, filterName);

    m_query->exec();

    QMap<QString, QMap<QVersionNumber, ContentsData>> contentsMap;

    while (m_query->next()) {
        const QString namespaceName = m_query->value(0).toString();
        const QByteArray contents = m_query->value(2).toByteArray();
        const QString versionString = m_query->value(3).toString();

        const QString title = getTitle(contents);
        const QVersionNumber version = QVersionNumber::fromString(versionString);
        // get existing or insert a new one otherwise
        ContentsData &contentsData = contentsMap[title][version];
        contentsData.namespaceName = namespaceName;
        contentsData.folderName = m_query->value(1).toString();
        contentsData.contentsList.append(contents);
    }

    QList<QHelpCollectionHandler::ContentsData> result;
    for (const auto &versionContents : std::as_const(contentsMap)) {
        // insert items in the reverse order of version number
        const auto itBegin = versionContents.constBegin();
        auto it = versionContents.constEnd();
        while (it != itBegin) {
            --it;
            result.append(it.value());
        }
    }
    return result;
}

bool QHelpCollectionHandler::removeCustomValue(const QString &key)
{
    if (!isDBOpened())
        return false;

    m_query->prepare("DELETE FROM SettingsTable WHERE Key=?"_L1);
    m_query->bindValue(0, key);
    return m_query->exec();
}

QVariant QHelpCollectionHandler::customValue(const QString &key,
                                             const QVariant &defaultValue) const
{
    if (!m_query)
        return defaultValue;

    m_query->prepare("SELECT COUNT(Key) FROM SettingsTable WHERE Key=?"_L1);
    m_query->bindValue(0, key);
    if (!m_query->exec() || !m_query->next() || !m_query->value(0).toInt()) {
        m_query->clear();
        return defaultValue;
    }

    m_query->clear();
    m_query->prepare("SELECT Value FROM SettingsTable WHERE Key=?"_L1);
    m_query->bindValue(0, key);
    if (m_query->exec() && m_query->next()) {
        const QVariant &value = m_query->value(0);
        m_query->clear();
        return value;
    }
    return defaultValue;
}

bool QHelpCollectionHandler::setCustomValue(const QString &key,
                                            const QVariant &value)
{
    if (!isDBOpened())
        return false;

    m_query->prepare("SELECT Value FROM SettingsTable WHERE Key=?"_L1);
    m_query->bindValue(0, key);
    m_query->exec();
    if (m_query->next()) {
        m_query->prepare("UPDATE SettingsTable SET Value=? where Key=?"_L1);
        m_query->bindValue(0, value);
        m_query->bindValue(1, key);
    } else {
        m_query->prepare("INSERT INTO SettingsTable VALUES(?, ?)"_L1);
        m_query->bindValue(0, key);
        m_query->bindValue(1, value);
    }
    return m_query->exec();
}

bool QHelpCollectionHandler::registerFilterAttributes(const QList<QStringList> &attributeSets,
                                                      int nsId)
{
    if (!isDBOpened())
        return false;

    m_query->exec("SELECT Name FROM FilterAttributeTable"_L1);
    QSet<QString> atts;
    while (m_query->next())
        atts.insert(m_query->value(0).toString());

    for (const QStringList &attributeSet : attributeSets) {
        for (const QString &attribute : attributeSet) {
            if (!atts.contains(attribute)) {
                m_query->prepare("INSERT INTO FilterAttributeTable VALUES(NULL, ?)"_L1);
                m_query->bindValue(0, attribute);
                m_query->exec();
            }
        }
    }
    return registerFileAttributeSets(attributeSets, nsId);
}

bool QHelpCollectionHandler::registerFileAttributeSets(const QList<QStringList> &attributeSets,
                                                       int nsId)
{
    if (!isDBOpened())
        return false;

    if (attributeSets.isEmpty())
        return true;

    QVariantList nsIds;
    QVariantList attributeSetIds;
    QVariantList filterAttributeIds;

    if (!m_query->exec("SELECT MAX(FilterAttributeSetId) FROM FileAttributeSetTable"_L1)
        || !m_query->next()) {
        return false;
    }

    int attributeSetId = m_query->value(0).toInt();

    for (const QStringList &attributeSet : attributeSets) {
        ++attributeSetId;

        for (const QString &attribute : attributeSet) {
            m_query->prepare("SELECT Id FROM FilterAttributeTable WHERE Name=?"_L1);
            m_query->bindValue(0, attribute);

            if (!m_query->exec() || !m_query->next())
                return false;

            nsIds.append(nsId);
            attributeSetIds.append(attributeSetId);
            filterAttributeIds.append(m_query->value(0).toInt());
        }
    }

    m_query->prepare("INSERT INTO FileAttributeSetTable "
                     "(NamespaceId, FilterAttributeSetId, FilterAttributeId) "
                     "VALUES(?, ?, ?)"_L1);
    m_query->addBindValue(nsIds);
    m_query->addBindValue(attributeSetIds);
    m_query->addBindValue(filterAttributeIds);
    return m_query->execBatch();
}

QStringList QHelpCollectionHandler::filterAttributes() const
{
    QStringList list;
    if (m_query) {
        m_query->exec("SELECT Name FROM FilterAttributeTable"_L1);
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

QStringList QHelpCollectionHandler::filterAttributes(const QString &filterName) const
{
    QStringList list;
    if (m_query) {
        m_query->prepare(
            "SELECT "
                "FilterAttributeTable.Name "
            "FROM "
                "FilterAttributeTable, "
                "FilterTable, "
                "FilterNameTable "
            "WHERE FilterAttributeTable.Id = FilterTable.FilterAttributeId "
            "AND FilterTable.NameId = FilterNameTable.Id "
            "AND FilterNameTable.Name=?"_L1);
        m_query->bindValue(0, filterName);
        m_query->exec();
        while (m_query->next())
            list.append(m_query->value(0).toString());
    }
    return list;
}

QList<QStringList> QHelpCollectionHandler::filterAttributeSets(const QString &namespaceName) const
{
    if (!isDBOpened())
        return {};

    m_query->prepare(
        "SELECT "
            "FileAttributeSetTable.FilterAttributeSetId, "
            "FilterAttributeTable.Name "
        "FROM "
            "FileAttributeSetTable, "
            "FilterAttributeTable, "
            "NamespaceTable "
        "WHERE FileAttributeSetTable.FilterAttributeId = FilterAttributeTable.Id "
        "AND FileAttributeSetTable.NamespaceId = NamespaceTable.Id "
        "AND NamespaceTable.Name = ? "
        "ORDER BY FileAttributeSetTable.FilterAttributeSetId"_L1);
    m_query->bindValue(0, namespaceName);
    m_query->exec();
    int oldId = -1;
    QList<QStringList> result;
    while (m_query->next()) {
        const int id = m_query->value(0).toInt();
        if (id != oldId) {
            result.append(QStringList());
            oldId = id;
        }
        result.last().append(m_query->value(1).toString());
    }

    if (result.isEmpty())
        result.append(QStringList());
    return result;
}

QString QHelpCollectionHandler::namespaceVersion(const QString &namespaceName) const
{
    if (!m_query)
        return {};

    m_query->prepare(
        "SELECT "
            "VersionTable.Version "
        "FROM "
            "NamespaceTable, "
            "VersionTable "
        "WHERE NamespaceTable.Name = ? "
        "AND NamespaceTable.Id = VersionTable.NamespaceId"_L1);
    m_query->bindValue(0, namespaceName);
    if (!m_query->exec() || !m_query->next())
        return {};

    const QString ret = m_query->value(0).toString();
    m_query->clear();
    return ret;
}

int QHelpCollectionHandler::registerNamespace(const QString &nspace, const QString &fileName)
{
    const int errorValue = -1;
    if (!m_query)
        return errorValue;

    m_query->prepare("SELECT COUNT(Id) FROM NamespaceTable WHERE Name=?"_L1);
    m_query->bindValue(0, nspace);
    m_query->exec();
    while (m_query->next()) {
        if (m_query->value(0).toInt() > 0) {
            emit error(tr("Namespace %1 already exists.").arg(nspace));
            return errorValue;
        }
    }

    QFileInfo fi(m_collectionFile);
    m_query->prepare("INSERT INTO NamespaceTable VALUES(NULL, ?, ?)"_L1);
    m_query->bindValue(0, nspace);
    m_query->bindValue(1, fi.absoluteDir().relativeFilePath(fileName));
    int namespaceId = errorValue;
    if (m_query->exec()) {
        namespaceId = m_query->lastInsertId().toInt();
        m_query->clear();
    }
    if (namespaceId < 1) {
        emit error(tr("Cannot register namespace \"%1\".").arg(nspace));
        return errorValue;
    }
    return namespaceId;
}

int QHelpCollectionHandler::registerVirtualFolder(const QString &folderName, int namespaceId)
{
    if (!m_query)
        return false;

    m_query->prepare("INSERT INTO FolderTable VALUES(NULL, ?, ?)"_L1);
    m_query->bindValue(0, namespaceId);
    m_query->bindValue(1, folderName);

    int virtualId = -1;
    if (m_query->exec()) {
        virtualId = m_query->lastInsertId().toInt();
        m_query->clear();
    }
    if (virtualId < 1) {
        emit error(tr("Cannot register virtual folder '%1'.").arg(folderName));
        return -1;
    }
    if (registerComponent(folderName, namespaceId) < 0)
        return -1;
    return virtualId;
}

int QHelpCollectionHandler::registerComponent(const QString &componentName, int namespaceId)
{
    m_query->prepare("SELECT ComponentId FROM ComponentTable WHERE Name = ?"_L1);
    m_query->bindValue(0, componentName);
    if (!m_query->exec())
        return -1;

    if (!m_query->next()) {
        m_query->prepare("INSERT INTO ComponentTable VALUES(NULL, ?)"_L1);
        m_query->bindValue(0, componentName);
        if (!m_query->exec())
            return -1;

        m_query->prepare("SELECT ComponentId FROM ComponentTable WHERE Name = ?"_L1);
        m_query->bindValue(0, componentName);
        if (!m_query->exec() || !m_query->next())
            return -1;
    }

    const int componentId = m_query->value(0).toInt();

    m_query->prepare("INSERT INTO ComponentMapping VALUES(?, ?)"_L1);
    m_query->bindValue(0, componentId);
    m_query->bindValue(1, namespaceId);
    if (!m_query->exec())
        return -1;

    return componentId;
}

bool QHelpCollectionHandler::registerVersion(const QString &version, int namespaceId)
{
    if (!m_query)
        return false;

    m_query->prepare("INSERT INTO VersionTable (NamespaceId, Version) VALUES(?, ?)"_L1);
    m_query->addBindValue(namespaceId);
    m_query->addBindValue(version);
    return m_query->exec();
}

bool QHelpCollectionHandler::registerIndexAndNamespaceFilterTables(
        const QString &nameSpace, bool createDefaultVersionFilter)
{
    if (!isDBOpened())
        return false;

    m_query->prepare("SELECT Id, FilePath FROM NamespaceTable WHERE Name=?"_L1);
    m_query->bindValue(0, nameSpace);
    m_query->exec();
    if (!m_query->next())
        return false;

    const int nsId = m_query->value(0).toInt();
    const QString fileName = m_query->value(1).toString();

    m_query->prepare("SELECT Id, Name FROM FolderTable WHERE NamespaceId=?"_L1);
    m_query->bindValue(0, nsId);
    m_query->exec();
    if (!m_query->next())
        return false;

    const int vfId = m_query->value(0).toInt();
    const QString vfName = m_query->value(1).toString();

    const QString absFileName = absoluteDocPath(fileName);
    QHelpDBReader reader(absFileName, QHelpGlobal::uniquifyConnectionName(
                             fileName, this), this);
    if (!reader.init())
        return false;

    registerComponent(vfName, nsId);
    registerVersion(reader.version(), nsId);
    if (!registerFileAttributeSets(reader.filterAttributeSets(), nsId))
        return false;

    if (!registerIndexTable(reader.indexTable(), nsId, vfId, fileName))
        return false;

    if (createDefaultVersionFilter)
        createVersionFilter(reader.version());
    return true;
}

void QHelpCollectionHandler::createVersionFilter(const QString &version)
{
    if (version.isEmpty())
        return;

    const QVersionNumber versionNumber = QVersionNumber::fromString(version);
    if (versionNumber.isNull())
        return;

    const QString filterName = tr("Version %1").arg(version);
    if (filters().contains(filterName))
        return;

    QHelpFilterData filterData;
    filterData.setVersions({versionNumber});
    setFilterData(filterName, filterData);
}

bool QHelpCollectionHandler::registerIndexTable(const QHelpDBReader::IndexTable &indexTable,
                                                int nsId, int vfId, const QString &fileName)
{
    Transaction transaction(m_connectionName);

    QMap<QString, QVariantList> filterAttributeToNewFileId;

    QVariantList fileFolderIds;
    QVariantList fileNames;
    QVariantList fileTitles;
    const int fileSize = indexTable.fileItems.size();
    fileFolderIds.reserve(fileSize);
    fileNames.reserve(fileSize);
    fileTitles.reserve(fileSize);

    if (!m_query->exec("SELECT MAX(FileId) FROM FileNameTable"_L1) || !m_query->next())
        return false;

    const int maxFileId = m_query->value(0).toInt();

    int newFileId = 0;
    for (const QHelpDBReader::FileItem &item : indexTable.fileItems) {
        fileFolderIds.append(vfId);
        fileNames.append(item.name);
        fileTitles.append(item.title);

        for (const QString &filterAttribute : item.filterAttributes)
            filterAttributeToNewFileId[filterAttribute].append(maxFileId + newFileId + 1);
        ++newFileId;
    }

    m_query->prepare("INSERT INTO FileNameTable VALUES(?, ?, NULL, ?)"_L1);
    m_query->addBindValue(fileFolderIds);
    m_query->addBindValue(fileNames);
    m_query->addBindValue(fileTitles);
    if (!m_query->execBatch())
        return false;

    for (auto it = filterAttributeToNewFileId.cbegin(),
         end = filterAttributeToNewFileId.cend(); it != end; ++it) {
        const QString filterAttribute = it.key();
        m_query->prepare("SELECT Id From FilterAttributeTable WHERE Name = ?"_L1);
        m_query->bindValue(0, filterAttribute);
        if (!m_query->exec() || !m_query->next())
            return false;

        const int attributeId = m_query->value(0).toInt();

        QVariantList attributeIds;
        for (int i = 0; i < it.value().size(); i++)
            attributeIds.append(attributeId);

        m_query->prepare("INSERT INTO FileFilterTable VALUES(?, ?)"_L1);
        m_query->addBindValue(attributeIds);
        m_query->addBindValue(it.value());
        if (!m_query->execBatch())
            return false;
    }

    QMap<QString, QVariantList> filterAttributeToNewIndexId;

    if (!m_query->exec("SELECT MAX(Id) FROM IndexTable"_L1) || !m_query->next())
        return false;

    const int maxIndexId = m_query->value(0).toInt();
    int newIndexId = 0;

    QVariantList indexNames;
    QVariantList indexIdentifiers;
    QVariantList indexNamespaceIds;
    QVariantList indexFileIds;
    QVariantList indexAnchors;
    const int indexSize = indexTable.indexItems.size();
    indexNames.reserve(indexSize);
    indexIdentifiers.reserve(indexSize);
    indexNamespaceIds.reserve(indexSize);
    indexFileIds.reserve(indexSize);
    indexAnchors.reserve(indexSize);

    for (const QHelpDBReader::IndexItem &item : indexTable.indexItems) {
        indexNames.append(item.name);
        indexIdentifiers.append(item.identifier);
        indexNamespaceIds.append(nsId);
        indexFileIds.append(maxFileId + item.fileId + 1);
        indexAnchors.append(item.anchor);

        for (const QString &filterAttribute : item.filterAttributes)
            filterAttributeToNewIndexId[filterAttribute].append(maxIndexId + newIndexId + 1);
        ++newIndexId;
    }

    m_query->prepare("INSERT INTO IndexTable VALUES(NULL, ?, ?, ?, ?, ?)"_L1);
    m_query->addBindValue(indexNames);
    m_query->addBindValue(indexIdentifiers);
    m_query->addBindValue(indexNamespaceIds);
    m_query->addBindValue(indexFileIds);
    m_query->addBindValue(indexAnchors);
    if (!m_query->execBatch())
        return false;

    for (auto it = filterAttributeToNewIndexId.cbegin(),
         end = filterAttributeToNewIndexId.cend(); it != end; ++it) {
        const QString filterAttribute = it.key();
        m_query->prepare("SELECT Id From FilterAttributeTable WHERE Name = ?"_L1);
        m_query->bindValue(0, filterAttribute);
        if (!m_query->exec() || !m_query->next())
            return false;

        const int attributeId = m_query->value(0).toInt();

        QVariantList attributeIds;
        for (int i = 0; i < it.value().size(); i++)
            attributeIds.append(attributeId);

        m_query->prepare("INSERT INTO IndexFilterTable VALUES(?, ?)"_L1);
        m_query->addBindValue(attributeIds);
        m_query->addBindValue(it.value());
        if (!m_query->execBatch())
            return false;
    }

    QMap<QString, QVariantList> filterAttributeToNewContentsId;

    QVariantList contentsNsIds;
    QVariantList contentsData;
    const int contentsSize = indexTable.contentsItems.size();
    contentsNsIds.reserve(contentsSize);
    contentsData.reserve(contentsSize);

    if (!m_query->exec("SELECT MAX(Id) FROM ContentsTable"_L1) || !m_query->next())
        return false;

    const int maxContentsId = m_query->value(0).toInt();

    int newContentsId = 0;
    for (const QHelpDBReader::ContentsItem &item : indexTable.contentsItems) {
        contentsNsIds.append(nsId);
        contentsData.append(item.data);

        for (const QString &filterAttribute : item.filterAttributes) {
            filterAttributeToNewContentsId[filterAttribute]
                    .append(maxContentsId + newContentsId + 1);
        }
        ++newContentsId;
    }

    m_query->prepare("INSERT INTO ContentsTable VALUES(NULL, ?, ?)"_L1);
    m_query->addBindValue(contentsNsIds);
    m_query->addBindValue(contentsData);
    if (!m_query->execBatch())
        return false;

    for (auto it = filterAttributeToNewContentsId.cbegin(),
         end = filterAttributeToNewContentsId.cend(); it != end; ++it) {
        const QString filterAttribute = it.key();
        m_query->prepare("SELECT Id From FilterAttributeTable WHERE Name = ?"_L1);
        m_query->bindValue(0, filterAttribute);
        if (!m_query->exec() || !m_query->next())
            return false;

        const int attributeId = m_query->value(0).toInt();

        QVariantList attributeIds;
        for (int i = 0; i < it.value().size(); i++)
            attributeIds.append(attributeId);

        m_query->prepare("INSERT INTO ContentsFilterTable VALUES(?, ?)"_L1);
        m_query->addBindValue(attributeIds);
        m_query->addBindValue(it.value());
        if (!m_query->execBatch())
            return false;
    }

    QVariantList filterNsIds;
    QVariantList filterAttributeIds;
    for (const QString &filterAttribute : indexTable.usedFilterAttributes) {
        filterNsIds.append(nsId);

        m_query->prepare("SELECT Id From FilterAttributeTable WHERE Name = ?"_L1);
        m_query->bindValue(0, filterAttribute);
        if (!m_query->exec() || !m_query->next())
            return false;

        filterAttributeIds.append(m_query->value(0).toInt());
    }

    m_query->prepare("INSERT INTO OptimizedFilterTable "
                     "(NamespaceId, FilterAttributeId) VALUES(?, ?)"_L1);
    m_query->addBindValue(filterNsIds);
    m_query->addBindValue(filterAttributeIds);
    if (!m_query->execBatch())
        return false;

    m_query->prepare("INSERT INTO TimeStampTable "
                     "(NamespaceId, FolderId, FilePath, Size, TimeStamp) "
                     "VALUES(?, ?, ?, ?, ?)"_L1);
    m_query->addBindValue(nsId);
    m_query->addBindValue(vfId);
    m_query->addBindValue(fileName);
    const QFileInfo fi(absoluteDocPath(fileName));
    m_query->addBindValue(fi.size());
    QDateTime lastModified = fi.lastModified(QTimeZone::UTC);
    if (qEnvironmentVariableIsSet("SOURCE_DATE_EPOCH")) {
        const QString sourceDateEpochStr = qEnvironmentVariable("SOURCE_DATE_EPOCH");
        bool ok;
        const qlonglong sourceDateEpoch = sourceDateEpochStr.toLongLong(&ok);
        if (ok && sourceDateEpoch < lastModified.toSecsSinceEpoch())
            lastModified.setSecsSinceEpoch(sourceDateEpoch);
    }
    m_query->addBindValue(lastModified);
    if (!m_query->exec())
        return false;

    transaction.commit();
    return true;
}

bool QHelpCollectionHandler::unregisterIndexTable(int nsId, int vfId)
{
    m_query->prepare("DELETE FROM IndexFilterTable WHERE IndexId IN "
                         "(SELECT Id FROM IndexTable WHERE NamespaceId = ?)"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM IndexTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM FileFilterTable WHERE FileId IN "
                         "(SELECT FileId FROM FileNameTable WHERE FolderId = ?)"_L1);
    m_query->bindValue(0, vfId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM FileNameTable WHERE FolderId = ?"_L1);
    m_query->bindValue(0, vfId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM ContentsFilterTable WHERE ContentsId IN "
                         "(SELECT Id FROM ContentsTable WHERE NamespaceId = ?)"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM ContentsTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM FileAttributeSetTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM OptimizedFilterTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM TimeStampTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("DELETE FROM VersionTable WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("SELECT ComponentId FROM ComponentMapping WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    if (!m_query->next())
        return false;

    const int componentId = m_query->value(0).toInt();

    m_query->prepare("DELETE FROM ComponentMapping WHERE NamespaceId = ?"_L1);
    m_query->bindValue(0, nsId);
    if (!m_query->exec())
        return false;

    m_query->prepare("SELECT ComponentId FROM ComponentMapping WHERE ComponentId = ?"_L1);
    m_query->bindValue(0, componentId);
    if (!m_query->exec())
        return false;

    if (!m_query->next()) { // no more namespaces refer to the componentId
        m_query->prepare("DELETE FROM ComponentTable WHERE ComponentId = ?"_L1);
        m_query->bindValue(0, componentId);
        if (!m_query->exec())
            return false;
    }

    return true;
}

QUrl QHelpCollectionHandler::buildQUrl(const QString &ns, const QString &folder,
                                       const QString &relFileName, const QString &anchor)
{
    QUrl url;
    url.setScheme("qthelp"_L1);
    url.setAuthority(ns);
    url.setPath(u'/' + folder + u'/' + relFileName);
    url.setFragment(anchor);
    return url;
}

QList<QHelpLink> QHelpCollectionHandler::documentsForIdentifier(
        const QString &id, const QStringList &filterAttributes) const
{
    return documentsForField("Identifier"_L1, id, filterAttributes);
}

QList<QHelpLink> QHelpCollectionHandler::documentsForKeyword(
        const QString &keyword, const QStringList &filterAttributes) const
{
    return documentsForField("Name"_L1, keyword, filterAttributes);
}

QList<QHelpLink> QHelpCollectionHandler::documentsForField(const QString &fieldName,
        const QString &fieldValue, const QStringList &filterAttributes) const
{
    if (!isDBOpened())
        return {};

    const QString filterlessQuery =
        "SELECT "
            "FileNameTable.Title, "
            "NamespaceTable.Name, "
            "FolderTable.Name, "
            "FileNameTable.Name, "
            "IndexTable.Anchor "
        "FROM "
            "IndexTable, "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE IndexTable.FileId = FileNameTable.FileId "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND IndexTable.NamespaceId = NamespaceTable.Id "
        "AND IndexTable.%1 = ?"_L1.arg(fieldName);

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterAttributes.size(), "IndexTable"_L1, "Id"_L1,
                                 "IndexFilterTable"_L1, "IndexId"_L1);

    m_query->prepare(filterQuery);
    m_query->bindValue(0, fieldValue);
    bindFilterQuery(m_query.get(), 1, filterAttributes);

    m_query->exec();

    QList<QHelpLink> docList;
    while (m_query->next()) {
        QString title = m_query->value(0).toString();
        if (title.isEmpty()) // generate a title + corresponding path
            title = fieldValue + " : "_L1 + m_query->value(3).toString();

        const QUrl url = buildQUrl(m_query->value(1).toString(),
                                   m_query->value(2).toString(),
                                   m_query->value(3).toString(),
                                   m_query->value(4).toString());
        docList.append(QHelpLink {url, title});
    }
    return docList;
}

QList<QHelpLink> QHelpCollectionHandler::documentsForIdentifier(
        const QString &id, const QString &filterName) const
{
    return documentsForField("Identifier"_L1, id, filterName);
}

QList<QHelpLink> QHelpCollectionHandler::documentsForKeyword(
        const QString &keyword, const QString &filterName) const
{
    return documentsForField("Name"_L1, keyword, filterName);
}

QMultiMap<QString, QUrl> QHelpCollectionHandler::linksForField(const QString &fieldName,
        const QString &fieldValue, const QString &filterName) const
{
    QMultiMap<QString, QUrl> linkMap;
    const auto documents = documentsForField(fieldName, fieldValue, filterName);
    for (const auto &document : documents)
        linkMap.insert(document.title, document.url);
    return linkMap;
}

QList<QHelpLink> QHelpCollectionHandler::documentsForField(const QString &fieldName,
        const QString &fieldValue, const QString &filterName) const
{
    if (!isDBOpened())
        return {};

    const QString filterlessQuery =
        "SELECT "
            "FileNameTable.Title, "
            "NamespaceTable.Name, "
            "FolderTable.Name, "
            "FileNameTable.Name, "
            "IndexTable.Anchor "
        "FROM "
            "IndexTable, "
            "FileNameTable, "
            "FolderTable, "
            "NamespaceTable "
        "WHERE IndexTable.FileId = FileNameTable.FileId "
        "AND FileNameTable.FolderId = FolderTable.Id "
        "AND IndexTable.NamespaceId = NamespaceTable.Id "
        "AND IndexTable.%1 = ?"_L1.arg(fieldName);

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterName)
            + " ORDER BY LOWER(FileNameTable.Title), FileNameTable.Title"_L1;

    m_query->prepare(filterQuery);
    m_query->bindValue(0, fieldValue);
    bindFilterQuery(m_query.get(), 1, filterName);

    m_query->exec();

    QList<QHelpLink> docList;
    while (m_query->next()) {
        QString title = m_query->value(0).toString();
        if (title.isEmpty()) // generate a title + corresponding path
            title = fieldValue + " : "_L1 + m_query->value(3).toString();

        const QUrl url = buildQUrl(m_query->value(1).toString(),
                                   m_query->value(2).toString(),
                                   m_query->value(3).toString(),
                                   m_query->value(4).toString());
        docList.append(QHelpLink {url, title});
    }
    return docList;
}

QStringList QHelpCollectionHandler::namespacesForFilter(const QString &filterName) const
{
    QStringList namespaceList;

    if (!isDBOpened())
        return namespaceList;

    const QString filterlessQuery =
        "SELECT "
            "NamespaceTable.Name "
        "FROM "
            "NamespaceTable "
        "WHERE TRUE"_L1;

    const QString filterQuery = filterlessQuery
            + prepareFilterQuery(filterName);

    m_query->prepare(filterQuery);
    bindFilterQuery(m_query.get(), 0, filterName);

    m_query->exec();

    while (m_query->next())
        namespaceList.append(m_query->value(0).toString());
    return namespaceList;
}

QT_END_NAMESPACE
