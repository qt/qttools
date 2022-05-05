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

#include "xmlparser.h"

QT_BEGIN_NAMESPACE

bool XmlParser::parse()
{
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            fatalError(reader.lineNumber(), reader.columnNumber(), reader.errorString());
            return false;
        }

        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            if (!startElement(reader.namespaceUri(), reader.name(), reader.qualifiedName(),
                              reader.attributes())) {
                return false;
            }
            break;
        case QXmlStreamReader::EndElement:
            if (!endElement(reader.namespaceUri(), reader.name(), reader.qualifiedName())) {
                return false;
            }
            break;
        case QXmlStreamReader::Characters:
            if (reportWhitespaceOnlyData
                || (!reader.isWhitespace() && !reader.text().toString().trimmed().isEmpty())) {
                if (!characters(reader.text()))
                    return false;
            }
            break;
        default:
            break;
        }
    }
    if (reader.isEndDocument() && !endDocument())
        return false;

    return true;
}

bool XmlParser::startElement(QStringView namespaceURI, QStringView localName,
                             QStringView qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    Q_UNUSED(qName);
    Q_UNUSED(atts);
    return true;
}

bool XmlParser::endElement(QStringView namespaceURI, QStringView localName,
                           QStringView qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    Q_UNUSED(qName);
    return true;
}

bool XmlParser::characters(QStringView text)
{
    Q_UNUSED(text);
    return true;
}

bool XmlParser::fatalError(qint64 line, qint64 column, const QString &message)
{
    Q_UNUSED(line);
    Q_UNUSED(column);
    Q_UNUSED(message);
    return true;
}

bool XmlParser::endDocument()
{
    return true;
}

QT_END_NAMESPACE
