// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>
#include <QtUiTools>

#include "mywidget.h"

using namespace Qt::StringLiterals;

//! [0]
QWidget *loadCustomWidget(const QString &className, QWidget *parent)
{
    QUiLoader loader;
    QStringList availableWidgets = loader.availableWidgets();

    if (!availableWidgets.contains(className)) {
        qWarning() << "Cannot create widget" << className;
        return nullptr;
    }

    return loader.createWidget(className, parent);
}
//! [0]

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    widget.show();

    if (QWidget *customWidget = loadCustomWidget("AnalogClock"_L1, nullptr))
        customWidget->show();
    return app.exec();
}
