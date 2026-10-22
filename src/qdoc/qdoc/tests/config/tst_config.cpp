// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdoc/config.h"
#include "qdoc/outputconfiguration.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtTest/QtTest>

using namespace Qt::StringLiterals;

namespace {

PipelineCapabilities capabilitiesFor(Pipeline pipeline, const QStringList &formats)
{
    PipelineCapabilities capabilities;
    capabilities.pipeline = pipeline;
    const QHash<QString, QString> defaults{
        {u"HTML"_s, u"html"_s}, {u"Markdown"_s, u"md"_s}, {u"DocBook"_s, u"xml"_s}
    };
    for (const auto &format : formats) {
        capabilities.supportedFormats.insert(format);
        capabilities.formatExtensions.insert(format, defaults.value(format, u"out"_s));
    }
    return capabilities;
}

} // namespace

class tst_Config : public QObject
{
    Q_OBJECT

private slots:
    void classMembersInitializeToFalseOrEmpty();
    void includePathsFromCommandLine();
    void variables();
    void paths();
    void includepaths();
    void getExampleProjectFile();
    void expandVars();
    void sourceLink();
    void showInternalPrecedence();
    void parseCppCommentsPrecedence();
    void internalFilePatterns();
    void internalFilePatternsMatching();
    void internalFilePatternsPathMatching();
    void internalFilePatternsWildcards();
    void selectionModes();
    void selectionRequests();
    void selectionOwnership();
    void selectionExtensions();
    void selectionNotices();

private:
    Config &initConfig(const QStringList &args = QStringList(),
                       const char *qdocconf = nullptr);
    Config &initConfig(const char *qdocconf)
    {
        return initConfig(QStringList(), qdocconf);
    }
};

/*
  Initializes the Config with optional arguments and a .qdocconf file
  to load, and  returns a reference to it.

  Resets persistent command-line overrides so test cases remain independent.
*/
Config &tst_Config::initConfig(const QStringList &args, const char *qdocconf)
{
    Config::overrideOutputDir = QString();
    Config::overrideOutputFormats.clear();
    QStringList fullArgs = { QStringLiteral("./qdoc") };
    fullArgs << args;
    Config::instance().init("QDoc Test", fullArgs);

    if (qdocconf) {
        const auto configFile = QFINDTESTDATA(qdocconf);
        if (!configFile.isEmpty())
            Config::instance().load(configFile);
    }

    return Config::instance();
}

void tst_Config::classMembersInitializeToFalseOrEmpty()
{
    auto &config = initConfig();
    QCOMPARE(config.showInternal(), false);
    QCOMPARE(config.singleExec(), false);

    QVERIFY(config.defines().isEmpty());
    QVERIFY(config.includePaths().isEmpty());
    QVERIFY(config.dependModules().isEmpty());
    QVERIFY(config.indexDirs().isEmpty());
    QVERIFY(config.currentDir().isEmpty());
    QVERIFY(config.previousCurrentDir().isEmpty());
}

void tst_Config::includePathsFromCommandLine()
{
    const auto mockIncludePath1 = QString("-I" + QDir().absoluteFilePath("/qt5/qtdoc/doc/."));
    const auto mockIncludePath2 = QString("-I" + QDir().absoluteFilePath("/qt5/qtbase/mkspecs/linux-g++"));
    const QStringList commandLineArgs = { mockIncludePath1, mockIncludePath2 };
    auto &config = initConfig(commandLineArgs);

    const QStringList expected = { mockIncludePath1, mockIncludePath2 };
    const QStringList actual = config.includePaths();

    QCOMPARE(actual, expected);
}

// Tests different types of variables; string, string list, bool, int,
// empty and undefined variables, and subvariables.
void tst_Config::variables()
{
    auto &config = initConfig("/testdata/configs/vars.qdocconf");

    const QStringList list = { "testing", "line", "by\n", "line" };
    QCOMPARE(config.get("list").asStringList(), list);
    QCOMPARE(config.get("list").asString(), "testing line by\nline");
    QCOMPARE(config.get("true").asBool(), true);
    QCOMPARE(config.get("untrue").asBool(), false);
    QCOMPARE(config.get("int").asInt(), 2);
    QCOMPARE(config.get("void").asString(), QString());
    QVERIFY(!config.get("void").asString().isNull());
    QCOMPARE(config.get("void").asString("undefined"), QString());
    QCOMPARE(config.get("undefined").asString("undefined"), "undefined");
    QVERIFY(config.get("undefined").asString().isNull());

    QSet<QString> subVars = { "thing", "where", "time" };
    QCOMPARE(config.subVars("some"), subVars);
}

// Tests whether paths or variables are resolved correctly.
void tst_Config::paths()
{
    auto &config = initConfig();
    const auto docConfig = QFINDTESTDATA("/testdata/configs/paths.qdocconf");
    if (!docConfig.isEmpty())
        config.load(docConfig);

    auto rootDir = QFileInfo(docConfig).dir();
    QVERIFY(rootDir.cdUp());

    const auto paths = config.getCanonicalPathList("sourcedirs");
    QVERIFY(paths.size() == 3);

    QCOMPARE(paths[0], rootDir.absoluteFilePath("paths/includes"));
    QCOMPARE(paths[1], rootDir.absoluteFilePath("configs"));
    QCOMPARE(paths[2], rootDir.absoluteFilePath("configs/includes"));
}

// Tests whether includepaths are resolved correctly
void tst_Config::includepaths()
{
    auto &config = initConfig();
    const auto docConfig = QFINDTESTDATA("/testdata/configs/includepaths.qdocconf");
    if (!docConfig.isEmpty())
        config.load(docConfig);

    auto rootDir = QFileInfo(docConfig).dir();
    QVERIFY(rootDir.cdUp());

    const auto paths = config.getCanonicalPathList("includepaths",
                                                   Config::IncludePaths);
    QVERIFY(paths.size() == 5);

    QCOMPARE(paths[0], "-I" + rootDir.absoluteFilePath("includepaths/include"));
    QCOMPARE(paths[0], paths[1]);
    QCOMPARE(paths[2], "-I" + rootDir.absoluteFilePath("includepaths/include/more"));
    QCOMPARE(paths[3], "-F" + rootDir.absoluteFilePath("includepaths/include/framework"));
    QCOMPARE(paths[4], "-isystem" + rootDir.absoluteFilePath("includepaths/include/system"));
}

void::tst_Config::getExampleProjectFile()
{
    auto &config = initConfig();
    const auto docConfig = QFINDTESTDATA("/testdata/configs/exampletest.qdocconf");
    if (!docConfig.isEmpty())
        config.load(docConfig);

    auto rootDir = QFileInfo(docConfig).dir();
    QVERIFY(rootDir.cd("../exampletest/examples/test"));

    QVERIFY(config.getExampleProjectFile("invalid").isEmpty());
    QVERIFY(config.getExampleProjectFile("test/empty").isEmpty());

    QCOMPARE(config.getExampleProjectFile("test/example1"),
             rootDir.absoluteFilePath("example1/example1.pro"));
    QCOMPARE(config.getExampleProjectFile("test/example2"),
             rootDir.absoluteFilePath("example2/example2.qmlproject"));
    QCOMPARE(config.getExampleProjectFile("test/example3"),
             rootDir.absoluteFilePath("example3/example3.pyproject"));
    QCOMPARE(config.getExampleProjectFile("test/example4"),
             rootDir.absoluteFilePath("example4/CMakeLists.txt"));
}

void::tst_Config::expandVars()
{
    qputenv("QDOC_TSTCONFIG_LIST", QByteArray("a b c"));
    auto &config = initConfig("/testdata/configs/expandvars.qdocconf");

    QCOMPARE(config.get("expanded1").asString(), "foo");
    QCOMPARE(config.get("expanded2").asString(), "foo,bar");
    QCOMPARE(config.get("expanded3").asString(), "foobar foobar baz");
    QCOMPARE(config.get("literally").asString(), "$data ${data}");
    QCOMPARE(config.get("csvlist").asString(), "a,b,c");
}

void::tst_Config::sourceLink()
{
    auto &config = initConfig();
    const auto docConfig = QFINDTESTDATA("/testdata/configs/sourcelink.qdocconf");
    if (!docConfig.isEmpty())
        config.load(docConfig);

    auto dir = QFileInfo(docConfig).dir();
    QCOMPARE(config.getSourceLink().rootPath, dir.absolutePath());
    QCOMPARE(config.getSourceLink().baseUrl, "http://localhost/");
    QCOMPARE(config.getSourceLink().linkText, "link");
    QCOMPARE(config.getSourceLink().enabled, false);
}

void::tst_Config::showInternalPrecedence()
{
    // Test 1: Config file sets showinternal=false, no command line option
    {
        auto &config = initConfig();
        const auto docConfig = QFINDTESTDATA("/testdata/configs/showinternal_false.qdocconf");
        if (!docConfig.isEmpty())
            config.load(docConfig);

        QCOMPARE(config.showInternal(), false);
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), false);
    }

    // Test 2: Config file sets showinternal=true, no command line option
    {
        auto &config = initConfig();
        const auto docConfig = QFINDTESTDATA("/testdata/configs/showinternal_true.qdocconf");
        if (!docConfig.isEmpty())
            config.load(docConfig);

        QCOMPARE(config.showInternal(), true);
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), true);
    }

    // Test 3: Config file sets showinternal=false, command line has -showinternal
    {
        const QStringList args = { "-showinternal" };
        auto &config = initConfig(args);
        const auto docConfig = QFINDTESTDATA("/testdata/configs/showinternal_false.qdocconf");
        if (!docConfig.isEmpty())
            config.load(docConfig);

        // Command line should override config file
        QCOMPARE(config.showInternal(), true);
        // Config variable should still reflect what's in the file
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), false);
    }

    // Test 4: Config file sets showinternal=true, command line has -showinternal
    {
        const QStringList args = { "-showinternal" };
        auto &config = initConfig(args);
        const auto docConfig = QFINDTESTDATA("/testdata/configs/showinternal_true.qdocconf");
        if (!docConfig.isEmpty())
            config.load(docConfig);

        // Both should be true
        QCOMPARE(config.showInternal(), true);
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), true);
    }

    // Test 5: No config file, command line has -showinternal
    {
        const QStringList args = { "-showinternal" };
        auto &config = initConfig(args);

        QCOMPARE(config.showInternal(), true);
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), true);
    }
}

void tst_Config::parseCppCommentsPrecedence()
{
    // Test 1: Neither parsecppcomments nor moduleheader set; C++ parsing enabled by default
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_default.qdocconf");
        QVERIFY(config.parseCppComments);
    }

    // Test 2: moduleheader set to empty; C++ parsing disabled (legacy behavior)
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_moduleheader_empty.qdocconf");
        QVERIFY(!config.parseCppComments);
    }

    // Test 3: moduleheader set to a non-empty value; C++ parsing enabled
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_moduleheader_nonempty.qdocconf");
        QVERIFY(config.parseCppComments);
    }

    // Test 4: parsecppcomments explicitly set to false; C++ parsing disabled
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_false.qdocconf");
        QVERIFY(!config.parseCppComments);
    }

    // Test 5: parsecppcomments explicitly set to true; C++ parsing enabled
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_true.qdocconf");
        QVERIFY(config.parseCppComments);
    }

    // Test 6: parsecppcomments=true takes precedence over empty moduleheader
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_true_moduleheader_empty.qdocconf");
        QVERIFY(config.parseCppComments);
    }

    // Test 7: parsecppcomments=false takes precedence over non-empty moduleheader
    {
        auto &config = initConfig("/testdata/configs/parsecppcomments_false_moduleheader_nonempty.qdocconf");
        QVERIFY(!config.parseCppComments);
    }
}

void tst_Config::internalFilePatterns()
{
    auto &config = initConfig("/testdata/configs/internalpatterns.qdocconf");

    const QStringList expected = {"*_p.h", "*_p_p.h"};
    const QSet<QString> expectedSet(expected.cbegin(), expected.cend());

    QCOMPARE(config.getInternalFilePatterns(), expectedSet);
}

void tst_Config::internalFilePatternsMatching()
{
    auto &config = initConfig("/testdata/configs/internalpatterns.qdocconf");
    const Config::InternalFilePatterns &patterns = config.getInternalFilePatternsCompiled();

    // Should match - testing path/filename matching
    QVERIFY(Config::matchesInternalFilePattern("myclass_p.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("foo_p_p.h", patterns));

    // Should not match
    QVERIFY(!Config::matchesInternalFilePattern("myclass.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("public.h", patterns));
}

void tst_Config::internalFilePatternsPathMatching()
{
    // Create a config with path-based patterns
    initConfig();
    Config::InternalFilePatterns patterns;

    // Add a pattern that includes directory structure using regex
    // Regex patterns match against the full path
    QRegularExpression re(R"(.*/internal/.*\.h)");
    re.optimize();
    patterns.regexPatterns.append(re);

    // Should match - patterns with 'internal' directory followed by .h file
    QVERIFY(Config::matchesInternalFilePattern("foo/internal/widget.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("src/internal/helper.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("/absolute/path/internal/file.h", patterns));

    // Should match - Windows-style backslash paths (testing normalization)
    // First verify that forward slash version works
    QVERIFY(Config::matchesInternalFilePattern("src/internal/helper.h", patterns));

    // Now test backslash versions
    QVERIFY(Config::matchesInternalFilePattern(R"(src\internal\helper.h)", patterns));
    QVERIFY(Config::matchesInternalFilePattern(R"(C:\project\internal\widget.h)", patterns));
    QVERIFY(Config::matchesInternalFilePattern(R"(foo\bar\internal\file.h)", patterns));

    // Should not match - 'internal' not in path or wrong position
    QVERIFY(!Config::matchesInternalFilePattern("foo/public/widget.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("internal.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("internal/widget.cpp", patterns)); // .cpp not .h
    QVERIFY(!Config::matchesInternalFilePattern("foo/internal_dir/widget.h", patterns)); // not exact 'internal'
    QVERIFY(!Config::matchesInternalFilePattern(R"(src\public\file.h)", patterns)); // Windows path, but 'public' not 'internal'
}

void tst_Config::internalFilePatternsWildcards()
{
    initConfig();
    Config::InternalFilePatterns patterns;

    // Test single character wildcard '?'
    QRegularExpression re1(QRegularExpression::wildcardToRegularExpression("test?.h"));
    re1.optimize();
    patterns.globPatterns.append(re1);

    // Test multi-pattern matching
    QRegularExpression re2(QRegularExpression::wildcardToRegularExpression("*_impl.h"));
    re2.optimize();
    patterns.globPatterns.append(re2);

    // Add exact match for comparison
    patterns.exactMatches.insert("exact.h");

    // Test '?' wildcard - matches single character (glob patterns match filename only)
    QVERIFY(Config::matchesInternalFilePattern("test1.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("testX.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("/path/to/test1.h", patterns)); // glob matches filename
    QVERIFY(Config::matchesInternalFilePattern("/path/to/testX.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("test.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("test12.h", patterns));

    // Test '*' wildcard - matches multiple characters (glob patterns match filename only)
    QVERIFY(Config::matchesInternalFilePattern("widget_impl.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("foo_impl.h", patterns));
    QVERIFY(Config::matchesInternalFilePattern("/some/path/widget_impl.h", patterns)); // glob matches filename
    QVERIFY(!Config::matchesInternalFilePattern("widget.h", patterns));

    // Test exact match (exact matches check full path)
    QVERIFY(Config::matchesInternalFilePattern("exact.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("exact_x.h", patterns));
    QVERIFY(!Config::matchesInternalFilePattern("/path/exact.h", patterns)); // exact match requires full path match
}

void tst_Config::selectionModes()
{
    auto &config = initConfig();
    OutputConfigurationInput input;
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s})};
    const auto compatibility = OutputSelection::resolve(config, input);
    QCOMPARE(compatibility.mode, OutputSelection::Mode::Compatibility);
    QVERIFY(compatibility.formats.isEmpty());
    QVERIFY(compatibility.pipelines.isEmpty());
    QVERIFY(compatibility.diagnostics.isEmpty());

    input.cliFormats = {u"HTML"_s};
    QCOMPARE(OutputSelection::resolve(config, input).mode, OutputSelection::Mode::Compatibility);
    input.cliPipelines = {u"legacy"_s};
    const auto optedIn = OutputSelection::resolve(config, input);
    QCOMPARE(optedIn.mode, OutputSelection::Mode::New);
    QVERIFY(!optedIn.hasErrors());
    QCOMPARE(optedIn.pipelines, QVector<Pipeline>{Pipeline::Legacy});

    config.setStringList(CONFIG_PIPELINE, {u"template"_s});
    QVERIFY(!OutputSelection::resolve(config, input).hasErrors());
    input.cliPipelines.clear();
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    config.setStringList(CONFIG_PIPELINE, {});
    QVERIFY(config.contains(CONFIG_PIPELINE));
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s, u"legacy"_s});
    QCOMPARE(OutputSelection::resolve(config, input).pipelines.size(), 1);
    input.cliPipelines = {u""_s};
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    input.cliPipelines = {u"unknown"_s};
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    input.cliPipelines = {u"legacy"_s};
    input.pipelineCapabilities.first().available = false;
    const auto unavailable = OutputSelection::resolve(config, input);
    QVERIFY(unavailable.hasErrors());
    QVERIFY(unavailable.pipelines.isEmpty());
}

void tst_Config::selectionRequests()
{
    auto &config = initConfig();
    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s, u"HTML"_s});
    OutputConfigurationInput input;
    input.pipelineCapabilities = {
        capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s, u"DocBook"_s})
    };
    const auto configured = OutputSelection::resolve(config, input);
    QVERIFY(!configured.hasErrors());
    QCOMPARE(configured.formats.size(), 2);
    QCOMPARE(configured.formats.at(0).format, u"HTML"_s);
    QCOMPARE(configured.formats.at(1).format, u"Markdown"_s);

    input.cliFormats = {u"DocBook"_s, u"Markdown"_s, u"DocBook"_s};
    const auto replaced = OutputSelection::resolve(config, input);
    QVERIFY(!replaced.hasErrors());
    QCOMPARE(replaced.formats.size(), 3);
    QCOMPARE(replaced.formats.at(0).format, u"HTML"_s);
    QCOMPARE(replaced.formats.at(1).format, u"Markdown"_s);
    QCOMPARE(replaced.formats.at(2).format, u"DocBook"_s);
    QVERIFY(!replaced.formats.at(0).requested);
    QVERIFY(replaced.formats.at(1).requested);
    QVERIFY(replaced.formats.at(2).requested);
    QVERIFY(replaced.formats.at(0).configured);
    QVERIFY(!replaced.formats.at(2).configured);
    QVERIFY(!replaced.findFormat(u"html"_s));

    input.cliFormats = {u""_s};
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    config.setStringList(CONFIG_OUTPUTFORMATS, {});
    input.cliFormats.clear();
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    input.cliFormats = {u"DocBook"_s};
    QVERIFY(!OutputSelection::resolve(config, input).hasErrors());
    input.cliFormats = {u"bad.format"_s};
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
}

void tst_Config::selectionOwnership()
{
    auto &config = initConfig();
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"DocBook"_s});
    OutputConfigurationInput input;
    auto legacy = capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"DocBook"_s});
    auto templated = capabilitiesFor(Pipeline::Template, {u"HTML"_s, u"DocBook"_s});
    for (const QStringList &order : {QStringList{u"legacy"_s, u"template"_s},
                                    QStringList{u"template"_s, u"legacy"_s}}) {
        config.setStringList(CONFIG_PIPELINE, order);
        input.pipelineCapabilities = {legacy, templated};
        const auto owned = OutputSelection::resolve(config, input);
        QVERIFY(!owned.hasErrors());
        QCOMPARE(owned.findFormat(u"HTML"_s)->pipeline, Pipeline::Template);
        QCOMPARE(owned.findFormat(u"DocBook"_s)->pipeline, Pipeline::Template);

        auto &templateInput = input.pipelineCapabilities.last();
        templateInput.brokenFormats.insert(u"DocBook"_s);
        QVERIFY(OutputSelection::resolve(config, input).hasErrors());
        templateInput.supportedFormats.remove(u"DocBook"_s);
        const auto broken = OutputSelection::resolve(config, input);
        QVERIFY(broken.hasErrors());
        QCOMPARE(broken.findFormat(u"DocBook"_s)->pipeline, Pipeline::Template);
        QCOMPARE(broken.diagnostics.size(), 1);
        QVERIFY(broken.diagnostics.first().message.contains("broken"_L1));
        templateInput.brokenFormats.clear();
        const auto fallback = OutputSelection::resolve(config, input);
        QVERIFY(!fallback.hasErrors());
        QCOMPARE(fallback.findFormat(u"DocBook"_s)->pipeline, Pipeline::Legacy);
    }

    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    legacy.supportedFormats.remove(u"DocBook"_s);
    legacy.brokenFormats.insert(u"DocBook"_s);
    input.pipelineCapabilities = {legacy};
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    legacy.brokenFormats.clear();
    input.pipelineCapabilities = {legacy};
    const auto unsupported = OutputSelection::resolve(config, input);
    QVERIFY(unsupported.hasErrors());
    QVERIFY(unsupported.diagnostics.first().message.contains("not supported"_L1));
}

void tst_Config::selectionExtensions()
{
    auto &config = initConfig("/testdata/configs/selection_include.qdocconf");
    OutputConfigurationInput input;
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Template, {u"HTML"_s})};
    const auto inherited = OutputSelection::resolve(config, input);
    QVERIFY(!inherited.hasErrors());
    QCOMPARE(inherited.formats.first().fileExtension, u"xhtml"_s);
    QCOMPARE(inherited.formats.first().pipeline, Pipeline::Template);
    QCOMPARE(inherited.formats.first().location.filePath(),
             QDir::cleanPath(QFINDTESTDATA("/testdata/configs/selection_base.qdocconf")));
    QCOMPARE(inherited.formats.first().location.lineNo(), 2);

    config.setStringList(u"HTML.extension"_s, {});
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    config.setStringList(u"HTML.extension"_s, {u"html"_s, u"xhtml"_s});
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    config.setStringList(u"HTML.extension"_s, {u"html"_s});
    QCOMPARE(OutputSelection::resolve(config, input).formats.first().fileExtension, u"html"_s);

    input.cliFormats = {u"Custom"_s};
    auto &capability = input.pipelineCapabilities.first();
    capability.supportedFormats.insert(u"Custom"_s);
    QVERIFY(OutputSelection::resolve(config, input).hasErrors());
    capability.formatExtensions.insert(u"Custom"_s, u"xml"_s);
    const auto custom = OutputSelection::resolve(config, input);
    QVERIFY(!custom.hasErrors());
    QCOMPARE(custom.findFormat(u"Custom"_s)->fileExtension, u"xml"_s);
}

void tst_Config::selectionNotices()
{
    auto &config = initConfig();
    config.setStringList(CONFIG_PUBLICATIONLAYOUT, {u"flat"_s});
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {u"package"_s});
    config.setStringList(u"DocBook.publicationlayout"_s, {u"per-module"_s});
    const auto result = OutputSelection::resolve(config);
    QCOMPARE(result.mode, OutputSelection::Mode::Compatibility);
    QVERIFY(!result.hasErrors());
    QCOMPARE(result.diagnostics.size(), 3);
    for (const auto &diagnostic : result.diagnostics) {
        QCOMPARE(diagnostic.kind, OutputSelection::Diagnostic::Kind::Notice);
        QVERIFY(diagnostic.message.contains("pipeline"_L1));
    }
}

QTEST_APPLESS_MAIN(tst_Config)

#include "tst_config.moc"
