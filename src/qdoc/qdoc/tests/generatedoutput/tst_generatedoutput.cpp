// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QProcess>
#include <QTemporaryDir>
#include <QDirIterator>
#include <QtTest>

class tst_generatedOutput : public QObject
{
    Q_OBJECT

public:
    void setRegenerate() { m_regen = true; }

private slots:
    void initTestCase();
    void init();

    // HTML generator
    void htmlFromCpp();

    // Output format independent tests
    void inheritedQmlPropertyGroups();
    void crossModuleLinking();
    void indexLinking();
    void includeFromExampleDirs();
    void singleExec();
    void preparePhase();
    void generatePhase();
    void noAutoList();

private:
    QScopedPointer<QTemporaryDir> m_outputDir;
    QString m_qdoc;
    QDir m_expectedDir;
    QString m_extraParams;
    bool m_regen = false;

    void runQDocProcess(const QStringList &arguments);
    void compareLineByLine(const QStringList &expectedFiles);
    void testAndCompare(const char *input, const char *outNames, const char *extraParams = nullptr);
    void copyIndexFiles();
};

void tst_generatedOutput::initTestCase()
{
    // Build the path to the QDoc binary the same way moc tests do for moc.
    const auto binpath = QLibraryInfo::path(QLibraryInfo::BinariesPath);
    const auto extension = QSysInfo::productType() == "windows" ? ".exe" : "";
    m_qdoc = binpath + QLatin1String("/qdoc") + extension;
    m_expectedDir.setPath(QFINDTESTDATA("expected_output"));

    // Resolve the path to the file containing extra parameters
    m_extraParams = QFileInfo(QTest::currentAppName()).dir().filePath("qdocincludepaths.inc");
    if (!QFileInfo::exists(m_extraParams)) {
        qWarning().nospace() << QStringLiteral("Cannot locate")
                             << qUtf8Printable(m_extraParams);
        m_extraParams.clear();
    } else {
        m_extraParams.insert(0, '@');
    }
}

void tst_generatedOutput::init()
{
    m_outputDir.reset(new QTemporaryDir());
    if (!m_outputDir->isValid()) {
        const QString errorMessage =
                "Couldn't create temporary directory: " + m_outputDir->errorString();
        QFAIL(qPrintable(errorMessage));
    }
}

void tst_generatedOutput::runQDocProcess(const QStringList &arguments)
{
    QProcess qdocProcess;
    qdocProcess.setProgram(m_qdoc);
    qdocProcess.setArguments(arguments);

    auto failQDoc = [&](QProcess::ProcessError) {
        QFAIL(qPrintable(QStringLiteral("Running qdoc failed with exit code %1: %2")
                .arg(qdocProcess.exitCode()).arg(qdocProcess.errorString())));
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

void tst_generatedOutput::compareLineByLine(const QStringList &expectedFiles)
{
    for (const auto &file : expectedFiles) {
        QString expected(m_expectedDir.filePath(file));
        QString actual(m_outputDir->filePath(file));

        QFile expectedFile(expected);
        if (!expectedFile.open(QIODevice::ReadOnly))
            QFAIL(qPrintable(QString("Cannot open expected data file: %1").arg(expected)));
        QTextStream expectedIn(&expectedFile);

        QFile actualFile(actual);
        if (!actualFile.open(QIODevice::ReadOnly))
            QFAIL(qPrintable(QString("Cannot open actual data file: %1").arg(actual)));
        QTextStream actualIn(&actualFile);

        const QLatin1String delim(": ");
        int lineNumber = 0;
        while (!expectedIn.atEnd() && !actualIn.atEnd()) {
            lineNumber++;
            QString prefix = file + delim + QString::number(lineNumber) + delim;
            QString expectedLine = prefix + expectedIn.readLine();
            QString actualLine = prefix + actualIn.readLine();
            QCOMPARE(actualLine, expectedLine);
        }
    }
}

void tst_generatedOutput::testAndCompare(const char *input, const char *outNames,
                                         const char *extraParams)
{
    QStringList args{ "-outputdir", m_outputDir->path() + "/", QFINDTESTDATA(input) };
    if (extraParams)
        args << QString(QLatin1String(extraParams)).split(QChar(' '));

    runQDocProcess(args);

    if (QTest::currentTestFailed())
        return;

    QStringList expectedOuts(QString(QLatin1String(outNames)).split(QChar(' ')));
    if (m_regen) {
        QVERIFY(m_expectedDir.mkpath(m_expectedDir.path()));
        for (const auto &file : std::as_const(expectedOuts)) {
            QFileInfo fileInfo(m_expectedDir.filePath(file));
            fileInfo.dir().remove(fileInfo.fileName()); // Allowed to fail
            QVERIFY(m_expectedDir.mkpath(fileInfo.dir().path()));
            QVERIFY2(QFile::copy(m_outputDir->filePath(file), fileInfo.filePath()),
                     qPrintable(QStringLiteral("Failed to copy '%1'").arg(file)));
        }
        QSKIP("Regenerated expected output only.");
    }

    compareLineByLine(expectedOuts);
}

// Copy <project>.index to <project>/<project>.index in the outputdir
void tst_generatedOutput::copyIndexFiles()
{
    QDirIterator it(m_outputDir->path(), QStringList("*.index"), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo fileInfo(it.next());
        QDir indexDir(m_outputDir->path());
        QVERIFY(indexDir.mkpath(fileInfo.baseName()));
        QVERIFY(indexDir.cd(fileInfo.baseName()));
        if (!indexDir.exists(fileInfo.fileName()))
            QVERIFY(QFile::copy(fileInfo.filePath(), indexDir.filePath(fileInfo.fileName())));
    }
}

void tst_generatedOutput::htmlFromCpp()
{
    testAndCompare("testdata/configs/testcpp.qdocconf",
                   "testcpp-module.html "
                   "testqdoc-test.html "
                   "testqdoc-test-members.html "
                   "testqdoc-test-obsolete.html "
                   "testqdoc-testderived.html "
                   "testqdoc-testderived-members.html "
                   "testqdoc-testderived-obsolete.html "
                   "obsolete-classes.html "
                   "autolinking.html "
                   "cpptypes.html "
                   "testqdoc.html");
}

void tst_generatedOutput::inheritedQmlPropertyGroups()
{
    testAndCompare("testdata/qmlpropertygroups/qmlpropertygroups.qdocconf",
                   "qmlpropertygroups/qml-qdoc-test-anotherchild-members.html "
                   "qmlpropertygroups/qml-qdoc-test-parent.html "
                   "qmlpropertygroups-docbook/qml-qdoc-test-parent.xml");
}

void tst_generatedOutput::indexLinking()
{
    {
        QScopedValueRollback<bool> skipRegen(m_regen, false);
        inheritedQmlPropertyGroups();
    }
    copyIndexFiles();
    QString indexDir = QLatin1String("-indexdir ") +  m_outputDir->path();
    testAndCompare("testdata/indexlinking/indexlinking.qdocconf",
                   "index-linking.html "
                   "qml-linkmodule-grandchild-members.html",
                   indexDir.toLatin1().data());
}

void tst_generatedOutput::crossModuleLinking()
{
    {
        QScopedValueRollback<bool> skipRegen(m_regen, false);
        htmlFromCpp();
    }
    copyIndexFiles();
    QString indexDir = QLatin1String("-indexdir ") +  m_outputDir->path();
    testAndCompare("testdata/crossmodule/crossmodule.qdocconf",
                   "crossmodule/testtype.html "
                   "crossmodule/testtype-members.html "
                   "crossmodule/crossmoduleref-sub-crossmodule.html",
                   indexDir.toLatin1().data());
}

void tst_generatedOutput::includeFromExampleDirs()
{
    testAndCompare("testdata/includefromexampledirs/includefromexampledirs.qdocconf",
                   "includefromexampledirs/index.html "
                   "includefromexampledirs/qml-qdoc-test-abstractparent.html "
                   "includefromexampledirs/qml-qdoc-test-abstractparent-members.html");
}

void tst_generatedOutput::singleExec()
{
    // Build both testcpp and crossmodule projects in single-exec mode
    testAndCompare("testdata/singleexec/singleexec.qdocconf",
                   "testcpp/testcpp-module.html "
                   "testcpp/testqdoc-test.html "
                   "testcpp/testqdoc-test-members.html "
                   "testcpp/testqdoc.html "
                   "testcpp/crossmoduleref.html "
                   "crossmodule/crossmodule/all-namespaces.html "
                   "crossmodule/crossmodule/testtype.html "
                   "crossmodule/crossmodule/testtype-members.html",
                   "-single-exec");
}

void tst_generatedOutput::preparePhase()
{
    testAndCompare("testdata/configs/testcpp.qdocconf",
                   "testcpp.index",
                   "-prepare");
}

void tst_generatedOutput::generatePhase()
{
    testAndCompare("testdata/configs/testcpp.qdocconf",
                   "testcpp-module.html "
                   "testqdoc-test.html "
                   "testqdoc-test-members.html "
                   "testqdoc.html",
                   "-generate");
}

void tst_generatedOutput::noAutoList()
{
    testAndCompare("testdata/configs/noautolist.qdocconf",
                   "noautolist/testcpp-module.html "
                   "noautolist/test-componentset-example.html "
                   "noautolist/qdoc-test-qmlmodule.html "
                   "noautolist-docbook/testcpp-module.xml "
                   "noautolist-docbook/test-componentset-example.xml "
                   "noautolist-docbook/qdoc-test-qmlmodule.xml");
}

int main(int argc, char *argv[])
{
    tst_generatedOutput tc;
    // Re-populate expected data and skip tests if option -regenerate is set
    if (argc == 2 && QByteArray(argv[1]) == "-regenerate") {
        tc.setRegenerate();
        --argc;
    }
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_generatedoutput.moc"
