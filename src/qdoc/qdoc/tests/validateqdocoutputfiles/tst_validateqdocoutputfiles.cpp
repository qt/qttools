// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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

void tst_validateQdocOutputFiles::initTestCase()
{
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

    QDirIterator qdocconfit(m_testDataDirectory, QStringList { u"*.qdocconf"_s },
                            QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (qdocconfit.hasNext()) {
        const QFileInfo configFile = qdocconfit.nextFileInfo();
        if (configFile.baseName() != configFile.dir().dirName())
            continue;

        const QString testName =
                configFile.dir().dirName() + u'/' + configFile.fileName();

        QTest::newRow(testName.toUtf8().constData())
                << configFile.absoluteFilePath() << configFile.dir().absolutePath() + "/expected/";
    }
}

void tst_validateQdocOutputFiles::qdocProjects()
{
    QFETCH(const QString, qdocconf);
    QFETCH(const QString, expectedPath);

    const QString actualPath = m_outputDir->path();
    const QStringList arguments{ "-outputdir", actualPath, m_extraParams, qdocconf };

    runQDocProcess(arguments);
    std::optional<QByteArray> gitDiff = gitDiffDirectories(actualPath, expectedPath);
    if (gitDiff.has_value()) {
        qInfo() << qUtf8Printable(gitDiff.value());
        QFAIL("Inspect the output for details.");
    }
    QVERIFY(true);
}

QTEST_MAIN(tst_validateQdocOutputFiles)
#include "tst_validateqdocoutputfiles.moc"
