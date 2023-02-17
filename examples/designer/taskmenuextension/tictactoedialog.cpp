// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tictactoe.h"
#include "tictactoedialog.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>

//! [0]
TicTacToeDialog::TicTacToeDialog(TicTacToe *tic, QWidget *parent)
    : QDialog(parent)
    , editor(new TicTacToe)
    , ticTacToe(tic)
    , buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel
                                     | QDialogButtonBox::Reset))
{
    editor->setState(ticTacToe->state());

    connect(buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked,
            this, &TicTacToeDialog::resetState);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TicTacToeDialog::saveState);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(editor);
    mainLayout->addWidget(buttonBox);

    setWindowTitle(tr("Edit State"));
}
//! [0]

//! [1]
QSize TicTacToeDialog::sizeHint() const
{
    return {250, 250};
}
//! [1]

//! [2]
void TicTacToeDialog::resetState()
{
    editor->clearBoard();
}
//! [2]

//! [3]
void TicTacToeDialog::saveState()
{
//! [3] //! [4]
    if (auto *formWindow = QDesignerFormWindowInterface::findFormWindow(ticTacToe))
        formWindow->cursor()->setProperty("state", editor->state());
//! [4] //! [5]
    accept();
}
//! [5]
