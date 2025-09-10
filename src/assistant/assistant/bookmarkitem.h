// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H

#include <QtCore/QVariant>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

enum {
    UserRoleUrl = Qt::UserRole + 50,
    UserRoleFolder = Qt::UserRole + 100,
    UserRoleExpanded = Qt::UserRole + 150
};

typedef QList<QVariant> DataVector;

class BookmarkItem
{
public:
    explicit BookmarkItem(const DataVector &data, BookmarkItem *parent = nullptr);
    ~BookmarkItem();

    BookmarkItem *parent() const;
    void setParent(BookmarkItem *parent);

    void addChild(BookmarkItem *child);
    BookmarkItem *child(int number) const;

    int childCount() const;
    int childNumber() const;

    QVariant data(int column) const;
    void setData(const DataVector &data);
    bool setData(int column, const QVariant &value);

    bool insertChildren(bool isFolder, int position, int count);
    bool removeChildren(int position, int count);

    void dumpTree(int indent) const;

private:
    DataVector m_data;

    BookmarkItem *m_parent;
    QList<BookmarkItem*> m_children;
};

QT_END_NAMESPACE

#endif // BOOKMARKITEM_H
