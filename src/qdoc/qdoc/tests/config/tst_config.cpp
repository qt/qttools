// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdoc/config.h"
#include "qdoc/outputconfiguration.h"
#include "qdoc/templatepackage.h"

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

QString builtinRoot()
{
    return QDir::cleanPath(QStringLiteral(QDOC_TEMPLATE_DIR));
}

TemplatePackageInput packageInput()
{
    TemplatePackageInput input;
    input.builtinRoot = builtinRoot();
    return input;
}

QString packageDir(const QString &name)
{
    return QDir::cleanPath(
            QFileInfo(QFINDTESTDATA(u"/testdata/packages/"_s + name)).absoluteFilePath());
}

QString messages(const TemplatePackage &package)
{
    QStringList lines;
    for (const auto &diagnostic : package.diagnostics)
        lines.append(diagnostic.location.toString() + u": "_s + diagnostic.message);
    return lines.join(u'\n');
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
    void publicationLayout_data();
    void publicationLayout();
    void publicationDestinations();
    void destinationConflicts_data();
    void destinationConflicts();
    void publicationProvenance();
    void moduleIdentity_data();
    void moduleIdentity();
    void templatePackageBuiltin();
    void templatePackageInheritance();
    void templatePackageProvenance();
    void templatePackageSelection();
    void templatePackageErrors_data();
    void templatePackageErrors();
    void templatePackageBrokenFormat();

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

    qputenv("QDOC_SHOW_INTERNAL", "1");
    const auto unsetShowInternal = qScopeGuard([] { qunsetenv("QDOC_SHOW_INTERNAL"); });

    // Test 6: No config file, QDOC_SHOW_INTERNAL is set
    {
        auto &config = initConfig();

        QCOMPARE(config.showInternal(), true);
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), true);
    }

    // Test 7: Config file sets showinternal=false, QDOC_SHOW_INTERNAL is set
    {
        auto &config = initConfig("/testdata/configs/showinternal_false.qdocconf");

        // The environment variable should override the config file
        QCOMPARE(config.showInternal(), true);
        QCOMPARE(config.get(CONFIG_SHOWINTERNAL).asBool(), false);
    }

    // Test 8: Config file sets showinternal=true, QDOC_SHOW_INTERNAL is set
    {
        auto &config = initConfig("/testdata/configs/showinternal_true.qdocconf");

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

void tst_Config::publicationLayout_data()
{
    QTest::addColumn<QString>("shared");
    QTest::addColumn<QString>("perFormat");
    QTest::addColumn<QString>("alias");
    QTest::addColumn<bool>("perModule");
    QTest::addColumn<int>("notices");
    QTest::addColumn<bool>("error");
    const QString absent;
    QTest::newRow("default-flat") << absent << absent << absent << false << 0 << false;
    QTest::newRow("shared") << u"per-module"_s << absent << absent << true << 0 << false;
    QTest::newRow("per-format-wins") << u"per-module"_s << u"flat"_s << absent << false << 0 << false;
    QTest::newRow("shared-beats-alias") << u"flat"_s << absent << u"false"_s << false << 0 << false;
    QTest::newRow("per-format-beats-alias") << absent << u"flat"_s << u"false"_s << false << 0 << false;
    QTest::newRow("alias-false") << absent << absent << u"false"_s << true << 1 << false;
    QTest::newRow("alias-true") << absent << absent << u"true"_s << false << 1 << false;
    QTest::newRow("alias-empty") << absent << absent << u""_s << true << 1 << false;
    QTest::newRow("invalid-shared") << u"sideways"_s << absent << absent << false << 0 << true;
    QTest::newRow("empty-shared-no-fallback") << u""_s << absent << u"false"_s << false << 0 << true;
    QTest::newRow("invalid-per-format") << u"per-module"_s << u"sideways"_s << absent << false << 0 << true;
    QTest::newRow("empty-per-format") << u"per-module"_s << u""_s << u"false"_s << false << 0 << true;
}

void tst_Config::publicationLayout()
{
    QFETCH(QString, shared);
    QFETCH(QString, perFormat);
    QFETCH(QString, alias);
    QFETCH(bool, perModule);
    QFETCH(int, notices);
    QFETCH(bool, error);
    auto &config = initConfig();
    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s});
    config.setStringList(CONFIG_PROJECT, {u"SampleProject"_s});
    config.setStringList(CONFIG_OUTPUTDIR, {u"docs"_s});
    if (!shared.isNull())
        config.setStringList(CONFIG_PUBLICATIONLAYOUT, {shared});
    if (!perFormat.isNull())
        config.setStringList(u"HTML.publicationlayout"_s, {perFormat});
    if (!alias.isNull())
        config.setStringList(u"HTML.nosubdirs"_s, {alias});
    OutputConfigurationInput input;
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s})};
    const auto result = OutputConfiguration::resolve(config, input);
    QCOMPARE(result.hasErrors(), error);
    QCOMPARE(result.formats.first().layout,
             perModule ? PublicationLayout::PerModule : PublicationLayout::Flat);
    QCOMPARE(result.formats.first().moduleDir, perModule ? u"sampleproject"_s : QString());
    int actualNotices = 0;
    int errors = 0;
    for (const auto &diagnostic : result.diagnostics) {
        if (diagnostic.kind == OutputConfiguration::Diagnostic::Kind::Notice)
            ++actualNotices;
        else
            ++errors;
    }
    QCOMPARE(actualNotices, notices);
    QCOMPARE(errors, error ? 1 : 0);
}

void tst_Config::publicationDestinations()
{
    auto &config = initConfig();
    const auto compatibility = OutputConfiguration::resolve(config);
    QCOMPARE(compatibility.mode, OutputConfiguration::Mode::Compatibility);
    QVERIFY(compatibility.root.isEmpty());
    QVERIFY(compatibility.formats.isEmpty());
    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s, u"DocBook"_s});
    config.setStringList(CONFIG_OUTPUTDIR, {u"docs"_s});
    config.setStringList(CONFIG_PROJECT, {u"SampleProject"_s});
    OutputConfigurationInput input;
    input.pipelineCapabilities = {
        capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s, u"DocBook"_s})
    };
    const auto defaults = OutputConfiguration::resolve(config, input);
    QVERIFY(!defaults.hasErrors());
    QCOMPARE(defaults.destination(defaults.formats.at(0)), u"docs"_s);
    QCOMPARE(defaults.destination(defaults.formats.at(1)), u"docs/markdown"_s);
    QCOMPARE(defaults.destination(defaults.formats.at(2)), u"docs/docbook"_s);
    config.setStringList(u"HTML.outputsubdir"_s, {u"html"_s});
    input.cliFormats = {u"Markdown"_s};
    const auto filtered = OutputConfiguration::resolve(config, input);
    QVERIFY(!filtered.hasErrors());
    QVERIFY(!filtered.findFormat(u"HTML"_s)->requested);
    QVERIFY(!filtered.findFormat(u"DocBook"_s)->requested);
    QVERIFY(filtered.findFormat(u"Markdown"_s)->requested);
    QCOMPARE(filtered.destination(filtered.formats.at(0)), u"docs/html"_s);
    QCOMPARE(filtered.destination(filtered.formats.at(1)), u"docs/markdown"_s);

    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s});
    input.cliFormats = {u"DocBook"_s};
    const auto cliOnly = OutputConfiguration::resolve(config, input);
    QVERIFY(!cliOnly.hasErrors());
    QVERIFY(!cliOnly.findFormat(u"HTML"_s)->requested);
    QCOMPARE(cliOnly.destination(*cliOnly.findFormat(u"DocBook"_s)), u"docs/docbook"_s);
    config.setStringList(u"DocBook.outputsubdir"_s, {u"db"_s});
    const auto overridden = OutputConfiguration::resolve(config, input);
    QCOMPARE(overridden.destination(*overridden.findFormat(u"DocBook"_s)), u"docs/db"_s);

    config.setStringList(CONFIG_OUTPUTFORMATS, {u"Markdown"_s, u"HTML"_s});
    input.cliFormats.clear();
    const auto reversed = OutputConfiguration::resolve(config, input);
    QVERIFY(!reversed.hasErrors());
    QCOMPARE(reversed.destination(reversed.formats.first()), u"docs"_s);
    config.setStringList(u"HTML.outputsubdir"_s, {u"../web"_s});
    config.setStringList(u"HTML.nosubdirs"_s, {u"false"_s});
    const auto literal = OutputConfiguration::resolve(config, input);
    QVERIFY(!literal.hasErrors());
    QCOMPARE(literal.destination(*literal.findFormat(u"HTML"_s)), u"web/sampleproject"_s);
    config.setStringList(CONFIG_SINGLEEXEC, {u"true"_s});
    const auto singleExec = OutputConfiguration::resolve(config, input);
    for (const auto &plan : singleExec.formats)
        QCOMPARE(singleExec.destination(plan), literal.destination(*literal.findFormat(plan.format)));
    config.setStringList(u"HTML.outputsubdir"_s, {u" web "_s});
    const auto spaces = OutputConfiguration::resolve(config, input);
    QVERIFY(!spaces.hasErrors());
    QCOMPARE(spaces.findFormat(u"HTML"_s)->formatDir, u" web "_s);
    QCOMPARE(spaces.destination(*spaces.findFormat(u"HTML"_s)), u"docs/ web /sampleproject"_s);
    config.setStringList(u"HTML.outputsubdir"_s, {u"one"_s, u"two"_s});
    QVERIFY(OutputConfiguration::resolve(config, input).hasErrors());

    const QString cliRoot = QDir::tempPath() + u"/qdoc-publication-root"_s;
    auto &cliConfig = initConfig({u"-outputdir"_s, cliRoot});
    cliConfig.setStringList(CONFIG_OUTPUTDIR, {u"ignored"_s});
    cliConfig.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    cliConfig.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s});
    const auto relocated = OutputConfiguration::resolve(cliConfig, input);
    QVERIFY(!relocated.hasErrors());
    QCOMPARE(relocated.root, cliRoot);
    QCOMPARE(relocated.destination(relocated.formats.first()), cliRoot);
    QCOMPARE(relocated.destination(relocated.formats.last()), cliRoot + u"/markdown"_s);
    cliConfig.setStringList(CONFIG_PROJECT, {u"SampleProject"_s});
    cliConfig.setStringList(CONFIG_PUBLICATIONLAYOUT, {u"per-module"_s});
    const auto relocatedModules = OutputConfiguration::resolve(cliConfig, input);
    QVERIFY(!relocatedModules.hasErrors());
    QCOMPARE(relocatedModules.destination(relocatedModules.formats.last()),
             cliRoot + u"/markdown/sampleproject"_s);
}

void tst_Config::destinationConflicts_data()
{
    QTest::addColumn<QString>("root");
    QTest::addColumn<QString>("subdir");
    QTest::newRow("dot-root") << u"docs/./"_s << u"."_s;
    QTest::newRow("trailing-root") << u"docs/"_s << u""_s;
    QTest::newRow("parent-root") << u"docs/child/.."_s << u""_s;
    QTest::newRow("parent-subdir") << u"docs"_s << u"../docs"_s;
    QTest::newRow("empty-root") << u""_s << u"."_s;
    QTest::newRow("empty-subdirs") << u"docs"_s << u""_s;
}

void tst_Config::destinationConflicts()
{
    QFETCH(QString, root);
    QFETCH(QString, subdir);
    auto &config = initConfig();
    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s});
    config.setStringList(CONFIG_OUTPUTDIR, {root});
    config.setStringList(u"Markdown.outputsubdir"_s, {subdir});
    OutputConfigurationInput input;
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s})};
    const auto result = OutputConfiguration::resolve(config, input);
    QVERIFY(result.hasErrors());
    QCOMPARE(result.diagnostics.size(), 1);
    const QString expected = root.isEmpty() ? u"."_s : QDir::cleanPath(root);
    QCOMPARE(result.destination(result.formats.first()), expected);
    QCOMPARE(result.destination(result.formats.last()), expected);
    input.cliFormats = {u"Markdown"_s};
    QVERIFY(!OutputConfiguration::resolve(config, input).hasErrors());
}

void tst_Config::publicationProvenance()
{
    OutputConfigurationInput input;
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s})};
    auto &config = initConfig("/testdata/configs/publication_include.qdocconf");
    const auto result = OutputConfiguration::resolve(config, input);
    QVERIFY(!result.hasErrors());
    QVERIFY(result.diagnostics.isEmpty());
    const auto &html = result.formats.first();
    QCOMPARE(html.layout, PublicationLayout::Flat);
    QCOMPARE(html.layoutLocation.filePath(),
             QFINDTESTDATA("/testdata/configs/publication_include.qdocconf"));
    QCOMPARE(html.layoutLocation.lineNo(), 2);
    QCOMPARE(html.formatDirLocation.filePath(),
             QDir::cleanPath(QFINDTESTDATA("/testdata/configs/publication_base.qdocconf")));
    QCOMPARE(html.formatDirLocation.lineNo(), 6);
    QCOMPARE(result.destination(html), u"web"_s);
    QCOMPARE(result.destination(result.formats.last()), u"docs/markdown/sampleproject"_s);

    auto &collision = initConfig("/testdata/configs/destination_conflict.qdocconf");
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Foo"_s, u"foo"_s})};
    const auto invalid = OutputConfiguration::resolve(collision, input);
    QVERIFY(invalid.hasErrors());
    QCOMPARE(invalid.diagnostics.size(), 1);
    QCOMPARE(invalid.diagnostics.first().location.filePath(),
             QDir::cleanPath(QFINDTESTDATA("/testdata/configs/destination_conflict_base.qdocconf")));
    QCOMPARE(invalid.diagnostics.first().location.lineNo(), 3);
    QVERIFY(invalid.diagnostics.first().details.contains("destination_conflict_base.qdocconf"_L1));

    auto &unsafe = initConfig("/testdata/configs/publication_unsafe.qdocconf");
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s})};
    const auto unsafeResult = OutputConfiguration::resolve(unsafe, input);
    QVERIFY(unsafeResult.hasErrors());
    QCOMPARE(unsafeResult.diagnostics.size(), 1);
    QCOMPARE(unsafeResult.diagnostics.first().location.filePath(),
             QFINDTESTDATA("/testdata/configs/publication_unsafe.qdocconf"));
    QCOMPARE(unsafeResult.diagnostics.first().location.lineNo(), 2);
}

void tst_Config::moduleIdentity_data()
{
    QTest::addColumn<QString>("project");
    QTest::addColumn<bool>("valid");
    QTest::newRow("empty") << u""_s << false;
    QTest::newRow("dot") << u"."_s << false;
    QTest::newRow("parent") << u".."_s << false;
    QTest::newRow("escape") << u"../other"_s << false;
    QTest::newRow("separator") << u"a/b"_s << false;
    QTest::newRow("backslash") << u"a\\b"_s << false;
    QTest::newRow("colon") << u"a:b"_s << false;
    QTest::newRow("leading-space") << u" a"_s << false;
    QTest::newRow("trailing-dot") << u"a."_s << false;
    QTest::newRow("control") << u"a\n"_s << false;
    QTest::newRow("normal") << u"SampleProject"_s << true;
    QTest::newRow("spaces") << u"My documentation project"_s << true;
}

void tst_Config::moduleIdentity()
{
    QFETCH(QString, project);
    QFETCH(bool, valid);
    auto &config = initConfig();
    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s});
    config.setStringList(CONFIG_OUTPUTDIR, {u"docs"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s});
    config.setStringList(CONFIG_PUBLICATIONLAYOUT, {u"per-module"_s});
    config.setStringList(CONFIG_PROJECT, {project});
    OutputConfigurationInput input;
    input.pipelineCapabilities = {capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s})};
    const auto result = OutputConfiguration::resolve(config, input);
    QCOMPARE(result.hasErrors(), !valid);
    if (valid) {
        QCOMPARE(result.destination(result.formats.first()), u"docs/"_s + project.toLower());
    } else {
        QCOMPARE(result.diagnostics.size(), 1);
        QVERIFY(result.diagnostics.first().message.contains("project"_L1));
    }
    config.setStringList(CONFIG_PUBLICATIONLAYOUT, {u"flat"_s});
    QVERIFY(!OutputConfiguration::resolve(config, input).hasErrors());
}

void tst_Config::templatePackageBuiltin()
{
    auto &config = initConfig();
    const auto package = TemplatePackage::resolve(config, packageInput());
    QVERIFY2(!package.hasErrors(), qPrintable(messages(package)));
    QVERIFY(package.loaded);
    QVERIFY(package.location.isEmpty());
    QCOMPARE(package.id, TemplatePackage::builtinId());
    QCOMPARE(package.searchPaths, QStringList{builtinRoot()});
    QCOMPARE(package.formats.size(), 2);
    const QString manifest = builtinRoot() + u"/qdoc-template.json"_s;
    const auto *html = package.findFormat(u"HTML"_s);
    QVERIFY(html);
    QVERIFY(!html->broken);
    QCOMPARE(html->extension, u"html"_s);
    QCOMPARE(html->extensionLocation.filePath(), manifest);
    QCOMPARE(html->templates.keys(),
             QStringList({u"collection"_s, u"cppref"_s, u"members"_s, u"obsolete"_s, u"page"_s,
                          u"qmltype"_s}));
    QCOMPARE(html->templates.value(u"cppref"_s).path, builtinRoot() + u"/cppref.html"_s);
    QCOMPARE(html->templates.value(u"cppref"_s).location.filePath(), manifest);
    const auto *markdown = package.findFormat(u"Markdown"_s);
    QVERIFY(markdown);
    QCOMPARE(markdown->extension, u"md"_s);
    QCOMPARE(markdown->templates.value(u"page"_s).path, builtinRoot() + u"/page.md"_s);
    QVERIFY(!package.findFormat(u"html"_s));

    const auto capabilities = package.capabilities();
    QCOMPARE(capabilities.pipeline, Pipeline::Template);
    QVERIFY(capabilities.available);
    QCOMPARE(capabilities.supportedFormats, (QSet<QString>{u"HTML"_s, u"Markdown"_s}));
    QVERIFY(capabilities.brokenFormats.isEmpty());
    QCOMPARE(capabilities.formatExtensions.value(u"HTML"_s), u"html"_s);
    QCOMPARE(capabilities.formatExtensions.value(u"Markdown"_s), u"md"_s);

    config.setStringList(CONFIG_TEMPLATEPACKAGE, {TemplatePackage::builtinId()});
    const auto explicitBuiltin = TemplatePackage::resolve(config, packageInput());
    QVERIFY2(!explicitBuiltin.hasErrors(), qPrintable(messages(explicitBuiltin)));
    QCOMPARE(explicitBuiltin.id, TemplatePackage::builtinId());
    QCOMPARE(explicitBuiltin.searchPaths, package.searchPaths);

    config.setStringList(CONFIG_PIPELINE, {u"template"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"Markdown"_s});
    OutputConfigurationInput selectionInput;
    selectionInput.pipelineCapabilities = {capabilities};
    const auto selection = OutputSelection::resolve(config, selectionInput);
    QVERIFY(!selection.hasErrors());
    QCOMPARE(selection.findFormat(u"HTML"_s)->pipeline, Pipeline::Template);
    QCOMPARE(selection.findFormat(u"Markdown"_s)->fileExtension, u"md"_s);

    TemplatePackageInput unavailable;
    unavailable.builtinRoot = QDir::tempPath() + u"/qdoc-no-such-package"_s;
    const auto missing = TemplatePackage::resolve(config, unavailable);
    QVERIFY(missing.hasErrors());
    QVERIFY(!missing.loaded);
    QVERIFY(missing.formats.isEmpty());
    QVERIFY(!missing.capabilities().available);
    QVERIFY(missing.diagnostics.first().message.contains("manifest"_L1));
}

void tst_Config::templatePackageInheritance()
{
    auto &config = initConfig();
    const QString custom = packageDir(u"custom"_s);
    const QString customManifest = custom + u"/qdoc-template.json"_s;
    const QString builtinManifest = builtinRoot() + u"/qdoc-template.json"_s;
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {custom});
    const auto package = TemplatePackage::resolve(config, packageInput());
    QVERIFY2(!package.hasErrors(), qPrintable(messages(package)));
    QCOMPARE(package.id, custom);
    QCOMPARE(package.searchPaths, (QStringList{custom, builtinRoot()}));
    QCOMPARE(package.formats.size(), 2);
    QVERIFY(!package.findFormat(u"Markdown"_s));

    const auto *docbook = package.findFormat(u"DocBook"_s);
    QVERIFY(docbook);
    QVERIFY(!docbook->broken);
    QCOMPARE(docbook->extension, u"xml"_s);
    QCOMPARE(docbook->extensionLocation.filePath(), customManifest);
    QCOMPARE(docbook->templates.size(), 6);
    QCOMPARE(docbook->templates.value(u"cppref"_s).path, custom + u"/docbook/cppref.xml"_s);
    QCOMPARE(docbook->templates.value(u"cppref"_s).location.filePath(), customManifest);

    const auto *html = package.findFormat(u"HTML"_s);
    QVERIFY(html);
    QVERIFY(!html->broken);
    QCOMPARE(html->extension, u"html"_s);
    QCOMPARE(html->extensionLocation.filePath(), builtinManifest);
    QCOMPARE(html->location.filePath(), customManifest);
    QCOMPARE(html->templates.value(u"page"_s).path, custom + u"/html/page.html"_s);
    QCOMPARE(html->templates.value(u"page"_s).location.filePath(), customManifest);
    QCOMPARE(html->templates.value(u"cppref"_s).path, builtinRoot() + u"/cppref.html"_s);
    QCOMPARE(html->templates.value(u"cppref"_s).location.filePath(), builtinManifest);

    const auto capabilities = package.capabilities();
    QCOMPARE(capabilities.supportedFormats, (QSet<QString>{u"DocBook"_s, u"HTML"_s}));
    QVERIFY(!capabilities.formatExtensions.contains(u"Markdown"_s));
    QCOMPARE(capabilities.formatExtensions.value(u"DocBook"_s), u"xml"_s);

    config.setStringList(CONFIG_PIPELINE, {u"legacy"_s, u"template"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s, u"DocBook"_s, u"Markdown"_s});
    OutputConfigurationInput selectionInput;
    selectionInput.pipelineCapabilities = {
        capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s, u"DocBook"_s}), capabilities
    };
    const auto selection = OutputSelection::resolve(config, selectionInput);
    QVERIFY(!selection.hasErrors());
    QCOMPARE(selection.findFormat(u"HTML"_s)->pipeline, Pipeline::Template);
    QCOMPARE(selection.findFormat(u"DocBook"_s)->pipeline, Pipeline::Template);
    QCOMPARE(selection.findFormat(u"DocBook"_s)->fileExtension, u"xml"_s);
    QCOMPARE(selection.findFormat(u"Markdown"_s)->pipeline, Pipeline::Legacy);

    const QString grandchild = packageDir(u"grandchild"_s);
    const QString grandchildManifest = grandchild + u"/qdoc-template.json"_s;
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {grandchild});
    const auto derived = TemplatePackage::resolve(config, packageInput());
    QVERIFY2(!derived.hasErrors(), qPrintable(messages(derived)));
    QCOMPARE(derived.searchPaths, (QStringList{grandchild, custom, builtinRoot()}));
    QCOMPARE(derived.formats.size(), 3);
    const auto *derivedDocBook = derived.findFormat(u"DocBook"_s);
    QVERIFY(derivedDocBook);
    QCOMPARE(derivedDocBook->extension, u"docbook"_s);
    QCOMPARE(derivedDocBook->extensionLocation.filePath(), grandchildManifest);
    QCOMPARE(derivedDocBook->templates.value(u"cppref"_s).path, custom + u"/docbook/cppref.xml"_s);
    QCOMPARE(derivedDocBook->templates.value(u"cppref"_s).location.filePath(), customManifest);
    const auto *derivedMarkdown = derived.findFormat(u"Markdown"_s);
    QVERIFY(derivedMarkdown);
    QCOMPARE(derivedMarkdown->extension, u"markdown"_s);
    QCOMPARE(derivedMarkdown->templates.size(), 6);
    QCOMPARE(derivedMarkdown->templates.value(u"members"_s).path, grandchild + u"/md/any.md"_s);
    const auto *derivedHtml = derived.findFormat(u"HTML"_s);
    QVERIFY(derivedHtml);
    QCOMPARE(derivedHtml->extension, u"html"_s);
    QCOMPARE(derivedHtml->templates.value(u"page"_s).path, custom + u"/html/page.html"_s);
    QCOMPARE(derivedHtml->templates.value(u"cppref"_s).path, builtinRoot() + u"/cppref.html"_s);
}

void tst_Config::templatePackageProvenance()
{
    auto &config = initConfig("/testdata/configs/package_include.qdocconf");
    const auto package = TemplatePackage::resolve(config, packageInput());
    QVERIFY2(!package.hasErrors(), qPrintable(messages(package)));
    QCOMPARE(QFileInfo(package.id).canonicalFilePath(),
             QFileInfo(packageDir(u"custom"_s)).canonicalFilePath());
    QCOMPARE(package.location.filePath(),
             QDir::cleanPath(QFINDTESTDATA("/testdata/configs/package_base.qdocconf")));
    QCOMPARE(package.location.lineNo(), 1);
    QCOMPARE(package.searchPaths.size(), 2);
    QCOMPARE(package.findFormat(u"HTML"_s)->templates.value(u"page"_s).location.filePath(),
             package.id + u"/qdoc-template.json"_s);
}

void tst_Config::templatePackageSelection()
{
    auto &config = initConfig();
    const auto errorContaining = [&](const QString &text) {
        const auto package = TemplatePackage::resolve(config, packageInput());
        if (!package.hasErrors() || package.loaded)
            return false;
        for (const auto &diagnostic : package.diagnostics) {
            if (diagnostic.message.contains(text))
                return true;
        }
        return false;
    };
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {});
    QVERIFY(errorContaining(u"is empty"_s));
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {u""_s});
    QVERIFY(errorContaining(u"is empty"_s));
    config.setStringList(CONFIG_TEMPLATEPACKAGE,
                         {packageDir(u"custom"_s), packageDir(u"grandchild"_s)});
    QVERIFY(errorContaining(u"single value"_s));
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {u"qdoc:other"_s});
    QVERIFY(errorContaining(u"built-in"_s));
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {QDir::tempPath() + u"/qdoc-no-such-package"_s});
    QVERIFY(errorContaining(u"manifest"_s));

    config.setStringList(CONFIG_TEMPLATEPACKAGE, {packageDir(u"requires"_s)});
    QVERIFY(errorContaining(u"capability"_s));
    auto input = packageInput();
    input.engineCapabilities = {u"combined-book"_s};
    const auto satisfied = TemplatePackage::resolve(config, input);
    QVERIFY2(!satisfied.hasErrors(), qPrintable(messages(satisfied)));
    QCOMPARE(satisfied.formats.size(), 2);
}

void tst_Config::templatePackageErrors_data()
{
    QTest::addColumn<QString>("package");
    QTest::addColumn<QString>("message");
    QTest::addColumn<bool>("loaded");
    QTest::newRow("cycle") << u"cycle-a"_s << u"cycle"_s << false;
    QTest::newRow("self-cycle") << u"self"_s << u"cycle"_s << false;
    QTest::newRow("bad-json") << u"bad-json"_s << u"parse"_s << false;
    QTest::newRow("not-object") << u"not-object"_s << u"JSON object"_s << false;
    QTest::newRow("schema-version") << u"schema-version"_s << u"schema version 2"_s << false;
    QTest::newRow("api-version") << u"api-version"_s << u"API version 99"_s << false;
    QTest::newRow("requires") << u"requires"_s << u"capability 'combined-book'"_s << false;
    QTest::newRow("empty-extension") << u"empty-extension"_s << u"empty extension"_s << false;
    QTest::newRow("empty-template") << u"empty-template"_s << u"empty template"_s << false;
    QTest::newRow("unknown-role") << u"unknown-role"_s << u"role 'index'"_s << false;
    QTest::newRow("bad-format-value") << u"bad-format-value"_s << u"object or false"_s << false;
    QTest::newRow("invalid-identity") << u"invalid-identity"_s << u"identity 'Doc Book'"_s << false;
    QTest::newRow("unknown-key") << u"unknown-key"_s << u"key 'extend'"_s << false;
    QTest::newRow("missing-parent") << u"missing-parent"_s << u"manifest"_s << false;
    QTest::newRow("empty-extends") << u"empty-extends"_s << u"extends"_s << false;
    QTest::newRow("unknown-builtin") << u"unknown-builtin"_s << u"built-in"_s << false;
    QTest::newRow("missing-template") << u"missing-template"_s << u"does not exist"_s << true;
    QTest::newRow("incomplete") << u"incomplete"_s << u"no template for role"_s << true;
}

void tst_Config::templatePackageErrors()
{
    QFETCH(QString, package);
    QFETCH(QString, message);
    QFETCH(bool, loaded);
    auto &config = initConfig();
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {packageDir(package)});
    const auto result = TemplatePackage::resolve(config, packageInput());
    QVERIFY(result.hasErrors());
    QCOMPARE(result.loaded, loaded);
    QCOMPARE(result.capabilities().available, loaded);
    bool found = false;
    for (const auto &diagnostic : result.diagnostics) {
        QCOMPARE(diagnostic.kind, TemplatePackage::Diagnostic::Kind::Error);
        found = found || diagnostic.message.contains(message);
    }
    QVERIFY2(found, qPrintable(messages(result)));
}

void tst_Config::templatePackageBrokenFormat()
{
    auto &config = initConfig();
    const QString broken = packageDir(u"missing-template"_s);
    config.setStringList(CONFIG_TEMPLATEPACKAGE, {broken});
    const auto package = TemplatePackage::resolve(config, packageInput());
    QVERIFY(package.hasErrors());
    QVERIFY(package.loaded);
    const auto *html = package.findFormat(u"HTML"_s);
    QVERIFY(html);
    QVERIFY(html->broken);
    QCOMPARE(html->templates.value(u"page"_s).path, broken + u"/nope.html"_s);
    QCOMPARE(html->templates.value(u"cppref"_s).path, builtinRoot() + u"/cppref.html"_s);
    QVERIFY(!package.findFormat(u"Markdown"_s)->broken);
    const auto capabilities = package.capabilities();
    QVERIFY(capabilities.available);
    QCOMPARE(capabilities.brokenFormats, QSet<QString>{u"HTML"_s});
    QCOMPARE(capabilities.supportedFormats, QSet<QString>{u"Markdown"_s});
    QCOMPARE(capabilities.formatExtensions.value(u"HTML"_s), u"html"_s);

    config.setStringList(CONFIG_PIPELINE, {u"template"_s, u"legacy"_s});
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"HTML"_s});
    OutputConfigurationInput selectionInput;
    selectionInput.pipelineCapabilities = {
        capabilitiesFor(Pipeline::Legacy, {u"HTML"_s, u"Markdown"_s}), capabilities
    };
    const auto rejected = OutputSelection::resolve(config, selectionInput);
    QVERIFY(rejected.hasErrors());
    QCOMPARE(rejected.findFormat(u"HTML"_s)->pipeline, Pipeline::Template);
    config.setStringList(CONFIG_OUTPUTFORMATS, {u"Markdown"_s});
    QVERIFY(!OutputSelection::resolve(config, selectionInput).hasErrors());
}

QTEST_APPLESS_MAIN(tst_Config)

#include "tst_config.moc"
