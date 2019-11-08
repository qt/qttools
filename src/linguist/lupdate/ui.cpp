/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "lupdate.h"

#include <translator.h>
#include <xmlparser.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QXmlStreamReader>

QT_BEGIN_NAMESPACE

class UiReader : public XmlParser
{
public:
    UiReader(Translator &translator, ConversionData &cd, QXmlStreamReader &reader)
        : XmlParser(reader),
          m_translator(translator),
          m_cd(cd),
          m_lineNumber(-1),
          m_isTrString(false),
          m_insideStringList(false),
          m_idBasedTranslations(false)
    {
    }
    ~UiReader() override = default;

private:
    bool startElement(const QStringRef &namespaceURI, const QStringRef &localName,
                      const QStringRef &qName, const QXmlStreamAttributes &atts) override;
    bool endElement(const QStringRef &namespaceURI, const QStringRef &localName,
                    const QStringRef &qName) override;
    bool characters(const QStringRef &ch) override;
    bool fatalError(qint64 line, qint64 column, const QString &message) override;

    void flush();
    void readTranslationAttributes(const QXmlStreamAttributes &atts);

    Translator &m_translator;
    ConversionData &m_cd;
    QString m_context;
    QString m_source;
    QString m_comment;
    QString m_extracomment;
    QString m_id;

    QString m_accum;
    int m_lineNumber;
    bool m_isTrString;
    bool m_insideStringList;
    bool m_idBasedTranslations;
};

bool UiReader::startElement(const QStringRef &namespaceURI, const QStringRef &localName,
                            const QStringRef &qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (qName == QLatin1String("string")) {
        flush();
        if (!m_insideStringList)
            readTranslationAttributes(atts);
    } else if (qName == QLatin1String("stringlist")) {
        flush();
        m_insideStringList = true;
        readTranslationAttributes(atts);
    } else if (qName == QLatin1String("ui")) { // UI "header"
        const auto attr = QStringLiteral("idbasedtr");
        m_idBasedTranslations =
                atts.hasAttribute(attr) && atts.value(attr) == QLatin1String("true");
    }
    m_accum.clear();
    return true;
}

bool UiReader::endElement(const QStringRef &namespaceURI, const QStringRef &localName,
                          const QStringRef &qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    m_accum.replace(QLatin1String("\r\n"), QLatin1String("\n"));

    if (qName == QLatin1String("class")) { // UI "header"
        if (m_context.isEmpty())
            m_context = m_accum;
    } else if (qName == QLatin1String("string") && m_isTrString) {
        m_source = m_accum;
    } else if (qName == QLatin1String("comment")) { // FIXME: what's that?
        m_comment = m_accum;
        flush();
    } else if (qName == QLatin1String("stringlist")) {
        m_insideStringList = false;
    } else {
        flush();
    }
    return true;
}

bool UiReader::characters(const QStringRef &ch)
{
    m_accum += ch.toString();
    return true;
}

bool UiReader::fatalError(qint64 line, qint64 column, const QString &message)
{
    QString msg = LU::tr("XML error: Parse error at line %1, column %2 (%3).")
                          .arg(line)
                          .arg(column)
                          .arg(message);
    m_cd.appendError(msg);
    return false;
}

void UiReader::flush()
{
    if (!m_context.isEmpty() && !m_source.isEmpty()) {
        TranslatorMessage msg(m_context, m_source,
           m_comment, QString(), m_cd.m_sourceFileName,
           m_lineNumber, QStringList());
        msg.setExtraComment(m_extracomment);
        msg.setId(m_id);
        m_translator.extend(msg, m_cd);
    }
    m_source.clear();
    if (!m_insideStringList) {
        m_comment.clear();
        m_extracomment.clear();
        m_id.clear();
    }
}

void UiReader::readTranslationAttributes(const QXmlStreamAttributes &atts)
{
    const auto notr = atts.value(QStringLiteral("notr"));
    if (notr.isEmpty() || notr != QStringLiteral("true")) {
        m_isTrString = true;
        m_comment = atts.value(QStringLiteral("comment")).toString();
        m_extracomment = atts.value(QStringLiteral("extracomment")).toString();
        if (m_idBasedTranslations)
            m_id = atts.value(QStringLiteral("id")).toString();
        if (!m_cd.m_noUiLines)
            m_lineNumber = static_cast<int>(reader.lineNumber());
    } else {
        m_isTrString = false;
    }
}

bool loadUI(Translator &translator, const QString &filename, ConversionData &cd)
{
    cd.m_sourceFileName = filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        cd.appendError(LU::tr("Cannot open %1: %2").arg(filename, file.errorString()));
        return false;
    }

    QXmlStreamReader reader(&file);
    reader.setNamespaceProcessing(false);

    UiReader uiReader(translator, cd, reader);
    bool result = uiReader.parse();
    if (!result)
        cd.appendError(LU::tr("Parse error in UI file"));
    return result;
}

QT_END_NAMESPACE
