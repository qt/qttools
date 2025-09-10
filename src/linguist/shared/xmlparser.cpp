// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
