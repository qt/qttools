// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef XBELSUPPORT_H
#define XBELSUPPORT_H

#include <QtCore/QXmlStreamReader>
#include <QtCore/QPersistentModelIndex>

QT_FORWARD_DECLARE_CLASS(QIODevice)
QT_FORWARD_DECLARE_CLASS(QModelIndex)

QT_BEGIN_NAMESPACE

class BookmarkModel;

class XbelWriter : public QXmlStreamWriter
{
public:
    XbelWriter(BookmarkModel *model);
    void writeToFile(QIODevice *device);

private:
    void writeData(const QModelIndex &index);

private:
    BookmarkModel *bookmarkModel;
};

class XbelReader : public QXmlStreamReader
{
public:
    XbelReader(BookmarkModel *model);
    bool readFromFile(QIODevice *device);

private:
    void readXBEL();
    void readFolder();
    void readBookmark();
    void readUnknownElement();

private:
    BookmarkModel *bookmarkModel;
    QList<QPersistentModelIndex> parents;
};

QT_END_NAMESPACE

#endif  // XBELSUPPORT_H
