// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translator.h"

#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtCore/QHash>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringConverter>
#include <QtCore/QTextStream>

#include <ctype.h>

// Uncomment if you wish to hard wrap long lines in .po files. Note that this
// affects only msg strings, not comments.
//#define HARD_WRAP_LONG_WORDS

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

static const int MAX_LEN = 79;

static QString poEscapedString(const QString &prefix, const QString &keyword,
                               bool noWrap, const QString &ba)
{
    QStringList lines;
    int off = 0;
    QString res;
    while (off < ba.size()) {
        ushort c = ba[off++].unicode();
        switch (c) {
        case '\n':
            res += "\\n"_L1;
            lines.append(res);
            res.clear();
            break;
        case '\r':
            res += "\\r"_L1;
            break;
        case '\t':
            res += "\\t"_L1;
            break;
        case '\v':
            res += "\\v"_L1;
            break;
        case '\a':
            res += "\\a"_L1;
            break;
        case '\b':
            res += "\\b"_L1;
            break;
        case '\f':
            res += "\\f"_L1;
            break;
        case '"':
            res += QLatin1String("\\\"");
            break;
        case '\\':
            res += "\\\\"_L1;
            break;
        default:
            if (c < 32) {
                res += "\\x"_L1;
                res += QString::number(c, 16);
                if (off < ba.size() && isxdigit(ba[off].unicode()))
                    res += QLatin1String("\"\"");
            } else {
                res += QChar(c);
            }
            break;
        }
    }
    if (!res.isEmpty())
        lines.append(res);
    if (!lines.isEmpty()) {
        if (!noWrap) {
            if (lines.size() != 1 ||
                lines.first().size() > MAX_LEN - keyword.size() - prefix.size() - 3)
            {
                const QStringList olines = lines;
                lines = QStringList(QString());
                const int maxlen = MAX_LEN - prefix.size() - 2;
                for (const QString &line : olines) {
                    int off = 0;
                    while (off + maxlen < line.size()) {
                        int idx = line.lastIndexOf(u' ', off + maxlen - 1) + 1;
                        if (idx == off) {
#ifdef HARD_WRAP_LONG_WORDS
                            // This doesn't seem too nice, but who knows ...
                            idx = off + maxlen;
#else
                            idx = line.indexOf(u' ', off + maxlen) + 1;
                            if (!idx)
                                break;
#endif
                        }
                        lines.append(line.mid(off, idx - off));
                        off = idx;
                    }
                    lines.append(line.mid(off));
                }
            }
        } else if (lines.size() > 1) {
            lines.prepend(QString());
        }
    }
    return prefix + keyword + QLatin1String(" \"")
            + lines.join(QLatin1String("\"\n") + prefix + u'"') + QLatin1String("\"\n");
}

static QString poEscapedLines(const QString &prefix, bool addSpace, const QStringList &lines)
{
    QString out;
    for (const QString &line : lines) {
        out += prefix;
        if (addSpace && !line.isEmpty())
            out += QLatin1Char(' ' );
        out += line;
        out += u'\n';
    }
    return out;
}

static QString poEscapedLines(const QString &prefix, bool addSpace, const QString &in0)
{
    QString in = in0;
    if (in == QString::fromLatin1("\n"))
        in.chop(1);
    return poEscapedLines(prefix, addSpace, in.split(u'\n'));
}

static QString poWrappedEscapedLines(const QString &prefix, bool addSpace, const QString &line)
{
    const int maxlen = MAX_LEN - prefix.size() - addSpace;
    QStringList lines;
    int off = 0;
    while (off + maxlen < line.size()) {
        int idx = line.lastIndexOf(u' ', off + maxlen - 1);
        if (idx < off) {
#if 0 //def HARD_WRAP_LONG_WORDS
            // This cannot work without messing up semantics, so do not even try.
#else
            idx = line.indexOf(u' ', off + maxlen);
            if (idx < 0)
                break;
#endif
        }
        lines.append(line.mid(off, idx - off));
        off = idx + 1;
    }
    lines.append(line.mid(off));
    return poEscapedLines(prefix, addSpace, lines);
}

struct PoItem
{
public:
    PoItem()
      : isPlural(false), isFuzzy(false)
    {}


public:
    QByteArray id;
    QByteArray context;
    QByteArray tscomment;
    QByteArray oldTscomment;
    QByteArray lineNumber;
    QByteArray fileName;
    QByteArray references;
    QByteArray translatorComments;
    QByteArray automaticComments;
    QByteArray msgId;
    QByteArray oldMsgId;
    QList<QByteArray> msgStr;
    bool isPlural;
    bool isFuzzy;
    QHash<QString, QString> extra;
};


static bool isTranslationLine(const QByteArray &line)
{
    return line.startsWith("#~ msgstr") || line.startsWith("msgstr");
}

static QByteArray slurpEscapedString(const QList<QByteArray> &lines, int &l,
        int offset, const QByteArray &prefix, ConversionData &cd)
{
    QByteArray msg;
    int stoff;

    for (; l < lines.size(); ++l) {
        const QByteArray &line = lines.at(l);
        if (line.isEmpty() || !line.startsWith(prefix))
            break;
        while (isspace(line[offset])) // No length check, as string has no trailing spaces.
            offset++;
        if (line[offset] != '"')
            break;
        offset++;
        forever {
            if (offset == line.size())
                goto premature_eol;
            uchar c = line[offset++];
            if (c == '"') {
                if (offset == line.size())
                    break;
                while (isspace(line[offset]))
                    offset++;
                if (line[offset++] != '"') {
                    cd.appendError(QString::fromLatin1(
                            "PO parsing error: extra characters on line %1.")
                            .arg(l + 1));
                    break;
                }
                continue;
            }
            if (c == '\\') {
                if (offset == line.size())
                    goto premature_eol;
                c = line[offset++];
                switch (c) {
                case 'r':
                    msg += '\r'; // Maybe just throw it away?
                    break;
                case 'n':
                    msg += '\n';
                    break;
                case 't':
                    msg += '\t';
                    break;
                case 'v':
                    msg += '\v';
                    break;
                case 'a':
                    msg += '\a';
                    break;
                case 'b':
                    msg += '\b';
                    break;
                case 'f':
                    msg += '\f';
                    break;
                case '"':
                    msg += '"';
                    break;
                case '\\':
                    msg += '\\';
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    stoff = offset - 1;
                    while ((c = line[offset]) >= '0' && c <= '7')
                        if (++offset == line.size())
                            goto premature_eol;
                    msg += line.mid(stoff, offset - stoff).toUInt(0, 8);
                    break;
                case 'x':
                    stoff = offset;
                    while (isxdigit(line[offset]))
                        if (++offset == line.size())
                            goto premature_eol;
                    msg += line.mid(stoff, offset - stoff).toUInt(0, 16);
                    break;
                default:
                    cd.appendError(QString::fromLatin1(
                            "PO parsing error: invalid escape '\\%1' (line %2).")
                            .arg(QChar((uint)c)).arg(l + 1));
                    msg += '\\';
                    msg += c;
                    break;
                }
            } else {
                msg += c;
            }
        }
        offset = prefix.size();
    }
    --l;
    return msg;

premature_eol:
    cd.appendError(QString::fromLatin1(
            "PO parsing error: premature end of line %1.").arg(l + 1));
    return QByteArray();
}

static void slurpComment(QByteArray &msg, const QList<QByteArray> &lines, int & l)
{
    int firstLine = l;
    QByteArray prefix = lines.at(l);
    for (int i = 1; ; i++) {
        if (prefix.at(i) != ' ') {
            prefix.truncate(i);
            break;
        }
    }
    for (; l < lines.size(); ++l) {
        const QByteArray &line = lines.at(l);
        if (line.startsWith(prefix)) {
            if (l > firstLine)
                msg += '\n';
            msg += line.mid(prefix.size());
        } else if (line == "#") {
            msg += '\n';
        } else {
            break;
        }
    }
    --l;
}

static void splitContext(QByteArray *comment, QByteArray *context)
{
    char *data = comment->data();
    int len = comment->size();
    int sep = -1, j = 0;

    for (int i = 0; i < len; i++, j++) {
        if (data[i] == '~' && i + 1 < len)
            i++;
        else if (data[i] == '|')
            sep = j;
        data[j] = data[i];
    }
    if (sep >= 0) {
        QByteArray tmp = comment->mid(sep + 1, j - sep - 1);
        comment->truncate(sep);
        *context = *comment;
        *comment = tmp;
    } else {
        comment->truncate(j);
    }
}

static QString makePoHeader(const QString &str)
{
    return "po-header-"_L1 + str.toLower().replace(u'-', u'_');
}

static QByteArray QByteArrayList_join(const QList<QByteArray> &that, char sep)
{
    int totalLength = 0;
    const int size = that.size();

    for (int i = 0; i < size; ++i)
        totalLength += that.at(i).size();

    if (size > 0)
        totalLength += size - 1;

    QByteArray res;
    if (totalLength == 0)
        return res;
    res.reserve(totalLength);
    for (int i = 0; i < that.size(); ++i) {
        if (i)
            res += sep;
        res += that.at(i);
    }
    return res;
}

bool loadPO(Translator &translator, QIODevice &dev, ConversionData &cd)
{
    QStringDecoder toUnicode(QStringConverter::Utf8, QStringDecoder::Flag::Stateless);
    bool error = false;

    // format of a .po file entry:
    // white-space
    // #  translator-comments
    // #. automatic-comments
    // #: reference...
    // #, flag...
    // #~ msgctxt, msgid*, msgstr - used for obsoleted messages
    // #| msgctxt, msgid* previous untranslated-string - for fuzzy message
    // #~| msgctxt, msgid* previous untranslated-string - for fuzzy obsoleted messages
    // msgctx string-context
    // msgid untranslated-string
    // -- For singular:
    // msgstr translated-string
    // -- For plural:
    // msgid_plural untranslated-string-plural
    // msgstr[0] translated-string
    // ...

    // we need line based lookahead below.
    QList<QByteArray> lines;
    while (!dev.atEnd())
        lines.append(dev.readLine().trimmed());
    lines.append(QByteArray());

    int l = 0, lastCmtLine = -1;
    bool qtContexts = false;
    PoItem item;
    for (; l != lines.size(); ++l) {
        QByteArray line = lines.at(l);
        if (line.isEmpty())
           continue;
        if (isTranslationLine(line)) {
            bool isObsolete = line.startsWith("#~ msgstr");
            const QByteArray prefix = isObsolete ? "#~ " : "";
            while (true) {
                int idx = line.indexOf(' ', prefix.size());
                QByteArray str = slurpEscapedString(lines, l, idx, prefix, cd);
                item.msgStr.append(str);
                if (l + 1 >= lines.size() || !isTranslationLine(lines.at(l + 1)))
                    break;
                ++l;
                line = lines.at(l);
            }
            if (item.msgId.isEmpty()) {
                QHash<QString, QByteArray> extras;
                QList<QByteArray> hdrOrder;
                QByteArray pluralForms;
                const auto &headers = item.msgStr.constFirst().split('\n');
                for (const QByteArray &hdr : headers) {
                    if (hdr.isEmpty())
                        continue;
                    int idx = hdr.indexOf(':');
                    if (idx < 0) {
                        cd.appendError(QString::fromLatin1("Unexpected PO header format '%1'")
                            .arg(QString::fromLatin1(hdr)));
                        error = true;
                        break;
                    }
                    QByteArray hdrName = hdr.left(idx).trimmed();
                    QByteArray hdrValue = hdr.mid(idx + 1).trimmed();
                    hdrOrder << hdrName;
                    if (hdrName == "X-Language") {
                        translator.setLanguageCode(QString::fromLatin1(hdrValue));
                    } else if (hdrName == "X-Source-Language") {
                        translator.setSourceLanguageCode(QString::fromLatin1(hdrValue));
                    } else if (hdrName == "X-Qt-Contexts") {
                        qtContexts = (hdrValue == "true");
                    } else if (hdrName == "Plural-Forms") {
                        pluralForms  = hdrValue;
                    } else if (hdrName == "MIME-Version") {
                        // just assume it is 1.0
                    } else if (hdrName == "Content-Type") {
                            if (!hdrValue.startsWith("text/plain; charset=")) {
                                cd.appendError(QString::fromLatin1("Unexpected Content-Type header '%1'")
                                    .arg(QString::fromLatin1(hdrValue)));
                                error = true;
                                // This will avoid a flood of conversion errors.
                                toUnicode = QStringDecoder(QStringConverter::Latin1);
                            } else {
                                QByteArray cod = hdrValue.mid(20);
                                auto enc = QStringConverter::encodingForName(cod);
                                if (!enc) {
                                    cd.appendError(QString::fromLatin1("Unsupported encoding '%1'")
                                            .arg(QString::fromLatin1(cod)));
                                    error = true;
                                    // This will avoid a flood of conversion errors.
                                    toUnicode = QStringDecoder(QStringConverter::Latin1);
                                } else {
                                    toUnicode = QStringDecoder(*enc);
                                }
                            }
                    } else if (hdrName == "Content-Transfer-Encoding") {
                        if (hdrValue != "8bit") {
                            cd.appendError(QString::fromLatin1("Unexpected Content-Transfer-Encoding '%1'")
                                .arg(QString::fromLatin1(hdrValue)));
                            return false;
                        }
                    } else if (hdrName == "X-Virgin-Header") {
                        // legacy
                    } else {
                        extras[makePoHeader(QString::fromLatin1(hdrName))] = hdrValue;
                    }
                }
                if (!pluralForms.isEmpty()) {
                    if (translator.languageCode().isEmpty()) {
                        extras[makePoHeader("Plural-Forms"_L1)] = pluralForms;
                    } else {
                         // FIXME: have fun with making a consistency check ...
                    }
                }
                // Eliminate the field if only headers we added are present in standard order.
                // Keep in sync with savePO
                static const char * const dfltHdrs[] = {
                    "MIME-Version", "Content-Type", "Content-Transfer-Encoding",
                    "Plural-Forms", "X-Language", "X-Source-Language", "X-Qt-Contexts"
                };
                uint cdh = 0;
                for (int cho = 0; cho < hdrOrder.size(); cho++) {
                    for (;; cdh++) {
                        if (cdh == sizeof(dfltHdrs)/sizeof(dfltHdrs[0])) {
                            extras["po-headers"_L1] = QByteArrayList_join(hdrOrder, ',');
                            goto doneho;
                        }
                        if (hdrOrder.at(cho) == dfltHdrs[cdh]) {
                            cdh++;
                            break;
                        }
                    }
                }
              doneho:
                if (lastCmtLine != -1) {
                    extras["po-header_comment"_L1] =
                            QByteArrayList_join(lines.mid(0, lastCmtLine + 1), '\n');
                }
                for (auto it = extras.cbegin(), end = extras.cend(); it != end; ++it)
                    translator.setExtra(it.key(), toUnicode(it.value()));
                item = PoItem();
                continue;
            }
            // build translator message
            TranslatorMessage msg;
            msg.setContext(toUnicode(item.context));
            if (!item.references.isEmpty()) {
                QString xrefs;
                const auto &refs = QString(toUnicode(item.references))
                                           .split(QRegularExpression("\\s"_L1), Qt::SkipEmptyParts);
                for (const QString &ref : refs) {
                    int pos = ref.indexOf(u':');
                    int lpos = ref.lastIndexOf(u':');
                    if (pos != -1 && pos == lpos) {
                        bool ok;
                        int lno = ref.mid(pos + 1).toInt(&ok);
                        if (ok) {
                            msg.addReference(ref.left(pos), lno, -1, -1);
                            continue;
                        }
                    }
                    if (!xrefs.isEmpty())
                        xrefs += u' ';
                    xrefs += ref;
                }
                if (!xrefs.isEmpty())
                    item.extra["po-references"_L1] = xrefs;
            }
            msg.setId(toUnicode(item.id));
            msg.setSourceText(toUnicode(item.msgId));
            msg.setOldSourceText(toUnicode(item.oldMsgId));
            msg.setComment(toUnicode(item.tscomment));
            msg.setOldComment(toUnicode(item.oldTscomment));
            msg.setExtraComment(toUnicode(item.automaticComments));
            msg.setTranslatorComment(toUnicode(item.translatorComments));
            msg.setPlural(item.isPlural || item.msgStr.size() > 1);
            QStringList translations;
            for (const QByteArray &bstr : std::as_const(item.msgStr)) {
                QString str = toUnicode(bstr);
                str.replace(Translator::TextVariantSeparator, Translator::BinaryVariantSeparator);
                translations << str;
            }
            msg.setTranslations(translations);
            bool isFuzzy = item.isFuzzy || (!msg.sourceText().isEmpty() && !msg.isTranslated());
            if (isObsolete && isFuzzy)
                msg.setType(TranslatorMessage::Obsolete);
            else if (isObsolete)
                msg.setType(TranslatorMessage::Vanished);
            else if (isFuzzy)
                msg.setType(TranslatorMessage::Unfinished);
            else
                msg.setType(TranslatorMessage::Finished);
            msg.setExtras(item.extra);

            //qDebug() << "WRITE: " << context;
            //qDebug() << "SOURCE: " << msg.sourceText();
            //qDebug() << flags << msg.m_extra;
            translator.append(msg);
            item = PoItem();
        } else if (line.startsWith('#')) {
            switch (line.size() < 2 ? 0 : line.at(1)) {
                case ':':
                    item.references += line.mid(3);
                    item.references += '\n';
                    break;
                case ',': {
                    QStringList flags =
                            QString::fromLatin1(line.mid(2))
                                    .split(QRegularExpression("[, ]"_L1), Qt::SkipEmptyParts);
                    if (flags.removeOne("fuzzy"_L1))
                        item.isFuzzy = true;
                    flags.removeOne("qt-format"_L1);
                    const auto it = item.extra.constFind("po-flags"_L1);
                    if (it != item.extra.cend())
                        flags.prepend(*it);
                    if (!flags.isEmpty())
                        item.extra["po-flags"_L1] = flags.join(", "_L1);
                    break;
                }
                case 0:
                    item.translatorComments += '\n';
                    break;
                case ' ':
                    slurpComment(item.translatorComments, lines, l);
                    break;
                case '.':
                    if (line.startsWith("#. ts-context ")) { // legacy
                        item.context = line.mid(14);
                    } else if (line.startsWith("#. ts-id ")) {
                        item.id = line.mid(9);
                    } else {
                        item.automaticComments += line.mid(3);

                    }
                    break;
                case '|':
                    if (line.startsWith("#| msgid ")) {
                        item.oldMsgId = slurpEscapedString(lines, l, 9, "#| ", cd);
                    } else if (line.startsWith("#| msgid_plural ")) {
                        QByteArray extra = slurpEscapedString(lines, l, 16, "#| ", cd);
                        if (extra != item.oldMsgId)
                            item.extra["po-old_msgid_plural"_L1] = toUnicode(extra);
                    } else if (line.startsWith("#| msgctxt ")) {
                        item.oldTscomment = slurpEscapedString(lines, l, 11, "#| ", cd);
                        if (qtContexts)
                            splitContext(&item.oldTscomment, &item.context);
                    } else {
                        cd.appendError(QString("PO-format parse error in line %1: '%2'"_L1)
                                               .arg(l + 1)
                                               .arg(toUnicode(lines[l])));
                        error = true;
                    }
                    break;
                case '~':
                    if (line.startsWith("#~ msgid ")) {
                        item.msgId = slurpEscapedString(lines, l, 9, "#~ ", cd);
                    } else if (line.startsWith("#~ msgid_plural ")) {
                        QByteArray extra = slurpEscapedString(lines, l, 16, "#~ ", cd);
                        if (extra != item.msgId)
                            item.extra["po-msgid_plural"_L1] = toUnicode(extra);
                        item.isPlural = true;
                    } else if (line.startsWith("#~ msgctxt ")) {
                        item.tscomment = slurpEscapedString(lines, l, 11, "#~ ", cd);
                        if (qtContexts)
                            splitContext(&item.tscomment, &item.context);
                    } else if (line.startsWith("#~| msgid ")) {
                        item.oldMsgId = slurpEscapedString(lines, l, 10, "#~| ", cd);
                    } else if (line.startsWith("#~| msgid_plural ")) {
                        QByteArray extra = slurpEscapedString(lines, l, 17, "#~| ", cd);
                        if (extra != item.oldMsgId)
                            item.extra["po-old_msgid_plural"_L1] = toUnicode(extra);
                    } else if (line.startsWith("#~| msgctxt ")) {
                        item.oldTscomment = slurpEscapedString(lines, l, 12, "#~| ", cd);
                        if (qtContexts)
                            splitContext(&item.oldTscomment, &item.context);
                    } else {
                        cd.appendError(QString("PO-format parse error in line %1: '%2'"_L1)
                                               .arg(l + 1)
                                               .arg(toUnicode(lines[l])));
                        error = true;
                    }
                    break;
                default:
                    cd.appendError(QString("PO-format parse error in line %1: '%2'"_L1)
                                           .arg(l + 1)
                                           .arg(toUnicode(lines[l])));
                    error = true;
                    break;
            }
            lastCmtLine = l;
        } else if (line.startsWith("msgctxt ")) {
            item.tscomment = slurpEscapedString(lines, l, 8, QByteArray(), cd);
            if (qtContexts)
                splitContext(&item.tscomment, &item.context);
        } else if (line.startsWith("msgid ")) {
            item.msgId = slurpEscapedString(lines, l, 6, QByteArray(), cd);
        } else if (line.startsWith("msgid_plural ")) {
            QByteArray extra = slurpEscapedString(lines, l, 13, QByteArray(), cd);
            if (extra != item.msgId)
                item.extra["po-msgid_plural"_L1] = toUnicode(extra);
            item.isPlural = true;
        } else {
            cd.appendError(QString("PO-format error in line %1: '%2'"_L1)
                                   .arg(l + 1)
                                   .arg(toUnicode(lines[l])));
            error = true;
        }
    }
    return !error && cd.errors().isEmpty();
}

static void addPoHeader(Translator::ExtraData &headers, QStringList &hdrOrder,
                        const char *name, const QString &value)
{
    QString qName = QLatin1String(name);
    if (!hdrOrder.contains(qName))
        hdrOrder << qName;
    headers[makePoHeader(qName)] = value;
}

static QString escapeComment(const QString &in, bool escape)
{
    QString out = in;
    if (escape) {
        out.replace(u'~', "~~"_L1);
        out.replace(u'|', "~|"_L1);
    }
    return out;
}

bool savePO(const Translator &translator, QIODevice &dev, ConversionData &)
{
    QString str_format = "-format"_L1;

    bool ok = true;
    QTextStream out(&dev);

    bool qtContexts = false;
    for (const TranslatorMessage &msg : translator.messages())
        if (!msg.context().isEmpty()) {
            qtContexts = true;
            break;
        }

    QString cmt = translator.extra("po-header_comment"_L1);
    if (!cmt.isEmpty())
        out << cmt << '\n';
    out << "msgid \"\"\n";
    Translator::ExtraData headers = translator.extras();
    QStringList hdrOrder = translator.extra("po-headers"_L1).split(u',', Qt::SkipEmptyParts);
    // Keep in sync with loadPO
    addPoHeader(headers, hdrOrder, "MIME-Version", "1.0"_L1);
    addPoHeader(headers, hdrOrder, "Content-Type", "text/plain; charset=UTF-8"_L1);
    addPoHeader(headers, hdrOrder, "Content-Transfer-Encoding", "8bit"_L1);
    if (!translator.languageCode().isEmpty()) {
        QLocale::Language l;
        QLocale::Territory c;
        Translator::languageAndTerritory(translator.languageCode(), &l, &c);
        const char *gettextRules;
        if (getNumerusInfo(l, c, 0, 0, &gettextRules))
            addPoHeader(headers, hdrOrder, "Plural-Forms", QLatin1String(gettextRules));
        addPoHeader(headers, hdrOrder, "X-Language", translator.languageCode());
    }
    if (!translator.sourceLanguageCode().isEmpty())
        addPoHeader(headers, hdrOrder, "X-Source-Language", translator.sourceLanguageCode());
    if (qtContexts)
        addPoHeader(headers, hdrOrder, "X-Qt-Contexts", "true"_L1);
    QString hdrStr;
    for (const QString &hdr : std::as_const(hdrOrder)) {
        hdrStr += hdr;
        hdrStr += ": "_L1;
        hdrStr += headers.value(makePoHeader(hdr));
        hdrStr += u'\n';
    }
    out << poEscapedString(QString(), QString::fromLatin1("msgstr"), true, hdrStr);

    for (const TranslatorMessage &msg : translator.messages()) {
        out << Qt::endl;

        if (!msg.translatorComment().isEmpty())
            out << poEscapedLines("#"_L1, true, msg.translatorComment());

        if (!msg.extraComment().isEmpty())
            out << poEscapedLines("#."_L1, true, msg.extraComment());

        if (!msg.id().isEmpty())
            out << "#. ts-id "_L1 << msg.id() << '\n';

        QString xrefs = msg.extra("po-references"_L1);
        if (!msg.fileName().isEmpty() || !xrefs.isEmpty()) {
            QStringList refs;
            for (const TranslatorMessage::Reference &ref : msg.allReferences())
                refs.append(QString("%2:%1"_L1).arg(ref.lineNumber()).arg(ref.fileName()));
            if (!xrefs.isEmpty())
                refs << xrefs;
            out << poWrappedEscapedLines("#:"_L1, true, refs.join(u' '));
        }

        bool noWrap = false;
        bool skipFormat = false;
        QStringList flags;
        if ((msg.type() == TranslatorMessage::Unfinished
             || msg.type() == TranslatorMessage::Obsolete) && msg.isTranslated())
            flags.append("fuzzy"_L1);
        const auto itr = msg.extras().constFind("po-flags"_L1);
        if (itr != msg.extras().cend()) {
            const QStringList atoms = itr->split(", "_L1);
            for (const QString &atom : atoms)
                if (atom.endsWith(str_format)) {
                    skipFormat = true;
                    break;
                }
            if (atoms.contains("no-wrap"_L1))
                noWrap = true;
            flags.append(*itr);
        }
        if (!skipFormat) {
            QString source = msg.sourceText();
            // This is fuzzy logic, as we don't know whether the string is
            // actually used with QString::arg().
            for (int off = 0; (off = source.indexOf(u'%', off)) >= 0;) {
                if (++off >= source.size())
                    break;
                if (source.at(off) == u'n' || source.at(off).isDigit()) {
                    flags.append("qt-format"_L1);
                    break;
                }
            }
        }
        if (!flags.isEmpty())
            out << "#, " << flags.join(", "_L1) << '\n';

        bool isObsolete = (msg.type() == TranslatorMessage::Obsolete
                           || msg.type() == TranslatorMessage::Vanished);
        QString prefix = QLatin1String(isObsolete ? "#~| " : "#| ");
        if (!msg.oldComment().isEmpty())
            out << poEscapedString(prefix, "msgctxt"_L1, noWrap,
                                   escapeComment(msg.oldComment(), qtContexts));
        if (!msg.oldSourceText().isEmpty())
            out << poEscapedString(prefix, "msgid"_L1, noWrap, msg.oldSourceText());
        QString plural = msg.extra("po-old_msgid_plural"_L1);
        if (!plural.isEmpty())
            out << poEscapedString(prefix, "msgid_plural"_L1, noWrap, plural);
        prefix = QLatin1String(isObsolete ? "#~ " : "");
        if (!msg.context().isEmpty())
            out << poEscapedString(prefix, "msgctxt"_L1, noWrap,
                                   escapeComment(msg.context(), true) + u'|'
                                           + escapeComment(msg.comment(), true));
        else if (!msg.comment().isEmpty())
            out << poEscapedString(prefix, "msgctxt"_L1, noWrap,
                                   escapeComment(msg.comment(), qtContexts));
        out << poEscapedString(prefix, "msgid"_L1, noWrap, msg.sourceText());
        if (!msg.isPlural()) {
            QString transl = msg.translation();
            transl.replace(Translator::BinaryVariantSeparator, Translator::TextVariantSeparator);
            out << poEscapedString(prefix, "msgstr"_L1, noWrap, transl);
        } else {
            QString plural = msg.extra("po-msgid_plural"_L1);
            if (plural.isEmpty())
                plural = msg.sourceText();
            out << poEscapedString(prefix, "msgid_plural"_L1, noWrap, plural);
            const QStringList &translations = msg.translations();
            for (int i = 0; i != translations.size(); ++i) {
                QString str = translations.at(i);
                str.replace(QChar(Translator::BinaryVariantSeparator),
                            QChar(Translator::TextVariantSeparator));
                out << poEscapedString(prefix, QString::fromLatin1("msgstr[%1]").arg(i), noWrap,
                                       str);
            }
        }
    }
    return ok;
}

static bool savePOT(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
    Translator ttor = translator;
    ttor.dropTranslations();
    return savePO(ttor, dev, cd);
}

int initPO()
{
    Translator::FileFormat format;
    format.extension = "po"_L1;
    format.untranslatedDescription = QT_TRANSLATE_NOOP("FMT", "GNU Gettext localization files");
    format.loader = &loadPO;
    format.saver = &savePO;
    format.fileType = Translator::FileFormat::TranslationSource;
    format.priority = 1;
    Translator::registerFileFormat(format);
    format.extension = "pot"_L1;
    format.untranslatedDescription = QT_TRANSLATE_NOOP("FMT", "GNU Gettext localization template files");
    format.loader = &loadPO;
    format.saver = &savePOT;
    format.fileType = Translator::FileFormat::TranslationSource;
    format.priority = -1;
    Translator::registerFileFormat(format);
    return 1;
}

Q_CONSTRUCTOR_FUNCTION(initPO)

QT_END_NAMESPACE
