// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PRINTPANEL_H
#define PRINTPANEL_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QGroupBox;
class QRadioButton;
QT_END_NAMESPACE

//! [0]
class PrintPanel : public QWidget
{
    Q_OBJECT
//! [0]

public:
    PrintPanel(QWidget *parent = 0);

private:
    QGroupBox *twoSidedGroupBox;
    QGroupBox *colorsGroupBox;
    QRadioButton *twoSidedEnabledRadio;
    QRadioButton *twoSidedDisabledRadio;
    QRadioButton *colorsEnabledRadio;
    QRadioButton *colorsDisabledRadio;
};

#endif
