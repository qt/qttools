// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"

#include "xbelsupport.h"

#include "bookmarkitem.h"
#include "bookmarkmodel.h"

#include <QtCore/QDate>
#include <QtCore/QModelIndex>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

struct Bookmark {
    QString title;
    QString url;
    bool folded;
};

XbelWriter::XbelWriter(BookmarkModel *model)
    : QXmlStreamWriter()
    , bookmarkModel(model)
{
    TRACE_OBJ
    setAutoFormatting(true);
}

void XbelWriter::writeToFile(QIODevice *device)
{
    TRACE_OBJ
    setDevice(device);

    writeStartDocument();
    writeDTD("<!DOCTYPE xbel>"_L1);
    writeStartElement("xbel"_L1);
    writeAttribute("version"_L1, "1.0"_L1);

    const QModelIndex root;
    for (int i = 0; i < bookmarkModel->rowCount(root); ++i)
        writeData(bookmarkModel->index(i, 0, root));
    writeEndDocument();
}

void XbelWriter::writeData(const QModelIndex &index)
{
    TRACE_OBJ
    if (index.isValid()) {
        Bookmark entry;
        entry.title = index.data().toString();
        entry.url = index.data(UserRoleUrl).toString();

        if (index.data(UserRoleFolder).toBool()) {
            writeStartElement("folder"_L1);
            entry.folded = !index.data(UserRoleExpanded).toBool();
            writeAttribute("folded"_L1, entry.folded ? "yes"_L1 : "no"_L1);
            writeTextElement("title"_L1, entry.title);
            for (int i = 0; i < bookmarkModel->rowCount(index); ++i)
                writeData(bookmarkModel->index(i, 0 , index));
            writeEndElement();
        } else {
            writeStartElement("bookmark"_L1);
            writeAttribute("href"_L1, entry.url);
            writeTextElement("title"_L1, entry.title);
            writeEndElement();
        }
    }
}

// -- XbelReader

XbelReader::XbelReader(BookmarkModel *model)
    : QXmlStreamReader()
    , bookmarkModel(model)
{
    TRACE_OBJ
}

bool XbelReader::readFromFile(QIODevice *device)
{
    TRACE_OBJ
    setDevice(device);

    while (!atEnd()) {
        readNext();

        if (isStartElement()) {
            if (name() == "xbel"_L1 && attributes().value("version"_L1) == "1.0"_L1) {
                const QModelIndex root;
                parents.append(bookmarkModel->addItem(root, true));
                readXBEL();
                bookmarkModel->setData(parents.first(),
                    QDate::currentDate().toString(Qt::ISODate), Qt::EditRole);
            } else {
                raiseError("The file is not an XBEL version 1.0 file."_L1);
            }
        }
    }

    return !error();
}

void XbelReader::readXBEL()
{
    TRACE_OBJ
    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == "folder"_L1)
                readFolder();
            else if (name() == "bookmark"_L1)
                readBookmark();
            else
                readUnknownElement();
        }
    }
}

void XbelReader::readFolder()
{
    TRACE_OBJ
    parents.append(bookmarkModel->addItem(parents.last(), true));
    bookmarkModel->setData(parents.last(), attributes().value("folded"_L1) == "no"_L1,
                           UserRoleExpanded);

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == "title"_L1)
                bookmarkModel->setData(parents.last(), readElementText(), Qt::EditRole);
            else if (name() == "folder"_L1)
                readFolder();
            else if (name() == "bookmark"_L1)
                readBookmark();
            else
                readUnknownElement();
        }
    }

    parents.removeLast();
}

void XbelReader::readBookmark()
{
    TRACE_OBJ
    const QModelIndex &index = bookmarkModel->addItem(parents.last(), false);
    if (BookmarkItem* item = bookmarkModel->itemFromIndex(index))
        item->setData(UserRoleUrl, attributes().value("href"_L1).toString());

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == "title"_L1)
                bookmarkModel->setData(index, readElementText(), Qt::EditRole);
            else
                readUnknownElement();
        }
    }
}

void XbelReader::readUnknownElement()
{
    TRACE_OBJ
    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement())
            readUnknownElement();
    }
}

QT_END_NAMESPACE
