// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QDebug>
#include "lib1.h"
#include "lib2.h"
#include "subdir2/excluded4.cpp"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    auto obj1 = new MyObject1(&app);
    auto obj2 = new MyObject2(&app);
    qDebug() << QCoreApplication::translate("app1", "Hello from app1!")
             << obj1->greeting()
             << obj2->greeting()
             << WossName::greeting();
    return app.exec();
}
