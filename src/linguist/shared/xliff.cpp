// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translator.h"
#include "xmlparser.h"

#include <QtCore/QDebug>
#include <QtCore/QMap>
#include <QtCore/QRegularExpression>
#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QTextStream>

// The string value is historical and reflects the main purpose: Keeping
// obsolete entries separate from the magic file message (which both have
// no location information, but typically reside at opposite ends of the file).
#define MAGIC_OBSOLETE_REFERENCE "Obsolete_PO_entries"

QT_BEGIN_NAMESPACE

/**
 * Implementation of XLIFF file format for Linguist
 */
//static const char *restypeDomain = "x-gettext-domain";
static const char *restypeContext = "x-trolltech-linguist-context";
static const char *restypePlurals = "x-gettext-plurals";
static const char *restypeDummy = "x-dummy";
static const char *dataTypeUIFile = "x-trolltech-designer-ui";
static const char *contextMsgctxt = "x-gettext-msgctxt"; // XXX Troll invention, so far.
static const char *contextOldMsgctxt = "x-gettext-previous-msgctxt"; // XXX Troll invention, so far.
static const char *attribPlural = "trolltech:plural";
static const char *XLIFF11namespaceURI = "urn:oasis:names:tc:xliff:document:1.1";
static const char *XLIFF12namespaceURI = "urn:oasis:names:tc:xliff:document:1.2";
static const char *TrollTsNamespaceURI = "urn:trolltech:names:ts:document:1.0";

#define COMBINE4CHARS(c1, c2, c3, c4) \
    (int(c1) << 24 | int(c2) << 16 | int(c3) << 8 | int(c4) )

static QString dataType(const TranslatorMessage &m)
{
    QByteArray fileName = m.fileName().toLatin1();
    unsigned int extHash = 0;
    int pos = fileName.size() - 1;
    for (int pass = 0; pass < 4 && pos >=0; ++pass, --pos) {
        if (fileName.at(pos) == '.')
            break;
        extHash |= ((int)fileName.at(pos) << (8*pass));
    }

    switch (extHash) {
        case COMBINE4CHARS(0,'c','p','p'):
        case COMBINE4CHARS(0,'c','x','x'):
        case COMBINE4CHARS(0,'c','+','+'):
        case COMBINE4CHARS(0,'h','p','p'):
        case COMBINE4CHARS(0,'h','x','x'):
        case COMBINE4CHARS(0,'h','+','+'):
            return QLatin1String("cpp");
        case COMBINE4CHARS(0, 0 , 0 ,'c'):
        case COMBINE4CHARS(0, 0 , 0 ,'h'):
        case COMBINE4CHARS(0, 0 ,'c','c'):
        case COMBINE4CHARS(0, 0 ,'c','h'):
        case COMBINE4CHARS(0, 0 ,'h','h'):
            return QLatin1String("c");
        case COMBINE4CHARS(0, 0 ,'u','i'):
            return QLatin1String(dataTypeUIFile);   //### form?
        default:
            return QLatin1String("plaintext");      // we give up
    }
}

static void writeIndent(QTextStream &ts, int indent)
{
    ts << QString().fill(QLatin1Char(' '), indent * 2);
}

struct CharMnemonic
{
    char ch;
    char escape;
    const char *mnemonic;
};

static const CharMnemonic charCodeMnemonics[] = {
    {0x07, 'a', "bel"},
    {0x08, 'b', "bs"},
    {0x09, 't', "tab"},
    {0x0a, 'n', "lf"},
    {0x0b, 'v', "vt"},
    {0x0c, 'f', "ff"},
    {0x0d, 'r', "cr"}
};

static char charFromEscape(char escape)
{
    for (uint i = 0; i < sizeof(charCodeMnemonics)/sizeof(CharMnemonic); ++i) {
        CharMnemonic cm =  charCodeMnemonics[i];
        if (cm.escape == escape)
            return cm.ch;
    }
    Q_ASSERT(0);
    return escape;
}

static QString xlNumericEntity(int ch, bool makePhs)
{
    // ### This needs to be reviewed, to reflect the updated XLIFF-PO spec.
    if (!makePhs || ch < 7 || ch > 0x0d)
        return QString::fromLatin1("&#x%1;").arg(QString::number(ch, 16));

    CharMnemonic cm = charCodeMnemonics[int(ch) - 7];
    QString name = QLatin1String(cm.mnemonic);
    char escapechar = cm.escape;

    static int id = 0;
    return QString::fromLatin1("<ph id=\"ph%1\" ctype=\"x-ch-%2\">\\%3</ph>")
              .arg(++id) .arg(name) .arg(escapechar);
}

static QString xlProtect(const QString &str, bool makePhs = true)
{
    QString result;
    int len = str.size();
    for (int i = 0; i != len; ++i) {
        uint c = str.at(i).unicode();
        switch (c) {
        case '\"':
            result += QLatin1String("&quot;");
            break;
        case '&':
            result += QLatin1String("&amp;");
            break;
        case '>':
            result += QLatin1String("&gt;");
            break;
        case '<':
            result += QLatin1String("&lt;");
            break;
        case '\'':
            result += QLatin1String("&apos;");
            break;
        default:
            if (c < 0x20 && c != '\r' && c != '\n' && c != '\t')
                result += xlNumericEntity(c, makePhs);
            else // this also covers surrogates
                result += QChar(c);
        }
    }
    return result;
}


static void writeExtras(QTextStream &ts, int indent,
                        const TranslatorMessage::ExtraData &extras, QRegularExpression drops)
{
    for (auto it = extras.cbegin(), end = extras.cend(); it != end; ++it) {
        if (!drops.match(it.key()).hasMatch()) {
            writeIndent(ts, indent);
            ts << "<trolltech:" << it.key() << '>'
               << xlProtect(it.value())
               << "</trolltech:" << it.key() << ">\n";
        }
    }
}

static void writeLineNumber(QTextStream &ts, const TranslatorMessage &msg, int indent)
{
    if (msg.lineNumber() == -1)
        return;
    writeIndent(ts, indent);
    ts << "<context-group purpose=\"location\"><context context-type=\"linenumber\">"
       << msg.lineNumber() << "</context></context-group>\n";
    const auto refs = msg.extraReferences();
    for (const TranslatorMessage::Reference &ref : refs) {
        writeIndent(ts, indent);
        ts << "<context-group purpose=\"location\">";
        if (ref.fileName() != msg.fileName())
            ts << "<context context-type=\"sourcefile\">" << ref.fileName() << "</context>";
        ts << "<context context-type=\"linenumber\">" << ref.lineNumber()
           << "</context></context-group>\n";
    }
}

static void writeComment(QTextStream &ts, const TranslatorMessage &msg, const QRegularExpression &drops, int indent)
{
    if (!msg.comment().isEmpty()) {
        writeIndent(ts, indent);
        ts << "<context-group><context context-type=\"" << contextMsgctxt << "\">"
           << xlProtect(msg.comment(), false)
           << "</context></context-group>\n";
    }
    if (!msg.oldComment().isEmpty()) {
        writeIndent(ts, indent);
        ts << "<context-group><context context-type=\"" << contextOldMsgctxt << "\">"
           << xlProtect(msg.oldComment(), false)
           << "</context></context-group>\n";
    }
    writeExtras(ts, indent, msg.extras(), drops);
    if (!msg.extraComment().isEmpty()) {
        writeIndent(ts, indent);
        ts << "<note annotates=\"source\" from=\"developer\">"
           << xlProtect(msg.extraComment()) << "</note>\n";
    }
    if (!msg.translatorComment().isEmpty()) {
        writeIndent(ts, indent);
        ts << "<note from=\"translator\">"
           << xlProtect(msg.translatorComment()) << "</note>\n";
    }
}

static void writeTransUnits(QTextStream &ts, const TranslatorMessage &msg, const QRegularExpression &drops, int indent)
{
    static int msgid;
    QString msgidstr = !msg.id().isEmpty() ? msg.id() : QString::fromLatin1("_msg%1").arg(++msgid);

    QStringList translns = msg.translations();
    QString pluralStr;
    QStringList sources(msg.sourceText());
    const auto &extras = msg.extras();
    const auto extrasEnd = extras.cend();
    if (const auto it = extras.constFind(QString::fromLatin1("po-msgid_plural")); it != extrasEnd)
        sources.append(*it);
    QStringList oldsources;
    if (!msg.oldSourceText().isEmpty())
        oldsources.append(msg.oldSourceText());
    if (const auto it = extras.constFind(QString::fromLatin1("po-old_msgid_plural")); it != extrasEnd) {
        if (oldsources.isEmpty()) {
            if (sources.size() == 2)
                oldsources.append(QString());
            else
                pluralStr = QLatin1Char(' ') + QLatin1String(attribPlural) + QLatin1String("=\"yes\"");
        }
        oldsources.append(*it);
    }

    auto srcit = sources.cbegin(), srcend = sources.cend(),
         oldsrcit = oldsources.cbegin(), oldsrcend = oldsources.cend(),
         transit = translns.cbegin(), transend = translns.cend();
    int plural = 0;
    QString source;
    while (srcit != srcend || oldsrcit != oldsrcend || transit != transend) {
        QByteArray attribs;
        QByteArray state;
        if ((msg.type() == TranslatorMessage::Obsolete
             || msg.type() == TranslatorMessage::Vanished)
            && !msg.isPlural()) {
            attribs = " translate=\"no\"";
        }
        if (msg.type() == TranslatorMessage::Finished
            || msg.type() == TranslatorMessage::Vanished) {
            attribs += " approved=\"yes\"";
        } else if (msg.type() == TranslatorMessage::Unfinished
                   && transit != transend && !transit->isEmpty()) {
            state = " state=\"needs-review-translation\"";
        }
        writeIndent(ts, indent);
        ts << "<trans-unit id=\"" << msgidstr;
        if (msg.isPlural())
            ts << "[" << plural++ << "]";
        ts << "\"" << attribs << ">\n";
        ++indent;

        writeIndent(ts, indent);
        if (srcit != srcend) {
            source = *srcit;
            ++srcit;
        } // else just repeat last element
        ts << "<source xml:space=\"preserve\">" << xlProtect(source) << "</source>\n";

        bool puttrans = false;
        QString translation;
        if (transit != transend) {
            translation = *transit;
            translation.replace(QChar(Translator::BinaryVariantSeparator),
                                QChar(Translator::TextVariantSeparator));
            ++transit;
            puttrans = true;
        }
        do {
            if (oldsrcit != oldsrcend && !oldsrcit->isEmpty()) {
                writeIndent(ts, indent);
                ts << "<alt-trans>\n";
                ++indent;
                writeIndent(ts, indent);
                ts << "<source xml:space=\"preserve\"" << pluralStr << '>' << xlProtect(*oldsrcit) << "</source>\n";
                if (!puttrans) {
                    writeIndent(ts, indent);
                    ts << "<target restype=\"" << restypeDummy << "\"/>\n";
                }
            }

            if (puttrans) {
                writeIndent(ts, indent);
                ts << "<target xml:space=\"preserve\"" << state << ">" << xlProtect(translation) << "</target>\n";
            }

            if (oldsrcit != oldsrcend) {
                if (!oldsrcit->isEmpty()) {
                    --indent;
                    writeIndent(ts, indent);
                    ts << "</alt-trans>\n";
                }
                ++oldsrcit;
            }

            puttrans = false;
        } while (srcit == srcend && oldsrcit != oldsrcend);

        if (!msg.isPlural()) {
            writeLineNumber(ts, msg, indent);
            writeComment(ts, msg, drops, indent);
        }

        --indent;
        writeIndent(ts, indent);
        ts << "</trans-unit>\n";
    }
}

static void writeMessage(QTextStream &ts, const TranslatorMessage &msg, const QRegularExpression &drops, int indent)
{
    if (msg.isPlural()) {
        writeIndent(ts, indent);
        ts << "<group restype=\"" << restypePlurals << "\"";
        if (!msg.id().isEmpty())
            ts << " id=\"" << msg.id() << "\"";
        if (msg.type() == TranslatorMessage::Obsolete || msg.type() == TranslatorMessage::Vanished)
            ts << " translate=\"no\"";
        ts << ">\n";
        ++indent;
        writeLineNumber(ts, msg, indent);
        writeComment(ts, msg, drops, indent);

        writeTransUnits(ts, msg, drops, indent);
        --indent;
        writeIndent(ts, indent);
        ts << "</group>\n";
    } else {
        writeTransUnits(ts, msg, drops, indent);
    }
}

class XLIFFHandler : public XmlParser
{
public:
    XLIFFHandler(Translator &translator, ConversionData &cd, QXmlStreamReader &reader);
    ~XLIFFHandler() override = default;

private:
    bool startElement(QStringView namespaceURI, QStringView localName,
                      QStringView qName, const QXmlStreamAttributes &atts) override;
    bool endElement(QStringView namespaceURI, QStringView localName,
                    QStringView qName) override;
    bool characters(QStringView ch) override;
    bool fatalError(qint64 line, qint64 column, const QString &message) override;

    bool endDocument() override;

    enum XliffContext {
        XC_xliff,
        XC_group,
        XC_trans_unit,
        XC_context_group,
        XC_context_group_any,
        XC_context,
        XC_context_filename,
        XC_context_linenumber,
        XC_context_context,
        XC_context_comment,
        XC_context_old_comment,
        XC_ph,
        XC_extra_comment,
        XC_translator_comment,
        XC_restype_context,
        XC_restype_translation,
        XC_restype_plurals,
        XC_alt_trans
    };
    void pushContext(XliffContext ctx);
    bool popContext(XliffContext ctx);
    XliffContext currentContext() const;
    bool hasContext(XliffContext ctx) const;
    bool finalizeMessage(bool isPlural);

private:
    Translator &m_translator;
    ConversionData &m_cd;
    QString m_language;
    QString m_sourceLanguage;
    QString m_context;
    QString m_id;
    QStringList m_sources;
    QStringList m_oldSources;
    QString m_comment;
    QString m_oldComment;
    QString m_extraComment;
    QString m_translatorComment;
    bool m_translate;
    bool m_approved;
    bool m_isPlural;
    bool m_hadAlt;
    QStringList m_translations;
    QString m_fileName;
    int     m_lineNumber;
    QString m_extraFileName;
    TranslatorMessage::References m_refs;
    TranslatorMessage::ExtraData m_extra;

    QString accum;
    QString m_ctype;
    const QString m_URITT;  // convenience and efficiency
    const QString m_URI;  // ...
    const QString m_URI12;  // ...
    QStack<int> m_contextStack;
};

XLIFFHandler::XLIFFHandler(Translator &translator, ConversionData &cd, QXmlStreamReader &reader)
    : XmlParser(reader, true),
      m_translator(translator),
      m_cd(cd),
      m_translate(true),
      m_approved(true),
      m_lineNumber(-1),
      m_URITT(QLatin1String(TrollTsNamespaceURI)),
      m_URI(QLatin1String(XLIFF11namespaceURI)),
      m_URI12(QLatin1String(XLIFF12namespaceURI))
{}


void XLIFFHandler::pushContext(XliffContext ctx)
{
    m_contextStack.push_back(ctx);
}

// Only pops it off if the top of the stack contains ctx
bool XLIFFHandler::popContext(XliffContext ctx)
{
    if (!m_contextStack.isEmpty() && m_contextStack.top() == ctx) {
        m_contextStack.pop();
        return true;
    }
    return false;
}

XLIFFHandler::XliffContext XLIFFHandler::currentContext() const
{
    if (!m_contextStack.isEmpty())
        return (XliffContext)m_contextStack.top();
    return XC_xliff;
}

// traverses to the top to check all of the parent contexes.
bool XLIFFHandler::hasContext(XliffContext ctx) const
{
    for (int i = m_contextStack.size() - 1; i >= 0; --i) {
        if (m_contextStack.at(i) == ctx)
            return true;
    }
    return false;
}

bool XLIFFHandler::startElement(QStringView namespaceURI, QStringView localName,
                                QStringView qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(qName);
    if (namespaceURI == m_URITT)
        goto bail;
    if (namespaceURI != m_URI && namespaceURI != m_URI12) {
        return fatalError(reader.lineNumber(), reader.columnNumber(),
                          QLatin1String("Unknown namespace in the XLIFF file"));
    }
    if (localName == QLatin1String("xliff")) {
        // make sure that the stack is not empty during parsing
        pushContext(XC_xliff);
    } else if (localName == QLatin1String("file")) {
        m_fileName = atts.value(QLatin1String("original")).toString();
        m_language = atts.value(QLatin1String("target-language")).toString();
        m_language.replace(QLatin1Char('-'), QLatin1Char('_'));
        m_sourceLanguage = atts.value(QLatin1String("source-language")).toString();
        m_sourceLanguage.replace(QLatin1Char('-'), QLatin1Char('_'));
        if (m_sourceLanguage == QLatin1String("en"))
            m_sourceLanguage.clear();
    } else if (localName == QLatin1String("group")) {
        if (atts.value(QLatin1String("restype")) == QLatin1String(restypeContext)) {
            m_context = atts.value(QLatin1String("resname")).toString();
            pushContext(XC_restype_context);
        } else {
            if (atts.value(QLatin1String("restype")) == QLatin1String(restypePlurals)) {
                pushContext(XC_restype_plurals);
                m_id = atts.value(QLatin1String("id")).toString();
                if (atts.value(QLatin1String("translate")) == QLatin1String("no"))
                    m_translate = false;
            } else {
                pushContext(XC_group);
            }
        }
    } else if (localName == QLatin1String("trans-unit")) {
        if (!hasContext(XC_restype_plurals) || m_sources.isEmpty() /* who knows ... */)
            if (atts.value(QLatin1String("translate")) == QLatin1String("no"))
                m_translate = false;
        if (!hasContext(XC_restype_plurals)) {
            m_id = atts.value(QLatin1String("id")).toString();
            if (m_id.startsWith(QLatin1String("_msg")))
                m_id.clear();
        }
        if (atts.value(QLatin1String("approved")) != QLatin1String("yes"))
            m_approved = false;
        pushContext(XC_trans_unit);
        m_hadAlt = false;
    } else if (localName == QLatin1String("alt-trans")) {
        pushContext(XC_alt_trans);
    } else if (localName == QLatin1String("source")) {
        m_isPlural = atts.value(QLatin1String(attribPlural)) == QLatin1String("yes");
    } else if (localName == QLatin1String("target")) {
        if (atts.value(QLatin1String("restype")) != QLatin1String(restypeDummy))
            pushContext(XC_restype_translation);
    } else if (localName == QLatin1String("context-group")) {
        if (atts.value(QLatin1String("purpose")) == QLatin1String("location"))
            pushContext(XC_context_group);
        else
            pushContext(XC_context_group_any);
    } else if (currentContext() == XC_context_group && localName == QLatin1String("context")) {
        const auto ctxtype = atts.value(QLatin1String("context-type"));
        if (ctxtype == QLatin1String("linenumber"))
            pushContext(XC_context_linenumber);
        else if (ctxtype == QLatin1String("sourcefile"))
            pushContext(XC_context_filename);
    } else if (currentContext() == XC_context_group_any && localName == QLatin1String("context")) {
        const auto ctxtype = atts.value(QLatin1String("context-type"));
        if (ctxtype == QLatin1String(contextMsgctxt))
            pushContext(XC_context_comment);
        else if (ctxtype == QLatin1String(contextOldMsgctxt))
            pushContext(XC_context_old_comment);
    } else if (localName == QLatin1String("note")) {
        if (atts.value(QLatin1String("annotates")) == QLatin1String("source") &&
            atts.value(QLatin1String("from")) == QLatin1String("developer"))
            pushContext(XC_extra_comment);
        else
            pushContext(XC_translator_comment);
    } else if (localName == QLatin1String("ph")) {
        QString ctype = atts.value(QLatin1String("ctype")).toString();
        if (ctype.startsWith(QLatin1String("x-ch-")))
            m_ctype = ctype.mid(5);
        pushContext(XC_ph);
    }
bail:
    if (currentContext() != XC_ph)
        accum.clear();
    return true;
}

bool XLIFFHandler::endElement(QStringView namespaceURI, QStringView localName,
                              QStringView qName)
{
    Q_UNUSED(qName);
    if (namespaceURI == m_URITT) {
        if (hasContext(XC_trans_unit) || hasContext(XC_restype_plurals))
            m_extra[localName.toString()] = accum;
        else
            m_translator.setExtra(localName.toString(), accum);
        return true;
    }
    if (namespaceURI != m_URI && namespaceURI != m_URI12) {
        return fatalError(reader.lineNumber(), reader.columnNumber(),
                          QLatin1String("Unknown namespace in the XLIFF file"));
    }
    //qDebug() << "URI:" <<  namespaceURI << "QNAME:" << qName;
    if (localName == QLatin1String("xliff")) {
        popContext(XC_xliff);
    } else if (localName == QLatin1String("source")) {
        if (hasContext(XC_alt_trans)) {
            if (m_isPlural && m_oldSources.isEmpty())
                m_oldSources.append(QString());
            m_oldSources.append(accum);
            m_hadAlt = true;
        } else {
            m_sources.append(accum);
        }
    } else if (localName == QLatin1String("target")) {
        if (popContext(XC_restype_translation)) {
            accum.replace(QChar(Translator::TextVariantSeparator),
                          QChar(Translator::BinaryVariantSeparator));
            m_translations.append(accum);
        }
    } else if (localName == QLatin1String("context-group")) {
        if (popContext(XC_context_group)) {
            m_refs.append(TranslatorMessage::Reference(
                m_extraFileName.isEmpty() ? m_fileName : m_extraFileName, m_lineNumber));
            m_extraFileName.clear();
            m_lineNumber = -1;
        } else {
            popContext(XC_context_group_any);
        }
    } else if (localName == QLatin1String("context")) {
        if (popContext(XC_context_linenumber)) {
            bool ok;
            m_lineNumber = accum.trimmed().toInt(&ok);
            if (!ok)
                m_lineNumber = -1;
        } else if (popContext(XC_context_filename)) {
            m_extraFileName = accum;
        } else if (popContext(XC_context_comment)) {
            m_comment = accum;
        } else if (popContext(XC_context_old_comment)) {
            m_oldComment = accum;
        }
    } else if (localName == QLatin1String("note")) {
        if (popContext(XC_extra_comment))
            m_extraComment = accum;
        else if (popContext(XC_translator_comment))
            m_translatorComment = accum;
    } else if (localName == QLatin1String("ph")) {
        m_ctype.clear();
        popContext(XC_ph);
    } else if (localName == QLatin1String("trans-unit")) {
        popContext(XC_trans_unit);
        if (!m_hadAlt)
            m_oldSources.append(QString());
        if (!hasContext(XC_restype_plurals)) {
            if (!finalizeMessage(false)) {
                return fatalError(reader.lineNumber(), reader.columnNumber(),
                                  QLatin1String("Element processing failed"));
            }
        }
    } else if (localName == QLatin1String("alt-trans")) {
        popContext(XC_alt_trans);
    } else if (localName == QLatin1String("group")) {
        if (popContext(XC_restype_plurals)) {
            if (!finalizeMessage(true)) {
                return fatalError(reader.lineNumber(), reader.columnNumber(),
                                  QLatin1String("Element processing failed"));
            }
        } else if (popContext(XC_restype_context)) {
            m_context.clear();
        } else {
            popContext(XC_group);
        }
    }
    return true;
}

bool XLIFFHandler::characters(QStringView ch)
{
    if (currentContext() == XC_ph) {
        // handle the content of <ph> elements
        for (int i = 0; i < ch.size(); ++i) {
            QChar chr = ch.at(i);
            if (accum.endsWith(QLatin1Char('\\')))
                accum[accum.size() - 1] = QLatin1Char(charFromEscape(chr.toLatin1()));
            else
                accum.append(chr);
        }
    } else {
        QString t = ch.toString();
        t.replace(QLatin1String("\r"), QLatin1String(""));
        accum.append(t);
    }
    return true;
}

bool XLIFFHandler::endDocument()
{
    m_translator.setLanguageCode(m_language);
    m_translator.setSourceLanguageCode(m_sourceLanguage);
    return true;
}

bool XLIFFHandler::finalizeMessage(bool isPlural)
{
    if (m_sources.isEmpty()) {
        m_cd.appendError(QLatin1String("XLIFF syntax error: Message without source string."));
        return false;
    }
    if (!m_translate && m_refs.size() == 1
        && m_refs.at(0).fileName() == QLatin1String(MAGIC_OBSOLETE_REFERENCE))
        m_refs.clear();
    TranslatorMessage::Type type
            = m_translate ? (m_approved ? TranslatorMessage::Finished : TranslatorMessage::Unfinished)
                          : (m_approved ? TranslatorMessage::Vanished : TranslatorMessage::Obsolete);
    TranslatorMessage msg(m_context, m_sources[0],
                          m_comment, QString(), QString(), -1,
                          m_translations, type, isPlural);
    msg.setId(m_id);
    msg.setReferences(m_refs);
    msg.setOldComment(m_oldComment);
    msg.setExtraComment(m_extraComment);
    msg.setTranslatorComment(m_translatorComment);
    if (m_sources.size() > 1 && m_sources[1] != m_sources[0])
        m_extra.insert(QLatin1String("po-msgid_plural"), m_sources[1]);
    if (!m_oldSources.isEmpty()) {
        if (!m_oldSources[0].isEmpty())
            msg.setOldSourceText(m_oldSources[0]);
        if (m_oldSources.size() > 1 && m_oldSources[1] != m_oldSources[0])
            m_extra.insert(QLatin1String("po-old_msgid_plural"), m_oldSources[1]);
    }
    msg.setExtras(m_extra);
    m_translator.append(msg);

    m_id.clear();
    m_sources.clear();
    m_oldSources.clear();
    m_translations.clear();
    m_comment.clear();
    m_oldComment.clear();
    m_extraComment.clear();
    m_translatorComment.clear();
    m_extra.clear();
    m_refs.clear();
    m_translate = true;
    m_approved = true;
    return true;
}

bool XLIFFHandler::fatalError(qint64 line, qint64 column, const QString &message)
{
    QString msg = QString::asprintf("XML error: Parse error at line %d, column %d (%s).\n",
                                    static_cast<int>(line), static_cast<int>(column),
                                    message.toLatin1().data());
    m_cd.appendError(msg);
    return false;
}

bool loadXLIFF(Translator &translator, QIODevice &dev, ConversionData &cd)
{
    QXmlStreamReader reader(&dev);
    XLIFFHandler hand(translator, cd, reader);
    return hand.parse();
}

bool saveXLIFF(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
    bool ok = true;
    int indent = 0;

    QTextStream ts(&dev);

    QStringList dtgs = cd.dropTags();
    dtgs << QLatin1String("po-(old_)?msgid_plural");
    QRegularExpression drops(QRegularExpression::anchoredPattern(dtgs.join(QLatin1Char('|'))));

    QHash<QString, QHash<QString, QList<TranslatorMessage> > > messageOrder;
    QHash<QString, QList<QString> > contextOrder;
    QList<QString> fileOrder;
    for (const TranslatorMessage &msg : translator.messages()) {
        QString fn = msg.fileName();
        if (fn.isEmpty() && msg.type() == TranslatorMessage::Obsolete)
            fn = QLatin1String(MAGIC_OBSOLETE_REFERENCE);
        QHash<QString, QList<TranslatorMessage> > &file = messageOrder[fn];
        if (file.isEmpty())
            fileOrder.append(fn);
        QList<TranslatorMessage> &context = file[msg.context()];
        if (context.isEmpty())
            contextOrder[fn].append(msg.context());
        context.append(msg);
    }

    ts.setFieldAlignment(QTextStream::AlignRight);
    ts << "<?xml version=\"1.0\"";
    ts << " encoding=\"utf-8\"?>\n";
    ts << "<xliff version=\"1.2\" xmlns=\"" << XLIFF12namespaceURI
       << "\" xmlns:trolltech=\"" << TrollTsNamespaceURI << "\">\n";
    ++indent;
    writeExtras(ts, indent, translator.extras(), drops);
    QString sourceLanguageCode = translator.sourceLanguageCode();
    if (sourceLanguageCode.isEmpty() || sourceLanguageCode == QLatin1String("C"))
        sourceLanguageCode = QLatin1String("en");
    else
        sourceLanguageCode.replace(QLatin1Char('_'), QLatin1Char('-'));
    QString languageCode = translator.languageCode();
    languageCode.replace(QLatin1Char('_'), QLatin1Char('-'));
    for (const QString &fn : std::as_const(fileOrder)) {
        writeIndent(ts, indent);
        ts << "<file original=\"" << fn << "\""
            << " datatype=\"" << dataType(messageOrder[fn].cbegin()->first()) << "\""
            << " source-language=\"" << sourceLanguageCode.toLatin1() << "\""
            << " target-language=\"" << languageCode.toLatin1() << "\""
            << "><body>\n";
        ++indent;

        for (const QString &ctx : std::as_const(contextOrder[fn])) {
            if (!ctx.isEmpty()) {
                writeIndent(ts, indent);
                ts << "<group restype=\"" << restypeContext << "\""
                    << " resname=\"" << xlProtect(ctx) << "\">\n";
                ++indent;
            }

            for (const TranslatorMessage &msg : std::as_const(messageOrder[fn][ctx]))
                writeMessage(ts, msg, drops, indent);

            if (!ctx.isEmpty()) {
                --indent;
                writeIndent(ts, indent);
                ts << "</group>\n";
            }
        }

        --indent;
        writeIndent(ts, indent);
        ts << "</body></file>\n";
    }
    --indent;
    writeIndent(ts, indent);
    ts << "</xliff>\n";

    return ok;
}

int initXLIFF()
{
    Translator::FileFormat format;
    format.extension = QLatin1String("xlf");
    format.untranslatedDescription = QT_TRANSLATE_NOOP("FMT", "XLIFF localization files");
    format.fileType = Translator::FileFormat::TranslationSource;
    format.priority = 1;
    format.loader = &loadXLIFF;
    format.saver = &saveXLIFF;
    Translator::registerFileFormat(format);
    return 1;
}

Q_CONSTRUCTOR_FUNCTION(initXLIFF)

QT_END_NAMESPACE
