/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QtCore/qglobal.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class XmlParser
{
public:
    XmlParser(QXmlStreamReader &r, bool whitespaceOnlyData = false)
        : reader(r), reportWhitespaceOnlyData(whitespaceOnlyData)
    {
    }
    virtual ~XmlParser() = default;

    bool parse();

protected:
    virtual bool startElement(QStringView namespaceURI, QStringView localName,
                              QStringView qName, const QXmlStreamAttributes &atts);
    virtual bool endElement(QStringView namespaceURI, QStringView localName,
                            QStringView qName);
    virtual bool characters(QStringView text);
    virtual bool endDocument();
    virtual bool fatalError(qint64 line, qint64 column, const QString &message);

    QXmlStreamReader &reader;
    bool reportWhitespaceOnlyData;
};

QT_END_NAMESPACE

#endif // XMLPARSER_H
