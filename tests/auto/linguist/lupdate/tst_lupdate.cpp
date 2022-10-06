// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#if CHECK_SIMTEXTH
#include "../shared/simtexth.h"
#endif

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/private/qconfig_p.h>
#include <QtCore/QSet>

#include <QtTest/QtTest>
#include <QtTools/private/qttools-config_p.h>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

class tst_lupdate : public QObject
{
    Q_OBJECT
public:
    tst_lupdate();

private slots:
    void good_data();
    void good();
#if CHECK_SIMTEXTH
    void simtexth();
    void simtexth_data();
#endif

private:
    QString m_cmdLupdate;
    QString m_basePath;

    void doCompare(QStringList actual, const QString &expectedFn, bool err);
    void doCompare(const QString &actualFn, const QString &expectedFn, bool err);
};


tst_lupdate::tst_lupdate()
{
    QString binPath = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    m_cmdLupdate = binPath + QLatin1String("/lupdate");
    m_basePath = QFINDTESTDATA("testdata/");
}

static bool prepareMatch(const QString &expect, QString *tmpl, int *require, int *accept)
{
    if (expect.startsWith(QLatin1Char('\\'))) {
        *tmpl = expect.mid(1);
        *require = *accept = 1;
    } else if (expect.startsWith(QLatin1Char('?'))) {
        *tmpl = expect.mid(1);
        *require = 0;
        *accept = 1;
    } else if (expect.startsWith(QLatin1Char('*'))) {
        *tmpl = expect.mid(1);
        *require = 0;
        *accept = INT_MAX;
    } else if (expect.startsWith(QLatin1Char('+'))) {
        *tmpl = expect.mid(1);
        *require = 1;
        *accept = INT_MAX;
    } else if (expect.startsWith(QLatin1Char('{'))) {
        int brc = expect.indexOf(QLatin1Char('}'), 1);
        if (brc < 0)
            return false;
        *tmpl = expect.mid(brc + 1);
        QString sub = expect.mid(1, brc - 1);
        int com = sub.indexOf(QLatin1Char(','));
        bool ok;
        if (com < 0) {
            *require = *accept = sub.toInt(&ok);
            return ok;
        } else {
            *require = sub.left(com).toInt();
            *accept = sub.mid(com + 1).toInt(&ok);
            if (!ok)
                *accept = INT_MAX;
            return *accept >= *require;
        }
    } else {
        *tmpl = expect;
        *require = *accept = 1;
    }
    return true;
}

void tst_lupdate::doCompare(QStringList actual, const QString &expectedFn, bool err)
{
    QFile file(expectedFn);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(expectedFn));
    QStringList expected = QString(file.readAll()).split('\n');

    for (int i = actual.size() - 1; i >= 0; --i) {
        if (actual.at(i).startsWith(QLatin1String("Info: creating stash file ")))
            actual.removeAt(i);
    }

    int ei = 0, ai = 0, em = expected.size(), am = actual.size();
    int oei = 0, oai = 0, oem = em, oam = am;
    int require = 0, accept = 0;
    QString tmpl;
    forever {
        if (!accept) {
            oei = ei, oai = ai;
            if (ei == em) {
                if (ai == am)
                    return;
                break;
            }
            if (!prepareMatch(expected.at(ei++), &tmpl, &require, &accept))
                QFAIL(qPrintable(QString("Malformed expected %1 at %3:%2")
                                 .arg(err ? "output" : "result").arg(ei).arg(expectedFn)));
        }
        if (ai == am) {
            if (require <= 0) {
                accept = 0;
                continue;
            }
            break;
        }
        if (err ? !QRegularExpression(QRegularExpression::anchoredPattern(tmpl)).match(actual.at(ai)).hasMatch() : (actual.at(ai) != tmpl)) {
            if (require <= 0) {
                accept = 0;
                continue;
            }
            ei--;
            require = accept = 0;
            forever {
                if (!accept) {
                    oem = em, oam = am;
                    if (ei == em)
                        break;
                    if (!prepareMatch(expected.at(--em), &tmpl, &require, &accept))
                        QFAIL(qPrintable(QString("Malformed expected %1 at %3:%2")
                                         .arg(err ? "output" : "result")
                                         .arg(em + 1).arg(expectedFn)));
                }
                if (ai == am || (err ? !QRegularExpression(QRegularExpression::anchoredPattern(tmpl)).match(actual.at(am - 1)).hasMatch() :
                                       (actual.at(am - 1) != tmpl))) {
                    if (require <= 0) {
                        accept = 0;
                        continue;
                    }
                    break;
                }
                accept--;
                require--;
                am--;
            }
            break;
        }
        accept--;
        require--;
        ai++;
    }
    QString diff;
    for (int j = qMax(0, oai - 3); j < oai; j++)
        diff += actual.at(j) + '\n';
    diff += "<<<<<<< got\n";
    for (int j = oai; j < oam; j++) {
        diff += actual.at(j) + '\n';
        if (j >= oai + 5) {
            diff += "...\n";
            break;
        }
    }
    diff += "=========\n";
    for (int j = oei; j < oem; j++) {
        diff += expected.at(j) + '\n';
        if (j >= oei + 5) {
            diff += "...\n";
            break;
        }
    }
    diff += ">>>>>>> expected\n";
    for (int j = oam; j < qMin(oam + 3, actual.size()); j++)
        diff += actual.at(j) + '\n';
    QFAIL(qPrintable((err ? "Output for " : "Result for ")
                     + expectedFn + " does not meet expectations:\n" + diff));
    }

void tst_lupdate::doCompare(const QString &actualFn, const QString &expectedFn, bool err)
{
    QFile afile(actualFn);
    QVERIFY2(afile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(actualFn));
    QStringList actual = QString(afile.readAll()).split('\n');

    doCompare(actual, expectedFn, err);
}

void tst_lupdate::good_data()
{
    QTest::addColumn<QString>("directory");
    QTest::addColumn<bool>("useClangCpp");

    QDir parsingDir(m_basePath + "good");
    QStringList dirs = parsingDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

#ifndef Q_OS_WIN
    dirs.removeAll(QLatin1String("backslashes"));
#endif
#ifndef Q_OS_MACOS
    dirs.removeAll(QLatin1String("parseobjc"));
#endif
    QSet<QString> ignoredTests = {
        "lacksqobject_clang_parser", "parsecontexts_clang_parser", "parsecpp2_clang_parser",
        "parsecpp_clang_parser",     "prefix_clang_parser",        "preprocess_clang_parser",
        "parsecpp_clang_only"};

    // Add test rows for the "classic" lupdate
    for (const QString &dir : dirs) {
        if (ignoredTests.contains(dir))
            continue;
        QTest::newRow(dir.toLocal8Bit()) << dir << false;
    }

#if QT_CONFIG(clangcpp) && QT_CONFIG(widgets)
    // Add test rows for the clang-based lupdate
    ignoredTests = {
        "lacksqobject",
        "parsecontexts",
        "parsecpp",
        "parsecpp2",
        "parseqrc_json",
        "prefix",
        "preprocess",
        "proparsing2", // llvm8 cannot handle file name without extension
        "respfile", //@lst not supported with the new parser yet (include not properly set in the compile_command.json)
        "cmdline_deeppath", //no project file, new parser does not support (yet) this way of launching lupdate
        "cmdline_order", // no project, new parser do not pickup on macro defined but not used. Test not needed for new parser.
        "cmdline_recurse", // recursive scan without project file not supported (yet) with the new parser
    };
    for (const QString &dir : dirs) {
        if (ignoredTests.contains(dir))
            continue;
        QTest::newRow("clang-" + dir.toLocal8Bit()) << dir << true;
    }
#endif
}

void tst_lupdate::good()
{
    QFETCH(QString, directory);
    QFETCH(bool, useClangCpp);

    QString dir = m_basePath + "good/" + directory;
    QString workDir = dir;
    QStringList generatedtsfiles(QLatin1String("project.ts"));
    QStringList lupdateArguments;

    QFile file(dir + "/lupdatecmd");
    if (file.exists()) {
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.fileName()));
        while (!file.atEnd()) {
            QByteArray cmdstring = file.readLine().simplified();
            if (cmdstring.startsWith('#'))
                continue;
            if (cmdstring.startsWith("lupdate")) {
                for (auto argument : cmdstring.mid(8).simplified().split(' '))
                    lupdateArguments += argument;
                break;
            } else if (cmdstring.startsWith("TRANSLATION:")) {
                cmdstring.remove(0, 12);
                generatedtsfiles.clear();
                const auto parts = cmdstring.split(' ');
                for (const QByteArray &s : parts)
                    if (!s.isEmpty())
                        generatedtsfiles << s;
            } else if (cmdstring.startsWith("cd ")) {
                cmdstring.remove(0, 3);
                workDir = QDir::cleanPath(dir + QLatin1Char('/') + cmdstring);
            }
        }
        file.close();
    }

    for (const QString &ts : std::as_const(generatedtsfiles)) {
        QString genTs = workDir + QLatin1Char('/') + ts;
        QFile::remove(genTs);
        QString beforetsfile = dir + QLatin1Char('/') + ts + QLatin1String(".before");
        if (QFile::exists(beforetsfile))
            QVERIFY2(QFile::copy(beforetsfile, genTs), qPrintable(beforetsfile));
    }

    file.setFileName(workDir + QStringLiteral("/.qmake.cache"));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    if (lupdateArguments.isEmpty()) {
        // Automatically pass "project.pro" or "-project project.json".
        if (QFile::exists(dir + u"/project.json"_s)) {
            lupdateArguments << u"-project"_s << u"project.json"_s;
        } else {
            lupdateArguments.append(QLatin1String("project.pro"));
        }
    }

    lupdateArguments.prepend("-silent");
    if (useClangCpp)
        lupdateArguments.append("-clang-parser");

    QProcess proc;
    proc.setWorkingDirectory(workDir);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    const QString command = m_cmdLupdate + ' ' + lupdateArguments.join(' ');
    proc.start(m_cmdLupdate, lupdateArguments, QIODevice::ReadWrite | QIODevice::Text);
    QVERIFY2(proc.waitForStarted(), qPrintable(command + QLatin1String(" :") + proc.errorString()));
    QVERIFY2(proc.waitForFinished(30000), qPrintable(command));
    const QString output = QString::fromLocal8Bit(proc.readAll());
    QVERIFY2(proc.exitStatus() == QProcess::NormalExit,
             qPrintable(QLatin1Char('"') + command + "\" crashed\n" + output));
    QVERIFY2(!proc.exitCode(),
             qPrintable(QLatin1Char('"') + command + "\" exited with code " +
             QString::number(proc.exitCode()) + '\n' + output));

    // If the file expectedoutput.txt exists, compare the
    // console output with the content of that file
    QFile outfile(dir + "/expectedoutput.txt");
    if (outfile.exists()) {
        QStringList errslist = output.split(QLatin1Char('\n'));
        doCompare(errslist, outfile.fileName(), true);
        if (QTest::currentTestFailed())
            return;
    }

    for (const QString &ts : std::as_const(generatedtsfiles)) {
        if (dir.endsWith("preprocess_clang_parser")) {
            doCompare(workDir + QLatin1Char('/') + ts,
                      dir + QLatin1Char('/') + ts + QLatin1String(".result"), true);
        } else {
        doCompare(workDir + QLatin1Char('/') + ts,
                  dir + QLatin1Char('/') + ts + QLatin1String(".result"), false);
        }
    }
}

#if CHECK_SIMTEXTH
void tst_lupdate::simtexth()
{
    QFETCH(QString, one);
    QFETCH(QString, two);
    QFETCH(int, expected);

    int measured = getSimilarityScore(one, two.toLatin1());
    QCOMPARE(measured, expected);
}


void tst_lupdate::simtexth_data()
{
    using namespace QTest;

    addColumn<QString>("one");
    addColumn<QString>("two");
    addColumn<int>("expected");

    newRow("00") << "" << "" << 1024;
    newRow("01") << "a" << "a" << 1024;
    newRow("02") << "ab" << "ab" << 1024;
    newRow("03") << "abc" << "abc" << 1024;
    newRow("04") << "abcd" << "abcd" << 1024;
}
#endif

QTEST_MAIN(tst_lupdate)
#include "tst_lupdate.moc"
