// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <algorithm>
#include <utility>

// Whether a fixture's qdoc run is expected to succeed, or to fail gracefully by
// reporting the failing page, skipping it, and exiting non-zero without crashing.
enum class ExpectedExitCode { Zero, NonZero };

class tst_validateTemplateGeneratorOutput : public QObject
{
    Q_OBJECT
private:
    void runQDocProcess(const QStringList &arguments,
                        ExpectedExitCode expectedExitCode = ExpectedExitCode::Zero,
                        const QStringList &requiredStandardErrorText = {});
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

void tst_validateTemplateGeneratorOutput::initTestCase()
{
#if (defined(__has_feature) && __has_feature(address_sanitizer)) || defined(__SANITIZE_ADDRESS__)
    // The QDoc subprocess this test spawns links against a libclang built
    // without LLVM_USE_SANITIZER=Address. The asymmetric configuration
    // trips an upstream LLVM defect in BumpPtrAllocator: inline
    // poison/unpoison calls are gated on a per-translation-unit macro,
    // so the slow-path StartNewSlab poisons via QDoc's instrumented copy
    // while downstream consumers inside libclang read those bytes through
    // compiler-instrumented loads that ASan reports as use-after-poison.
    // The reports surface across many libclang code paths and include
    // direct loads that ASan suppression files cannot mask.
    //
    // Re-enable this test under ASan once an upstream LLVM fix is rolled
    // out to the Coin libclang artefact, or once a sanitiser-instrumented
    // libclang variant replaces the asymmetric configuration. See the
    // tracking JIRA for the upstream-issue URL and the conditions under
    // which the skip should be removed.
    QSKIP("Disabled under AddressSanitizer: upstream LLVM BumpPtrAllocator "
          "defect (asymmetric instrumentation against an unsanitised libclang).");
#endif

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

void tst_validateTemplateGeneratorOutput::init()
{
    m_outputDir.reset(new QTemporaryDir());
    if (!m_outputDir->isValid()) {
        const QString errorMessage =
                "Couldn't create temporary directory: " + m_outputDir->errorString();
        QFAIL(qPrintable(errorMessage));
    }
}

void tst_validateTemplateGeneratorOutput::runQDocProcess(
        const QStringList &arguments, ExpectedExitCode expectedExitCode,
        const QStringList &requiredStandardErrorText)
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

    const QString errors = qdocProcess.readAllStandardError();

    const bool missingRequiredText =
            std::any_of(requiredStandardErrorText.cbegin(), requiredStandardErrorText.cend(),
                        [&errors](const QString &required) { return !errors.contains(required); });

    if (!errors.isEmpty() && (qdocProcess.exitCode() != 0 || missingRequiredText))
        qInfo().nospace() << "Received errors:\n" << qUtf8Printable(errors);

    for (const QString &required : requiredStandardErrorText) {
        const QString message =
                QStringLiteral("qdoc's standard error doesn't contain the required text: ")
                + required;
        QVERIFY2(errors.contains(required), qPrintable(message));
    }

    if (expectedExitCode == ExpectedExitCode::NonZero) {
        QVERIFY2(qdocProcess.exitStatus() == QProcess::NormalExit,
                 "qdoc crashed; a graceful non-zero exit was expected");
        QVERIFY2(qdocProcess.exitCode() != 0,
                 "qdoc exited 0, but this fixture expects a non-zero exit after skipping a page");
        return;
    }

    if (qdocProcess.exitCode() == 0)
        return;

    if (!QTest::currentTestFailed())
        failQDoc(QProcess::UnknownError);
}

std::optional<QByteArray>
tst_validateTemplateGeneratorOutput::gitDiffDirectories(const QString &actualPath, const QString &expectedPath)
{
    QProcess gitProcess;
    gitProcess.setProgram("git");

    const QStringList arguments{"diff", "--", expectedPath, actualPath};
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

void tst_validateTemplateGeneratorOutput::qdocProjects_data()
{
    using namespace Qt::StringLiterals;
    QTest::addColumn<QString>("qdocconf");
    QTest::addColumn<QString>("expectedPath");
    QTest::addColumn<QString>("extraArgs");
    QTest::addColumn<ExpectedExitCode>("expectedExitCode");
    QTest::addColumn<QStringList>("requiredStandardErrorText");

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

        // A fixture opts into "graceful non-zero exit expected" by dropping an
        // expect-nonzero-exit marker file in its directory, mirroring how
        // args.txt opts into extra arguments.
        const ExpectedExitCode expectedExitCode =
                QFileInfo::exists(configFile.dir().absolutePath() + u"/expect-nonzero-exit"_s)
                ? ExpectedExitCode::NonZero
                : ExpectedExitCode::Zero;

        // A fixture asserts on qdoc's diagnostics by listing one required
        // standard-error substring per line in an expect-stderr-contains file.
        // Surrounding whitespace is ignored, and so are blank lines and lines
        // that start with '#', which let the file explain itself in the same
        // self-documenting style as the marker above.
        QStringList requiredStandardErrorText;
        QFile requiredText{configFile.dir().absolutePath() + u"/expect-stderr-contains"_s};
        if (requiredText.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream{&requiredText};
            while (!stream.atEnd()) {
                const QString line = stream.readLine();
                const QStringView trimmed = QStringView{line}.trimmed();
                if (trimmed.isEmpty() || trimmed.startsWith(u'#'))
                    continue;
                requiredStandardErrorText << trimmed.toString();
            }
        }

        const QString testName =
                configFile.dir().dirName() + u'/' + configFile.fileName();

        QTest::newRow(testName.toUtf8().constData())
                << configFile.absoluteFilePath()
                << configFile.dir().absolutePath() + "/expected/"
                << extraArgs
                << expectedExitCode
                << requiredStandardErrorText;
    }
}

void tst_validateTemplateGeneratorOutput::qdocProjects()
{
    QFETCH(const QString, qdocconf);
    QFETCH(const QString, expectedPath);
    QFETCH(const QString, extraArgs);
    QFETCH(const ExpectedExitCode, expectedExitCode);
    QFETCH(const QStringList, requiredStandardErrorText);

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

    runQDocProcess(arguments, expectedExitCode, requiredStandardErrorText);

    // A failed expectation about the run itself is the real failure; don't bury
    // it under a diff of output that was never meant to be compared.
    if (QTest::currentTestFailed())
        return;

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

QTEST_MAIN(tst_validateTemplateGeneratorOutput)
#include "tst_validatetemplategeneratoroutput.moc"
