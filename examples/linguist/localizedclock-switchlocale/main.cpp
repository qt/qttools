// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            [](const QUrl &) { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("qtexamples.localizedclockswitchlocale", "Main");
    return app.exec();
}
