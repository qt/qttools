// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

























#include <QtCore>
class DClass
{

//: This is a comment, too.
 const char *c_noop_translate = QT_TRANSLATE_NOOP("context", "just a message");
  void func();
};
void DClass::func() {
//: commented
  qtTrId("lollipop");

//% "this is the source text"
//~ meta so-meta
//: even more commented
  qtTrId("lollipop");

//% "this is contradicting source text"
  qtTrId("lollipop");

//~ meta too-much-meta
  qtTrId("lollipop");



//~ meta so-meta
  auto a = QObject::tr("another message", "here with a lot of noise in the comment so it is long enough");

//~ meta too-much-meta
  auto aa = QObject::tr("another message", "here with a lot of noise in the comment so it is long enough"); // old parser: not picked up



//: commented
qtTrId("lollipop");
}
