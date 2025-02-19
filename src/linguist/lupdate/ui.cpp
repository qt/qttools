// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lupdate.h"

#include <translator.h>
#include <xmlparser.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QXmlStreamReader>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class UiReader : public XmlParser
{
public:
    UiReader(Translator &translator, ConversionData &cd, QXmlStreamReader &reader)
        : XmlParser(reader),
          m_translator(translator),
          m_cd(cd),
          m_lineNumber(-1),
          m_isTrString(false),
          m_insideStringList(false)
    {
    }
    ~UiReader() override = default;

private:
    bool startElement(QStringView namespaceURI, QStringView localName,
                      QStringView qName, const QXmlStreamAttributes &atts) override;
    bool endElement(QStringView namespaceURI, QStringView localName,
                    QStringView qName) override;
    bool characters(QStringView ch) override;
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
    QString m_label;

    QString m_accum;
    int m_lineNumber;
    bool m_isTrString;
    bool m_insideStringList;
};

bool UiReader::startElement(QStringView namespaceURI, QStringView localName,
                            QStringView qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (qName == "string"_L1) {
        flush();
        if (!m_insideStringList)
            readTranslationAttributes(atts);
    } else if (qName == "stringlist"_L1) {
        flush();
        m_insideStringList = true;
        readTranslationAttributes(atts);
    }
    m_accum.clear();
    return true;
}

bool UiReader::endElement(QStringView namespaceURI, QStringView localName,
                          QStringView qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    m_accum.replace("\r\n"_L1, "\n"_L1);

    if (qName == "class"_L1) { // UI "header"
        if (m_context.isEmpty())
            m_context = m_accum;
    } else if (qName == "string"_L1 && m_isTrString) {
        m_source = m_accum;
    } else if (qName == "comment"_L1) { // FIXME: what's that?
        m_comment = m_accum;
        flush();
    } else if (qName == "stringlist"_L1) {
        m_insideStringList = false;
    } else {
        flush();
    }
    return true;
}

bool UiReader::characters(QStringView ch)
{
    m_accum += ch.toString();
    return true;
}

bool UiReader::fatalError(qint64 line, qint64 column, const QString &message)
{
    QString msg = QStringLiteral("XML error: Parse error at line %1, column %2 (%3).")
                          .arg(line)
                          .arg(column)
                          .arg(message);
    m_cd.appendError(msg);
    return false;
}

void UiReader::flush()
{
    if ((!m_context.isEmpty() || !m_id.isEmpty()) && !m_source.isEmpty()) {
        TranslatorMessage msg(m_context, m_source,
           m_comment, QString(), m_cd.m_sourceFileName,
           m_lineNumber, QStringList());
        msg.setExtraComment(m_extracomment);
        msg.setLabel(m_label);
        msg.setId(m_id);
        if (!m_id.isEmpty())
            msg.setContext({});
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
        m_id = atts.value(QStringLiteral("id")).toString();
        QString label = atts.value(QStringLiteral("label")).toString();
        if (!m_id.isEmpty())
            m_label = std::move(label);
        else if (!label.isEmpty())
            m_cd.appendError("%1:%2: labels cannot be used with text-based translation. "
                             "Ignoring\n"_L1.arg(m_cd.m_sourceFileName)
                                     .arg(m_lineNumber));
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
        cd.appendError("Cannot open %1: %2"_L1.arg(filename, file.errorString()));
        return false;
    }

    QXmlStreamReader reader(&file);
    reader.setNamespaceProcessing(false);

    UiReader uiReader(translator, cd, reader);
    bool result = uiReader.parse();
    if (!result)
        cd.appendError(u"Parse error in UI file"_s);
    return result;
}

QT_END_NAMESPACE
