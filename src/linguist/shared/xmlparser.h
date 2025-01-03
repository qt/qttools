// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
