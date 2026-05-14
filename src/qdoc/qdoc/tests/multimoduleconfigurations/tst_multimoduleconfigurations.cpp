// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <optional>

class tst_multiModuleConfigurations : public QObject
{
    Q_OBJECT
private:
    void runQDocProcess(const QStringList &arguments);
    std::optional<QByteArray> gitDiffDirectories(const QString &actualPath,
                                                 const QString &expectedPath);
    QStringList readBuildOrder(const QString &orderFilePath, const QDir &fixtureDir);
    static QString readProjectName(const QString &qdocconfPath);
    static bool copyDirectoryRecursive(const QString &sourcePath, const QString &destinationPath);

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

void tst_multiModuleConfigurations::initTestCase()
{
    if (s_environment.contains(REGENERATE_ENVVAR)) {
        qInfo() << "Regenerating expected output for all tests.";
        regenerate = true;
        qInfo("Removing %s environment variable.", REGENERATE_ENVVAR.constData());
        s_environment.remove(REGENERATE_ENVVAR);
    }

    const QString optionString = s_environment.contains(ASAN_OPTIONS_ENVVAR) ? ",use_sigaltstack=0" : "use_sigaltstack=0";
    s_environment.insert(ASAN_OPTIONS_ENVVAR, s_environment.value(ASAN_OPTIONS_ENVVAR) + optionString);
    qInfo() << "Disabling ASan's alternate signal stack by setting `ASAN_OPTIONS=use_sigaltstack=0`.";

    const auto binpath = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    const auto extension = QSysInfo::productType() == "windows" ? ".exe" : "";
    m_qdocBinary = binpath + QLatin1String("/qdoc") + extension;
    QVERIFY(QFile::exists(m_qdocBinary));

    m_extraParams = QFileInfo(QTest::currentAppName()).dir().filePath(DOCINCPATH);
    if (!QFileInfo::exists(m_extraParams)) {
        qWarning("Cannot locate %s", m_extraParams.toLocal8Bit().constData());
        m_extraParams.clear();
    } else {
        m_extraParams.insert(0, '@');
    }
}

void tst_multiModuleConfigurations::init()
{
    m_outputDir.reset(new QTemporaryDir());
    if (!m_outputDir->isValid()) {
        const QString errorMessage =
                "Couldn't create temporary directory: " + m_outputDir->errorString();
        QFAIL(qPrintable(errorMessage));
    }
}

void tst_multiModuleConfigurations::runQDocProcess(const QStringList &arguments)
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
tst_multiModuleConfigurations::gitDiffDirectories(const QString &actualPath, const QString &expectedPath)
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

QStringList tst_multiModuleConfigurations::readBuildOrder(const QString &orderFilePath,
                                                          const QDir &fixtureDir)
{
    using namespace Qt::StringLiterals;
    QStringList qdocconfs;
    QFile file{orderFilePath};
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return qdocconfs;

    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(u'#'))
            continue;
        qdocconfs << fixtureDir.absoluteFilePath(line + u".qdocconf"_s);
    }
    return qdocconfs;
}

QString tst_multiModuleConfigurations::readProjectName(const QString &qdocconfPath)
{
    using namespace Qt::StringLiterals;
    QFile file{qdocconfPath};
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    static const QRegularExpression projectLine(uR"(^\s*project\s*=\s*(\S+))"_s);
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine());
        const QRegularExpressionMatch match = projectLine.match(line);
        if (match.hasMatch())
            return match.captured(1);
    }
    return QString();
}

bool tst_multiModuleConfigurations::copyDirectoryRecursive(const QString &sourcePath,
                                                            const QString &destinationPath)
{
    using namespace Qt::StringLiterals;
    QDir source{sourcePath};
    if (!source.exists())
        return false;

    QDir destination{destinationPath};
    if (!destination.exists() && !destination.mkpath(u"."_s))
        return false;

    const QFileInfoList entries =
            source.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (const QFileInfo &entry : entries) {
        const QString target = destination.absoluteFilePath(entry.fileName());
        if (entry.isDir()) {
            if (!copyDirectoryRecursive(entry.absoluteFilePath(), target))
                return false;
        } else if (!QFile::copy(entry.absoluteFilePath(), target)) {
            return false;
        }
    }
    return true;
}

void tst_multiModuleConfigurations::qdocProjects_data()
{
    using namespace Qt::StringLiterals;
    QTest::addColumn<QStringList>("qdocconfs");
    QTest::addColumn<QString>("expectedPath");
    QTest::addColumn<QString>("extraArgs");

    bool foundAny = false;
    QDirIterator orderit(m_testDataDirectory, QStringList{u"build-order.txt"_s},
                         QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (orderit.hasNext()) {
        const QFileInfo orderFile = orderit.nextFileInfo();
        const QDir fixtureDir = orderFile.dir();
        const QStringList qdocconfs = readBuildOrder(orderFile.absoluteFilePath(), fixtureDir);
        if (qdocconfs.isEmpty())
            continue;

        QString extraArgs{fixtureDir.absolutePath() + u"/args.txt"_s};
        if (QFileInfo::exists(extraArgs))
            extraArgs.insert(0, u'@');
        else
            extraArgs.clear();

        const QString testName = fixtureDir.dirName();

        QTest::newRow(testName.toUtf8().constData())
                << qdocconfs
                << fixtureDir.absolutePath() + u"/expected/"_s
                << extraArgs;
        foundAny = true;
    }

    // Qt Test treats a data-driven slot with zero rows as a hard error.
    // Emit one placeholder row so the slot's QSKIP path runs instead.
    if (!foundAny)
        QTest::newRow("no_fixtures") << QStringList{} << QString{} << QString{};
}

void tst_multiModuleConfigurations::qdocProjects()
{
    using namespace Qt::StringLiterals;
    QFETCH(const QStringList, qdocconfs);
    QFETCH(const QString, expectedPath);
    QFETCH(const QString, extraArgs);

    if (qdocconfs.isEmpty())
        QSKIP("Fixture has no qdocconfs declared in build-order.txt");

    QString lastActualPath;
    if (regenerate) {
        QDir pathToRemove{expectedPath};
        if (pathToRemove.exists() && !pathToRemove.removeRecursively())
            qCritical("Cannot remove expected output directory, aborting!");
        lastActualPath = expectedPath;
    } else {
        lastActualPath = m_outputDir->path() + QLatin1String("/final");
    }

    QStringList indexdirArgs;
    for (qsizetype i = 0; i < qdocconfs.size(); ++i) {
        const QString &qdocconf = qdocconfs[i];
        if (!QFileInfo::exists(qdocconf)) {
            QFAIL(qPrintable(u"Declared qdocconf does not exist: "_s + qdocconf));
            return;
        }

        QString projectName = readProjectName(qdocconf);
        if (projectName.isEmpty())
            projectName = QFileInfo(qdocconf).baseName();
        const QString moduleSubdir = projectName.toLower();
        const bool isLast = (i == qdocconfs.size() - 1);

        const QString moduleBaseDir = isLast ? lastActualPath : m_outputDir->path();
        const QString moduleOutputDir = moduleBaseDir + QLatin1Char('/') + moduleSubdir;

        QStringList arguments{u"-outputdir"_s, moduleOutputDir};
        arguments << indexdirArgs;
        if (!m_extraParams.isEmpty())
            arguments << m_extraParams;
        if (!extraArgs.isEmpty())
            arguments << extraArgs;
        arguments << qdocconf;

        runQDocProcess(arguments);

        indexdirArgs << u"-indexdir"_s << moduleOutputDir;
    }

    if (regenerate) {
        const QString message = u"Regenerated expected output files for "_s + qdocconfs.last();
        QSKIP(message.toLocal8Bit().constData());
    }

    std::optional<QByteArray> gitDiff = gitDiffDirectories(lastActualPath, expectedPath);
    if (gitDiff.has_value()) {
        qInfo() << qUtf8Printable(gitDiff.value());
        QFAIL("Inspect the output for details.");
    }
    QVERIFY(true);
}

QTEST_MAIN(tst_multiModuleConfigurations)
#include "tst_multimoduleconfigurations.moc"

