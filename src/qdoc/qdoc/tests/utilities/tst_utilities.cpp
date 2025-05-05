// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdoc/utilities.h"

#include "qdoc/inode.h"
#include "qdoc/location.h"

#include <QtTest/QtTest>

QT_BEGIN_NAMESPACE

/*!
    \brief A basic Node implementation for serialization function tests.
  */
enum class Genus : unsigned char { DontCare };
enum class NodeType : unsigned char { NoType };

class TestNode : public INode
{
public:
    explicit TestNode(QString name) : m_name(std::move(name)) {}

    [[nodiscard]] Genus genus() const override { return Genus::DontCare; }
    [[nodiscard]] NodeType nodeType() const override { return NodeType::NoType; }

    [[nodiscard]] const QString &name() const override { return m_name; }
    [[nodiscard]] QString fullName() const override { return m_name; }

private:
    QString m_name;
};

QT_END_NAMESPACE

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
    void uniqueId();

    // stringForNode and nodeForString
    void stringForNode_ValidPointer();
    void stringForNode_NullPointer();
    void nodeForString_ValidString();
    void nodeForString_ZeroString();
    void nodeForString_InvalidString();
    void roundTrip_ValidPointer();
    void roundTrip_NullPointer();
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

void tst_Utilities::uniqueId()
{
    const QString expected = QStringLiteral("prefix-path-1");
    Location loc { "../some/path" };
    loc.start();
    QCOMPARE(Utilities::uniqueIdentifier(loc, "prefix"), expected);
}

void tst_Utilities::stringForNode_ValidPointer()
{
    TestNode testNode(QStringLiteral("TestNode"));
    INode* nodePtr = &testNode;

    QString nodeString = Utilities::stringForNode(nodePtr);

    QVERIFY(!nodeString.isEmpty());
    bool ok = false;
    quintptr addressFromString = nodeString.toULongLong(&ok);
    QVERIFY(ok);
    QCOMPARE(addressFromString, reinterpret_cast<quintptr>(nodePtr));
}

void tst_Utilities::stringForNode_NullPointer()
{
    INode* nodePtr = nullptr;
    QString nodeString = Utilities::stringForNode(nodePtr);

    // A null pointer should result in the string "0"
    QCOMPARE(nodeString, QStringLiteral("0"));
}

void tst_Utilities::nodeForString_ValidString()
{
    TestNode testNode(QStringLiteral("TestNode"));
    INode* originalPtr = &testNode;
    QString nodeString = Utilities::stringForNode(originalPtr);

    const INode* recoveredPtr = Utilities::nodeForString(nodeString);

    QCOMPARE(recoveredPtr, originalPtr);
}

void tst_Utilities::nodeForString_ZeroString()
{
    QString zeroString = QStringLiteral("0");
    const INode* recoveredPtr = Utilities::nodeForString(zeroString);

    // The string "0" should result in a null pointer
    QCOMPARE(recoveredPtr, nullptr);
}

void tst_Utilities::roundTrip_ValidPointer()
{
    TestNode testNode(QStringLiteral("TestNode"));
    INode* originalPtr = &testNode;

    // Convert pointer to string
    QString nodeString = Utilities::stringForNode(originalPtr);
    // Convert string back to pointer
    const INode* recoveredPtr = Utilities::nodeForString(nodeString);

    // The final pointer must match the original
    QCOMPARE(recoveredPtr, originalPtr);
}

void tst_Utilities::roundTrip_NullPointer()
{
    INode* originalPtr = nullptr;

    // Convert null pointer to string
    QString nodeString = Utilities::stringForNode(originalPtr);
    // Convert string ("0") back to pointer
    const INode* recoveredPtr = Utilities::nodeForString(nodeString);

    // The final pointer must also be null
    QCOMPARE(recoveredPtr, nullptr);
}

void tst_Utilities::nodeForString_InvalidString()
{
    // Passing a non‑numeric string should yield a null pointer
    const INode *back = Utilities::nodeForString(QStringLiteral("not a number"));
    QCOMPARE(back, nullptr);
}

QTEST_APPLESS_MAIN(tst_Utilities)

#include "tst_utilities.moc"
