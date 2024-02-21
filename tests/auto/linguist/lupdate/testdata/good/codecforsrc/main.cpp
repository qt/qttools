// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include <QtWidgets/QApplication>
#include <QtGui>
#include <QtCore>
#include <QtWidgets/QLabel>
#include <QtWidgets/QBoxLayout>
int main(int argc, char **argv)
{
        QApplication a(argc, argv);


        QWidget w;
        QLabel label1(QObject::tr("abc", "ascii"), &w);
        QLabel label2(QObject::tr("æøå", "utf-8"), &w);
        QLabel label2a(QObject::tr("\303\246\303\270\303\245", "utf-8 oct"), &w);
        QLabel label3(QObject::tr("Für Élise", "trUtf8"), &w); // trUtf8 is obsolete
        QLabel label3a(QObject::tr("F\303\274r \303\211lise", "trUtf8 oct"), &w); // trUtf8 is obsolete

        QBoxLayout *ly = new QVBoxLayout(&w);
        ly->addWidget(&label1);
        ly->addWidget(&label2);
        ly->addWidget(&label2a);
        ly->addWidget(&label3);
        ly->addWidget(&label3a);

        w.show();
        return a.exec();
}
