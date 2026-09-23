// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>

#include <QtCore/QScopeGuard>
#include <QtCore/QTemporaryDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpSearchEngineCore>
#include <QtHelp/QHelpSearchResult>

class tst_QHelpSearchEngineCore : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void search_data();
    void search();

private:
    QTemporaryDir m_dir;
    std::unique_ptr<QHelpEngineCore> m_helpEngine;
    std::unique_ptr<QHelpSearchEngineCore> m_searchEngine;
};

static bool addFile(QSqlQuery &query, int fileId, const QString &name, const QByteArray &data)
{
    query.prepare("INSERT INTO FileDataTable (Id, Data) VALUES (?, ?)");
    query.addBindValue(fileId);
    query.addBindValue(qCompress(data));
    if (!query.exec())
        return false;

    query.prepare("INSERT INTO FileNameTable (FolderId, Name, FileId, Title) VALUES (1, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(fileId);
    query.addBindValue(name);
    if (!query.exec())
        return false;

    // Make the file part of every filter attribute set of test.qch.
    query.prepare("INSERT INTO FileFilterTable (FilterAttributeId, FileId) "
                  "SELECT Id, ? FROM FilterAttributeTable");
    query.addBindValue(fileId);
    return query.exec();
}

static QByteArray html(const QByteArray &word)
{
    return "<html><head><title>" + word + "</title></head><body><p>" + word
            + "</p></body></html>";
}

void tst_QHelpSearchEngineCore::initTestCase()
{
    QVERIFY(m_dir.isValid());

    const QString qchFile = m_dir.filePath("test.qch");
    QVERIFY(QFile::copy(QLatin1String(SRCDIR) + "/data/test.qch", qchFile));
    QVERIFY(QFile::setPermissions(qchFile, QFile::WriteUser | QFile::ReadUser));

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "searchdb");
        const auto cleanup = qScopeGuard([&db] {
            db.close();
            db = {};
            QSqlDatabase::removeDatabase("searchdb");
        });
        db.setDatabaseName(qchFile);
        QVERIFY(db.open());
        QSqlQuery query(db);
        QVERIFY(addFile(query, 100, "extra.html", html("quokkahtml")));
        QVERIFY(addFile(query, 101, "page.htm", html("quokkahtm")));
        QVERIFY(addFile(query, 102, "notes.txt", "quokkatxt"));
        QVERIFY(addFile(query, 103, "style.css", "quokkacss { color: red; }"));
    }

    m_helpEngine.reset(new QHelpEngineCore(m_dir.filePath("collection.qhc")));
    m_helpEngine->setReadOnly(false);
    QVERIFY(m_helpEngine->setupData());
    QVERIFY(m_helpEngine->registerDocumentation(qchFile));

    m_searchEngine.reset(new QHelpSearchEngineCore(m_helpEngine.get()));
    QSignalSpy indexingSpy(m_searchEngine.get(), &QHelpSearchEngineCore::indexingFinished);
    m_searchEngine->reindexDocumentation();
    QVERIFY(indexingSpy.wait(10000));
}

void tst_QHelpSearchEngineCore::search_data()
{
    QTest::addColumn<QString>("word");
    QTest::addColumn<QString>("expectedFile");
    QTest::addColumn<bool>("needsFullQtHelp");

    QTest::newRow("html") << "quokkahtml" << "extra.html" << true;
    QTest::newRow("htm") << "quokkahtm" << "page.htm" << true;
    QTest::newRow("txt") << "quokkatxt" << "notes.txt" << false;
    QTest::newRow("css") << "quokkacss" << QString() << false;
}

void tst_QHelpSearchEngineCore::search()
{
    QFETCH(QString, word);
    QFETCH(QString, expectedFile);
    QFETCH(bool, needsFullQtHelp);

#ifndef TEST_FULLQTHELP
    if (needsFullQtHelp)
        QSKIP("The contents of HTML files are indexed only with fullqthelp.");
#else
    Q_UNUSED(needsFullQtHelp);
#endif

    QSignalSpy searchingSpy(m_searchEngine.get(), &QHelpSearchEngineCore::searchingFinished);
    m_searchEngine->search(word);
    QVERIFY(searchingSpy.wait(10000));

    const QList<QHelpSearchResult> results
            = m_searchEngine->searchResults(0, m_searchEngine->searchResultCount());
    QSet<QString> files;
    for (const QHelpSearchResult &result : results)
        files.insert(result.url().path().section(u'/', -1));

    if (expectedFile.isEmpty())
        QVERIFY2(files.isEmpty(), qPrintable(QStringList(files.cbegin(), files.cend()).join(u',')));
    else
        QCOMPARE(files, QSet<QString>{expectedFile});
}

QTEST_MAIN(tst_QHelpSearchEngineCore)
#include "tst_qhelpsearchenginecore.moc"
