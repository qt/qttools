// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utilities.h"

#include <QtTest/QtTest>

class tst_Utilities : public QObject
{
    Q_OBJECT

private slots:
    void loggingCategoryName();
    void loggingCategoryDefaults();
    void startDebugging();
    void stopDebugging();
    void debugging();
    void callSeparatorForOneWord();
    void callSeparatorForMoreThanOneWord();
    void callCommaForOneWord();
    void callCommaForTwoWords();
    void callCommaForThreeWords();
};

void tst_Utilities::loggingCategoryName()
{
    const QString expected = "qt.qdoc";
    QCOMPARE(lcQdoc().categoryName(), expected);
}

void tst_Utilities::loggingCategoryDefaults()
{
    QVERIFY(lcQdoc().isCriticalEnabled());
    QVERIFY(lcQdoc().isWarningEnabled());
    QVERIFY(!lcQdoc().isDebugEnabled());
    QVERIFY(lcQdoc().isInfoEnabled());
}

void tst_Utilities::startDebugging()
{
    QVERIFY(!lcQdoc().isDebugEnabled());
    Utilities::startDebugging("test");
    QVERIFY(lcQdoc().isDebugEnabled());
}

void tst_Utilities::stopDebugging()
{
    Utilities::startDebugging("test");
    QVERIFY(lcQdoc().isDebugEnabled());
    Utilities::stopDebugging("test");
    QVERIFY(!lcQdoc().isDebugEnabled());
}

void tst_Utilities::debugging()
{
    QVERIFY(!lcQdoc().isDebugEnabled());
    QVERIFY(!Utilities::debugging());
    Utilities::startDebugging("test");
    QVERIFY(lcQdoc().isDebugEnabled());
    QVERIFY(Utilities::debugging());
}

void tst_Utilities::callSeparatorForOneWord()
{
    const QStringList listOfWords { "one" };
    const QString expected = QStringLiteral("one.");

    int index = 0;
    QString result;
    for (const auto &word : listOfWords) {
        result.append(word);
        result.append(Utilities::separator(index++, listOfWords.size()));
    }
    QCOMPARE(result, expected);
}

void tst_Utilities::callSeparatorForMoreThanOneWord()
{
    const QStringList listOfWords { "one", "two" };
    const QString expected = QStringLiteral("one and two.");

    int index = 0;
    QString result;
    for (const auto &word : listOfWords) {
        result.append(word);
        result.append(Utilities::separator(index++, listOfWords.size()));
    }
    QCOMPARE(result, expected);
}

void tst_Utilities::callCommaForOneWord()
{
    const QStringList listOfWords { "one" };
    const QString expected = QStringLiteral("one");

    int index = 0;
    QString result;
    for (const auto &word : listOfWords) {
        result.append(word);
        result.append(Utilities::comma(index++, listOfWords.size()));
    }
    QCOMPARE(result, expected);
}
void tst_Utilities::callCommaForTwoWords()
{
    const QStringList listOfWords { "one", "two" };
    const QString expected = QStringLiteral("one and two");

    int index = 0;
    QString result;
    for (const auto &word : listOfWords) {
        result.append(word);
        result.append(Utilities::comma(index++, listOfWords.size()));
    }
    QCOMPARE(result, expected);
}

void tst_Utilities::callCommaForThreeWords()
{
    const QStringList listOfWords { "one", "two", "three" };
    const QString expected = QStringLiteral("one, two, and three");

    int index = 0;
    QString result;
    for (const auto &word : listOfWords) {
        result.append(word);
        result.append(Utilities::comma(index++, listOfWords.size()));
    }
    QCOMPARE(result, expected);
}

QTEST_APPLESS_MAIN(tst_Utilities)

#include "tst_utilities.moc"
