// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TEXTFINDER_H
#define TEXTFINDER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QTextEdit;
QT_END_NAMESPACE

//! [0]
class TextFinder : public QWidget
{
    Q_OBJECT

public:
    explicit TextFinder(QWidget *parent = nullptr);

private slots:
    void on_findButton_clicked();

private:
    QPushButton *ui_findButton;
    QTextEdit *ui_textEdit;
    QLineEdit *ui_lineEdit;
};
//! [0]

#endif // TEXTFINDER_H
