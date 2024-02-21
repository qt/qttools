// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include <QtCore>
// The first line in this file should always be empty, its part of the test!!
class Foo : public QObject
{
    Q_OBJECT
public:
    Foo();
};

Foo::Foo()
    : QObject()
{
    tr("This is the first entry.");
    tr("A second message."); tr("And a second one on the same line.");
    tr("This string did move from the bottom.");
    tr("This tr is new.");
    tr("This one moved in from another file.");
    tr("Now again one which is just where it was.");

    tr("Just as this one.");
    tr("Another alien.");
    tr("This is from the bottom, too.");
    tr("Third string from the bottom.");
    tr("Fourth one!");
    tr("They are coming!");
    tr("They are everywhere!");
    tr("An earthling again.");
}
