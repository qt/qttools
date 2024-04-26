// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QCoreApplication>
#include <QtCore/QTranslator>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QString qmFile = ":/i18n/app1_de.qm";
    std::cout << "Loading translation '" << qPrintable(qmFile) << "'..." << std::endl;
    QTranslator *translator = new QTranslator(&app);
    if (!translator->load(qmFile)) {
        std::cerr << "Error: failed to load the translation" << std::endl;
        return 1;
    }
    app.installTranslator(translator);

    std::cout << "Checking translated text..." << std::endl;
    const QString translated = QCoreApplication::translate("main", "Hello from main!");
    if (!translated.startsWith("Hallo aus")) {
        std::cerr << "Error: translation doesn't seem to work. "
                  << "The translated text is '" << qPrintable(translated) << "'" << std::endl;
        return 2;
    }

    std::cout << "All good." << std::endl;
    return 0;
}
