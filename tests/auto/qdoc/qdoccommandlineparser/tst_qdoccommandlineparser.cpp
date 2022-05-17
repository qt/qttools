// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdoccommandlineparser.h"

#include <QtCore/qstringlist.h>
#include <QtTest/QtTest>

class tst_QDocCommandLineParser : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructor();
    void process();
    void argumentsFromCommandLineAndFile();
};

void tst_QDocCommandLineParser::defaultConstructor()
{
    QDocCommandLineParser parser;

    QVERIFY2(parser.applicationDescription() == QStringLiteral("Qt documentation generator"),
             "The application description is incorrect.");
}

void tst_QDocCommandLineParser::process()
{
    const QStringList arguments =
        QStringLiteral("/src/qt5/qtbase/bin/qdoc "
                       "-outputdir "
                       "/src/qt5/qtbase/doc/qtgamepad "
                       "-installdir "
                       "/src/qt5/qtbase/doc "
                       "/src/qt5/qtgamepad/src/gamepad/doc/qtgamepad.qdocconf "
                       "-prepare "
                       "-indexdir "
                       "/src/qt5/qtbase/doc "
                       "-no-link-errors "
                       "-I. "
                       "-I/src/qt5/qtbase/include "
                       "-I/src/qt5/qtbase/include/QtGamepad "
                       "-I/src/qt5/qtbase/include/QtGamepad/5.14.0 "
                       "-I/src/qt5/qtbase/include/QtGamepad/5.14.0/QtGamepad "
                       "-I/src/qt5/qtbase/include/QtCore/5.14.0 "
                       "-I/src/qt5/qtbase/include/QtCore/5.14.0/QtCore "
                       "-I/src/qt5/qtbase/include/QtGui "
                       "-I/src/qt5/qtbase/include/QtCore "
                       "-I.moc "
                       "-isystem "
                       "/usr/include/libdrm "
                       "-I/src/qt5/qtbase/mkspecs/linux-g++"
        ).split(QString(" "));
    const QStringList expectedIncludePaths =
        QStringLiteral(". "
                       "/src/qt5/qtbase/include "
                       "/src/qt5/qtbase/include/QtGamepad "
                       "/src/qt5/qtbase/include/QtGamepad/5.14.0 "
                       "/src/qt5/qtbase/include/QtGamepad/5.14.0/QtGamepad "
                       "/src/qt5/qtbase/include/QtCore/5.14.0 "
                       "/src/qt5/qtbase/include/QtCore/5.14.0/QtCore "
                       "/src/qt5/qtbase/include/QtGui "
                       "/src/qt5/qtbase/include/QtCore "
                       ".moc "
                       "/src/qt5/qtbase/mkspecs/linux-g++"
        ).split(QString(" "));
    const QStringList expectedSystemIncludePath(QStringLiteral("/usr/include/libdrm"));

    QDocCommandLineParser parser;
    parser.process(arguments);

    QVERIFY(parser.isSet(parser.outputDirOption));
    QCOMPARE(parser.value(parser.outputDirOption), QStringLiteral("/src/qt5/qtbase/doc/qtgamepad"));
    QVERIFY(parser.isSet(parser.installDirOption));
    QCOMPARE(parser.value(parser.installDirOption), QStringLiteral("/src/qt5/qtbase/doc"));
    QVERIFY(parser.isSet(parser.prepareOption));
    QVERIFY(parser.isSet(parser.indexDirOption));
    QCOMPARE(parser.value(parser.indexDirOption), QStringLiteral("/src/qt5/qtbase/doc"));
    QVERIFY(parser.isSet(parser.noLinkErrorsOption));
    QVERIFY(parser.isSet(parser.includePathOption));
    QCOMPARE(parser.values(parser.includePathOption), expectedIncludePaths);
    QVERIFY(parser.isSet(parser.includePathSystemOption));
    QCOMPARE(parser.values(parser.includePathSystemOption), expectedSystemIncludePath);

    QVERIFY(!parser.isSet(parser.timestampsOption));
    QVERIFY(!parser.isSet(parser.dependsOption));
    QVERIFY(!parser.isSet(parser.highlightingOption));
    QVERIFY(!parser.isSet(parser.showInternalOption));
    QVERIFY(!parser.isSet(parser.redirectDocumentationToDevNullOption));
    QVERIFY(!parser.isSet(parser.noExamplesOption));
    QVERIFY(!parser.isSet(parser.autoLinkErrorsOption));
    QVERIFY(!parser.isSet(parser.debugOption));
    QVERIFY(!parser.isSet(parser.generateOption));
    QVERIFY(!parser.isSet(parser.logProgressOption));
    QVERIFY(!parser.isSet(parser.singleExecOption));
    QVERIFY(!parser.isSet(parser.frameworkOption));

    const QStringList expectedPositionalArgument = {
        QStringLiteral("/src/qt5/qtgamepad/src/gamepad/doc/qtgamepad.qdocconf")
    };
    QCOMPARE(parser.positionalArguments(), expectedPositionalArgument);
}

void tst_QDocCommandLineParser::argumentsFromCommandLineAndFile()
{
    const QString atFilePath("@" + QFINDTESTDATA("tst_arguments.txt"));
    const QStringList arguments { "/src/qt5/qtbase/bin/qdoc", atFilePath };

    QDocCommandLineParser parser;
    parser.process(arguments);

    const QStringList expectedIncludePaths =
        QStringLiteral(". "
                       "/src/qt5/qtbase/include "
                       "/src/qt5/qtbase/include/QtGamepad "
                       "/src/qt5/qtbase/include/QtGamepad/5.14.0 "
                       "/src/qt5/qtbase/include/QtGamepad/5.14.0/QtGamepad "
                       "/src/qt5/qtbase/include/QtCore/5.14.0 "
                       "/src/qt5/qtbase/include/QtCore/5.14.0/QtCore "
                       "/src/qt5/qtbase/include/QtGui "
                       "/src/qt5/qtbase/include/QtCore "
                       ".moc "
                       "/src/qt5/qtbase/mkspecs/linux-g++"
        ).split(QString(" "));
    const QStringList expectedSystemIncludePath(QStringLiteral("/usr/include/libdrm"));
    const QStringList expectedPositionalArgument = {
        QStringLiteral("/src/qt5/qtgamepad/src/gamepad/doc/qtgamepad.qdocconf")
    };

    QVERIFY(parser.isSet(parser.outputDirOption));
    QCOMPARE(parser.value(parser.outputDirOption), QStringLiteral("/src/qt5/qtbase/doc/qtgamepad"));
    QVERIFY(parser.isSet(parser.installDirOption));
    QCOMPARE(parser.value(parser.installDirOption), QStringLiteral("/src/qt5/qtbase/doc"));
    QVERIFY(parser.isSet(parser.prepareOption));
    QVERIFY(parser.isSet(parser.indexDirOption));
    QCOMPARE(parser.value(parser.indexDirOption), QStringLiteral("/src/qt5/qtbase/doc"));
    QVERIFY(parser.isSet(parser.noLinkErrorsOption));
    QVERIFY(parser.isSet(parser.includePathOption));
    QCOMPARE(parser.values(parser.includePathOption), expectedIncludePaths);
    QVERIFY(parser.isSet(parser.includePathSystemOption));
    QCOMPARE(parser.values(parser.includePathSystemOption), expectedSystemIncludePath);

    QVERIFY(!parser.isSet(parser.timestampsOption));
    QVERIFY(!parser.isSet(parser.dependsOption));
    QVERIFY(!parser.isSet(parser.highlightingOption));
    QVERIFY(!parser.isSet(parser.showInternalOption));
    QVERIFY(!parser.isSet(parser.redirectDocumentationToDevNullOption));
    QVERIFY(!parser.isSet(parser.noExamplesOption));
    QVERIFY(!parser.isSet(parser.autoLinkErrorsOption));
    QVERIFY(!parser.isSet(parser.debugOption));
    QVERIFY(!parser.isSet(parser.generateOption));
    QVERIFY(!parser.isSet(parser.logProgressOption));
    QVERIFY(!parser.isSet(parser.singleExecOption));
    QVERIFY(!parser.isSet(parser.frameworkOption));

    QCOMPARE(parser.positionalArguments(), expectedPositionalArgument);
}

QTEST_APPLESS_MAIN(tst_QDocCommandLineParser)

#include "tst_qdoccommandlineparser.moc"
