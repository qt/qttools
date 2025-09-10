// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>

#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE

class BookmarkItem;
class QMimeData;
class QTreeView;

typedef QMap<BookmarkItem*, QPersistentModelIndex> ItemModelIndexCache;

class BookmarkModel : public QAbstractItemModel
{
     Q_OBJECT
public:
    BookmarkModel();
    ~BookmarkModel() override;

    QByteArray bookmarks() const;
    void setBookmarks(const QByteArray &bookmarks);

    void setItemsEditable(bool editable);
    void expandFoldersIfNeeeded(QTreeView *treeView);

    QModelIndex addItem(const QModelIndex &parent, bool isFolder = false);
    bool removeItem(const QModelIndex &index);

    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    int columnCount(const QModelIndex &index = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &index) const override;

    Qt::DropActions supportedDropActions () const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    void setData(const QModelIndex &index, const QList<QVariant> &data);
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QModelIndex indexFromItem(BookmarkItem *item) const;
    BookmarkItem *itemFromIndex(const QModelIndex &index) const;
    QList<QPersistentModelIndex> indexListFor(const QString &label) const;

    bool insertRows(int position, int rows, const QModelIndex &parent) override;
    bool removeRows(int position, int rows, const QModelIndex &parent) override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
        int column, const QModelIndex &parent) override;

private:
    void setupCache(const QModelIndex &parent);
    QModelIndexList collectItems(const QModelIndex &parent) const;
    void collectItems(const QModelIndex &parent, qint32 depth,
        QDataStream *stream) const;

private:
    bool m_folder;
    bool m_editable;
    QIcon folderIcon;
    QIcon bookmarkIcon;
    BookmarkItem *rootItem;
    ItemModelIndexCache cache;
};

QT_END_NAMESPACE

#endif  // BOOKMARKMODEL_H
