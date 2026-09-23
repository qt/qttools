// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>

#include <QtCore/QUrl>
#include <QtCore/QDataStream>
#include <QtCore/QFileInfo>
#include <QtCore/QScopeGuard>
#include <QtCore/QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <QtHelp/QHelpContentItem>
#include <QtHelp/QHelpEngineCore>

using namespace Qt::StringLiterals;

class tst_QHelpEngineCore : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void setupData();
    void setupDataMissingCollectionFile();
    void setupDataEmptyCollectionFile();
    void setupDataReadOnlyLocation();
    void collectionFile();
    void setCollectionFile();
    void copyCollectionFile();

    void namespaceName();
    void registeredDocumentations();
    void registerDocumentation();
    void registerDocumentationFailure();
    void unregisterDocumentation();
    void unregisterDocumentationWithoutFolder();
    void documentationFileName();

    void customFilters();
    void removeCustomFilter();
    void removeCustomFilterFailure();
    void addCustomFilter();
    void filterAttributes();
    void currentFilter();
    void setCurrentFilter();

    void filterAttributeSets();
    void files();
    void fileData();

    void customValue();
    void setCustomValue();
    void removeCustomValue();

    void setAutoSaveFilter();

    void metaData();

    void requestContentDepths_data();
    void requestContentDepths();

private:
    QString m_path;
    QString m_colFile;
};

void tst_QHelpEngineCore::init()
{
    // defined in profile
    m_path = QLatin1String(SRCDIR);

    m_path = QFileInfo(m_path).absoluteFilePath();

    m_colFile = m_path + QLatin1String("/data/col.qhc");
    if (QFile::exists(m_colFile))
        QDir::current().remove(m_colFile);
    if (!QFile::copy(m_path + "/data/collection.qhc", m_colFile))
        QFAIL("Cannot copy file!");
    QFile f(m_colFile);
    f.setPermissions(QFile::WriteUser|QFile::ReadUser);
}

void tst_QHelpEngineCore::setupData()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
}

void tst_QHelpEngineCore::setupDataMissingCollectionFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString colFile = dir.filePath(QLatin1String("missing.qhc"));

    QHelpEngineCore help(colFile, nullptr);
    QVERIFY(help.isReadOnly());
    QVERIFY(!help.setupData());
    QVERIFY(!help.error().isEmpty());
    // A read-only engine must not create the collection file.
    QVERIFY(!QFile::exists(colFile));
}

void tst_QHelpEngineCore::setupDataEmptyCollectionFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString colFile = dir.filePath(QLatin1String("empty.qhc"));
    {
        QFile file(colFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }

    QHelpEngineCore help(colFile, nullptr);
    QVERIFY(help.isReadOnly());
    QVERIFY(!help.setupData());
    QVERIFY(!help.error().isEmpty());
}

void tst_QHelpEngineCore::setupDataReadOnlyLocation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString colFile = dir.filePath(QLatin1String("collection.qhc"));
    QVERIFY(QFile::copy(m_path + "/data/collection.qhc", colFile));

    // Mimic a documentation set installed into a read-only location: neither the
    // collection file nor the directory holding it may be written to (QTBUG-72174).
    // Note that data/collection.qhc is in the old format, without the index tables,
    // so the engine must not attempt to add them here.
    QFile::setPermissions(colFile, QFile::ReadOwner);
    QFile::setPermissions(dir.path(), QFile::ReadOwner | QFile::ExeOwner);
    // Let QTemporaryDir remove its contents again when going out of scope.
    const auto permissionGuard = qScopeGuard([&dir, &colFile] {
        QFile::setPermissions(dir.path(),
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        QFile::setPermissions(colFile, QFile::ReadOwner | QFile::WriteOwner);
    });

    // Not every platform and file system enforces the permissions set above, and
    // the superuser bypasses them altogether. Check for what the test relies on
    // instead of guessing from the platform or from the user id.
    QFile probe(dir.filePath(QLatin1String("probe")));
    if (probe.open(QIODevice::WriteOnly)) {
        probe.close();
        probe.remove();
        QSKIP("The read-only permissions are not enforced in this environment.");
    }

    const qint64 sizeBefore = QFileInfo(colFile).size();
    const QStringList entriesBefore = QDir(dir.path()).entryList(QDir::Files, QDir::Name);

    QHelpEngineCore help(colFile, nullptr);
    QVERIFY(help.isReadOnly());
    QVERIFY(help.setupData());
    QVERIFY(help.error().isEmpty());
    QVERIFY(!help.registeredDocumentations().isEmpty());

    // The collection file must be left untouched, without any journal file next to it.
    QCOMPARE(QFileInfo(colFile).size(), sizeBefore);
    QCOMPARE(QDir(dir.path()).entryList(QDir::Files, QDir::Name), entriesBefore);
}

void tst_QHelpEngineCore::collectionFile()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.collectionFile(), QFileInfo(m_colFile).absoluteFilePath());
}

void tst_QHelpEngineCore::setCollectionFile()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.collectionFile(), QFileInfo(m_colFile).absoluteFilePath());

    QString col1File = m_path + QLatin1String("/data/collection1.qhc");
    help.setCollectionFile(col1File);
    QCOMPARE(help.collectionFile(), QFileInfo(col1File).absoluteFilePath());

    QStringList docs = help.registeredDocumentations();
    QCOMPARE(docs.size(), 1);
    QCOMPARE(docs.first(), QLatin1String("trolltech.com.1.0.0.test"));
}

void tst_QHelpEngineCore::copyCollectionFile()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.collectionFile(), QFileInfo(m_colFile).absoluteFilePath());

    QString copiedFile = m_path + QLatin1String("/collectionCopy.qhc");
    if (QFile::exists(copiedFile))
        QDir::current().remove(copiedFile);

    QCOMPARE(help.copyCollectionFile(copiedFile), true);

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "testdb");
        db.setDatabaseName(copiedFile);
        if (!db.open()) {
            QSqlDatabase::removeDatabase("testdb");
            QFAIL("Created database seems to be corrupt!");
        }
        QSqlQuery *m_query = new QSqlQuery(db);

        m_query->exec("SELECT Key, Value FROM SettingsTable");
        if (m_query->next()) {
            QCOMPARE(m_query->value(0).toString(), QString("CurrentFilter"));
            QCOMPARE(m_query->value(1).toString(), QString("unfiltered"));
        } else {
            QFAIL("Settingstable is corrupt!");
        }

        m_query->exec("SELECT NameId, FilterAttributeId FROM FilterTable");
        int i = 0;
        while (m_query->next()) {
            if (i == 3) {
                QCOMPARE(m_query->value(0).toInt(), 2);
                QCOMPARE(m_query->value(1).toInt(), 6);
            }
            ++i;
        }
        QCOMPARE(i, 7);
        m_query->clear();

        m_query->exec("SELECT Name, FilePath FROM NamespaceTable");
        i = 0;
        while (m_query->next()) {
            if (i == 0) {
                QCOMPARE(m_query->value(0).toString(), QString("trolltech.com.3-3-8.qmake"));
                QCOMPARE(m_query->value(1).toString(), QString("data/qmake-3.3.8.qch"));
            }
            ++i;
        }
        QCOMPARE(i, 3);

        m_query->clear();
        delete m_query;
    }
    QSqlDatabase::removeDatabase("testdb");
}

void tst_QHelpEngineCore::namespaceName()
{
    QCOMPARE(QHelpEngineCore::namespaceName(m_path + "/data/qmake-3.3.8.qch"),
        QString("trolltech.com.3-3-8.qmake"));
    QCOMPARE(QHelpEngineCore::namespaceName(m_path + "/data/linguist-3.3.8.qch"),
        QString("trolltech.com.3-3-8.linguist"));
}

void tst_QHelpEngineCore::registeredDocumentations()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    const QStringList docs = help.registeredDocumentations();
    QCOMPARE(docs.size(), 3);
    QStringList lst;
    lst << "trolltech.com.3-3-8.qmake" << "trolltech.com.4-3-0.qmake"
        << "trolltech.com.1.0.0.test";
    for (const QString &s : docs)
        lst.removeAll(s);
    QCOMPARE(lst.isEmpty(), true);
}

void tst_QHelpEngineCore::registerDocumentation()
{
    if (QFile::exists(m_colFile))
        QDir::current().remove(m_colFile);
    {
        QHelpEngineCore c(m_colFile);
        c.setReadOnly(false);
        QCOMPARE(c.setupData(), true);
        c.registerDocumentation(m_path + "/data/qmake-3.3.8.qch");
        QCOMPARE(c.registeredDocumentations().size(), 1);
        c.registerDocumentation(m_path + "/data/qmake-3.3.8.qch");
        QCOMPARE(c.registeredDocumentations().size(), 1);
        c.registerDocumentation(m_path + "/data/linguist-3.3.8.qch");
        QCOMPARE(c.registeredDocumentations().size(), 2);
    }

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "testdb");
        db.setDatabaseName(m_colFile);
        if (!db.open()) {
            QSqlDatabase::removeDatabase("testdb");
            QFAIL("Created database seems to be corrupt!");
        }
        QSqlQuery query(db);
        query.exec("SELECT FilePath FROM NamespaceTable WHERE "
            "Name=\'trolltech.com.3-3-8.linguist\'");
        if (query.next())
            QCOMPARE(query.value(0).toString(),
                QString("linguist-3.3.8.qch"));
        else
            QFAIL("Query error!");
    }
    QSqlDatabase::removeDatabase("testdb");
}

void tst_QHelpEngineCore::registerDocumentationFailure()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString colFile = dir.filePath("register.qhc");

    QHelpEngineCore c(colFile);
    c.setReadOnly(false);
    QVERIFY(c.setupData());

    const auto execQuery = [&colFile](const QString &statement) {
        const QString connectionName = "registerDocumentationFailure";
        int result = -1;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(colFile);
            if (db.open()) {
                QSqlQuery query(db);
                if (query.exec(statement))
                    result = query.next() ? query.value(0).toInt() : 0;
            }
        }
        QSqlDatabase::removeDatabase(connectionName);
        return result;
    };
    // Makes the registration fail after the namespace was added.
    QCOMPARE(execQuery("DROP TABLE TimeStampTable"), 0);

    QCOMPARE(c.registerDocumentation(m_path + "/data/qmake-3.3.8.qch"), false);
    QVERIFY(c.registeredDocumentations().isEmpty());
    QCOMPARE(execQuery("SELECT COUNT(*) FROM NamespaceTable"), 0);
    QCOMPARE(execQuery("SELECT COUNT(*) FROM FolderTable"), 0);
    QCOMPARE(execQuery("SELECT COUNT(*) FROM FileNameTable"), 0);
}

void tst_QHelpEngineCore::unregisterDocumentation()
{
    QHelpEngineCore c(m_colFile);
    c.setReadOnly(false);
    QCOMPARE(c.setupData(), true);
    QCOMPARE(c.registeredDocumentations().size(), 3);
    c.unregisterDocumentation("trolltech.com.3-3-8.qmake");
    QCOMPARE(c.registeredDocumentations().size(), 2);
    QCOMPARE(c.unregisterDocumentation("noexisting"), false);
}

void tst_QHelpEngineCore::unregisterDocumentationWithoutFolder()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString colFile = dir.filePath("unregister.qhc");
    const QString ns = "trolltech.com.3-3-8.qmake";
    const QString orphanNs = "orphan.namespace";

    {
        QHelpEngineCore c(colFile);
        c.setReadOnly(false);
        QVERIFY(c.setupData());
        QVERIFY(c.registerDocumentation(m_path + "/data/qmake-3.3.8.qch"));
    }

    const auto execQuery = [&colFile](const QString &statement) {
        const QString connectionName = "unregisterDocumentationWithoutFolder";
        int result = -1;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(colFile);
            if (db.open()) {
                QSqlQuery query(db);
                if (query.exec(statement))
                    result = query.next() ? query.value(0).toInt() : 0;
            }
        }
        QSqlDatabase::removeDatabase(connectionName);
        return result;
    };
    // A namespace left behind by a registration that failed before adding its folder.
    QCOMPARE(execQuery("INSERT INTO NamespaceTable VALUES(NULL, '%1', 'orphan.qch')"_L1
                       .arg(orphanNs)), 0);
    const int fileCount = execQuery("SELECT COUNT(*) FROM FileNameTable");
    QVERIFY(fileCount > 0);

    QHelpEngineCore c(colFile);
    c.setReadOnly(false);
    QVERIFY(c.setupData());
    QCOMPARE(c.unregisterDocumentation(orphanNs), false);
    QCOMPARE(execQuery("SELECT COUNT(*) FROM NamespaceTable"), 2);
    QCOMPARE(execQuery("SELECT COUNT(*) FROM FileNameTable"), fileCount);
    QCOMPARE(c.registeredDocumentations(), QStringList(ns));
}

void tst_QHelpEngineCore::documentationFileName()
{
    QHelpEngineCore c(m_colFile);
    QCOMPARE(c.setupData(), true);
    QCOMPARE(c.documentationFileName(QLatin1String("trolltech.com.3-3-8.qmake")),
        QString(m_path + "/data/qmake-3.3.8.qch"));
    QCOMPARE(c.documentationFileName(QLatin1String("trolltech.com.1.0.0.test")),
        QString(m_path + "/data/test.qch"));
    QCOMPARE(c.documentationFileName(QLatin1String("trolltech.com.empty")),
        QString());
}

void tst_QHelpEngineCore::customFilters()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    const QStringList custom = help.customFilters();
    QCOMPARE(custom.size(), 4);
    QStringList lst;
    lst << "qmake Manual" << "Custom Filter 1"
        << "Custom Filter 2" << "unfiltered";
    for (const QString &s : custom)
        lst.removeAll(s);
    QCOMPARE(lst.size(), 0);
}

void tst_QHelpEngineCore::removeCustomFilter()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    help.removeCustomFilter("Custom Filter 1");
    QStringList custom = help.customFilters();
    QCOMPARE(custom.size(), 3);
    QCOMPARE((bool)custom.contains("Custom Filter 1"), false);
}

void tst_QHelpEngineCore::removeCustomFilterFailure()
{
    const auto execQuery = [this](const QString &statement) {
        const QString connectionName = "removeCustomFilterFailure";
        int result = -1;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(m_colFile);
            if (db.open()) {
                QSqlQuery query(db);
                if (query.exec(statement))
                    result = query.next() ? query.value(0).toInt() : 0;
            }
        }
        QSqlDatabase::removeDatabase(connectionName);
        return result;
    };

    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    // Makes removing the filter attributes of the filter fail.
    QCOMPARE(execQuery("DROP TABLE FilterTable"), 0);

    QCOMPARE(help.removeCustomFilter("Custom Filter 1"), false);
    QVERIFY(help.customFilters().contains("Custom Filter 1"));
}

void tst_QHelpEngineCore::addCustomFilter()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    help.addCustomFilter("Qt Tools", QStringList() << "tools" << "qt");
    QStringList custom = help.customFilters();
    QCOMPARE(custom.size(), 5);
    QCOMPARE((bool)custom.contains("Qt Tools"), true);
}

void tst_QHelpEngineCore::filterAttributes()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    const QStringList atts = help.filterAttributes("qmake Manual");
    QCOMPARE(atts.size(), 3);
    QStringList lst;
    lst << "qmake" << "tools" << "qt";
    for (const QString &s : atts)
        lst.removeAll(s);
    QCOMPARE(lst.size(), 0);
}

void tst_QHelpEngineCore::currentFilter()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    QCOMPARE(help.currentFilter(), QString("unfiltered"));
}

void tst_QHelpEngineCore::setCurrentFilter()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    QCOMPARE(help.currentFilter(), QString("unfiltered"));
    help.setCurrentFilter("qmake Manual");
    QCOMPARE(help.currentFilter(), QString("qmake Manual"));
    QCOMPARE(help.customValue("CurrentFilter").toString(),
        QString("qmake Manual"));
}

void tst_QHelpEngineCore::filterAttributeSets()
{
    QHelpEngineCore help(m_colFile, 0);
    help.setReadOnly(false);
    QCOMPARE(help.setupData(), true);
    QList<QStringList> lst = help.filterAttributeSets("trolltech.com.1.0.0.test");
    QCOMPARE(lst.size(), 2);
    QCOMPARE(lst.first().size(), 2);
    QCOMPARE((bool)lst.first().contains("filter1"), true);
    QCOMPARE((bool)lst.last().contains("filter2"), true);
}

void tst_QHelpEngineCore::files()
{
    QHelpEngineCore help(m_colFile, 0);
    help.setReadOnly(false);
    QCOMPARE(help.setupData(), true);
    QList<QUrl> lst = help.files("trolltech.com.4-3-0.qmake",
        QStringList());
    QCOMPARE(lst.size(), 16);
    lst = help.files("trolltech.com.4-3-0.qmake",
        QStringList(), "png");
    QCOMPARE(lst.size(), 2);
    lst = help.files("trolltech.com.4-3-0.qmake",
        QStringList() << "qt", "html");
    QCOMPARE(lst.size(), 13);
    lst = help.files("trolltech.com.4-3-0.qmake",
        QStringList() << "qt" << "qmake", "html");
    QCOMPARE(lst.size(), 13);
    lst = help.files("trolltech.com.4-3-0.qmake",
        QStringList() << "qt" << "qmake" << "bla", "html");
    QCOMPARE(lst.size(), 0);
    lst = help.files("trolltech.com.4-3-0.qmake",
        QStringList() << "qt" << "qmake", "foo");

    // print 'lst' if test fails:
    auto printRemainder = qScopeGuard([&]{ for (const QUrl &url : lst) qDebug() << url; });

    QCOMPARE(lst.size(), 0);

    printRemainder.dismiss();
}

void tst_QHelpEngineCore::fileData()
{
    QHelpEngineCore help(m_colFile, 0);
    help.setReadOnly(false);
    QCOMPARE(help.setupData(), true);
    QByteArray ba = help.fileData(QUrl("NotExisting"));
    QCOMPARE(ba.size(), 0);
    ba = help.fileData(QUrl("qthelp://trolltech.com.1.0.0.test/testFolder/test.html"));
    QTextStream s(ba, QIODevice::ReadOnly|QIODevice::Text);
    QFile f(m_path + "/data/test.html");
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text))
        QFAIL("Cannot open original file!");
    QTextStream ts(&f);
    QCOMPARE(s.readAll(), ts.readAll());
}

void tst_QHelpEngineCore::customValue()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    QCOMPARE(help.customValue("CurrentFilter").toString(),
        QString("unfiltered"));
}

void tst_QHelpEngineCore::setCustomValue()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    QCOMPARE(help.setCustomValue("Test", 3), true);
    QCOMPARE(help.customValue("Test").toInt(), 3);
    QCOMPARE(help.removeCustomValue("Test"), true);
    QCOMPARE(help.customValue("Test"), QVariant());
}

void tst_QHelpEngineCore::removeCustomValue()
{
    setCustomValue();
}

void tst_QHelpEngineCore::setAutoSaveFilter()
{
    QHelpEngineCore help(m_colFile, 0);
    QCOMPARE(help.setupData(), true);
    QCOMPARE(help.currentFilter(), QString("unfiltered"));

    help.setAutoSaveFilter(false);
    help.setCurrentFilter("qmake Manual");
    QCOMPARE(help.currentFilter(), QString("qmake Manual"));
    QCOMPARE(help.customValue("CurrentFilter").toString(),
        QString("unfiltered"));
}

void tst_QHelpEngineCore::metaData()
{
    QCOMPARE(QHelpEngineCore::metaData(m_path + "/data/test.qch", "author").toString(),
        QString("Digia Plc and/or its subsidiary(-ies)"));
    QCOMPARE(QHelpEngineCore::metaData(m_path + "/data/test.qch", "notExisting").isValid(),
        false);
}

void tst_QHelpEngineCore::requestContentDepths_data()
{
    QTest::addColumn<QList<int>>("depths");
    QTest::addColumn<QString>("expectedTree");

    // Items are titled A, B, C, ... in order. The tree is written as title(children).
    QTest::newRow("valid") << QList<int>{0, 1, 2, 1, 0} << "A(B(C),D),E";
    QTest::newRow("gap") << QList<int>{0, 2, 2, 1} << "A(B,C,D)";
    QTest::newRow("first-deep") << QList<int>{2, 0} << "A,B";
    QTest::newRow("first-deep-children") << QList<int>{2, 3, 1} << "A(B),C";
    QTest::newRow("negative") << QList<int>{0, 1, -5, 1} << "A(B),C(D)";
    QTest::newRow("gap-back-into-gap") << QList<int>{0, 3, 2} << "A(B,C)";
    QTest::newRow("gap-then-deeper") << QList<int>{0, 3, 4, 1, 2} << "A(B(C),D(E))";
    QTest::newRow("gap-inside") << QList<int>{0, 1, 3, 2} << "A(B(C,D))";
    QTest::newRow("gap-inside2") << QList<int>{0, 1, 3, 1} << "A(B(C),D)";
    QTest::newRow("huge") << QList<int>{0, std::numeric_limits<int>::max(), 1} << "A(B,C)";
    QTest::newRow("huge-nested") << QList<int>{0, 1, std::numeric_limits<int>::max(), 2}
                                 << "A(B(C,D))";
    QTest::newRow("huge-first") << QList<int>{std::numeric_limits<int>::max(), 1} << "A,B";
}

static QString childrenToString(const QHelpContentItem *item)
{
    QStringList children;
    for (int i = 0; i < item->childCount(); ++i) {
        const QHelpContentItem *child = item->child(i);
        QString str = child->title();
        if (child->childCount())
            str += u'(' + childrenToString(child) + u')';
        children.append(str);
    }
    return children.join(u',');
}

void tst_QHelpEngineCore::requestContentDepths()
{
    QFETCH(QList<int>, depths);
    QFETCH(QString, expectedTree);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString qchFile = dir.filePath("test.qch");
    QVERIFY(QFile::copy(m_path + "/data/test.qch", qchFile));
    QVERIFY(QFile::setPermissions(qchFile, QFile::WriteUser | QFile::ReadUser));

    QByteArray contents;
    {
        QDataStream s(&contents, QIODevice::WriteOnly);
        for (int i = 0; i < depths.size(); ++i) {
            const QString title(QChar(u'A' + i));
            s << depths.at(i) << QString(title + ".html") << title;
        }
    }

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "depthsdb");
        const auto cleanup = qScopeGuard([&db] {
            db.close();
            db = {};
            QSqlDatabase::removeDatabase("depthsdb");
        });
        db.setDatabaseName(qchFile);
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(query.exec("DELETE FROM ContentsTable WHERE Id != 1"));
        QVERIFY(query.prepare("UPDATE ContentsTable SET Data = ? WHERE Id = 1"));
        query.addBindValue(contents);
        QVERIFY(query.exec());
    }

    QHelpEngineCore help(dir.filePath("collection.qhc"));
    help.setReadOnly(false);
    help.setUsesFilterEngine(true);
    QVERIFY(help.setupData());
    QVERIFY(help.registerDocumentation(qchFile));

    const std::shared_ptr<QHelpContentItem> root = help.requestContent({}).result();
    QVERIFY(root);
    QCOMPARE(childrenToString(root.get()), expectedTree);
}

QTEST_MAIN(tst_QHelpEngineCore)
#include "tst_qhelpenginecore.moc"
