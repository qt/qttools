// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























// IMPORTANT!!!! If you want to add testdata to this file,
// always add it to the end in order to not change the linenumbers of translations!!!

// nothing here

// sickness: multi-\
line c++ comment } (with brace)

#define This is a closing brace } which was ignored
} // complain here

#define This is another \
    closing brace } which was ignored
} // complain here

#define This is another /* comment in } define */\
     something /* comment )
       spanning {multiple} lines */ \
    closing brace } which was ignored
} // complain here

#define This is another // comment in } define \
     something } comment
} // complain here
#include <QtCore>


// Nested class in same file
class TopLevel {
    Q_OBJECT

    class Nested;
};

class TopLevel::Nested {
    void foo();
};

void TopLevel::Nested::foo()
{
    TopLevel::tr("TopLevel");
}

// Nested class in other file
#include "main.h"

class TopLevel2::Nested {
    void foo();
};

void TopLevel2::Nested::foo()
{
    TopLevel2::tr("TopLevel2");
}



namespace NameSpace {
class ToBeUsed;
}

// using statement before class definition
using NameSpace::ToBeUsed;

class NameSpace::ToBeUsed {
    Q_OBJECT
    void caller();
};

void ToBeUsed::caller()
{
    tr("NameSpace::ToBeUsed");
}
#include <QtWidgets/QApplication>

bool me = false;
// QTBUG-11818
//% "Foo"
QString s1 = QObject::tr("Hello World");
QString s2 = QObject::tr("Hello World");
//% "Bar"
QString s3 = QApplication::translate("QObject", "Hello World");
QString s4 = QApplication::translate("QObject", "Hello World");
//% "Baz"
bool clear = me;
QString s5 = QObject::tr("Hello World");



// QTBUG-11843: complain about missing source in id-based messages
QString s6 = qtTrId("no_source");

QString s7 = QObject::tr(R"(simple one)" R"delim(enter
)delim" R"delim(with delimiter )delim inside)delim" u8R"(with quote " inside)");

QLatin1String not_translated(R"(
                            This is a test string
)");
const char valid[] = QT_TRANSLATE_NOOP("global", R"(
"The time has come," the Walrus said,
"To talk of many things:
Of shoes - and ships - and sealing-wax -
Of cabbages - and kings -
And why the sea is boiling hot -
And whether pigs have wings."
)");

const QString nodelimiter(QObject::tr(R"(
                            This is a test string
)"));
const QString withdelimiter = QObject::tr(R"delim(
This is a test string
)delim");


// New in C++14: integer literals may contain single quotes as separator.
struct IntLiteralsWithSeparators {
    long d = 10'000'000'0'00;
    int x = 0x1'AF'FE;
    int X = 0X2'E5E7;
};


// QTBUG-59802: prefixed string literals
class PrefixedStringLiterals : public QObject {
    Q_OBJECT
    void foo()
    {
        #if 0
        tr(u8"UTF-8 string literal");
        tr(u8R"(UTF-8 raw string literal)");
        tr(u"UTF-16 string literal");
        tr(uR"(UTF-16 raw string literal)");
        tr(U"UTF-32 string literal");
        tr(UR"(UTF-32 raw string literal)");
        tr(L"wide string literal");
        tr(LR"(wide raw string literal)");
        #endif
    }
};

// QTBUG-110949: trailing return types with template parameters
class TrailingReturnType : public QObject {
    Q_OBJECT
    auto f1() -> QString
    {
        return tr("f1: trailing return type");
    }
    auto f2() -> QString;
    auto f3() -> std::vector<QString>
    {
        return { tr("f3: trailing return type") };
    }
    auto f4() -> std::vector<QString>
    {
        return { tr("f4: trailing return type") };
    }
    auto f5() -> decltype([]() { return 1; })
    {
        tr("f5: trailing return type");
    }
    auto f6() -> decltype([]() { return 1; });
};

auto TrailingReturnType::f2() -> QString
{
    return tr("f2: trailing return type");
}

auto TrailingReturnType::f4() -> std::vector<QString>
{
    return { tr("f4: trailing return type") };
}

auto TrailingReturnType::f6() -> decltype([]() { return 1; })
{
    tr("f6: trailing return type");
    return {};
}

// Check that our -> handling doesn't break the common cases.
class SomeClassWithArrowInMethods : public QObject {
    Q_OBJECT
    void f1()
    {
        mainWindow->setWindowTitle(QObject::tr("SomeClassWithArrowInMethods::f1"));
    }
    void f2();
    void f3()
    {
        mainWindow->setWindowTitle(tr("SomeClassWithArrowInMethods::f3"));
    }
    void f4();
};

SomeClassWithArrowInMethods::f2()
{
    mainWindow->setWindowTitle(QObject::tr("SomeClassWithArrowInMethods::f2"));
}

SomeClassWithArrowInMethods::f4()
{
    mainWindow->setWindowTitle(tr("SomeClassWithArrowInMethods::f4"));
}
