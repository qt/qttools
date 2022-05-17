// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Stephen Kelly <stephen.kelly@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

#include "some_include.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTranslator *myappTranslator = new QTranslator;

    QLocale::setDefault(QLocale("de"));

    if (!myappTranslator->load(QLocale(), "myobject", "_", qApp->applicationDirPath()))
        qFatal("Could not load translation file!");

    app.installTranslator(myappTranslator);

    QString text = QCoreApplication::translate("main", "Hello, world!");
    if (text != QLatin1String("Hallo, Welt!"))
        qFatal("Translation not found!");

    std::fprintf(stdout, "%s\n", qPrintable(text));
    return 0;
}
