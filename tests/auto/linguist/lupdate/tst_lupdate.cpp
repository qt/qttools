// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#if CHECK_SIMTEXTH
#include "../shared/simtexth.h"
#endif

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QElapsedTimer>
#include <QtCore/private/qconfig_p.h>
#include <QtCore/QSet>
#include <QtCore/QSysInfo>

#include <QtTest/QtTest>
#include <QtTools/private/qttools-config_p.h>

#include <algorithm>
#include <limits>

using namespace Qt::Literals::StringLiterals;

// The slowest test (clang-proparsing) has been observed to take 22s in COIN/Linux.
// Windows does not run the clang tests.
static constexpr int TIMEOUT = 120000;

class tst_lupdate : public QObject
{
    Q_OBJECT
public:
    tst_lupdate();

private slots:
    void cleanupTestCase();
    void good_data();
    void good();
#if CHECK_SIMTEXTH
    void simtexth();
    void simtexth_data();
#endif

private:
    QString m_cmdLupdate;
    QString m_basePath;
    QElapsedTimer m_timer;
    qint64 m_maxElapsed = -1;

    static void doCompare(QList<QStringView> actual, const QString &expectedFn, bool err);
    void doCompare(const QString &actualFn, const QString &expectedFn, bool err);
};


tst_lupdate::tst_lupdate()
{
    m_timer.start();
    QString binPath = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    m_cmdLupdate = binPath + "/lupdate"_L1;
    m_basePath = QFINDTESTDATA("testdata/");
}

void tst_lupdate::cleanupTestCase()
{
    if (m_maxElapsed > 0)
        qInfo().noquote().nospace() << "max elapsed: " << m_maxElapsed << "ms";
}

static bool prepareMatch(QStringView expect, QString *tmpl, qsizetype *require, qsizetype *accept)
{
    if (expect.startsWith(QLatin1Char('\\'))) {
        *tmpl = expect.sliced(1).toString();
        *require = *accept = 1;
    } else if (expect.startsWith(u'?')) {
        *tmpl = expect.sliced(1).toString();
        *require = 0;
        *accept = 1;
    } else if (expect.startsWith(u'*')) {
        *tmpl = expect.sliced(1).toString();
        *require = 0;
        *accept = std::numeric_limits<qsizetype>::max();
    } else if (expect.startsWith(u'+')) {
        *tmpl = expect.sliced(1).toString();
        *require = 1;
        *accept = std::numeric_limits<qsizetype>::max();
    } else if (expect.startsWith(u'{')) {
        const auto brc = expect.indexOf(u'}', 1);
        if (brc < 0)
            return false;
        *tmpl = expect.sliced(brc + 1).toString();
        auto sub = expect.sliced(1, brc - 1);
        const auto com = sub.indexOf(u',');
        bool ok{};
        if (com < 0) {
            *require = *accept = sub.toInt(&ok);
            return ok;
        }
        *require = sub.left(com).toInt();
        *accept = sub.sliced(com + 1).toInt(&ok);
        if (!ok)
            *accept = std::numeric_limits<qsizetype>::max();
        return *accept >= *require;
    } else {
        *tmpl = expect.toString();
        *require = *accept = 1;
    }
    return true;
}

static bool isStashMessage(QStringView v)
{
    return v.startsWith("Info: creating stash file "_L1);
}

void tst_lupdate::doCompare(QList<QStringView> actual, const QString &expectedFn, bool err)
{
    QFile file(expectedFn);
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(expectedFn));
    const QString expectedS = QString::fromUtf8(file.readAll());
    auto expected = QStringView{expectedS}.split(u'\n');

    actual.erase(std::remove_if(actual.begin(), actual.end(), isStashMessage), actual.end());

    qsizetype ei = 0, ai = 0, em = expected.size(), am = actual.size();
    qsizetype oei = 0, oai = 0, oem = em, oam = am;
    qsizetype require = 0, accept = 0;
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
                QFAIL(qPrintable(QString("Malformed expected %1 at %3:%2"_L1)
                                 .arg(err ? "output"_L1 : "result"_L1).arg(ei).arg(expectedFn)));
        }
        if (ai == am) {
            if (require <= 0) {
                accept = 0;
                continue;
            }
            break;
        }
        if (err
             ? !QRegularExpression(QRegularExpression::anchoredPattern(tmpl)).matchView(actual.at(ai)).hasMatch()
             : (actual.at(ai) != tmpl)) {
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
                        QFAIL(qPrintable(QString("Malformed expected %1 at %3:%2"_L1)
                                         .arg(err ? "output"_L1 : "result"_L1)
                                         .arg(em + 1).arg(expectedFn)));
                }
                if (ai == am || (err
                                 ? !QRegularExpression(QRegularExpression::anchoredPattern(tmpl)).matchView(actual.at(am - 1)).hasMatch()
                                 : (actual.at(am - 1) != tmpl))) {
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
    for (qsizetype j = qMax(qsizetype(0), oai - 3); j < oai; j++) {
        diff += actual.at(j);
        diff += u'\n';
    }
    diff += "<<<<<<< got\n"_L1;
    for (qsizetype j = oai; j < oam; j++) {
        diff += actual.at(j);
        diff += u'\n';
        if (j >= oai + 5) {
            diff += "...\n"_L1;
            break;
        }
    }
    diff += "=========\n"_L1;
    for (qsizetype j = oei; j < oem; j++) {
        diff += expected.at(j);
        diff += u'\n';
        if (j >= oei + 5) {
            diff += "...\n"_L1;
            break;
        }
    }
    diff += ">>>>>>> expected\n"_L1;
    for (qsizetype j = oam; j < qMin(oam + 3, actual.size()); j++) {
        diff += actual.at(j);
        diff += u'\n';
    }
    QFAIL(qPrintable((err ? "Output for "_L1 : "Result for "_L1)
                     + expectedFn + " does not meet expectations:\n"_L1 + diff));
    }

void tst_lupdate::doCompare(const QString &actualFn, const QString &expectedFn, bool err)
{
    QFile afile(actualFn);
    QVERIFY2(afile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(actualFn));
    const QString actual = QString::fromUtf8(afile.readAll());

    doCompare(QStringView{actual}.split(u'\n'), expectedFn, err);
}

void tst_lupdate::good_data()
{
    QTest::addColumn<QString>("directory");
    QTest::addColumn<bool>("useClangCpp");

    QDir parsingDir(m_basePath + "good"_L1);
    QStringList dirs = parsingDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

#ifndef Q_OS_WIN
    dirs.removeAll("backslashes"_L1);
#endif
#ifndef Q_OS_MACOS
    dirs.removeAll("parseobjc"_L1);
#endif
    QSet<QString> ignoredTests = {
        "lacksqobject_clang_parser"_L1, "parsecontexts_clang_parser"_L1, "parsecpp2_clang_parser"_L1,
        "parsecpp_clang_parser"_L1,     "prefix_clang_parser"_L1,        "preprocess_clang_parser"_L1,
        "parsecpp_clang_only"_L1};

    // Add test rows for the "classic" lupdate
    for (const QString &dir : dirs) {
        if (ignoredTests.contains(dir))
            continue;
        QTest::newRow(dir.toLocal8Bit()) << dir << false;
    }

#if QT_CONFIG(clangcpp) && QT_CONFIG(widgets)
    if (QSysInfo::currentCpuArchitecture() == "arm64"_L1
        && QSysInfo::kernelType() == "linux"_L1 ) {
        qDebug("clangcpp tests are skipped on linux arm64, see also QTBUG-127751");
        return;
    }

    // Add test rows for the clang-based lupdate
    ignoredTests = {
        "lacksqobject"_L1,
        "parsecontexts"_L1,
        "parsecpp"_L1,
        "parsecpp2",
        "parseqrc_json"_L1,
        "prefix"_L1,
        "preprocess"_L1,
        "proparsing2"_L1, // llvm8 cannot handle file name without extension
        "respfile"_L1, //@lst not supported with the new parser yet (include not properly set in the compile_command.json)
        "cmdline_deeppath"_L1, //no project file, new parser does not support (yet) this way of launching lupdate
        "cmdline_order"_L1, // no project, new parser do not pickup on macro defined but not used. Test not needed for new parser.
        "cmdline_recurse"_L1 // recursive scan without project file not supported (yet) with the new parser
    };
    for (const QString &dir : dirs) {
        if (ignoredTests.contains(dir))
            continue;
        QTest::newRow(("clang-"_ba + dir.toLocal8Bit()).constData()) << dir << true;
    }
#endif
}

static QByteArray msgStartFailed(const QProcess &process)
{
    const QString result = u'"' + process.program() + u' ' + process.arguments().join(u' ')
                           +  "\": "_L1 + process.errorString();
    return result.toLocal8Bit();
}

static QByteArray msgTimeout(const QProcess &process)
{
    const QString result = u'"' + process.program() + u' ' + process.arguments().join(u' ')
                           +  "\" timed out: "_L1 + process.errorString();
    return result.toLocal8Bit();
}

static QByteArray msgCrashed(const QProcess &process, const QByteArray &output)
{
    const QString result = u'"' + process.program() + u' ' + process.arguments().join(u' ')
                           + "\" crashed\n"_L1;
    return result.toLocal8Bit() + output;
}

static QByteArray msgExitCode(const QProcess &process, const QByteArray &output)
{
    const QString result = u'"' + process.program() + u' ' + process.arguments().join(u' ')
                           + "\" exited with code "_L1 + QString::number(process.exitCode())
                           + u'\n';
    return result.toLocal8Bit() + output;
}

void tst_lupdate::good()
{
    QFETCH(QString, directory);
    QFETCH(bool, useClangCpp);

    QString dir = m_basePath + "good/"_L1 + directory;
    QString workDir = dir;
    QStringList generatedtsfiles("project.ts"_L1);
    QStringList lupdateArguments;

    QFile file(dir + "/lupdatecmd"_L1);
    if (file.exists()) {
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.fileName()));
        while (!file.atEnd()) {
            QByteArray cmdstring = file.readLine().simplified();
            if (cmdstring.startsWith(u'#'))
                continue;
            if (cmdstring.startsWith("lupdate"_L1)) {
                for (auto argument : cmdstring.sliced(8).simplified().split(' '))
                    lupdateArguments += argument;
                break;
            }
            if (cmdstring.startsWith("TRANSLATION:"_L1)) {
                cmdstring.remove(0, 12);
                generatedtsfiles.clear();
                const auto parts = cmdstring.split(u' ');
                for (const QByteArray &s : parts)
                    if (!s.isEmpty())
                        generatedtsfiles << QLatin1StringView(s);
            } else if (cmdstring.startsWith("cd "_L1)) {
                cmdstring.remove(0, 3);
                workDir = QDir::cleanPath(dir + u'/' + QLatin1StringView(cmdstring));
            }
        }
        file.close();
    }

    for (const QString &ts : std::as_const(generatedtsfiles)) {
        QString genTs = workDir + u'/' + ts;
        QFile::remove(genTs);
        QString beforetsfile = dir + u'/' + ts + ".before"_L1;
        if (QFile::exists(beforetsfile))
            QVERIFY2(QFile::copy(beforetsfile, genTs), qPrintable(beforetsfile));
    }

    file.setFileName(workDir + "/.qmake.cache"_L1);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();

    if (lupdateArguments.isEmpty()) {
        // Automatically pass "project.pro" or "-project project.json".
        if (QFile::exists(dir + u"/project.json"_s)) {
            lupdateArguments << u"-project"_s << u"project.json"_s;
        } else {
            lupdateArguments.append("project.pro"_L1);
        }
    }

    lupdateArguments.prepend("-silent"_L1);
    if (useClangCpp)
        lupdateArguments.append("-clang-parser"_L1);

    QProcess proc;
    proc.setWorkingDirectory(workDir);
    proc.setProcessChannelMode(QProcess::MergedChannels);
    const auto startTime = m_timer.elapsed();
    proc.start(m_cmdLupdate, lupdateArguments, QIODevice::ReadWrite | QIODevice::Text);
    QVERIFY2(proc.waitForStarted(), msgStartFailed(proc).constData());
    if (!proc.waitForFinished(TIMEOUT)) {
        const auto message = msgTimeout(proc);
        proc.kill();
        proc.waitForFinished(50);
        QFAIL(message.constData());
    }
    const auto elapsed = m_timer.elapsed() - startTime;
    if (elapsed > m_maxElapsed)
        m_maxElapsed = elapsed;

    const QByteArray output = proc.readAll();
    QVERIFY2(proc.exitStatus() == QProcess::NormalExit, msgCrashed(proc, output).constData());
    QVERIFY2(proc.exitCode() == 0, msgExitCode(proc, output).constData());

    qInfo().noquote().nospace() << elapsed << "ms";

    // If the file expectedoutput.txt exists, compare the
    // console output with the content of that file
    QFile outfile(dir + "/expectedoutput.txt"_L1);
    if (outfile.exists()) {
        const QString errslist = QString::fromLocal8Bit(output);
        doCompare(QStringView{errslist}.split(u'\n'), outfile.fileName(), true);
        if (QTest::currentTestFailed())
            return;
    }

    for (const QString &ts : std::as_const(generatedtsfiles)) {
        if (dir.endsWith("preprocess_clang_parser"_L1)) {
            doCompare(workDir + u'/' + ts,
                      dir + u'/' + ts + ".result"_L1, true);
        } else {
        doCompare(workDir + u'/' + ts,
                  dir + u'/' + ts + ".result"_L1, false);
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
