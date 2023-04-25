// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Stephen Kelly <stephen.kelly@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>

#include <cstdio>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTranslator *translator = new QTranslator();
    QLocale frenchLocale = QLocale(QLocale::French);
    QLocale::setDefault(frenchLocale);
    if (!translator->load(QLocale(), "myobject.qm"_L1, {},
                          qApp->applicationDirPath() + "/fr/"_L1)) {
        qFatal("Could not load translation file!");
    }

    app.installTranslator(translator);

    QString text = QCoreApplication::translate("main", "Hello, world!");
    if (text != "Bonjour le monde!"_L1)
        qFatal("Translation not found!");

    std::fprintf(stdout, "%s\n", qPrintable(text));
    return 0;
}
