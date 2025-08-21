// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QByteArray>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QLibraryInfo>
#include <QProcess>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

class tst_lcheck : public QObject
{
    Q_OBJECT

public:
    tst_lcheck()
        : lcheck(QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/lcheck"),
          dataDir(QFINDTESTDATA("testdata/"))
    {
    }

private:
    static QStringList extractValidationLines(const QByteArray &ba);

private slots:
    void invalidTranslations();
    void validTranslations();
    void checkFinished_defaultDisabled();
    void checkFinished_enabled();
    void disableAcceleratorCheck();
    void disableWhitespaceCheck();
    void disablePunctuationCheck();
    void disablePlaceMarkerCheck();
    void outputToFile_optionWritesReport();

private:
    void doCompare(const QStringList &actual, const QString &expectedFn);

    QString lcheck;
    QString dataDir;
};

void tst_lcheck::doCompare(const QStringList &actual, const QString &expectedFn)
{
    QFile file(expectedFn);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QStringList expected = QString(file.readAll()).trimmed().split('\n');

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
        } else if (!QRegularExpression(QRegularExpression::anchoredPattern(expected.at(i)))
                            .match(actual.at(i))
                            .hasMatch()) {
            while ((ei - 1) >= i && (gi - 1) >= i
                   && (QRegularExpression(QRegularExpression::anchoredPattern(expected.at(ei - 1)))
                               .match(actual.at(gi - 1)))
                              .hasMatch())
                ei--, gi--;
            break;
        }
    }
    QString diff;
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

QStringList tst_lcheck::extractValidationLines(const QByteArray &ba)
{
    const QString out = QString::fromUtf8(ba);
    QStringList actual;
    for (const QString &line : out.split('\n')) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("Validation error for translation '"_L1))
            actual << trimmed;
    }
    return actual;
}

void tst_lcheck::invalidTranslations()
{
    QProcess proc;
    proc.start(lcheck, { dataDir + "invalid_de.ts" });
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
    QCOMPARE(proc.exitCode(), 1);
    auto stderrOutput = proc.readAllStandardError();
    QVERIFY(stderrOutput.contains("Validation error for translation 'Text nicht &gefunden '"));
    QVERIFY(stderrOutput.contains("Validation error for translation 'Text gefunden %1'"));
}

void tst_lcheck::validTranslations()
{
    QProcess proc;
    proc.start(lcheck, { dataDir + "valid_de.ts" });
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
    QCOMPARE(proc.exitCode(), 0);
    const QByteArray err = proc.readAllStandardError();
    QVERIFY(!err.contains("Validation error for translation '"));
}

void tst_lcheck::checkFinished_defaultDisabled()
{
    QProcess proc;
    proc.start(lcheck, { dataDir + "finished_issues_de.ts" });
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
    QCOMPARE(proc.exitCode(), 0);

    const QByteArray err = proc.readAllStandardError();
    QVERIFY(!err.contains("Validation error for translation 'Text nicht gefunden '"));
    QVERIFY(!err.contains("Validation error for translation 'Beenden'"));
}

void tst_lcheck::checkFinished_enabled()
{
    QProcess proc;
    proc.start(lcheck, { "--check-finished", dataDir + "finished_issues_de.ts" });
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
    QCOMPARE(proc.exitCode(), 1);

    const QByteArray err = proc.readAllStandardError();
    QVERIFY(err.contains("Validation error for translation 'Text nicht gefunden '"));
    QVERIFY(err.contains("Validation error for translation 'Beenden'"));
}

void tst_lcheck::disableAcceleratorCheck()
{
    {
        QProcess proc;
        proc.start(lcheck, { dataDir + "accelerator_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 1);
        QVERIFY(proc.readAllStandardError().contains("Validation error for translation 'Beenden'"));
    }
    {
        QProcess proc;
        proc.start(lcheck, { "--no-accelerator", dataDir + "accelerator_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 0);
    }
}

void tst_lcheck::disableWhitespaceCheck()
{
    {
        QProcess proc;
        proc.start(lcheck, { dataDir + "whitespace_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 1);
        QVERIFY(proc.readAllStandardError().contains("Validation error for translation 'Bereit '"));
    }
    {
        QProcess proc;
        proc.start(lcheck, { "--no-whitespaces", dataDir + "whitespace_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 0);
    }
}

void tst_lcheck::disablePunctuationCheck()
{
    {
        QProcess proc;
        proc.start(lcheck, { dataDir + "punctuation_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 1);
        QVERIFY(proc.readAllStandardError().contains("Validation error for translation 'Bereit'"));
    }
    {
        QProcess proc;
        proc.start(lcheck, { "--no-punctuation", dataDir + "punctuation_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 0);
    }
}

void tst_lcheck::disablePlaceMarkerCheck()
{
    {
        QProcess proc;
        proc.start(lcheck, { dataDir + "placemarker_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 1);
        QVERIFY(proc.readAllStandardError().contains(
                "Validation error for translation 'Gefunden %1 Elemente'"));
    }
    {
        QProcess proc;
        proc.start(lcheck, { "--no-place-marker", dataDir + "placemarker_mismatch_de.ts" });
        QVERIFY(proc.waitForFinished());
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        QCOMPARE(proc.exitCode(), 0);
    }
}

void tst_lcheck::outputToFile_optionWritesReport()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString report = tmp.path() + "/report.txt";

    QProcess proc;
    proc.start(lcheck, { "-o", report, dataDir + "invalid_de.ts" });
    QVERIFY(proc.waitForFinished());
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
    QCOMPARE(proc.exitCode(), 1);

    const QByteArray err = proc.readAllStandardError();
    QVERIFY(!err.contains("Validation error for translation '"));

    QFile f(report);
    QVERIFY(f.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray fileData = f.readAll();
    QVERIFY(!fileData.isEmpty());

    const QStringList actual = extractValidationLines(fileData);
    doCompare(actual, dataDir + "expected_invalid_report.txt");
}

QTEST_MAIN(tst_lcheck)
#include "tst_lcheck.moc"
