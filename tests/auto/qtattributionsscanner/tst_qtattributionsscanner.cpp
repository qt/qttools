// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qdir.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qprocess.h>

#include <QtTest/qtest.h>

class tst_qtattributionsscanner : public QObject
{
    Q_OBJECT
public:
    tst_qtattributionsscanner();

private slots:
    void test_data();
    void test();

private:
    void readExpectedFile(const QString &baseDir, const QString &fileName, QByteArray *content);

    QString m_cmd;
    QString m_basePath;
};


tst_qtattributionsscanner::tst_qtattributionsscanner()
{
    QString binPath = QLibraryInfo::path(QLibraryInfo::LibraryExecutablesPath);
    m_cmd = binPath + QLatin1String("/qtattributionsscanner");
    m_basePath = QFINDTESTDATA("testdata");
}


void tst_qtattributionsscanner::test_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("stdout_file");
    QTest::addColumn<QString>("stderr_file");

    QTest::newRow("good")
            << QStringLiteral("good")
            << QStringLiteral("good/expected.json")
            << QStringLiteral("good/expected.error");
    QTest::newRow("warnings (incomplete)")
            << QStringLiteral("warnings/incomplete")
            << QStringLiteral("warnings/incomplete/expected.json")
            << QStringLiteral("warnings/incomplete/expected.error");
    QTest::newRow("warnings (unknown attribute)")
            << QStringLiteral("warnings/unknown")
            << QStringLiteral("warnings/unknown/expected.json")
            << QStringLiteral("warnings/unknown/expected.error");
    QTest::newRow("singlefile")
            << QStringLiteral("good/minimal/qt_attribution_test.json")
            << QStringLiteral("good/minimal/expected.json")
            << QStringLiteral("good/minimal/expected.error");
    QTest::newRow("variants") << QStringLiteral("good/variants/qt_attribution_test.json")
                              << QStringLiteral("good/variants/expected.json")
                              << QStringLiteral("good/variants/expected.error");
}

void tst_qtattributionsscanner::readExpectedFile(const QString &baseDir, const QString &fileName, QByteArray *content)
{
    QFile file(QDir(m_basePath).absoluteFilePath(fileName));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Could not open " + file.fileName().toLocal8Bit());
    *content = file.readAll();
    content->replace("%{PWD}", baseDir.toUtf8());
}

void tst_qtattributionsscanner::test()
{
    QFETCH(QString, input);
    QFETCH(QString, stdout_file);
    QFETCH(QString, stderr_file);

    QString dir = QDir(m_basePath).absoluteFilePath(input);
    if (QFileInfo(dir).isFile())
        dir = QFileInfo(dir).absolutePath();

    QProcess proc;
    const QStringList arguments{dir, "--output-format", "json"};
    QString command = m_cmd + ' ' + arguments.join(' ');
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("QT_ATTRIBUTIONSSCANNER_TEST", "1");
    proc.setProcessEnvironment(env);
    proc.start(m_cmd, arguments, QIODevice::ReadWrite | QIODevice::Text);

    QVERIFY2(proc.waitForStarted(), qPrintable(command + QLatin1String(" :") + proc.errorString()));
    QVERIFY2(proc.waitForFinished(30000), qPrintable(command));

    QVERIFY2(proc.exitStatus() == QProcess::NormalExit,
             "\"qtattributionsscanner " + m_cmd.toLatin1() + "\" crashed");
    QVERIFY2(!proc.exitCode(),
             "\"qtattributionsscanner " + m_cmd.toLatin1() + "\" exited with code " +
             QByteArray::number(proc.exitCode()));

    { // compare error output
        QByteArray stdErr = proc.readAllStandardError();
        stdErr.replace(QDir::separator().toLatin1(), "/");

        QByteArray expectedErrorOutput;
        readExpectedFile(dir, stderr_file, &expectedErrorOutput);

        QCOMPARE(stdErr, expectedErrorOutput);
    }

    { // compare json output
        QByteArray stdOut = proc.readAllStandardOutput();

        QJsonParseError jsonError;
        QJsonDocument actualJson = QJsonDocument::fromJson(stdOut, &jsonError);
        QVERIFY2(!actualJson.isNull(), "Invalid output: " + jsonError.errorString().toLatin1());

        QByteArray expectedOutput;
        readExpectedFile(dir, stdout_file, &expectedOutput);
        QJsonDocument expectedJson = QJsonDocument::fromJson(expectedOutput);

        if (!QTest::qCompare(actualJson, expectedJson, "actualJson", "expectedJson", __FILE__, __LINE__)) {
            qWarning() << "Actual (actualJson)    :" << actualJson;
            qWarning() << "Expected (expectedJson):" << expectedJson;
            return;
        }
    }
}

QTEST_MAIN(tst_qtattributionsscanner)
#include "tst_qtattributionsscanner.moc"
