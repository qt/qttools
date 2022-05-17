// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TICTACTOE_H
#define TICTACTOE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QRect;
class QSize;
QT_END_NAMESPACE

//! [0]
class TicTacToe : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state WRITE setState)

public:
    explicit TicTacToe(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void setState(const QString &newState);
    QString state() const;
    void clearBoard();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr char16_t Empty = '-';
    static constexpr char16_t Cross = 'X';
    static constexpr char16_t Nought = 'O';

    QRect cellRect(int position) const;
    int cellWidth() const { return width() / 3; }
    int cellHeight() const { return height() / 3; }

    QString myState;
    int turnNumber = 0;
};
//! [0]

#endif
