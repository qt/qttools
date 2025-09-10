// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include "ui_bookmarkdialog.h"

QT_BEGIN_NAMESPACE

class BookmarkModel;
class BookmarkFilterModel;
class BookmarkTreeModel;

class BookmarkDialog : public QDialog
{
    Q_OBJECT
public:
    BookmarkDialog(BookmarkModel *bookmarkModel, const QString &title,
        const QString &url, QWidget *parent = nullptr);
    ~BookmarkDialog() override;

private:
    bool isRootItem(const QModelIndex &index) const;
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void currentIndexChanged(int index);
    void currentIndexChanged(const QModelIndex &index);

    void accepted();
    void rejected();

    void addFolder();
    void toolButtonClicked();
    void textChanged(const QString& text);
    void customContextMenuRequested(const QPoint &point);

private:
    QString m_url;
    QString m_title;
    Ui::BookmarkDialog ui;
    QList<QPersistentModelIndex> cache;

    BookmarkModel *bookmarkModel;
    BookmarkTreeModel *bookmarkTreeModel;
    BookmarkFilterModel *bookmarkProxyModel;
};

QT_END_NAMESPACE

#endif  // BOOKMARKDIALOG_H
