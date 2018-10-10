/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qdocglobals.h"

#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtTest/QtTest>

class testQDocGlobals : public QObject
{
    Q_OBJECT

private slots:
    void testClassMembersInitializeToFalseOrEmpty();
    void testEnableHighlighting();
    void testSetShowInternal();
    void testSetSingleExec();
    void testSetWriteQaPages();
    void testRedirectDocumentationToDevNull();
    void testSetNoLinkErrors();
    void testSetAutoLinkErrors();
    void testSetObsoleteLinks();

    void testAddDefine();
    void testAddIncludePath();
    void testDependModules();
    void testAppendToIndexDirs();
    void testSetCurrentDir();
    void testPreviousCurrentDir();
    void testDefaults();
};

void testQDocGlobals::testClassMembersInitializeToFalseOrEmpty()
{
    QDocGlobals qdocTestGlobals;
    QCOMPARE(qdocTestGlobals.highlighting(), false);
    QCOMPARE(qdocTestGlobals.showInternal(), false);
    QCOMPARE(qdocTestGlobals.singleExec(), false);
    QCOMPARE(qdocTestGlobals.writeQaPages(), false);
    QCOMPARE(qdocTestGlobals.redirectDocumentationToDevNull(), false);
    QCOMPARE(qdocTestGlobals.noLinkErrors(), false);
    QCOMPARE(qdocTestGlobals.autolinkErrors(), false);
    QCOMPARE(qdocTestGlobals.obsoleteLinks(), false);

    QVERIFY(qdocTestGlobals.defines().isEmpty());
    QVERIFY(qdocTestGlobals.includesPaths().isEmpty());
    QVERIFY(qdocTestGlobals.dependModules().isEmpty());
    QVERIFY(qdocTestGlobals.indexDirs().isEmpty());
    QVERIFY(qdocTestGlobals.currentDir().isEmpty());
    QVERIFY(qdocTestGlobals.previousCurrentDir().isEmpty());
    QVERIFY(qdocTestGlobals.defaults().isEmpty());
}

void testQDocGlobals::testEnableHighlighting()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.enableHighlighting(true);
    QVERIFY(qdocTestGlobals.highlighting());
}

void testQDocGlobals::testSetShowInternal()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setShowInternal(true);
    QVERIFY(qdocTestGlobals.showInternal());
}

void testQDocGlobals::testSetSingleExec()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setSingleExec(true);
    QVERIFY(qdocTestGlobals.singleExec());
}

void testQDocGlobals::testSetWriteQaPages()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setWriteQaPages(true);
    QVERIFY(qdocTestGlobals.writeQaPages());
}

void testQDocGlobals::testRedirectDocumentationToDevNull()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setRedirectDocumentationToDevNull(true);
    QVERIFY(qdocTestGlobals.redirectDocumentationToDevNull());
}

void testQDocGlobals::testSetNoLinkErrors()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setNoLinkErrors(true);
    QVERIFY(qdocTestGlobals.noLinkErrors());
}

void testQDocGlobals::testSetAutoLinkErrors()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setAutolinkErrors(true);
    QVERIFY(qdocTestGlobals.autolinkErrors());
}

void testQDocGlobals::testSetObsoleteLinks()
{
    QDocGlobals qdocTestGlobals;
    qdocTestGlobals.setObsoleteLinks(true);
    QVERIFY(qdocTestGlobals.obsoleteLinks());
}

void testQDocGlobals::testAddDefine()
{
    QDocGlobals qdocTestGlobals;
    QStringList defineTestList1 = { QStringLiteral("qtforpython") };
    QStringList defineTestList2 = { QStringLiteral("example") };
    QStringList expected;
    expected << defineTestList1 << defineTestList2;

    qdocTestGlobals.addDefine(defineTestList1);
    QCOMPARE(qdocTestGlobals.defines().size(), 1);
    qdocTestGlobals.addDefine(defineTestList2);
    QCOMPARE(qdocTestGlobals.defines().size(), 2);
    QCOMPARE(qdocTestGlobals.defines(), expected);
}

void testQDocGlobals::testAddIncludePath()
{
    QDocGlobals qdocTestGlobals;
    QString testFlag = "-I";
    QString testPath0 = "/qt5/qtdoc/doc/.";
    QString testPath1 = "/qt5/qtbase/mkspecs/linux-g++";
    QStringList expected = { "-I/qt5/qtdoc/doc/.",
                             "-I/qt5/qtbase/mkspecs/linux-g++" };

    qdocTestGlobals.addIncludePath(testFlag, testPath0);
    qdocTestGlobals.addIncludePath(testFlag, testPath1);
    QStringList result = qdocTestGlobals.includesPaths();
    QCOMPARE(result, expected);
}

void testQDocGlobals::testDependModules()
{
    QDocGlobals qdocTestGlobals;
    QStringList expected = { "qdoc", "qmake", "qtcore", "qthelp",  "qtqml" };

    qdocTestGlobals.dependModules() = expected;
    QCOMPARE(qdocTestGlobals.dependModules().size(), 5);
    QCOMPARE(qdocTestGlobals.dependModules(), expected);
}

void testQDocGlobals::testAppendToIndexDirs()
{
    QDocGlobals qdocTestGlobals;
    QString testPath = "/qt5/qtbase/doc";
    QStringList expected;
    expected << testPath;

    qdocTestGlobals.appendToIndexDirs(testPath);
    QCOMPARE(qdocTestGlobals.indexDirs(), expected);
}

void testQDocGlobals::testSetCurrentDir()
{
    QDocGlobals qdocTestGlobals;
    QString expected = "/qt5/qtdoc/doc/config";

    qdocTestGlobals.setCurrentDir(expected);
    QCOMPARE(qdocTestGlobals.currentDir(), expected);
}

void testQDocGlobals::testPreviousCurrentDir()
{
    QDocGlobals qdocTestGlobals;
    QString expected = "/qt5/qtdoc/doc";

    qdocTestGlobals.setCurrentDir(expected);
    QCOMPARE(qdocTestGlobals.currentDir(), expected);
}

void testQDocGlobals::testDefaults()
{
    QDocGlobals qdocTestGlobals;

    QHash<QString, QString> expected = {
        {"codeindent", "0"}, {"falsehoods", "0"},
        {"fileextensions", "*.cpp *.h *.qdoc *.qml"}, {"language", "Cpp"},
        {"outputformats", "HTML"}, {"tabsize", "8"}};

    qdocTestGlobals.defaults().insert(QStringLiteral("codeindent"),
                                      QLatin1String("0"));
    qdocTestGlobals.defaults().insert(QStringLiteral("falsehoods"),
                                      QLatin1String("0"));
    qdocTestGlobals.defaults().insert(QStringLiteral("fileextensions"),
                                      QLatin1String("*.cpp *.h *.qdoc *.qml"));
    qdocTestGlobals.defaults().insert(QStringLiteral("language"),
                                      QLatin1String("Cpp"));
    qdocTestGlobals.defaults().insert(QStringLiteral("outputformats"),
                                      QLatin1String("HTML"));
    qdocTestGlobals.defaults().insert(QStringLiteral("tabsize"),
                                      QLatin1String("8"));

    QHash<QString, QString> result = qdocTestGlobals.defaults();
    QCOMPARE(result, expected);
}

QTEST_APPLESS_MAIN(testQDocGlobals)

#include "tst_qdocglobals.moc"
