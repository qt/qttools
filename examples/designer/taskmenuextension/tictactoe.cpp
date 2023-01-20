// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tictactoe.h"

#include <QMouseEvent>
#include <QPainter>

using namespace Qt::StringLiterals;

static inline QString defaultState() { return u"---------"_s; }

TicTacToe::TicTacToe(QWidget *parent)
    : QWidget(parent), myState(defaultState())
{
}

QSize TicTacToe::minimumSizeHint() const
{
    return {200, 200};
}

QSize TicTacToe::sizeHint() const
{
    return {200, 200};
}

void TicTacToe::setState(const QString &newState)
{
    turnNumber = 0;
    myState = defaultState();
    const int count = qMin(9, int(newState.length()));
    for (int position = 0; position < count; ++position) {
        const QChar mark = newState.at(position);
        if (mark == Cross || mark == Nought) {
            ++turnNumber;
            myState[position] = mark;
        }
    }
    update();
}

QString TicTacToe::state() const
{
    return myState;
}

void TicTacToe::clearBoard()
{
    myState = defaultState();
    turnNumber = 0;
    update();
}

void TicTacToe::mousePressEvent(QMouseEvent *event)
{
    if (turnNumber == 9) {
        clearBoard();
        return;
    }
    for (int position = 0; position < 9; ++position) {
        const QRect &cell = cellRect(position);
        if (cell.contains(event->position().toPoint())) {
            if (myState.at(position) == Empty) {
                myState[position] = turnNumber % 2 == 0
                                    ? Cross : Nought;
                ++turnNumber;
                update();
            }
        }
    }
}

void TicTacToe::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::darkGreen, 1));
    painter.drawLine(cellWidth(), 0, cellWidth(), height());
    painter.drawLine(2 * cellWidth(), 0, 2 * cellWidth(), height());
    painter.drawLine(0, cellHeight(), width(), cellHeight());
    painter.drawLine(0, 2 * cellHeight(), width(), 2 * cellHeight());

    painter.setPen(QPen(Qt::darkBlue, 2));

    for (int position = 0; position < 9; ++position) {
        const QRect &cell = cellRect(position);

        if (myState.at(position) == Cross) {
            painter.drawLine(cell.topLeft(), cell.bottomRight());
            painter.drawLine(cell.topRight(), cell.bottomLeft());
        } else if (myState.at(position) == Nought) {
            painter.drawEllipse(cell);
        }
    }

    painter.setPen(QPen(Qt::yellow, 3));

    for (int position = 0; position < 9; position = position + 3) {
        if (myState.at(position) != Empty
                && myState.at(position + 1) == myState.at(position)
                && myState.at(position + 2) == myState.at(position)) {
            const int y = cellRect(position).center().y();
            painter.drawLine(0, y, width(), y);
            turnNumber = 9;
        }
    }

    for (int position = 0; position < 3; ++position) {
        if (myState.at(position) != Empty
                && myState.at(position + 3) == myState.at(position)
                && myState.at(position + 6) == myState.at(position)) {
            const int x = cellRect(position).center().x();
            painter.drawLine(x, 0, x, height());
            turnNumber = 9;
        }
    }
    if (myState.at(0) != Empty && myState.at(4) == myState.at(0)
            && myState.at(8) == myState.at(0)) {
        painter.drawLine(0, 0, width(), height());
        turnNumber = 9;
    }
    if (myState.at(2) != Empty && myState.at(4) == myState.at(2)
            && myState.at(6) == myState.at(2)) {
        painter.drawLine(0, height(), width(), 0);
        turnNumber = 9;
    }
}

QRect TicTacToe::cellRect(int position) const
{
    const int HMargin = width() / 30;
    const int VMargin = height() / 30;
    const int row = position / 3;
    const int column = position % 3;
    const QPoint pos{column * cellWidth() + HMargin,
                     row * cellHeight() + VMargin};
    const QSize size{cellWidth() - 2 * HMargin,
                     cellHeight() - 2 * VMargin};
    return {pos, size};
}
