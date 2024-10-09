// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QObject>
#include <QTranslator>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    std::cout << "Loading German translation..." << std::endl;
    QTranslator translator;
    if (translator.load(":/i18n/myapp_de.qm")) {
        app.installTranslator(&translator);
    } else {
        std::cerr << "Cannot load .qm file." << std::endl;
        return 1;
    }

    std::cout << "Checking translation for context 'myapp'..." << std::endl;
    if (QCoreApplication::translate("myapp", "message from the application")
        != "Nachricht von der Anwendung") {
        std::cerr << "The app's translation doesn't work. This is surprising." << std::endl;
        return 2;
    }

    std::cout << "Checking translation for context 'qtbase'..." << std::endl;
    if (QCoreApplication::translate("qtbase", "message from qtbase")
        != "Nachricht von qtbase") {
        std::cerr << "The qtbase translation catalog has not been merged into the app's .qm file."
                  << std::endl;
        return 3;
    }

    std::cout << "The test was successful." << std::endl;
    return 0;
}
