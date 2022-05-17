// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OPENPAGESWIDGET_H
#define OPENPAGESWIDGET_H

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QTreeView>

QT_BEGIN_NAMESPACE

class OpenPagesModel;

class OpenPagesDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit OpenPagesDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    mutable QModelIndex pressedIndex;
};

class OpenPagesWidget : public QTreeView
{
    Q_OBJECT
public:
    OpenPagesWidget(OpenPagesModel *model);
    ~OpenPagesWidget() override;

    void selectCurrentPage();
    void allowContextMenu(bool ok);

signals:
    void setCurrentPage(const QModelIndex &index);
    void closePage(const QModelIndex &index);
    void closePagesExcept(const QModelIndex &index);

private slots:
    void contextMenuRequested(QPoint pos);
    void handlePressed(const QModelIndex &index);
    void handleClicked(const QModelIndex &index);

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool m_allowContextMenu;
    OpenPagesDelegate *m_delegate;
};

QT_END_NAMESPACE

#endif // OPENPAGESWIDGET_H
