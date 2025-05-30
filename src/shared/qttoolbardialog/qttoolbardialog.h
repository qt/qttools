// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTTOOLBARDIALOG_H
#define QTTOOLBARDIALOG_H

#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class QMainWindow;
class QAction;
class QToolBar;

class QtToolBarManagerPrivate;

class QtToolBarManager : public QObject
{
    Q_OBJECT
public:

    explicit QtToolBarManager(QObject *parent = 0);
    ~QtToolBarManager();

    void setMainWindow(QMainWindow *mainWindow);
    QMainWindow *mainWindow() const;

    void addAction(QAction *action, const QString &category);
    void removeAction(QAction *action);

    void addToolBar(QToolBar *toolBar, const QString &category);
    void removeToolBar(QToolBar *toolBar);

    QList<QToolBar *> toolBars() const;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

private:

    friend class QtToolBarDialog;
    QScopedPointer<QtToolBarManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtToolBarManager)
    Q_DISABLE_COPY_MOVE(QtToolBarManager)
};

class QtToolBarDialogPrivate;

class QtToolBarDialog : public QDialog
{
    Q_OBJECT
public:

    explicit QtToolBarDialog(QWidget *parent = 0, Qt::WindowFlags flags = {});
    ~QtToolBarDialog();

    void setToolBarManager(QtToolBarManager *toolBarManager);

protected:

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:

    QScopedPointer<QtToolBarDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtToolBarDialog)
    Q_DISABLE_COPY_MOVE(QtToolBarDialog)
};

QT_END_NAMESPACE

#endif
