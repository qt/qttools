// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


























#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::translate("text/c++", "test");
    return app.exec();
}
