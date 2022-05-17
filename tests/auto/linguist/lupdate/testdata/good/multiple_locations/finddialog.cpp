// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























#include <QtCore>


class AClass
{
    Q_OBJECT

    const char *c_noop_translate = QT_TRANSLATE_NOOP("context", "just a message");



//: This is one comment
    const char *c_noop_translate2 = QT_TRANSLATE_NOOP("context", "just a message");
};
