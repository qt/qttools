/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
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
