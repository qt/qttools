// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























.pragma library
.import QtQuick 2.0 as Quick2

qsTranslate();
qsTranslate(10);
qsTranslate(10, 20);
qsTranslate("10", 20);
qsTranslate(10, "20");

QT_TRANSLATE_NOOP();
QT_TRANSLATE_NOOP(10);
QT_TRANSLATE_NOOP(10, 20);
QT_TRANSLATE_NOOP("10", 20);
QT_TRANSLATE_NOOP(10, "20");

qsTr();
qsTr(10);

QT_TR_NOOP();
QT_TR_NOOP(10);

qsTrId();
qsTrId(10);

QT_TRID_NOOP();
QT_TRID_NOOP(10);

//% This is wrong
//% "This is not wrong" This is wrong
//% "I forgot to close the meta string
//% "Being evil \

//% "Should cause a warning"
qsTranslate("FooContext", "Hello");
//% "Should cause a warning"
QT_TRANSLATE_NOOP("FooContext", "World");
//% "Should cause a warning"
qsTr("Hello");
//% "Should cause a warning"
QT_TR_NOOP("World");

//: This comment will be discarded.
Math.sin(1);
//= id_foobar
Math.cos(2);
//~ underflow False
Math.tan(3);

/*
Not tested for now, because these should perhaps not cause
translation entries to be generated at all; see QTBUG-11843.

//= qtn_foo
qsTrId("qtn_foo");
//= qtn_bar
QT_TRID_NOOP("qtn_bar");
*/
