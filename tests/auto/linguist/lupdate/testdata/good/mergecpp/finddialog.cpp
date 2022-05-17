// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0




























#include <QtCore>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMainWindow>

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog(QMainWindow *parent);
    void reset();
};
FindDialog::FindDialog(QMainWindow *parent)
    : QDialog(parent)
{
    QString trans = tr("Enter the text you want to find.");
    trans = tr("Search reached end of the document");
    trans = tr("Search reached start of the document");
    trans = tr( "Text not found" );
}

void FindDialog::reset()
{
    tr("%n item(s)", "merge from singular to plural form", 4);
    tr("%n item(s)", "merge from a finished singular form to an unfinished plural form", 4);


    //~ meta matter
    //% "Hello"
    qtTrId("xx_hello");

    //% "New world"
    qtTrId("xx_world");


    //= new_id
    tr("this is just some text");


    //: A message without source string
    qtTrId("qtn_virtual");
}
