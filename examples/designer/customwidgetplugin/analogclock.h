// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANALOGCLOCK_H
#define ANALOGCLOCK_H

#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>

//! [0]
class QDESIGNER_WIDGET_EXPORT AnalogClock : public QWidget
{
    Q_OBJECT
//! [0]

public:
    explicit AnalogClock(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
//! [1]
};
//! [1]

#endif
