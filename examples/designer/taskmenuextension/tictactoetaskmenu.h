// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TICTACTOETASKMENU_H
#define TICTACTOETASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

QT_BEGIN_NAMESPACE
class QAction;
class QExtensionManager;
QT_END_NAMESPACE
class TicTacToe;

//! [0]
class TicTacToeTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    explicit TicTacToeTaskMenu(TicTacToe *tic, QObject *parent);

    QAction *preferredEditAction() const override;
    QList<QAction *> taskActions() const override;

private slots:
    void editState();

private:
    QAction *editStateAction;
    TicTacToe *ticTacToe;
};
//! [0]

//! [1]
class TicTacToeTaskMenuFactory : public QExtensionFactory
{
    Q_OBJECT

public:
    explicit TicTacToeTaskMenuFactory(QExtensionManager *parent = nullptr);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const override;
};
//! [1]

#endif
