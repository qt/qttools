// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























// IMPORTANT!!!! If you want to add testdata to this file,
// always add it to the end in order to not change the linenumbers of translations!!!

#define QTCORE <QtCore>
#include QTCORE // Hidden from lupdate, but compiles

class A: public QObject {
    Q_OBJECT
    void foo()
    {
        // same text match
        tr("this is the matched same text");

        // failed same text
        tr("this is the non-matched same text");
    }
};

