// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "tracer.h"

#include "xbelsupport.h"

#include "bookmarkitem.h"
#include "bookmarkmodel.h"

#include <QtCore/QDate>
#include <QtCore/QModelIndex>

QT_BEGIN_NAMESPACE

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
    writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writeStartElement(QLatin1String("xbel"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));

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
            writeStartElement(QLatin1String("folder"));
            entry.folded = !index.data(UserRoleExpanded).toBool();
            writeAttribute(QLatin1String("folded"), entry.folded
                ? QLatin1String("yes") : QLatin1String("no"));
            writeTextElement(QLatin1String("title"), entry.title);

            for (int i = 0; i < bookmarkModel->rowCount(index); ++i)
                writeData(bookmarkModel->index(i, 0 , index));
            writeEndElement();
        } else {
            writeStartElement(QLatin1String("bookmark"));
            writeAttribute(QLatin1String("href"), entry.url);
            writeTextElement(QLatin1String("title"), entry.title);
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
            if (name() == QLatin1String("xbel")
                && attributes().value(QLatin1String("version"))
                    == QLatin1String("1.0")) {
                const QModelIndex root;
                parents.append(bookmarkModel->addItem(root, true));
                readXBEL();
                bookmarkModel->setData(parents.first(),
                    QDate::currentDate().toString(Qt::ISODate), Qt::EditRole);
            } else {
                raiseError(QLatin1String("The file is not an XBEL version 1.0 file."));
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
            if (name() == QLatin1String("folder"))
                readFolder();
            else if (name() == QLatin1String("bookmark"))
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
    bookmarkModel->setData(parents.last(),
        attributes().value(QLatin1String("folded")) == QLatin1String("no"),
        UserRoleExpanded);

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == QLatin1String("title")) {
                bookmarkModel->setData(parents.last(), readElementText(),
                    Qt::EditRole);
            } else if (name() == QLatin1String("folder"))
                readFolder();
            else if (name() == QLatin1String("bookmark"))
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
    if (BookmarkItem* item = bookmarkModel->itemFromIndex(index)) {
        item->setData(UserRoleUrl, attributes().value(QLatin1String("href"))
            .toString());
    }

    while (!atEnd()) {
        readNext();

        if (isEndElement())
            break;

        if (isStartElement()) {
            if (name() == QLatin1String("title"))
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
