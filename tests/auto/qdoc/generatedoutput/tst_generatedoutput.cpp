// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
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
    void htmlFromQDocFile();
    void htmlFromCpp();
    void htmlFromQml();
    void htmlFromCppBug80259();

    // WebXML generator
    void webXmlFromQDocFile();
    void webXmlFromCpp();
    void webXmlFromQml();
    void webXmlFromCppBug80259();
    void illformated_documentation();

    // DocBook generator (wiht and without extensions)
    void docBookFromQDocFile();
    void docBookFromCpp();
    void docBookFromQml();
    void docBookWithExtensionsFromQDocFile();
    void docBookWithExtensionsFromCpp();
    void docBookWithExtensionsFromQml();

    // Output format independent tests
    void autoNavigation();
    void tocBreadcrumbs();
    void examplesManifestXmlAndQhp();
    void ignoresinceVariable();
    void templateParameters();
    void scopedEnum();
    void dontDocument();
    void inheritedQmlPropertyGroups();
    void crossModuleLinking();
    void indexLinking();
    void includeFromExampleDirs();
    void singleExec();
    void preparePhase();
    void generatePhase();
    void noAutoList();
    void nestedMacro();
    void headerFile();
    void usingDirective();
    void properties();
    void testTagFile();
    void testGlobalFunctions();
    void proxyPage();
    void nonAsciiCharacterInput();
    void lineComments();
    void tableAfterValue();
    void modulestateCommand();

private:
    QScopedPointer<QTemporaryDir> m_outputDir;
    QString m_qdoc;
    QDir m_expectedDir;
    QString m_extraParams;
    bool m_regen = false;

    void runQDocProcess(const QStringList &arguments);
    void compareLineByLine(const QStringList &expectedFiles);
    void testAndCompare(const char *input, const char *outNames, const char *extraParams = nullptr,
                        const char *outputPathPrefix = nullptr);
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
    QObject::connect(&qdocProcess, &QProcess::errorOccurred, failQDoc);

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
                                         const char *extraParams, const char *outputPathPrefix)
{
    QStringList args { "-outputdir", m_outputDir->path() + "/" + outputPathPrefix,
                       QFINDTESTDATA(input) };
    if (extraParams)
        args << QString(QLatin1String(extraParams)).split(QChar(' '));

    runQDocProcess(args);

    if (QTest::currentTestFailed())
        return;

    QStringList expectedOuts(QString(QLatin1String(outNames)).split(QChar(' ')));
    if (outputPathPrefix)
        for (auto &expectedOut : expectedOuts)
            expectedOut = QString(outputPathPrefix) + "/" + expectedOut;

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

void tst_generatedOutput::htmlFromQDocFile()
{
    testAndCompare("testdata/configs/test.qdocconf",
                   "qdoctests-qdocfileoutput.html "
                   "qdoctests-qdocfileoutput-linking.html "
                   "qdoctests-qdocmanuallikefileoutput.html "
                   "qdoctests-qdocfileoutput-exhaustive.html "
                   "toc.html");
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

void tst_generatedOutput::htmlFromQml()
{
    testAndCompare("testdata/configs/testqml.qdocconf",
                   "qmlmodules.html "
                   "test-componentset-example.html "
                   "test-cmaketest-example.html "
                   "uicomponents-qmlmodule.html "
                   "qdoc-test-qmlmodule.html "
                   "test-nover-qmlmodule.html "
                   "qml-qdoc-test-abstractparent.html "
                   "qml-qdoc-test-child.html "
                   "qml-qdoc-test-yetanotherchild.html "
                   "qml-qdoc-test-doctest.html "
                   "qml-qdoc-test-type-members.html "
                   "qml-qdoc-test-type-obsolete.html "
                   "qml-qdoc-test-type.html "
                   "qml-qdoc-test-oldtype.html "
                   "qml-test-nover-typenoversion.html "
                   "qml-test-nover-typenoversion-members.html "
                   "qml-uicomponents-progressbar.html "
                   "qml-uicomponents-switch.html "
                   "qml-uicomponents-tabwidget.html "
                   "qml-int.html");
}

void tst_generatedOutput::htmlFromCppBug80259()
{
    testAndCompare("testdata/bug80259/testmodule.qdocconf",
                   "first.html "
                   "second.html "
                   "third.html "
                   "index.html");
}

void tst_generatedOutput::webXmlFromQDocFile()
{
    testAndCompare("testdata/configs/webxml_test.qdocconf",
                   "html/qdoctests-qdocfileoutput.webxml "
                   "html/qdoctests-qdocmanuallikefileoutput.webxml "
                   "html/qdoctests-qdocfileoutput-linking.webxml "
                   "html/qdoctests-qdocfileoutput-exhaustive.webxml");
}

void tst_generatedOutput::webXmlFromCpp()
{
    testAndCompare("testdata/configs/webxml_testcpp.qdocconf",
                   "html/testcpp-module.webxml "
                   "html/testqdoc-test.webxml "
                   "html/testqdoc-testderived.webxml");
}

void tst_generatedOutput::webXmlFromQml()
{
    testAndCompare("testdata/configs/webxml_testqml.qdocconf",
                   "html/test-componentset-example.webxml "
                   "html/test-nover-qmlmodule.webxml "
                   "html/uicomponents-qmlmodule.webxml");
}

void tst_generatedOutput::webXmlFromCppBug80259()
{
    testAndCompare("testdata/bug80259/webxml_testmodule.qdocconf",
                   "html/first.webxml "
                   "html/second.webxml "
                   "html/third.webxml "
                   "html/index.webxml");
}

void tst_generatedOutput::illformated_documentation()
{
    testAndCompare("testdata/illformatted_documentation/illformatted_documentation.qdocconf",
                   "html/illformatted-examples.webxml "
                   "html/illformatteddocumentation-someexample-example.webxml "
                   "html/illformatteddocumentation.index "
                   "page-with-an-image-at-the-top.html "
                   "page-with-comment-after-brief.html "
                   "another-page-with-comments-in-the-brief.html "
                   "page-with-comment-in-brief.html "
                   "brief-adventures.html");
}

void tst_generatedOutput::tableAfterValue()
{
    testAndCompare("testdata/tables/table-after-value.qdocconf",
                   "tableaftervalue/tableaftervalue-members.html "
                   "tableaftervalue/tableaftervalue.html "
                   "tableaftervalue/tableaftervalue.index "
                   "tableaftervalue/tableaftervalue.webxml "
                   "tableaftervalue/tableaftervalue.xml");
}

void tst_generatedOutput::docBookFromQDocFile()
{
    testAndCompare("testdata/configs/docbook_test.qdocconf",
                   "docbook/qdoctests-qdocfileoutput.xml "
                   "docbook/qdoctests-qdocmanuallikefileoutput.xml "
                   "docbook/qdoctests-qdocfileoutput-linking.xml "
                   "docbook/qdoctests-qdocfileoutput-exhaustive.xml");
}

void tst_generatedOutput::docBookFromCpp()
{
    testAndCompare("testdata/configs/docbook_testcpp.qdocconf",
                   "docbook/testcpp-module.xml "
                   "docbook/testqdoc-test.xml "
                   "docbook/testqdoc-testderived.xml "
                   "docbook/cpptypes.xml "
                   "docbook/testqdoc.xml");
}

void tst_generatedOutput::docBookFromQml()
{
    testAndCompare("testdata/configs/docbook_testqml.qdocconf",
                   "docbook/test-componentset-example.xml "
                   "docbook/uicomponents-qmlmodule.xml "
                   "docbook/qdoc-test-qmlmodule.xml "
                   "docbook/test-nover-qmlmodule.xml "
                   "docbook/qml-qdoc-test-abstractparent.xml "
                   "docbook/qml-qdoc-test-child.xml "
                   "docbook/qml-qdoc-test-yetanotherchild.xml "
                   "docbook/qml-qdoc-test-doctest.xml "
                   "docbook/qml-qdoc-test-type.xml "
                   "docbook/qml-qdoc-test-oldtype.xml "
                   "docbook/qml-test-nover-typenoversion.xml "
                   "docbook/qml-uicomponents-progressbar.xml "
                   "docbook/qml-uicomponents-switch.xml "
                   "docbook/qml-uicomponents-tabwidget.xml "
                   "docbook/qml-int.xml");
}

void tst_generatedOutput::docBookWithExtensionsFromQDocFile()
{
    testAndCompare("testdata/configs/docbookext_test.qdocconf",
                   "docbookext/qdoctests-qdocfileoutput.xml "
                   "docbookext/qdoctests-qdocmanuallikefileoutput.xml "
                   "docbookext/qdoctests-qdocfileoutput-linking.xml "
                   "docbookext/qdoctests-qdocfileoutput-exhaustive.xml");
}

void tst_generatedOutput::docBookWithExtensionsFromCpp()
{
    testAndCompare("testdata/configs/docbookext_testcpp.qdocconf",
                   "docbookext/testcpp-module.xml "
                   "docbookext/testqdoc-test.xml "
                   "docbookext/testqdoc-testderived.xml "
                   "docbookext/testqdoc.xml");
}

void tst_generatedOutput::docBookWithExtensionsFromQml()
{
    testAndCompare("testdata/configs/docbookext_testqml.qdocconf",
                   "docbookext/test-componentset-example.xml "
                   "docbookext/uicomponents-qmlmodule.xml "
                   "docbookext/qdoc-test-qmlmodule.xml "
                   "docbookext/test-nover-qmlmodule.xml "
                   "docbookext/qml-qdoc-test-abstractparent.xml "
                   "docbookext/qml-qdoc-test-child.xml "
                   "docbookext/qml-qdoc-test-yetanotherchild.xml "
                   "docbookext/qml-qdoc-test-doctest.xml "
                   "docbookext/qml-qdoc-test-type.xml "
                   "docbookext/qml-test-nover-typenoversion.xml "
                   "docbookext/qml-uicomponents-progressbar.xml "
                   "docbookext/qml-uicomponents-switch.xml "
                   "docbookext/qml-uicomponents-tabwidget.xml "
                   "docbookext/qml-int.xml");
}

void tst_generatedOutput::autoNavigation()
{
    // Same expected files as htmlFromQdocFile, but with auto-generated navigation links
    testAndCompare("testdata/configs/tocnavigation.qdocconf",
                   "qdoctests-qdocfileoutput.html "
                   "qdoctests-qdocfileoutput-linking.html "
                   "qdoctests-qdocfileoutput-exhaustive.html "
                   "toc.html");
}

void tst_generatedOutput::tocBreadcrumbs()
{
    testAndCompare("testdata/configs/tocbreadcrumbs.qdocconf",
                   "tocbreadcrumbs/qdoctests-qdocfileoutput.html "
                   "tocbreadcrumbs/qdoctests-qdocfileoutput-linking.html "
                   "tocbreadcrumbs/qdoctests-qdocfileoutput-exhaustive.html "
                   "tocbreadcrumbs/toc-test.html");
}


void tst_generatedOutput::examplesManifestXmlAndQhp()
{
    testAndCompare("testdata/configs/examples-qhp.qdocconf",
                   "examples-manifest.xml "
                   "test-demos-demo-example.html "
                   "test.qhp");
}

void tst_generatedOutput::ignoresinceVariable()
{
    testAndCompare("testdata/configs/ignoresince.qdocconf",
                   "ignoresince/testqdoc.html "
                   "ignoresince/testqdoc-test.html");
}

void tst_generatedOutput::templateParameters()
{
    testAndCompare("testdata/configs/testtemplate.qdocconf",
                   "template/testqdoc-test.html "
                   "template/testqdoc-test-struct.html "
                   "template/testqdoc-vec.html "
                   "template/foo.html "
                   "template/bar.html "
                   "template/baz.html");
}

void tst_generatedOutput::scopedEnum()
{
    testAndCompare("testdata/configs/scopedenum.qdocconf",
                   "scopedenum/testqdoc-test.html "
                   "scopedenum/scoped-enum-linking.html "
                   "scopedenum/whatsnew.html "
                   "scopedenum-docbook/scoped-enum-linking.xml "
                   "scopedenum-docbook/testqdoc-test.xml");
}

void tst_generatedOutput::dontDocument()
{
    testAndCompare("testdata/dontdocument/dontdocument.qdocconf",
                   "dontdocument/classes.html "
                   "dontdocument/seenclass.html "
                   "dontdocument/dontdocument.qhp");
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
                   "testcpp-module.html "
                   "testqdoc-test.html "
                   "testqdoc-test-members.html "
                   "testqdoc.html "
                   "crossmoduleref.html "
                   "crossmodule/all-namespaces.html "
                   "crossmodule/testtype.html "
                   "crossmodule/testtype-members.html",
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

void tst_generatedOutput::nestedMacro()
{
    testAndCompare("testdata/configs/nestedmacro.qdocconf",
                   "nestedmacro/testcpp-module.html "
                   "docbook-nestedmacro/testcpp-module.xml");
}

void tst_generatedOutput::headerFile()
{
    testAndCompare("testdata/configs/headerfile.qdocconf",
                   "headerfile/testheader.html "
                   "headerfile/headers.html "
                   "headerfile-docbook/testheader.xml "
                   "headerfile-docbook/headers.xml");
}

void tst_generatedOutput::usingDirective()
{
    testAndCompare("testdata/configs/usingdirective.qdocconf", "space.html");
}

void tst_generatedOutput::properties()
{
    if (m_extraParams.isEmpty() && !m_regen) {
        QSKIP("Required include paths not available");
    }

    testAndCompare("testdata/configs/properties.qdocconf",
                   "properties/testqdoc-testderived.html "
                   "properties/testqdoc-testderived-members.html "
                   "properties/qml-thetype.html "
                   "properties/testcpp.index "
                   "properties-docbook/testqdoc-testderived.xml",
                   m_extraParams.toLatin1().data());
}

void tst_generatedOutput::testTagFile()
{
    testAndCompare("testdata/configs/tagfiles.qdocconf", "testtagfile.tags");
}

void tst_generatedOutput::testGlobalFunctions()
{
    testAndCompare("testdata/configs/testglobals.qdocconf", "globals.html");
}

void tst_generatedOutput::proxyPage()
{
    testAndCompare("testdata/proxypage/proxypage.qdocconf",
                   "proxypage/stdpair-proxypage-proxy.html "
                   "proxypage-docbook/stdpair-proxypage-proxy.xml");
}

void tst_generatedOutput::nonAsciiCharacterInput()
{
    testAndCompare(
            "testdata/non_ascii_character_input/non_ascii_character_input.qdocconf",
            "html/nonasciicharacterinput.index "
            "html/mozzarella-7c883eff.webxml "
            "html/santa-14209312.webxml "
            "html/seite-mit-ausschlie-lich-gro-buchstaben-im-titel-berschrift-htm-bfa91582.webxml "
            "html/8b5c72eb.webxml "
            "html/e85685de.webxml "
            "seite-mit-ausschlie-lich-gro-buchstaben-im-titel-berschrift-htm-bfa91582.html "
            "mozzarella-7c883eff.html "
            "santa-14209312.html "
            "8b5c72eb.html "
            "e85685de.html "
            "adventures-with-non-ascii-characters.html");
}

void tst_generatedOutput::lineComments()
{
    testAndCompare(
       "testdata/line_comments/line_comments.qdocconf",

   "linecomments/a-page-with-a-line-comment-in-the-see-also-command.html "
            "linecomments/a-page-with-a-line-comment-in-the-see-also-command.webxml "
            "linecomments/a-page-with-a-line-comment-in-the-see-also-command.xml "
            "linecomments/another-page-with-an-image-at-the-top.html "
            "linecomments/another-page-with-an-image-at-the-top.webxml "
            "linecomments/another-page-with-an-image-at-the-top.xml "
            "linecomments/line-comment-adventures.html "
            "linecomments/line-comment-adventures.webxml "
            "linecomments/line-comment-adventures.xml"
    );
}

void tst_generatedOutput::modulestateCommand()
{
    testAndCompare("testdata/modulestate/modulestate.qdocconf",
                   "modulestate/boringclass.html "
                   "modulestate/boringclass.webxml "
                   "modulestate/boringclass.xml "
                   "modulestate/excitingclass.html "
                   "modulestate/excitingclass.webxml "
                   "modulestate/excitingclass.xml "
                   "modulestate/moduleinstate-module.html "
                   "modulestate/moduleinstate-module.webxml "
                   "modulestate/moduleinstate-module.xml "
                   "modulestate/modulestate.index");
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
