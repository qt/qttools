/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
