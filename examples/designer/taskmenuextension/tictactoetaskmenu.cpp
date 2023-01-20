// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "tictactoe.h"
#include "tictactoedialog.h"
#include "tictactoetaskmenu.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QAction>

//! [0]
TicTacToeTaskMenu::TicTacToeTaskMenu(TicTacToe *tic, QObject *parent)
    : QObject(parent)
    , editStateAction(new QAction(tr("Edit State..."), this))
    , ticTacToe(tic)
{
    connect(editStateAction, &QAction::triggered, this, &TicTacToeTaskMenu::editState);
}
//! [0]

//! [1]
void TicTacToeTaskMenu::editState()
{
    TicTacToeDialog dialog(ticTacToe);
    dialog.exec();
}
//! [1]

//! [2]
QAction *TicTacToeTaskMenu::preferredEditAction() const
{
    return editStateAction;
}
//! [2]

//! [3]
QList<QAction *> TicTacToeTaskMenu::taskActions() const
{
    return QList<QAction *>{editStateAction};
}
//! [3]

//! [4]
TicTacToeTaskMenuFactory::TicTacToeTaskMenuFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}
//! [4]

//! [5]
QObject *TicTacToeTaskMenuFactory::createExtension(QObject *object,
                                                   const QString &iid,
                                                   QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return nullptr;

    if (auto *tic = qobject_cast<TicTacToe*>(object))
        return new TicTacToeTaskMenu(tic, parent);

    return nullptr;
}
//! [5]
