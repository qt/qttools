// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translator.h"

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>

#include <QtCore/QXmlStreamReader>

#include <algorithm>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

QDebug &operator<<(QDebug &d, const QXmlStreamAttribute &attr)
{
    return d << "[" << attr.name().toString() << "," << attr.value().toString() << "]";
}


class TSReader : public QXmlStreamReader
{
public:
    TSReader(QIODevice &dev, ConversionData &cd)
      : QXmlStreamReader(&dev), m_cd(cd)
    {}

    // the "real thing"
    bool read(Translator &translator);

private:
    bool elementStarts(const QString &str) const
    {
        return isStartElement() && name() == str;
    }

    bool isWhiteSpace() const
    {
        return isCharacters() && text().toString().trimmed().isEmpty();
    }

    // needed to expand <byte ... />
    QString readContents();
    // needed to join <lengthvariant>s
    QString readTransContents();

    void handleError();

    ConversionData &m_cd;
};

void TSReader::handleError()
{
    if (isComment())
        return;
    if (hasError() && error() == CustomError) // raised by readContents
        return;

    const QString loc = QString::fromLatin1("at %3:%1:%2")
        .arg(lineNumber()).arg(columnNumber()).arg(m_cd.m_sourceFileName);

    switch (tokenType()) {
    case NoToken: // Cannot happen
    default: // likewise
    case Invalid:
        raiseError(QString::fromLatin1("Parse error %1: %2").arg(loc, errorString()));
        break;
    case StartElement:
        raiseError(QString::fromLatin1("Unexpected tag <%1> %2").arg(name().toString(), loc));
        break;
    case Characters:
        {
            QString tok = text().toString();
            if (tok.size() > 30)
                tok = tok.left(30) + "[...]"_L1;
            raiseError(QString::fromLatin1("Unexpected characters '%1' %2").arg(tok, loc));
        }
        break;
    case EntityReference:
        raiseError(QString::fromLatin1("Unexpected entity '&%1;' %2").arg(name().toString(), loc));
        break;
    case ProcessingInstruction:
        raiseError(QString::fromLatin1("Unexpected processing instruction %1").arg(loc));
        break;
    }
}

static QString byteValue(QString value)
{
    int base = 10;
    if (value.startsWith("x"_L1)) {
        base = 16;
        value.remove(0, 1);
    }
    int n = value.toUInt(0, base);
    return (n != 0) ? QString(QChar(n)) : QString();
}

QString TSReader::readContents()
{
    static const QString strbyte = u"byte"_s;
    static const QString strvalue = u"value"_s;

    QString result;
    while (!atEnd()) {
        readNext();
        if (isEndElement()) {
            break;
        } else if (isCharacters()) {
            result += text();
        } else if (elementStarts(strbyte)) {
            // <byte value="...">
            result += byteValue(attributes().value(strvalue).toString());
            readNext();
            if (!isEndElement()) {
                handleError();
                break;
            }
        } else {
            handleError();
            break;
        }
    }
    //qDebug() << "TEXT: " << result;
    return result;
}

QString TSReader::readTransContents()
{
    static const QString strlengthvariant = u"lengthvariant"_s;
    static const QString strvariants = u"variants"_s;
    static const QString stryes = u"yes"_s;

    if (attributes().value(strvariants) == stryes) {
        QString result;
        while (!atEnd()) {
            readNext();
            if (isEndElement()) {
                break;
            } else if (isWhiteSpace()) {
                // ignore these, just whitespace
            } else if (elementStarts(strlengthvariant)) {
                if (!result.isEmpty())
                    result += QChar(Translator::BinaryVariantSeparator);
                result += readContents();
            } else {
                handleError();
                break;
            }
        }
        return result;
    } else {
        return readContents();
    }
}

bool TSReader::read(Translator &translator)
{
    static const QString strcatalog = u"catalog"_s;
    static const QString strcomment = u"comment"_s;
    static const QString strcontext = u"context"_s;
    static const QString strdependencies = u"dependencies"_s;
    static const QString strdependency = u"dependency"_s;
    static const QString strextracomment = u"extracomment"_s;
    static const QString strlabel = u"label"_s;
    static const QString strfilename = u"filename"_s;
    static const QString strid = u"id"_s;
    static const QString strlanguage = u"language"_s;
    static const QString strline = u"line"_s;
    static const QString strlocation = u"location"_s;
    static const QString strmessage = u"message"_s;
    static const QString strname = u"name"_s;
    static const QString strnumerus = u"numerus"_s;
    static const QString strnumerusform = u"numerusform"_s;
    static const QString strobsolete = u"obsolete"_s;
    static const QString stroldcomment = u"oldcomment"_s;
    static const QString stroldsource = u"oldsource"_s;
    static const QString strsource = u"source"_s;
    static const QString strsourcelanguage = u"sourcelanguage"_s;
    static const QString strtranslation = u"translation"_s;
    static const QString strtranslatorcomment = u"translatorcomment"_s;
    static const QString strTS = u"TS"_s;
    static const QString strtype = u"type"_s;
    static const QString strunfinished = u"unfinished"_s;
    static const QString struserdata = u"userdata"_s;
    static const QString strvanished = u"vanished"_s;
    //static const QString strversion = u"version"_s;
    static const QString stryes = u"yes"_s;

    static const QString strextrans("extra-"_L1);

    while (!atEnd()) {
        readNext();
        if (isStartDocument()) {
            // <!DOCTYPE TS>
            //qDebug() << attributes();
        } else if (isEndDocument()) {
            // <!DOCTYPE TS>
            //qDebug() << attributes();
        } else if (isDTD()) {
            // <!DOCTYPE TS>
            //qDebug() << tokenString();
        } else if (elementStarts(strTS)) {
            // <TS>
            //qDebug() << "TS " << attributes();
            QHash<QString, int> currentLine;
            QString currentFile;
            bool maybeRelative = false, maybeAbsolute = false;

            QXmlStreamAttributes atts = attributes();
            //QString version = atts.value(strversion).toString();
            translator.setLanguageCode(atts.value(strlanguage).toString());
            translator.setSourceLanguageCode(atts.value(strsourcelanguage).toString());
            while (!atEnd()) {
                readNext();
                if (isEndElement()) {
                    // </TS> found, finish local loop
                    break;
                } else if (isWhiteSpace()) {
                    // ignore these, just whitespace
                } else if (isStartElement()
                        && name().toString().startsWith(strextrans)) {
                    // <extra-...>
                    QString tag = name().toString();
                    translator.setExtra(tag.mid(6), readContents());
                    // </extra-...>
                } else if (elementStarts(strdependencies)) {
                    /*
                     * <dependencies>
                     *   <dependency catalog="qtsystems_no"/>
                     *   <dependency catalog="qtbase_no"/>
                     * </dependencies>
                     **/
                    QStringList dependencies;
                    while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                            // </dependencies> found, finish local loop
                            break;
                        } else if (elementStarts(strdependency)) {
                            // <dependency>
                            QXmlStreamAttributes atts = attributes();
                            dependencies.append(atts.value(strcatalog).toString());
                            while (!atEnd()) {
                                readNext();
                                if (isEndElement()) {
                                    // </dependency> found, finish local loop
                                    break;
                                }
                            }
                        }
                    }
                    translator.setDependencies(dependencies);
                } else if (elementStarts(strcontext)) {
                    // <context>
                    QString context;
                    while (!atEnd()) {
                        readNext();
                        if (isEndElement()) {
                            // </context> found, finish local loop
                            break;
                        } else if (isWhiteSpace()) {
                            // ignore these, just whitespace
                        } else if (elementStarts(strname)) {
                            // <name>
                            context = readElementText();
                            // </name>
                        } else if (elementStarts(strmessage)) {
                            // <message>
                            TranslatorMessage::References refs;
                            QString currentMsgFile = currentFile;

                            TranslatorMessage msg;
                            msg.setId(attributes().value(strid).toString());
                            msg.setContext(context);
                            msg.setType(TranslatorMessage::Finished);
                            msg.setPlural(attributes().value(strnumerus) == stryes);
                            msg.setTsLineNumber(lineNumber());
                            while (!atEnd()) {
                                readNext();
                                if (isEndElement()) {
                                    // </message> found, finish local loop
                                    msg.setReferences(refs);
                                    translator.append(msg);
                                    break;
                                } else if (isWhiteSpace()) {
                                    // ignore these, just whitespace
                                } else if (elementStarts(strsource)) {
                                    // <source>...</source>
                                    msg.setSourceText(readContents());
                                } else if (elementStarts(stroldsource)) {
                                    // <oldsource>...</oldsource>
                                    msg.setOldSourceText(readContents());
                                } else if (elementStarts(stroldcomment)) {
                                    // <oldcomment>...</oldcomment>
                                    msg.setOldComment(readContents());
                                } else if (elementStarts(strextracomment)) {
                                    // <extracomment>...</extracomment>
                                    msg.setExtraComment(readContents());
                                } else if (elementStarts(strlabel)) {
                                    // <label>...</label>
                                    msg.setLabel(readContents());
                                } else if (elementStarts(strtranslatorcomment)) {
                                    // <translatorcomment>...</translatorcomment>
                                    msg.setTranslatorComment(readContents());
                                } else if (elementStarts(strlocation)) {
                                    // <location/>
                                    maybeAbsolute = true;
                                    QXmlStreamAttributes atts = attributes();
                                    QString fileName = atts.value(strfilename).toString();
                                    if (fileName.isEmpty()) {
                                        fileName = currentMsgFile;
                                        maybeRelative = true;
                                    } else {
                                        if (refs.isEmpty())
                                            currentFile = fileName;
                                        currentMsgFile = fileName;
                                    }
                                    const QString lin = atts.value(strline).toString();
                                    if (lin.isEmpty()) {
                                        refs.append(TranslatorMessage::Reference(fileName, -1));
                                    } else {
                                        bool bOK;
                                        int lineNo = lin.toInt(&bOK);
                                        if (bOK) {
                                            if (lin.startsWith(u'+') || lin.startsWith(u'-')) {
                                                lineNo = (currentLine[fileName] += lineNo);
                                                maybeRelative = true;
                                            }
                                            refs.append(TranslatorMessage::Reference(fileName, lineNo));
                                        }
                                    }
                                    readContents();
                                } else if (elementStarts(strcomment)) {
                                    // <comment>...</comment>
                                    msg.setComment(readContents());
                                } else if (elementStarts(struserdata)) {
                                    // <userdata>...</userdata>
                                    msg.setUserData(readContents());
                                } else if (elementStarts(strtranslation)) {
                                    // <translation>
                                    QXmlStreamAttributes atts = attributes();
                                    QStringView type = atts.value(strtype);
                                    if (type == strunfinished)
                                        msg.setType(TranslatorMessage::Unfinished);
                                    else if (type == strvanished)
                                        msg.setType(TranslatorMessage::Vanished);
                                    else if (type == strobsolete)
                                        msg.setType(TranslatorMessage::Obsolete);
                                    if (msg.isPlural()) {
                                        QStringList translations;
                                        while (!atEnd()) {
                                            readNext();
                                            if (isEndElement()) {
                                                break;
                                            } else if (isWhiteSpace()) {
                                                // ignore these, just whitespace
                                            } else if (elementStarts(strnumerusform)) {
                                                translations.append(readTransContents());
                                            } else {
                                                handleError();
                                                break;
                                            }
                                        }
                                        msg.setTranslations(translations);
                                    } else {
                                        msg.setTranslation(readTransContents());
                                    }
                                    // </translation>
                                } else if (isStartElement()
                                           && name().toString().startsWith(strextrans)) {
                                    // <extra-...>
                                    QString tag = name().toString();
                                    msg.setExtra(tag.mid(6), readContents());
                                    // </extra-...>
                                } else {
                                    handleError();
                                }
                            }
                            // </message>
                        } else {
                            handleError();
                        }
                    }
                    // </context>
                } else {
                    handleError();
                }
                // if the file is empty adopt AbsoluteLocation (default location type for Translator)
                if (translator.messageCount() == 0)
                    maybeAbsolute = true;
                translator.setLocationsType(maybeRelative ? Translator::RelativeLocations :
                                            maybeAbsolute ? Translator::AbsoluteLocations :
                                                            Translator::NoLocations);
            } // </TS>
        } else {
            handleError();
        }
    }
    if (hasError()) {
        m_cd.appendError(errorString());
        return false;
    }
    return true;
}

static QString tsNumericEntity(int ch)
{
    return QString(ch <= 0x20 ? QLatin1String("<byte value=\"x%1\"/>") : "&#x%1;"_L1)
            .arg(ch, 0, 16);
}

static QString tsProtect(const QString &str)
{
    QString result;
    result.reserve(str.size() * 12 / 10);
    for (int i = 0; i != str.size(); ++i) {
        const QChar ch = str[i];
        uint c = ch.unicode();
        switch (c) {
        case '\"':
            result += "&quot;"_L1;
            break;
        case '&':
            result += "&amp;"_L1;
            break;
        case '>':
            result += "&gt;"_L1;
            break;
        case '<':
            result += "&lt;"_L1;
            break;
        case '\'':
            result += "&apos;"_L1;
            break;
        default:
            if ((c < 0x20 || (ch > QChar(0x7f) && ch.isSpace())) && c != '\n' && c != '\t')
                result += tsNumericEntity(c);
            else // this also covers surrogates
                result += QChar(c);
        }
    }
    return result;
}

static void writeExtras(QTextStream &t, const char *indent,
                        const TranslatorMessage::ExtraData &extras, QRegularExpression drops)
{
    QStringList outs;
    for (auto it = extras.cbegin(), end = extras.cend(); it != end; ++it) {
        if (!drops.match(it.key()).hasMatch()) {
            outs << (QStringLiteral("<extra-") + it.key() + u'>' + tsProtect(it.value())
                     + QStringLiteral("</extra-") + it.key() + u'>');
        }
    }
    outs.sort();
    for (const QString &out : std::as_const(outs))
        t << indent << out << Qt::endl;
}

static void writeVariants(QTextStream &t, const char *indent, const QString &input)
{
    int offset;
    if ((offset = input.indexOf(Translator::BinaryVariantSeparator)) >= 0) {
        t << " variants=\"yes\">";
        int start = 0;
        forever {
            t << "\n    " << indent << "<lengthvariant>"
              << tsProtect(input.mid(start, offset - start))
              << "</lengthvariant>";
            if (offset == input.size())
                break;
            start = offset + 1;
            offset = input.indexOf(Translator::BinaryVariantSeparator, start);
            if (offset < 0)
                offset = input.size();
        }
        t << "\n" << indent;
    } else {
        t << ">" << tsProtect(input);
    }
}

bool saveTS(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
    bool result = true;
    QTextStream t(&dev);

    // The xml prolog allows processors to easily detect the correct encoding
    t << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n";

    t << "<TS version=\"2.1\"";

    QString languageCode = translator.languageCode();
    if (!languageCode.isEmpty() && languageCode != "C"_L1)
        t << " language=\"" << languageCode << "\"";
    languageCode = translator.sourceLanguageCode();
    if (!languageCode.isEmpty() && languageCode != "C"_L1)
        t << " sourcelanguage=\"" << languageCode << "\"";
    t << ">\n";

    const QStringList deps = translator.dependencies();
    if (!deps.isEmpty()) {
        t << "<dependencies>\n";
        for (const QString &dep : deps)
            t << "    <dependency catalog=\"" << dep << "\"/>\n";
        t << "</dependencies>\n";
    }

    QRegularExpression drops(QRegularExpression::anchoredPattern(cd.dropTags().join(u'|')));

    writeExtras(t, "    ", translator.extras(), drops);

    QHash<QString, QList<TranslatorMessage> > messageOrder;
    QList<QString> contextOrder;
    for (const TranslatorMessage &msg : translator.messages()) {
        // no need for such noise
        if ((msg.type() == TranslatorMessage::Obsolete || msg.type() == TranslatorMessage::Vanished)
            && msg.translation().isEmpty()) {
            continue;
        }

        QList<TranslatorMessage> &context = messageOrder[msg.context()];
        if (context.isEmpty())
            contextOrder.append(msg.context());
        context.append(msg);
    }
    if (cd.sortContexts())
        std::sort(contextOrder.begin(), contextOrder.end());
    if (cd.sortMessages()) {
        auto messageComparator = [](const TranslatorMessage &m1, const TranslatorMessage &m2) {
            return m1.sourceText() < m2.sourceText();
        };
        for (QList<TranslatorMessage> &contextMessages : messageOrder)
            std::sort(contextMessages.begin(), contextMessages.end(), messageComparator);
    }

    QHash<QString, int> currentLine;
    QString currentFile;
    for (const QString &context : std::as_const(contextOrder)) {
        t << "<context>\n"
             "    <name>"
          << tsProtect(context)
          << "</name>\n";
        for (const TranslatorMessage &msg : std::as_const(messageOrder[context])) {
            //msg.dump();

                t << "    <message";
                if (!msg.id().isEmpty())
                    t << " id=\"" << tsProtect(msg.id()) << "\"";
                if (msg.isPlural())
                    t << " numerus=\"yes\"";
                t << ">\n";
                if (translator.locationsType() != Translator::NoLocations) {
                    QString cfile = currentFile;
                    bool first = true;
                    for (const TranslatorMessage::Reference &ref : msg.allReferences()) {
                        QString fn = cd.m_targetDir.relativeFilePath(ref.fileName())
                                             .replace(u'\\', u'/');
                        int ln = ref.lineNumber();
                        QString ld;
                        if (translator.locationsType() == Translator::RelativeLocations) {
                            if (ln != -1) {
                                int dlt = ln - currentLine[fn];
                                if (dlt >= 0)
                                    ld.append(u'+');
                                ld.append(QString::number(dlt));
                                currentLine[fn] = ln;
                            }

                            if (fn != cfile) {
                                if (first)
                                    currentFile = fn;
                                cfile = fn;
                            } else {
                                fn.clear();
                            }
                            first = false;
                        } else {
                            if (ln != -1)
                                ld = QString::number(ln);
                        }

                        if (!ld.isEmpty()) {
                            t << "        <location";
                            if (!fn.isEmpty())
                                t << " filename=\"" << fn << "\"";
                            t << " line=\"" << ld << "\"";
                            t << "/>\n";
                        }
                    }
                }

                t << "        <source>"
                  << tsProtect(msg.sourceText())
                  << "</source>\n";

                if (!msg.oldSourceText().isEmpty())
                    t << "        <oldsource>" << tsProtect(msg.oldSourceText()) << "</oldsource>\n";

                if (!msg.comment().isEmpty()) {
                    t << "        <comment>"
                      << tsProtect(msg.comment())
                      << "</comment>\n";
                }

                    if (!msg.oldComment().isEmpty())
                        t << "        <oldcomment>" << tsProtect(msg.oldComment()) << "</oldcomment>\n";

                    if (!msg.extraComment().isEmpty())
                        t << "        <extracomment>" << tsProtect(msg.extraComment())
                          << "</extracomment>\n";

                    if (!msg.label().isEmpty())
                        t << "        <label>" << tsProtect(msg.label()) << "</label>\n";

                    if (!msg.translatorComment().isEmpty())
                        t << "        <translatorcomment>" << tsProtect(msg.translatorComment())
                          << "</translatorcomment>\n";

                t << "        <translation";
                if (msg.type() == TranslatorMessage::Unfinished)
                    t << " type=\"unfinished\"";
                else if (msg.type() == TranslatorMessage::Vanished)
                    t << " type=\"vanished\"";
                else if (msg.type() == TranslatorMessage::Obsolete)
                    t << " type=\"obsolete\"";
                if (msg.isPlural()) {
                    t << ">";
                    const QStringList &translns = msg.translations();
                    for (int j = 0; j < translns.size(); ++j) {
                        t << "\n            <numerusform";
                        writeVariants(t, "            ", translns[j]);
                        t << "</numerusform>";
                    }
                    t << "\n        ";
                } else {
                    writeVariants(t, "        ", msg.translation());
                }
                t << "</translation>\n";

                writeExtras(t, "        ", msg.extras(), drops);

                if (!msg.userData().isEmpty())
                    t << "        <userdata>" << msg.userData() << "</userdata>\n";
                t << "    </message>\n";
        }
        t << "</context>\n";
    }

    t << "</TS>\n";
    return result;
}

bool loadTS(Translator &translator, QIODevice &dev, ConversionData &cd)
{
    TSReader reader(dev, cd);
    return reader.read(translator);
}

int initTS()
{
    Translator::FileFormat format;

    format.extension = "ts"_L1;
    format.fileType = Translator::FileFormat::TranslationSource;
    format.priority = 0;
    format.untranslatedDescription = QT_TRANSLATE_NOOP("FMT", "Qt translation sources");
    format.loader = &loadTS;
    format.saver = &saveTS;
    Translator::registerFileFormat(format);

    return 1;
}

Q_CONSTRUCTOR_FUNCTION(initTS)

QT_END_NAMESPACE
