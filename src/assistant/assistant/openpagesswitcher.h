// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
