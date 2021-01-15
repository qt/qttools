/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
****************************************************************************/
#include <QDebug>
#include "codegenerator.h"
using namespace CodeGenerator;

int main()
{
    // The code generator works on items. Each item has a generate() function:
    Item item("");
    qDebug() << item.generate(); // produces "".

    // There are several Item subclasses. Text items contains a text string which they
    // reproduce when generate is called:
    Text text(" Hi there");
    qDebug() << text.generate(); // produces " Hi there".

    // Items can be concatenated:
    Item sentence = text + Text(" Bye there") ;
    qDebug() << sentence.generate(); // produces "Hi there Bye there".
    // (Internally, this creates a tree of items, and generate is called recursively
    // for items that have children.)

    // Repeater items repeat their content when generate is called:
    Repeater repeater = text;
    repeater.setRepeatCount(3);
    qDebug() << repeater.generate(); // produces "Hi there Hi there Hi there".

    // Counters evaluate to the current repeat index.
    Repeater repeater2 = text + Counter();
    repeater2.setRepeatCount(3);
    qDebug() << repeater2.generate(); // produces "Hi there0 Hi there1 Hi there2".

    // Groups provide sub-groups which are repeated according to the current repeat index.
    // Counters inside Groups evaluate to the local repeat index for the Group.
    Group arguments("Arg" + Counter()  + " arg" + Counter());
    Repeater function("void foo(" + arguments + ");\n");
    function.setRepeatCount(3);
    qDebug() << function.generate();

    // Produces:
    // void foo(Arg1 arg1);
    // void foo(Arg1 arg1, Arg2 arg2);
    // void foo(Arg1 arg1, Arg2 arg2, Arg3 arg3);
}
