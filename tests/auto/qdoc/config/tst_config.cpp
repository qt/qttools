/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "config.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtTest/QtTest>

QT_BEGIN_NAMESPACE
Q_LOGGING_CATEGORY(lcQdoc, "qt.test")
QT_END_NAMESPACE

class tst_Config : public QObject
{
    Q_OBJECT

private slots:
    void classMembersInitializeToFalseOrEmpty();
    void includePathsFromCommandLine();
    void getExampleProjectFile();
};

void tst_Config::classMembersInitializeToFalseOrEmpty()
{
    QStringList commandLineArgs = { QStringLiteral("./qdoc") };

    Config::instance().init("QDoc Test", commandLineArgs);
    auto &config = Config::instance();
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
    const QStringList commandLineArgs = {
        QStringLiteral("./qdoc"),
        mockIncludePath1,
        mockIncludePath2
    };

    Config::instance().init("QDoc Test", commandLineArgs);
    auto &config = Config::instance();

    const QStringList expected = { mockIncludePath1, mockIncludePath2 };
    const QStringList actual = config.includePaths();

    QCOMPARE(actual, expected);
}

void::tst_Config::getExampleProjectFile()
{
    QStringList commandLineArgs = { QStringLiteral("./qdoc") };
    Config::instance().init("QDoc Test", commandLineArgs);
    auto &config = Config::instance();

    const auto docConfig = QFINDTESTDATA("/testdata/configs/exampletest.qdocconf");
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


QTEST_APPLESS_MAIN(tst_Config)

#include "tst_config.moc"
