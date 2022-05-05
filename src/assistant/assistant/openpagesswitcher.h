/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Assistant module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef OPENPAGESSWITCHER_H
#define OPENPAGESSWITCHER_H

#include <QtWidgets/QFrame>

QT_BEGIN_NAMESPACE

class OpenPagesModel;
class OpenPagesWidget;
class QModelIndex;

class OpenPagesSwitcher : public QFrame
{
    Q_OBJECT

public:
    OpenPagesSwitcher(OpenPagesModel *model);
    ~OpenPagesSwitcher() override;

    void gotoNextPage();
    void gotoPreviousPage();

    void selectAndHide();
    void selectCurrentPage();

    void setVisible(bool visible) override;
    void focusInEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

signals:
    void closePage(const QModelIndex &index);
    void setCurrentPage(const QModelIndex &index);

private:
    void selectPageUpDown(int summand);

private:
    OpenPagesModel *m_openPagesModel;
    OpenPagesWidget *m_openPagesWidget;
};

QT_END_NAMESPACE

#endif  // OPENPAGESSWITCHER_H
