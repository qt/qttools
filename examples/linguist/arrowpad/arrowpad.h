// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ARROWPAD_H
#define ARROWPAD_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

//! [0]
class ArrowPad : public QWidget
//! [0] //! [1]
{
//! [1] //! [2]
    Q_OBJECT
//! [2]

public:
    ArrowPad(QWidget *parent = 0);

private:
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *leftButton;
    QPushButton *rightButton;
};

#endif
