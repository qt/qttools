// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























// IMPORTANT!!!! If you want to add testdata to this file,
// always add it to the end in order to not change the linenumbers of translations!!!
#include <QtWidgets/QDialog>
class Dialog2 : public QDialog
{
    Q_OBJECT
    void func();

};

void Dialog2::func()
{
    tr("cat\xc3\xa9gorie");

    tr("F\xc3\xbcr \xc3\x88lise");
}


