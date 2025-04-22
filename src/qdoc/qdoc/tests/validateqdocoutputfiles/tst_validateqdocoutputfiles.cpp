// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <utility>

class tst_validateQdocOutputFiles : public QObject
{
    Q_OBJECT
private:
    void runQDocProcess(const QStringList &arguments);
    std::optional<QByteArray> gitDiffDirectories(const QString &actualPath,
                                                 const QString &expectedPath);

private slots:
    void initTestCase();
    void init();
    void qdocProjects_data();
    void qdocProjects();

private:
    const QString m_testDataDirectory = QFINDTESTDATA("testdata");
    QString m_qdocBinary{};
    QString m_extraParams{};
    QScopedPointer<QTemporaryDir> m_outputDir{};
};

static constexpr QLatin1StringView ASAN_OPTIONS_ENVVAR{"ASAN_OPTIONS"};
static inline bool regenerate{false};

//! Update `README.md` if you change the name of this environment variable!
static constexpr QLatin1StringView REGENERATE_ENVVAR{"QDOC_REGENERATE_TESTDATA"};
static QProcessEnvironment s_environment {QProcessEnvironment::systemEnvironment()};

void tst_validateQdocOutputFiles::initTestCase()
{
    if (s_environment.contains(REGENERATE_ENVVAR)) {
        qInfo() << "Regenerating expected output for all tests.";
        regenerate = true;
        qInfo("Removing %s environment variable.", REGENERATE_ENVVAR.constData());
        s_environment.remove(REGENERATE_ENVVAR);
    }

    // We must disable the use of sigaltstack for ASan to work properly with QDoc when
    // linked against libclang, to avoid a crash in ASan. This is a known issue and workaround,
    // see e.g. https://github.com/google/sanitizers/issues/849 and
    // https://github.com/KDE/kdevelop/commit/e306f3e39aba37b606dadba195fa5b7b73816f8f.
    // We do this for the process environment of the QDoc process only to avoid affecting
    // other processes that might be started by the test runner in COIN.
    const QString optionString = s_environment.contains(ASAN_OPTIONS_ENVVAR) ? ",use_sigaltstack=0" : "use_sigaltstack=0";
    s_environment.insert(ASAN_OPTIONS_ENVVAR, s_environment.value(ASAN_OPTIONS_ENVVAR) + optionString);
    qInfo() << "Disabling ASan's alternate signal stack by setting `ASAN_OPTIONS=use_sigaltstack=0`.";

    // Build the path to the QDoc binary the same way moc tests do for moc.
    const auto binpath = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    const auto extension = QSysInfo::productType() == "windows" ? ".exe" : "";
    m_qdocBinary = binpath + QLatin1String("/qdoc") + extension;
    QVERIFY(QFile::exists(m_qdocBinary));

    // Resolve the path to the file containing extra parameters
    m_extraParams = QFileInfo(QTest::currentAppName()).dir().filePath(DOCINCPATH);
    if (!QFileInfo::exists(m_extraParams)) {
        qWarning("Cannot locate %s", m_extraParams.toLocal8Bit().constData());
        m_extraParams.clear();
    } else {
        m_extraParams.insert(0, '@');
    }
}

void tst_validateQdocOutputFiles::init()
{
    m_outputDir.reset(new QTemporaryDir());
    if (!m_outputDir->isValid()) {
        const QString errorMessage =
                "Couldn't create temporary directory: " + m_outputDir->errorString();
        QFAIL(qPrintable(errorMessage));
    }
}

void tst_validateQdocOutputFiles::runQDocProcess(const QStringList &arguments)
{
    QProcess qdocProcess;
    qdocProcess.setProcessEnvironment(s_environment);
    qdocProcess.setProgram(m_qdocBinary);
    qdocProcess.setArguments(arguments);

    auto failQDoc = [&](QProcess::ProcessError) {
        qFatal("Running qdoc failed with exit code %i: %s",
               qdocProcess.exitCode(), qUtf8Printable(qdocProcess.errorString()));
    };
    QObject::connect(&qdocProcess, &QProcess::errorOccurred, this, failQDoc);

    qdocProcess.start();
    qdocProcess.waitForFinished();
    if (qdocProcess.exitCode() == 0)
        return;

    QString errors = qdocProcess.readAllStandardError();
    if (!errors.isEmpty())
        qInfo().nospace() << "Received errors:\n" << qUtf8Printable(errors);
    if (!QTest::currentTestFailed())
        failQDoc(QProcess::UnknownError);
}

std::optional<QByteArray>
tst_validateQdocOutputFiles::gitDiffDirectories(const QString &actualPath, const QString &expectedPath)
{
    QProcess gitProcess;
    gitProcess.setProgram("git");

    const QStringList arguments{"diff", "--", actualPath, expectedPath};
    gitProcess.setArguments(arguments);

    auto failGit = [&](QProcess::ProcessError) {
        qFatal("Running git failed with exit code %i: %s",
               gitProcess.exitCode(), gitProcess.errorString().toLocal8Bit().constData());
    };
    QObject::connect(&gitProcess, &QProcess::errorOccurred, this, failGit);

    gitProcess.start();
    gitProcess.waitForFinished();

    if (gitProcess.exitCode() == 0)
        return {};

    return gitProcess.readAllStandardOutput();
}

void tst_validateQdocOutputFiles::qdocProjects_data()
{
    using namespace Qt::StringLiterals;
    QTest::addColumn<QString>("qdocconf");
    QTest::addColumn<QString>("expectedPath");
    QTest::addColumn<QString>("extraArgs");

    QDirIterator qdocconfit(m_testDataDirectory, QStringList { u"*.qdocconf"_s },
                            QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (qdocconfit.hasNext()) {
        const QFileInfo configFile = qdocconfit.nextFileInfo();
        if (configFile.baseName() != configFile.dir().dirName())
            continue;

        QString extraArgs{configFile.dir().absolutePath() + u"/args.txt"_s};
        if (QFileInfo::exists(extraArgs))
            extraArgs.insert(0, u'@');
        else
            extraArgs.clear();

        const QString testName =
                configFile.dir().dirName() + u'/' + configFile.fileName();

        QTest::newRow(testName.toUtf8().constData())
                << configFile.absoluteFilePath()
                << configFile.dir().absolutePath() + "/expected/"
                << extraArgs;
    }
}

void tst_validateQdocOutputFiles::qdocProjects()
{
    QFETCH(const QString, qdocconf);
    QFETCH(const QString, expectedPath);
    QFETCH(const QString, extraArgs);

    QString actualPath{m_outputDir->path()};
    if (regenerate) {
        actualPath = expectedPath;
        QDir pathToRemove{expectedPath};
        if (!pathToRemove.removeRecursively())
            qCritical("Cannot remove expected output directory, aborting!");
    }

    QStringList arguments{ "-outputdir", actualPath, m_extraParams, qdocconf };
    if (!extraArgs.isEmpty())
        arguments << extraArgs;

    runQDocProcess(arguments);

    if (regenerate) {
        const QString message = "Regenerated expected output files for" + qdocconf;
        QSKIP(message.toLocal8Bit().constData());
    }

    std::optional<QByteArray> gitDiff = gitDiffDirectories(actualPath, expectedPath);
    if (gitDiff.has_value()) {
        qInfo() << qUtf8Printable(gitDiff.value());
        QFAIL("Inspect the output for details.");
    }
    QVERIFY(true);
}

QTEST_MAIN(tst_validateQdocOutputFiles)
#include "tst_validateqdocoutputfiles.moc"
