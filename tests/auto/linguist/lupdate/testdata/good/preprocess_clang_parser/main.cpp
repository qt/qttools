// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























// IMPORTANT!!!! If you want to add testdata to this file,
// always add it to the end in order to not change the linenumbers of translations!!!
#include <QtWidgets/QApplication>

void func1() {
    QApplication::tr("Hello world", "Platform-independent file");
}




void func2() {
#ifdef Q_OS_WIN
    QApplication::tr("KindType", "The other option has been skipped");
#else
    QApplication::tr("KindType", "The other option has been skipped");
#endif

}



void stringconcatenation()
{
    QApplication::tr("One string,"
    " three"
    " lines");

    QApplication::tr("a backslash followed by newline \
should be ignored \
and the next line should be syntactically considered to be \
on the same line");

}
