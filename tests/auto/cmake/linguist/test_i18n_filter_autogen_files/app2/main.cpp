// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QCoreApplication>

#include "ui_mainwindow.h"

class MyObject : public QObject
{
    Q_OBJECT
public:
    MyObject(QObject *parent = nullptr)
        : QObject(parent)
    {
        qDebug() << tr("Hello from app2!");
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    MyObject obj;
    Ui::MainWindow wnd;
    app.exec();
}

#include "main.moc"
