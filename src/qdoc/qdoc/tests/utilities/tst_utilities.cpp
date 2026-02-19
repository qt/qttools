// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdoc/utilities.h"
#include "qdoc/textutils.h"

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

    void isGeneratedFile();

    void linkForExampleFile_basicPath();
    void linkForExampleFile_pathWithSpecialChars();

    void asAsciiPrintable();

    void exampleFileTitle_fileMatch();
    void exampleFileTitle_imageMatch();
    void exampleFileTitle_noMatch();
    void exampleFileTitle_basenameOnly();
    void exampleFileTitle_duplicateInBothLists();
    void exampleFileTitle_kindOverload();
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
        result.append(TextUtils::separator(index++, listOfWords.size()));
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
        result.append(TextUtils::separator(index++, listOfWords.size()));
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
        result.append(TextUtils::comma(index++, listOfWords.size()));
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
        result.append(TextUtils::comma(index++, listOfWords.size()));
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
        result.append(TextUtils::comma(index++, listOfWords.size()));
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

void tst_Utilities::isGeneratedFile()
{
    const QStringList listOfPaths {
        "", "/", "abc.cpp", "moc.cpp", "moc/cpp", "/moc/cpp",
        "moc_abc.cpp", "ui_def.cpp", "qrc_ghi.cpp",
        "/moc_abc.cpp", "abc/ui_def.cpp", "/def/qrc_ghi.cpp"
    };
    const QVector<bool> expected = {
        false, false, false, false, false, false,
        true, true, true, true, true, true
    };
    int index = 0;
    QVector<bool> result(12);
    for (const auto &path : listOfPaths) {
        result[index++] = Utilities::isGeneratedFile(path);
    }
    QCOMPARE(result, expected);
}

void tst_Utilities::linkForExampleFile_basicPath()
{
    // Basic path: project prefix (lowercased) + path + extension
    QString result = Utilities::linkForExampleFile("examples/widgets/main.cpp", "QtWidgets", "html");
    QCOMPARE(result, QStringLiteral("qtwidgets-examples-widgets-main-cpp.html"));
}

void tst_Utilities::linkForExampleFile_pathWithSpecialChars()
{
    // Paths with special characters get sanitized via asAsciiPrintable
    QString result = Utilities::linkForExampleFile("examples/file with spaces.qml", "QtQuick", "html");
    QCOMPARE(result, QStringLiteral("qtquick-examples-file-with-spaces-qml.html"));
}

void tst_Utilities::asAsciiPrintable()
{
    QString result;
    result = TextUtils::asAsciiPrintable("");
    QCOMPARE(result, QStringLiteral(""));
    result = TextUtils::asAsciiPrintable(" ");
    QCOMPARE(result, QStringLiteral(""));
    result = TextUtils::asAsciiPrintable("a");
    QCOMPARE(result, QStringLiteral("a"));
    result = TextUtils::asAsciiPrintable("a ");
    QCOMPARE(result, QStringLiteral("a"));
    result = TextUtils::asAsciiPrintable(" a");
    QCOMPARE(result, QStringLiteral("a"));
    result = TextUtils::asAsciiPrintable("Hello World");
    QCOMPARE(result, QStringLiteral("hello-world"));
    result = TextUtils::asAsciiPrintable("Hello World ");
    QCOMPARE(result, QStringLiteral("hello-world"));
    result = TextUtils::asAsciiPrintable("Hello World-");
    QCOMPARE(result, QStringLiteral("hello-world-"));
    result = TextUtils::asAsciiPrintable("myfile.txt");
    QCOMPARE(result, QStringLiteral("myfile-txt"));
    result = TextUtils::asAsciiPrintable("Only printable!");
    QCOMPARE(result, QStringLiteral("only-printable"));
    result = TextUtils::asAsciiPrintable("Multiple  spaces");
    QCOMPARE(result, QStringLiteral("multiple-spaces"));
    result = TextUtils::asAsciiPrintable("Embedded - hyphen");
    QCOMPARE(result, QStringLiteral("embedded-hyphen"));
    result = TextUtils::asAsciiPrintable("Embedded -- hyphens");
    QCOMPARE(result, QStringLiteral("embedded--hyphens"));
    result = TextUtils::asAsciiPrintable("Separated - - hyphens");
    QCOMPARE(result, QStringLiteral("separated--hyphens"));
    result = TextUtils::asAsciiPrintable("Hello World!");
    QCOMPARE(result, QStringLiteral("hello-world"));
    result = TextUtils::asAsciiPrintable("Hello World! ");
    QCOMPARE(result, QStringLiteral("hello-world"));
    result = TextUtils::asAsciiPrintable("Hello World!-");
    QCOMPARE(result, QStringLiteral("hello-world-"));
    result = TextUtils::asAsciiPrintable("Hello!World");
    QCOMPARE(result, QStringLiteral("hello-world"));
    result = TextUtils::asAsciiPrintable("Hello!!World");
    QCOMPARE(result, QStringLiteral("hello-world"));
    result = TextUtils::asAsciiPrintable("myfile!.txt");
    QCOMPARE(result, QStringLiteral("myfile-txt"));
    result = TextUtils::asAsciiPrintable("Double non-ASCII!!");
    QCOMPARE(result, QStringLiteral("double-non-ascii"));
    result = TextUtils::asAsciiPrintable("Character! - proximity 1");
    QCOMPARE(result, QStringLiteral("character-proximity-1"));
    result = TextUtils::asAsciiPrintable("Character !- proximity 2");
    QCOMPARE(result, QStringLiteral("character-proximity-2"));
    result = TextUtils::asAsciiPrintable("Character -! proximity 3");
    QCOMPARE(result, QStringLiteral("character-proximity-3"));
    result = TextUtils::asAsciiPrintable("Character - !proximity 4");
    QCOMPARE(result, QStringLiteral("character-proximity-4"));
    result = TextUtils::asAsciiPrintable("Multiple! -- proximity 1");
    QCOMPARE(result, QStringLiteral("multiple--proximity-1"));
    result = TextUtils::asAsciiPrintable("Multiple !-- proximity 2");
    QCOMPARE(result, QStringLiteral("multiple--proximity-2"));
    result = TextUtils::asAsciiPrintable("Multiple -!- proximity 3");
    QCOMPARE(result, QStringLiteral("multiple--proximity-3"));
    result = TextUtils::asAsciiPrintable("Multiple --! proximity 4");
    QCOMPARE(result, QStringLiteral("multiple--proximity-4"));
    result = TextUtils::asAsciiPrintable("Multiple -- !proximity 5");
    QCOMPARE(result, QStringLiteral("multiple--proximity-5"));
}

void tst_Utilities::exampleFileTitle_fileMatch()
{
    QStringList files = {"src/main.cpp", "src/widget.cpp"};
    QStringList images = {"images/logo.png"};
    QString result = Utilities::exampleFileTitle(files, images, "src/main.cpp");
    QCOMPARE(result, QStringLiteral("main.cpp Example File"));
}

void tst_Utilities::exampleFileTitle_imageMatch()
{
    QStringList files = {"src/main.cpp"};
    QStringList images = {"images/logo.png", "images/icon.png"};
    QString result = Utilities::exampleFileTitle(files, images, "images/logo.png");
    QCOMPARE(result, QStringLiteral("logo.png Image File"));
}

void tst_Utilities::exampleFileTitle_noMatch()
{
    QStringList files = {"src/main.cpp"};
    QStringList images = {"images/logo.png"};
    QString result = Utilities::exampleFileTitle(files, images, "unknown/file.txt");
    QCOMPARE(result, QString{});
}

void tst_Utilities::exampleFileTitle_basenameOnly()
{
    // Input without path separator returns same basename + suffix
    QStringList files = {"main.cpp"};
    QStringList images;
    QString result = Utilities::exampleFileTitle(files, images, "main.cpp");
    QCOMPARE(result, QStringLiteral("main.cpp Example File"));
}

void tst_Utilities::exampleFileTitle_duplicateInBothLists()
{
    // If fileName appears in both lists, files takes precedence (checked first)
    QStringList files = {"shared/file.txt"};
    QStringList images = {"shared/file.txt"};
    QString result = Utilities::exampleFileTitle(files, images, "shared/file.txt");
    QCOMPARE(result, QStringLiteral("file.txt Example File"));
}

void tst_Utilities::exampleFileTitle_kindOverload()
{
    // Test the O(1) overload that takes ExampleFileKind directly
    using Utilities::ExampleFileKind;

    QString fileResult = Utilities::exampleFileTitle("src/widget.cpp", ExampleFileKind::File);
    QCOMPARE(fileResult, QStringLiteral("widget.cpp Example File"));

    QString imageResult = Utilities::exampleFileTitle("images/icon.png", ExampleFileKind::Image);
    QCOMPARE(imageResult, QStringLiteral("icon.png Image File"));

    // Basename-only input works too
    QString basenameResult = Utilities::exampleFileTitle("README.md", ExampleFileKind::File);
    QCOMPARE(basenameResult, QStringLiteral("README.md Example File"));
}

QTEST_APPLESS_MAIN(tst_Utilities)

#include "tst_utilities.moc"
