// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QLibraryInfo>
#include <QProcess>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace {
class TempCopy
{
public:
    TempCopy(const QString &fileName)
    {
        QFileInfo fi(fileName);
        if (fi.isFile()) {
            m_copyPath =
                    fi.absolutePath() + QDir::separator() + fi.baseName() + "_copy." + fi.suffix();
            if (QFile::exists(m_copyPath))
                QFile::remove(m_copyPath);
            QFile::copy(fileName, m_copyPath);
        } else {
            m_copyPath = fi.absolutePath() + QDir::separator() + fi.fileName() + "_copy";
            QDir dir(m_copyPath);
            if (dir.exists())
                dir.removeRecursively();
            std::filesystem::copy(fileName.toStdString(), dir.filesystemPath(),
                                  std::filesystem::copy_options::recursive);
        }
    }

    const QString &path() const { return m_copyPath; }

    ~TempCopy()
    {
        QFileInfo fi(m_copyPath);
        if (fi.isFile())
            QFile::remove(m_copyPath);
        else
            QDir(m_copyPath).removeRecursively();
    }

private:
    QString m_copyPath;
};

class FileGuard
{
public:
    FileGuard(const QString &fileName) : m_fileName(fileName)
    {
        if (QFile::exists(m_fileName))
            QFile::remove(m_fileName);
    }
    ~FileGuard()
    {
        if (QFile::exists(m_fileName))
            QFile::remove(m_fileName);
    }

private:
    const QString m_fileName;
};

void verify(QProcess &proc, const QString &name)
{
    QVERIFY2(proc.waitForStarted(), qPrintable(proc.errorString()));
    QVERIFY2(proc.waitForFinished(10000), qPrintable(name + " hung"_L1));
    QVERIFY2(proc.exitStatus() == QProcess::NormalExit, qPrintable(name + " crashed"));
    QVERIFY2(proc.exitCode() == 0,
             qPrintable(QString(name + " exited with status %1. Errors:\n%2")
                                .arg(proc.exitCode())
                                .arg(proc.readAllStandardError())));
}

void doCompare(const QString &actualOutput, const QString &expectedFn)
{
    QFile file1(actualOutput);
    QVERIFY(file1.open(QIODevice::ReadOnly | QIODevice::Text));
    QList<QByteArray> actual = file1.readAll().split('\n');

    QFile file2(expectedFn);
    QVERIFY(file2.open(QIODevice::ReadOnly | QIODevice::Text));
    QList<QByteArray> expected = file2.readAll().split('\n');

    int i = 0, ei = expected.size(), gi = actual.size();
    for (;; i++) {
        if (i == gi) {
            if (i == ei)
                return;
            gi = 0;
            break;
        } else if (i == ei) {
            ei = 0;
            break;
        } else if (actual.at(i) != expected.at(i)) {
            while ((ei - 1) >= i && (gi - 1) >= i && actual.at(gi - 1) == expected.at(ei - 1))
                ei--, gi--;
            break;
        }
    }
    QByteArray diff;
    for (int j = qMax(0, i - 3); j < i; j++)
        diff += expected.at(j) + '\n';
    diff += "<<<<<<< got\n";
    for (int j = i; j < gi; j++) {
        diff += actual.at(j) + '\n';
        if (j >= i + 5) {
            diff += "...\n";
            break;
        }
    }
    diff += "=========\n";
    for (int j = i; j < ei; j++) {
        diff += expected.at(j) + '\n';
        if (j >= i + 5) {
            diff += "...\n";
            break;
        }
    }
    diff += ">>>>>>> expected\n";
    for (int j = ei; j < qMin(ei + 3, expected.size()); j++)
        diff += expected.at(j) + '\n';
    QFAIL(qPrintable("Output for " + expectedFn + " does not meet expectations:\n" + diff));
}

} // namespace

class tst_ltext2id : public QObject
{
    Q_OBJECT

private slots:
    void testTransform_data();
    void testTransform();
    void test2wayTransform_data();
    void test2wayTransform();
    void testTransformationError_data();
    void testTransformationError();
    void testProject_data();
    void testProject();

private:
    const QString m_basePath = QFINDTESTDATA("data/");
    const QString m_ltext2id = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/ltext2id";
    const QString m_lupdate = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/lupdate";
};

void tst_ltext2id::testTransform_data()
{
    QTest::addColumn<QString>("inFileName");
    QTest::addColumn<QString>("expectedFileName");

    QTest::newRow("cpp file transform") << "all.cpp" << "all.cpp.out";
    QTest::newRow("js file transform") << "main.js" << "main.js.out";
    QTest::newRow("qml file transform") << "main.qml" << "main.qml.out";
    QTest::newRow("ui file transform") << "project.ui" << "project.ui.out";
    QTest::newRow("ts file transform") << "project.ts" << "project.ts.out";
}

void tst_ltext2id::testTransform()
{
    QFETCH(QString, inFileName);
    QFETCH(QString, expectedFileName);

    TempCopy tc(m_basePath + inFileName);

    QProcess ltext2id;
    ltext2id.start(m_ltext2id, QStringList{ tc.path() }, QIODeviceBase::ReadOnly | QIODevice::Text);
    verify(ltext2id, "ltext2id");

    doCompare(tc.path(), m_basePath + expectedFileName);
}

void tst_ltext2id::test2wayTransform_data()
{
    QTest::addColumn<QString>("inFileName");
    QTest::newRow("cpp file two way transform") << "2way.cpp";
    QTest::newRow("js file two way transform") << "2way.js";
    QTest::newRow("qml file two way transform") << "2way.qml";
    QTest::newRow("ui file two way transform") << "project.ui";
}

void tst_ltext2id::test2wayTransform()
{
    QFETCH(QString, inFileName);
    QString inFilePath = m_basePath + inFileName;

    QString translationFromOriginal = m_basePath + "original.ts";
    QString translationFromTransformed = m_basePath + "transformed.ts";
    FileGuard f1(translationFromOriginal);
    FileGuard f2(translationFromTransformed);

    TempCopy transformedSourceFile(inFilePath);

    // first route to id based translation file: lupdate to get TS, transform TS

    QProcess lupdate1;
    lupdate1.start(m_lupdate,
                   QStringList{ transformedSourceFile.path(), "-ts", translationFromOriginal,
                                "-locations", "none" });
    verify(lupdate1, "lupdate route 1");

    QProcess ltext2id1;
    ltext2id1.start(m_ltext2id, QStringList{ "--sort-messages", translationFromOriginal },
                    QIODeviceBase::ReadOnly | QIODevice::Text);
    verify(ltext2id1, "ltext2id route 1");

    // second route to id based translation file: transform source, lupdate on transformed source

    QProcess ltext2id2;
    ltext2id2.start(m_ltext2id, QStringList{ transformedSourceFile.path() },
                    QIODeviceBase::ReadOnly | QIODevice::Text);
    verify(ltext2id2, "ltext2id route 2");

    QProcess lupdate2;
    lupdate2.start(m_lupdate,
                   QStringList{ transformedSourceFile.path(), "-ts", translationFromTransformed,
                                "-sort-messages", "-locations", "none" });
    verify(lupdate2, "lupdate route 2");

    doCompare(translationFromOriginal, translationFromTransformed);
}

void tst_ltext2id::testTransformationError_data()
{
    QTest::addColumn<QString>("inFileName");
    QTest::addColumn<QString>("expectedFileName");
    QTest::addColumn<QString>("expectedOutput");
    QTest::newRow("cpp transform error")
            << "fails.cpp" << "fails.cpp.out" << "cpp_expectedoutput.txt";
}

void tst_ltext2id::testTransformationError()
{
    QFETCH(QString, inFileName);
    QFETCH(QString, expectedFileName);
    QFETCH(QString, expectedOutput);

    TempCopy tc(m_basePath + inFileName);

    QProcess ltext2id;
    ltext2id.setProcessChannelMode(QProcess::MergedChannels);
    ltext2id.start(m_ltext2id, QStringList{ tc.path() }, QIODeviceBase::ReadOnly | QIODevice::Text);
    QVERIFY2(ltext2id.waitForStarted(), qPrintable(ltext2id.errorString()));
    QVERIFY2(ltext2id.waitForFinished(10000), qPrintable("ltext2id hung"_L1));
    QVERIFY2(ltext2id.exitStatus() == QProcess::NormalExit, qPrintable("ltext2id crashed"));
    QVERIFY(ltext2id.exitCode() == 1);

    doCompare(tc.path(), m_basePath + expectedFileName);

    QFile file(m_basePath + expectedOutput);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArrayList expectedOut = file.readAll().split('\n');
    QByteArrayList actualOut = ltext2id.readAll().split('\n');
    auto actualLineItr = actualOut.cbegin();
    for (const QByteArray &line : std::as_const(expectedOut)) {
        if (actualLineItr == actualOut.cend())
            QFAIL("The actual output error does not match with the expected output error. Missing "
                  "errors.");
        const QByteArray &actualLine = *actualLineItr;
        QRegularExpression lineRegex(line);
        auto match = lineRegex.globalMatch(actualLine);
        if (!match.hasNext() || match.next().captured(0) != actualLine)
            QFAIL(qPrintable(
                    "The actual output error does not match with the expected output error. "
                    "expected \n'%1'\n got\n'%2'\n"_L1.arg(line)
                            .arg(actualLine)));
        actualLineItr++;
    }
    if (actualLineItr != actualOut.cend())
        QFAIL("The actual output error produced more errors than expected.");
}

void tst_ltext2id::testProject_data()
{
    QTest::addColumn<QString>("projectName");
    QTest::addColumn<QString>("expectedProjectName");
    QTest::newRow("cpp project") << "project" << "project.out";
}

void tst_ltext2id::testProject()
{
    QFETCH(QString, projectName);
    QFETCH(QString, expectedProjectName);
    TempCopy tc(m_basePath + projectName);
    QProcess ltext2id;
    ltext2id.start(m_ltext2id, QStringList{ tc.path() }, QIODeviceBase::ReadOnly | QIODevice::Text);
    QVERIFY2(ltext2id.waitForStarted(), qPrintable(ltext2id.errorString()));
    QVERIFY2(ltext2id.waitForFinished(10000), qPrintable("ltext2id hung"_L1));
    QVERIFY2(ltext2id.exitStatus() == QProcess::NormalExit, qPrintable("ltext2id crashed"));
    QVERIFY(ltext2id.exitCode() == 0);
    QDir expDir(m_basePath + expectedProjectName);
    QDir actualDir(tc.path());
    const QStringList files{ "main.cpp", "project.ts" };
    for (const QString &f : files)
        doCompare(actualDir.filePath(f), expDir.filePath(f));
}

QTEST_APPLESS_MAIN(tst_ltext2id)

#include "tst_ltext2id.moc"
