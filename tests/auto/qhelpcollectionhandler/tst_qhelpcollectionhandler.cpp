// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimeZone>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <QtHelp/QHelpContentItem>
#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpLink>

#include "../../../src/assistant/qhelpgenerator/helpgenerator.h"
#include "../../../src/assistant/qhelpgenerator/qhelpprojectdata_p.h"

#include <limits>
#include <memory>

using namespace Qt::StringLiterals;

// QHelpCollectionHandler isn't exported, so it's tested through QHelpEngineCore, which
// is how the collection files and the documentation files reach it. The fixtures are
// created by tampering with copies of a valid documentation file and of a valid
// collection file, the way a crafted or corrupted file may look like. The documentation
// file is generated from data/test.qhp.

static const QString DocNamespace = u"trolltech.com.1.0.0.test"_s;
static const QUrl TestFileUrl(u"qthelp://trolltech.com.1.0.0.test/testFolder/test.html"_s);
// The only keyword which references test.html.
static const QString TestKeyword = u"bar"_s;
// The id of test.html in the documentation file.
static const QString TestFileId =
        u"(SELECT FileId FROM FileNameTable WHERE Name = 'test.html')"_s;

static QByteArray readFile(const QString &fileName)
{
    QFile file(fileName);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

// Copies the file with its modification time, so that the time stamps stored in the
// collection file keep matching it.
static bool copyFile(const QString &source, const QString &target)
{
    if (!QFile::copy(source, target)
            || !QFile::setPermissions(target, QFile::ReadOwner | QFile::WriteOwner)) {
        return false;
    }
    QFile file(target);
    return file.open(QIODevice::ReadWrite)
            && file.setFileTime(QFileInfo(source).lastModified(QTimeZone::UTC),
                                QFileDevice::FileModificationTime);
}

static bool writeFile(const QString &fileName, const QByteArray &contents)
{
    QFile file(fileName);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate)
            && file.write(contents) == contents.size();
}

// Direct access to an SQLite file, bypassing Qt Help.
class Database
{
public:
    Q_DISABLE_COPY_MOVE(Database)

    explicit Database(const QString &fileName)
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(u"QSQLITE"_s, ConnectionName);
        db.setDatabaseName(fileName);
        m_open = db.open();
    }

    ~Database()
    {
        QSqlDatabase::database(ConnectionName, false).close();
        QSqlDatabase::removeDatabase(ConnectionName);
    }

    bool isOpen() const { return m_open; }

    bool exec(const QStringList &statements)
    {
        QSqlQuery query(QSqlDatabase::database(ConnectionName, false));
        for (const QString &statement : statements) {
            if (!query.exec(statement)) {
                qWarning() << statement << query.lastError().text();
                return false;
            }
        }
        return true;
    }

    // Returns the number of rows of every table, to check that nothing was written.
    QMap<QString, qint64> rowCounts()
    {
        QSqlQuery query(QSqlDatabase::database(ConnectionName, false));
        QStringList tables;
        if (query.exec(u"SELECT Name FROM sqlite_master WHERE type = 'table'"_s)) {
            while (query.next())
                tables.append(query.value(0).toString());
        }
        QMap<QString, qint64> result;
        for (const QString &table : std::as_const(tables)) {
            if (query.exec(u"SELECT COUNT(*) FROM %1"_s.arg(table)) && query.next())
                result.insert(table, query.value(0).toLongLong());
            else
                result.insert(table, -1);
        }
        return result;
    }

private:
    static inline const QString ConnectionName = u"tst_qhelpcollectionhandler"_s;
    bool m_open = false;
};

static QMap<QString, qint64> rowCounts(const QString &fileName)
{
    Database db(fileName);
    return db.rowCounts();
}

static bool execSql(const QString &fileName, const QStringList &statements)
{
    Database db(fileName);
    return db.isOpen() && db.exec(statements);
}

class tst_QHelpCollectionHandler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void setupMalformedCollection_data();
    void setupMalformedCollection();
    void setupLargeIds_data();
    void setupLargeIds();
    void setupMalformedTimeStamps_data();
    void setupMalformedTimeStamps();
    void danglingReferences_data();
    void danglingReferences();

    void registerNonDocumentationFile_data();
    void registerNonDocumentationFile();
    void registerMalformedDocumentation_data();
    void registerMalformedDocumentation();
    void registerFileNotFoundEntry();
    void registerIdsExhausted_data();
    void registerIdsExhausted();

    void fileDataMalformed_data();
    void fileDataMalformed();
    void requestContentMalformed_data();
    void requestContentMalformed();

private:
    QString documentationFile(const QStringList &statements = {}) const;
    QString emptyCollectionFile() const;
    QString validCollectionFile() const;

    QTemporaryDir m_generatedDir;
    QString m_testQch;
    QByteArray m_emptyCollection;
    QByteArray m_validCollection;
    std::unique_ptr<QTemporaryDir> m_dir;
};

void tst_QHelpCollectionHandler::initTestCase()
{
    QVERIFY(m_generatedDir.isValid());
    m_testQch = m_generatedDir.filePath(u"test.qch"_s);
    {
        QHelpProjectData data;
        QVERIFY(data.readData(QLatin1String(SRCDIR) + "/data/test.qhp"_L1));
        HelpGenerator generator(true);
        QVERIFY(generator.generate(&data, m_testQch));
    }

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString emptyColFile = dir.filePath(u"empty.qhc"_s);
    {
        QHelpEngineCore c(emptyColFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
    }
    m_emptyCollection = readFile(emptyColFile);
    QVERIFY(!m_emptyCollection.isEmpty());

    // The documentation is registered with a path relative to the collection file,
    // so the collection file stays valid when copied next to a copy of test.qch.
    const QString qchFile = dir.filePath(u"test.qch"_s);
    QVERIFY(copyFile(m_testQch, qchFile));
    const QString validColFile = dir.filePath(u"valid.qhc"_s);
    {
        QHelpEngineCore c(validColFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
        QVERIFY(c.registerDocumentation(qchFile));
        QCOMPARE(c.registeredDocumentations(), QStringList(DocNamespace));
    }
    m_validCollection = readFile(validColFile);
    QVERIFY(!m_validCollection.isEmpty());
}

void tst_QHelpCollectionHandler::init()
{
    m_dir.reset(new QTemporaryDir);
    QVERIFY(m_dir->isValid());
}

// Returns a writable copy of test.qch, with the statements applied.
QString tst_QHelpCollectionHandler::documentationFile(const QStringList &statements) const
{
    const QString fileName = m_dir->filePath(u"test.qch"_s);
    if (!copyFile(m_testQch, fileName))
        return {};
    if (!statements.isEmpty() && !execSql(fileName, statements))
        return {};
    return fileName;
}

// Returns a collection file without any documentation registered.
QString tst_QHelpCollectionHandler::emptyCollectionFile() const
{
    const QString fileName = m_dir->filePath(u"collection.qhc"_s);
    return writeFile(fileName, m_emptyCollection) ? fileName : QString();
}

// Returns a collection file with a copy of test.qch registered next to it.
QString tst_QHelpCollectionHandler::validCollectionFile() const
{
    const QString fileName = m_dir->filePath(u"collection.qhc"_s);
    if (documentationFile().isEmpty() || !writeFile(fileName, m_validCollection))
        return {};
    return fileName;
}

void tst_QHelpCollectionHandler::setupMalformedCollection_data()
{
    QTest::addColumn<QByteArray>("contents");
    QTest::addColumn<bool>("readOnly");

    const QByteArray garbage = QByteArray("This is not a collection file.\n").repeated(512);
    const QByteArray textWithHeader = QByteArray("SQLite format 3") + '\0' + garbage;
    const QByteArray headerOnly = m_validCollection.left(100);
    const QByteArray firstPage = m_validCollection.left(4096);
    const QByteArray truncated = m_validCollection.left(m_validCollection.size() / 2);

    for (const bool readOnly : {false, true}) {
        const char *mode = readOnly ? "read-only" : "writable";
        QTest::addRow("garbage-%s", mode) << garbage << readOnly;
        QTest::addRow("text-with-header-%s", mode) << textWithHeader << readOnly;
        QTest::addRow("header-only-%s", mode) << headerOnly << readOnly;
        QTest::addRow("first-page-%s", mode) << firstPage << readOnly;
        QTest::addRow("truncated-%s", mode) << truncated << readOnly;
    }
}

void tst_QHelpCollectionHandler::setupMalformedCollection()
{
    QFETCH(QByteArray, contents);
    QFETCH(bool, readOnly);

    const QString colFile = m_dir->filePath(u"collection.qhc"_s);
    QVERIFY(writeFile(colFile, contents));

    {
        QHelpEngineCore c(colFile);
        c.setReadOnly(readOnly);
        QVERIFY(!c.setupData());
        QVERIFY(!c.error().isEmpty());
        QVERIFY(c.registeredDocumentations().isEmpty());
        QVERIFY(c.fileData(TestFileUrl).isEmpty());
    }

    // A file which isn't a valid collection file must not be written to.
    QCOMPARE(readFile(colFile), contents);
}

void tst_QHelpCollectionHandler::setupLargeIds_data()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("column");
    QTest::addColumn<qint64>("id");
    QTest::addColumn<bool>("readOnly");

    static constexpr std::pair<const char *, const char *> idColumns[] = {
        {"NamespaceTable", "Id"},
        {"FolderTable", "Id"},
        {"FilterAttributeTable", "Id"},
        {"FilterNameTable", "Id"},
        {"FileNameTable", "FileId"},
        {"IndexTable", "Id"},
        {"ContentsTable", "Id"},
        {"Filter", "FilterId"},
        {"ComponentTable", "ComponentId"}
    };

    constexpr qint64 intMax = std::numeric_limits<int>::max();
    constexpr qint64 intMin = std::numeric_limits<int>::min();
    const std::pair<const char *, qint64> ids[] = {
        {"int-max", intMax},
        {"int-min", intMin},
        {"above-int-max", intMax + 1},
        {"below-int-min", intMin - 1},
        {"int64-max", std::numeric_limits<qint64>::max()}
    };

    for (const bool readOnly : {false, true}) {
        for (const auto &[table, column] : idColumns) {
            for (const auto &[idName, id] : ids) {
                QTest::addRow("%s-%s-%s", table, idName, readOnly ? "read-only" : "writable")
                        << QString::fromLatin1(table) << QString::fromLatin1(column) << id
                        << readOnly;
            }
        }
    }
}

void tst_QHelpCollectionHandler::setupLargeIds()
{
    QFETCH(QString, table);
    QFETCH(QString, column);
    QFETCH(qint64, id);
    QFETCH(bool, readOnly);

    const QString colFile = validCollectionFile();
    QVERIFY(!colFile.isEmpty());
    QVERIFY(execSql(colFile, {u"INSERT INTO %1 (%2) VALUES (%3)"_s.arg(table, column)
                                                               .arg(id)}));

    QHelpEngineCore c(colFile);
    c.setReadOnly(readOnly);
    QVERIFY(c.setupData());
    QCOMPARE(c.registeredDocumentations(), QStringList(DocNamespace));
    QCOMPARE(c.documentsForKeyword(TestKeyword).size(), 1);
}

void tst_QHelpCollectionHandler::setupMalformedTimeStamps_data()
{
    QTest::addColumn<QStringList>("statements");

    QTest::newRow("missing") << QStringList{u"DELETE FROM TimeStampTable"_s};
    QTest::newRow("null") << QStringList{u"UPDATE TimeStampTable SET NamespaceId = NULL, "
                                          "FolderId = NULL, FilePath = NULL, Size = NULL, "
                                          "TimeStamp = NULL"_s};
    QTest::newRow("garbage-size") << QStringList{u"UPDATE TimeStampTable SET Size = 'garbage'"_s};
    QTest::newRow("negative-size") << QStringList{u"UPDATE TimeStampTable SET Size = -1"_s};
    QTest::newRow("garbage-timestamp")
            << QStringList{u"UPDATE TimeStampTable SET TimeStamp = 'garbage'"_s};
    QTest::newRow("blob-timestamp")
            << QStringList{u"UPDATE TimeStampTable SET TimeStamp = x'00ff00ff'"_s};
    QTest::newRow("missing-file")
            << QStringList{u"UPDATE TimeStampTable SET FilePath = 'missing.qch'"_s};
    QTest::newRow("unknown-namespace")
            << QStringList{u"UPDATE TimeStampTable SET NamespaceId = 100000"_s};
    QTest::newRow("unknown-folder")
            << QStringList{u"UPDATE TimeStampTable SET FolderId = 100000"_s};
    QTest::newRow("duplicated")
            << QStringList{u"INSERT INTO TimeStampTable SELECT * FROM TimeStampTable"_s};
}

void tst_QHelpCollectionHandler::setupMalformedTimeStamps()
{
    QFETCH(QStringList, statements);

    const QString colFile = validCollectionFile();
    QVERIFY(!colFile.isEmpty());
    QMap<QString, qint64> countsBefore = rowCounts(colFile);
    QVERIFY(execSql(colFile, statements));

    {
        // An invalid time stamp makes the documentation to be indexed again.
        QHelpEngineCore c(colFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
        QCOMPARE(c.registeredDocumentations(), QStringList(DocNamespace));
        QCOMPARE(c.documentsForKeyword(TestKeyword).size(), 1);
        QVERIFY(!c.fileData(TestFileUrl).isEmpty());
    }

    // The documentation is indexed exactly once, without leftovers of the old index.
    // Valid time stamps are kept as they are, even if duplicated.
    QMap<QString, qint64> countsAfter = rowCounts(colFile);
    countsBefore.remove(u"TimeStampTable"_s);
    countsAfter.remove(u"TimeStampTable"_s);
    QCOMPARE(countsAfter, countsBefore);
}

void tst_QHelpCollectionHandler::danglingReferences_data()
{
    QTest::addColumn<QStringList>("statements");
    QTest::addColumn<QStringList>("registered");
    QTest::addColumn<int>("keywordLinks");
    QTest::addColumn<bool>("hasFileData");

    const QStringList ns(DocNamespace);

    QTest::newRow("valid") << QStringList() << ns << 1 << true;
    QTest::newRow("keyword-unknown-file")
            << QStringList{u"UPDATE IndexTable SET FileId = 100000 WHERE Name = 'bar'"_s}
            << ns << 0 << true;
    QTest::newRow("keyword-null-file")
            << QStringList{u"UPDATE IndexTable SET FileId = NULL WHERE Name = 'bar'"_s}
            << ns << 0 << true;
    QTest::newRow("keyword-unknown-namespace")
            << QStringList{u"UPDATE IndexTable SET NamespaceId = 100000 WHERE Name = 'bar'"_s}
            << ns << 0 << true;
    QTest::newRow("file-unknown-folder")
            << QStringList{u"UPDATE FileNameTable SET FolderId = 100000 "
                            "WHERE Name = 'test.html'"_s}
            << ns << 0 << false;
    QTest::newRow("folder-unknown-namespace")
            << QStringList{u"UPDATE FolderTable SET NamespaceId = 100000"_s}
            << QStringList() << 1 << false;
    QTest::newRow("no-folder")
            << QStringList{u"DELETE FROM FolderTable"_s}
            << QStringList() << 0 << false;
    QTest::newRow("no-namespace")
            << QStringList{u"DELETE FROM NamespaceTable"_s}
            << QStringList() << 0 << false;
    QTest::newRow("documentation-missing")
            << QStringList{u"UPDATE NamespaceTable SET FilePath = 'missing.qch'"_s}
            << ns << 1 << false;
    QTest::newRow("documentation-null-path")
            << QStringList{u"UPDATE NamespaceTable SET FilePath = NULL"_s}
            << ns << 1 << false;
    QTest::newRow("documentation-not-sqlite")
            << QStringList{u"UPDATE NamespaceTable SET FilePath = 'garbage.qch'"_s}
            << ns << 1 << false;
    QTest::newRow("documentation-is-collection")
            << QStringList{u"UPDATE NamespaceTable SET FilePath = 'collection.qhc'"_s}
            << ns << 1 << false;
}

void tst_QHelpCollectionHandler::danglingReferences()
{
    QFETCH(QStringList, statements);
    QFETCH(QStringList, registered);
    QFETCH(int, keywordLinks);
    QFETCH(bool, hasFileData);

    const QString colFile = validCollectionFile();
    QVERIFY(!colFile.isEmpty());
    QVERIFY(writeFile(m_dir->filePath(u"garbage.qch"_s),
                      QByteArray("This is not a documentation file.\n").repeated(512)));
    if (!statements.isEmpty())
        QVERIFY(execSql(colFile, statements));
    const QByteArray contentsBefore = readFile(colFile);

    {
        // In read-only mode the collection file is used as is, without being repaired.
        QHelpEngineCore c(colFile);
        QVERIFY(c.isReadOnly());
        QVERIFY(c.setupData());
        QCOMPARE(c.registeredDocumentations(), registered);

        const QList<QHelpLink> links = c.documentsForKeyword(TestKeyword);
        QCOMPARE(links.size(), keywordLinks);
        for (const QHelpLink &link : links) {
            QCOMPARE(link.url.scheme(), u"qthelp"_s);
            QCOMPARE(link.url.authority(), DocNamespace);
        }
        QCOMPARE(!c.fileData(TestFileUrl).isEmpty(), hasFileData);
    }
    QCOMPARE(readFile(colFile), contentsBefore);
}

void tst_QHelpCollectionHandler::registerNonDocumentationFile_data()
{
    QTest::addColumn<QByteArray>("contents");

    const QByteArray garbage = QByteArray("This is not a documentation file.\n").repeated(512);
    const QByteArray qch = readFile(m_testQch);

    QTest::newRow("empty") << QByteArray();
    QTest::newRow("garbage") << garbage;
    QTest::newRow("text-with-header") << QByteArray("SQLite format 3") + '\0' + garbage;
    QTest::newRow("header-only") << qch.left(100);
    QTest::newRow("first-page") << qch.left(1024);
    QTest::newRow("truncated") << qch.left(qch.size() / 2);
    QTest::newRow("empty-collection") << m_emptyCollection;
}

void tst_QHelpCollectionHandler::registerNonDocumentationFile()
{
    QFETCH(QByteArray, contents);

    const QString qchFile = m_dir->filePath(u"test.qch"_s);
    QVERIFY(writeFile(qchFile, contents));
    const QString colFile = emptyCollectionFile();
    QVERIFY(!colFile.isEmpty());
    const QMap<QString, qint64> countsBefore = rowCounts(colFile);

    {
        QHelpEngineCore c(colFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
        QVERIFY(QHelpEngineCore::namespaceName(qchFile).isEmpty());
        QVERIFY(!c.registerDocumentation(qchFile));
        QVERIFY(!c.error().isEmpty());
        QVERIFY(c.registeredDocumentations().isEmpty());
    }

    QCOMPARE(rowCounts(colFile), countsBefore);
    QCOMPARE(readFile(qchFile), contents);
}

void tst_QHelpCollectionHandler::registerMalformedDocumentation_data()
{
    QTest::addColumn<QStringList>("statements");
    QTest::addColumn<bool>("expected");

    QTest::newRow("valid") << QStringList() << true;
    QTest::newRow("no-namespace") << QStringList{u"DELETE FROM NamespaceTable"_s} << false;
    QTest::newRow("empty-namespace")
            << QStringList{u"UPDATE NamespaceTable SET Name = ''"_s} << false;
    QTest::newRow("null-namespace")
            << QStringList{u"UPDATE NamespaceTable SET Name = NULL"_s} << false;
    QTest::newRow("no-namespace-table") << QStringList{u"DROP TABLE NamespaceTable"_s} << false;
    QTest::newRow("no-folder-table") << QStringList{u"DROP TABLE FolderTable"_s} << false;
    QTest::newRow("no-file-name-table") << QStringList{u"DROP TABLE FileNameTable"_s} << false;
    QTest::newRow("keyword-unknown-file")
            << QStringList{u"UPDATE IndexTable SET FileId = 1000 WHERE Name = 'bar'"_s}
            << false;
    QTest::newRow("keyword-negative-file")
            << QStringList{u"UPDATE IndexTable SET FileId = -1 WHERE Name = 'bar'"_s} << false;
    QTest::newRow("keyword-null-file")
            << QStringList{u"UPDATE IndexTable SET FileId = NULL WHERE Name = 'bar'"_s}
            << false;
    QTest::newRow("keyword-file-outside-folder")
            << QStringList{u"UPDATE FileNameTable SET FolderId = 99 WHERE Name = 'test.html'"_s}
            << false;
}

void tst_QHelpCollectionHandler::registerMalformedDocumentation()
{
    QFETCH(QStringList, statements);
    QFETCH(bool, expected);

    const QString qchFile = documentationFile(statements);
    QVERIFY(!qchFile.isEmpty());
    const QString colFile = emptyCollectionFile();
    QVERIFY(!colFile.isEmpty());
    const QMap<QString, qint64> countsBefore = rowCounts(colFile);

    {
        QHelpEngineCore c(colFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
        QCOMPARE(c.registerDocumentation(qchFile), expected);
        if (expected) {
            QCOMPARE(c.registeredDocumentations(), QStringList(DocNamespace));
            QCOMPARE(c.documentsForKeyword(TestKeyword).size(), 1);
            return;
        }
        QVERIFY(!c.error().isEmpty());
        QVERIFY(c.registeredDocumentations().isEmpty());
    }

    // A failed registration leaves nothing behind.
    QCOMPARE(rowCounts(colFile), countsBefore);
}

void tst_QHelpCollectionHandler::registerFileNotFoundEntry()
{
    // The help generator writes an entry with no folder and no name for the keywords
    // referencing files which aren't part of the documentation, and makes them point at it.
    const QString qchFile = documentationFile(
            {u"INSERT INTO FileNameTable (FolderId, Name, FileId, Title) "
              "VALUES (0, '', 1000, '')"_s,
             u"UPDATE IndexTable SET FileId = 1000 WHERE Name = 'bar'"_s});
    QVERIFY(!qchFile.isEmpty());
    const QString colFile = emptyCollectionFile();
    QVERIFY(!colFile.isEmpty());

    QHelpEngineCore c(colFile);
    c.setReadOnly(false);
    QVERIFY(c.setupData());
    QVERIFY(c.registerDocumentation(qchFile));
    QVERIFY(c.error().isEmpty());
    QCOMPARE(c.registeredDocumentations(), QStringList(DocNamespace));
    // The keyword without a target is left out, the other ones are registered.
    QCOMPARE(c.documentsForKeyword(TestKeyword).size(), 0);
    QCOMPARE(c.documentsForKeyword(u"einstein"_s).size(), 1);
}

void tst_QHelpCollectionHandler::registerIdsExhausted_data()
{
    QTest::addColumn<QString>("table");
    QTest::addColumn<QString>("column");
    QTest::addColumn<bool>("expected");

    // Registering test.qch adds rows to all of these tables. The ids of the rows inserted
    // one by one are chosen by SQLite, so they don't run out. The ids of the batch inserted
    // rows are computed, so registering must fail when they would overflow.
    QTest::newRow("NamespaceTable") << u"NamespaceTable"_s << u"Id"_s << true;
    QTest::newRow("FolderTable") << u"FolderTable"_s << u"Id"_s << true;
    QTest::newRow("FilterAttributeTable") << u"FilterAttributeTable"_s << u"Id"_s << true;
    QTest::newRow("FileNameTable") << u"FileNameTable"_s << u"FileId"_s << false;
    QTest::newRow("IndexTable") << u"IndexTable"_s << u"Id"_s << false;
    QTest::newRow("ContentsTable") << u"ContentsTable"_s << u"Id"_s << false;
    QTest::newRow("ComponentTable") << u"ComponentTable"_s << u"ComponentId"_s << true;
}

void tst_QHelpCollectionHandler::registerIdsExhausted()
{
    QFETCH(QString, table);
    QFETCH(QString, column);
    QFETCH(bool, expected);

    const QString qchFile = documentationFile();
    QVERIFY(!qchFile.isEmpty());
    const QString colFile = emptyCollectionFile();
    QVERIFY(!colFile.isEmpty());
    QVERIFY(execSql(colFile, {u"INSERT INTO %1 (%2) VALUES (%3)"_s
                                      .arg(table, column)
                                      .arg(std::numeric_limits<qint64>::max())}));
    const QMap<QString, qint64> countsBefore = rowCounts(colFile);

    {
        QHelpEngineCore c(colFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
        QCOMPARE(c.registerDocumentation(qchFile), expected);
        if (expected) {
            QCOMPARE(c.registeredDocumentations(), QStringList(DocNamespace));
            QCOMPARE(c.documentsForKeyword(TestKeyword).size(), 1);
        } else {
            QVERIFY(!c.error().isEmpty());
            QVERIFY(c.registeredDocumentations().isEmpty());
        }
    }

    // A failed registration leaves nothing behind.
    if (!expected)
        QCOMPARE(rowCounts(colFile), countsBefore);
}

void tst_QHelpCollectionHandler::fileDataMalformed_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<bool>("expected");

    // The data is compressed with qCompress(), a 4-byte big-endian size followed by
    // a zlib stream. The zlib stream here is the one of an empty input.
    QTest::newRow("valid") << QString() << true;
    QTest::newRow("missing") << u"DELETE FROM FileDataTable WHERE Id = "_s + TestFileId
                             << false;
    QTest::newRow("null")
            << u"UPDATE FileDataTable SET Data = NULL WHERE Id = "_s + TestFileId << false;
    QTest::newRow("empty")
            << u"UPDATE FileDataTable SET Data = x'' WHERE Id = "_s + TestFileId << false;
    QTest::newRow("header-only")
            << u"UPDATE FileDataTable SET Data = x'0000' WHERE Id = "_s + TestFileId << false;
    QTest::newRow("declared-size-max")
            << u"UPDATE FileDataTable SET Data = x'ffffffff789c030000000001' WHERE Id = "_s
                       + TestFileId
            << false;
    QTest::newRow("declared-size-above-ratio")
            << u"UPDATE FileDataTable SET Data = x'00100000789c030000000001' WHERE Id = "_s
                       + TestFileId
            << false;
}

void tst_QHelpCollectionHandler::fileDataMalformed()
{
    QFETCH(QString, data);
    QFETCH(bool, expected);

    const QString qchFile = documentationFile(data.isEmpty() ? QStringList()
                                                             : QStringList(data));
    QVERIFY(!qchFile.isEmpty());
    const QString colFile = emptyCollectionFile();
    QVERIFY(!colFile.isEmpty());

    QHelpEngineCore c(colFile);
    c.setReadOnly(false);
    QVERIFY(c.setupData());
    QVERIFY(c.registerDocumentation(qchFile));
    QCOMPARE(!c.fileData(TestFileUrl).isEmpty(), expected);
}

void tst_QHelpCollectionHandler::requestContentMalformed_data()
{
    QTest::addColumn<QByteArray>("data");

    // The contents are a sequence of depth, link and title, serialized with QDataStream.
    QByteArray valid;
    {
        QDataStream s(&valid, QIODevice::WriteOnly);
        s << 0 << u"test.html"_s << u"Test"_s;
    }

    QTest::newRow("valid") << valid;
    QTest::newRow("empty") << QByteArray();
    QTest::newRow("garbage") << QByteArray("This is not a table of contents.\n").repeated(64);
    QTest::newRow("truncated") << valid.left(valid.size() - 3);
    QTest::newRow("depth-only") << valid.left(4);
    // The length of the link claims more data than there is.
    QTest::newRow("huge-string-length") << QByteArray::fromHex("000000007ffffffe0061");
    QTest::newRow("odd-string-length") << QByteArray::fromHex("00000000000000030061");
}

void tst_QHelpCollectionHandler::requestContentMalformed()
{
    QFETCH(QByteArray, data);

    // Keep only a single contents item.
    const QString qchFile = documentationFile(
            {u"DELETE FROM ContentsTable WHERE Id != (SELECT MIN(Id) FROM ContentsTable)"_s});
    QVERIFY(!qchFile.isEmpty());
    {
        Database db(qchFile);
        QVERIFY(db.isOpen());
        QSqlQuery query(QSqlDatabase::database(u"tst_qhelpcollectionhandler"_s, false));
        QVERIFY(query.prepare(u"UPDATE ContentsTable SET Data = ?"_s));
        query.addBindValue(data);
        QVERIFY(query.exec());
    }

    const QString colFile = emptyCollectionFile();
    QVERIFY(!colFile.isEmpty());

    QHelpEngineCore c(colFile);
    c.setReadOnly(false);
    c.setUsesFilterEngine(true);
    QVERIFY(c.setupData());
    QVERIFY(c.registerDocumentation(qchFile));

    const std::shared_ptr<QHelpContentItem> root = c.requestContent({}).result();
    QVERIFY(root);
    for (int i = 0; i < root->childCount(); ++i) {
        const QHelpContentItem *child = root->child(i);
        QVERIFY(child);
        QCOMPARE(child->parent(), root.get());
    }
}

QTEST_MAIN(tst_QHelpCollectionHandler)
#include "tst_qhelpcollectionhandler.moc"
