// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TICTACTOEDIALOG_H
#define TICTACTOEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
QT_END_NAMESPACE
class TicTacToe;

//! [0]
class TicTacToeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TicTacToeDialog(TicTacToe *plugin = nullptr, QWidget *parent = nullptr);

    QSize sizeHint() const override;

private slots:
    void resetState();
    void saveState();

private:
    TicTacToe *editor;
    TicTacToe *ticTacToe;
    QDialogButtonBox *buttonBox;
};
//! [0]

#endif
