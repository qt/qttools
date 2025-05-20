// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QTranslator>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption localeOption("locale"_L1, "Locale to be used in the user interface"_L1,
                                    "locale"_L1);
    parser.addOption(localeOption);
    parser.addHelpOption();
    parser.process(app);

    if (parser.isSet(localeOption)) {
        QLocale locale(parser.value(localeOption));
        qInfo() << "Setting locale to" << locale.name();
        QLocale::setDefault(locale);
    }

    qInfo() << "Current locale:" << QLocale();
    qInfo() << "UI languages in locale:" << QLocale().uiLanguages().join(", "_L1);

    // Always install English plurals
    QTranslator enPlurals;
    const auto enPluralsPath = ":/i18n/clock_en.qm"_L1;
    if (!enPlurals.load(enPluralsPath))
        qFatal("Could not load %s!", qUtf8Printable(enPluralsPath));
    app.installTranslator(&enPlurals);

    // Load translation based on QLocale::uiLanguages()
    QTranslator translation;
    if (QLocale().language() != QLocale::English) {
        if (translation.load(QLocale(), "clock"_L1, "_"_L1, ":/i18n/"_L1)) {
            qInfo("Loading translation %s",
                  qUtf8Printable(QDir::toNativeSeparators(translation.filePath())));
            if (!app.installTranslator(&translation))
                qWarning("Could not install %s!",
                         qUtf8Printable(QDir::toNativeSeparators(translation.filePath())));
        } else {
            qInfo("Could not load translation to %s. Using English.",
                  qUtf8Printable(QLocale().name()));
        }
    }

    QQmlApplicationEngine engine;
    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            [](const QUrl &) { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("qtexamples.localizedclock", "Main");

    return app.exec();
}
