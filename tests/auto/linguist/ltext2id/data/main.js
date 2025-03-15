// Copyright (C) 2025 The Qt Company Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

qsTr("One");
qsTranslate("FooContext", "Two");

qsTr("One", "not the same one");

//: My first comment.
qsTr("See comment");

//: My second comment.
qsTranslate("BarContext", "See other comment");

//: My third comment
//: spans two lines.
qsTr("The comment explains it all");

//: My fourth comment
//: spans a whopping
//: three lines.
qsTranslate("BazContext", "It should be clear by now");

/*: C-style comment. */
qsTr("I love C++");

/*: Another C-style comment. */
qsTranslate("FooContext", "I really love C++");

/*: C-style comment, followed by */
/*: another one. */
qsTr("Qt is the best");

/*: Another C-style comment, followed by */
/*: yet another one. */
qsTranslate("BarContext", "Qt is the very best");

// This comment doesn't have any effect.
qsTr("The comment had no effect");

// This comment doesn't have any effect either.
qsTranslate("BazContext", "The comment had no effect, really");

/* This C-style comment doesn't have any effect. */
qsTr("No comment to your comment");

/* This C-style comment doesn't have any effect either. */
qsTranslate("FooContext", "I refuse to comment on that");

//~ meta-id id_foo
qsTr("This string has an identifier");

//~ meta-id id_bar
qsTranslate("BarContext", "This string also has an identifier");

//~ loc-blank False
qsTr("This string has meta-data");

//~ loc-layout_id foo_dialog
qsTranslate("BazContext", "This string also has meta-data");

// This comment is to be ignored.
//: This is a comment for the translator.
//~ meta-id id_baz
//~ foo 123
//~ magic-stuff This means something special.
qsTr("This string has a lot of information");

// This comment is also to be ignored.
//: This is another comment for the translator.
//~ meta-id id_babar
//~ foo-bar Important stuff
//~ needle-in-haystack Found
//~ overflow True
qsTranslate("FooContext", "This string has even more information");

qsTr("This string has disambiguation", "Disambiguation");

qsTranslate("BarContext", "This string also has disambiguation", "Another disambiguation");

qsTr("This string contains plurals", "", 10);

//% "source1"
qsTrId("qtn_foo_bar");

//: qsTrId() with comment, meta-data and plurals.
//~ well-tested True
//% "source2"
qsTrId("qtn_bar_baz", 10);

//~ quoted " string with spaces "
qsTr("translation with extras-quoted field");
//~ quoted "empty string"
qsTr("");
