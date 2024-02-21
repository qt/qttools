// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Stephen Kelly <stephen.kelly@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QDebug>
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>

#include "some_include.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTranslator *myappTranslator = new QTranslator;
    QString localeName = QLocale::system().name();
    if (!myappTranslator->load("myobject_" + localeName + ".qm", qApp->applicationDirPath()))
        return 1;
    myappTranslator->setObjectName("myobject_" + localeName);
    app.installTranslator(myappTranslator);

    qDebug() << QObject::tr("Hello, world!");
    return 0;
}
