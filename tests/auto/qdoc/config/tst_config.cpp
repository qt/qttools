// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "config.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtTest/QtTest>

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
*/
Config &tst_Config::initConfig(const QStringList &args, const char *qdocconf)
{
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

QTEST_APPLESS_MAIN(tst_Config)

#include "tst_config.moc"
