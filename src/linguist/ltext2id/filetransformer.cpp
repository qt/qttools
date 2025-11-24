// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "filetransformer.h"
#include "fileverifier.h"
#include "utils.h"
#include "recorddirectory.h"
#include <translator.h>
#include <trlib/metastrings.h>
#include <trlib/trparser.h>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QSet>
#include <QRegularExpression>

using namespace Qt::StringLiterals;
using namespace Utils;

namespace {

// UI elements for XML processing
constexpr QLatin1String uiElement = "<ui "_L1;
constexpr QLatin1String stringElement = "<string"_L1;
constexpr QLatin1String stringListElement = "<stringlist"_L1;

QString escapeForCppStringLiteral(const QString &str)
{
    QString result = str;
    result.replace("\\", "\\\\");
    result.replace("\"", "\\\"");
    return result;
}

QString textMetaString(const QString &indentation, const QString &sourceText)
{
    const QStringList lines = sourceText.split('\n');
    QString metaString;
    for (int i = 0; i < lines.size(); i++) {
        metaString += indentation + "//" + MetaStrings::sourceTextAnotation + " \""
                + escapeForCppStringLiteral(lines[i]);
        if (i < lines.size() - 1)
            metaString += "\\n\"\n";
        else
            metaString += "\"\n";
    }
    return metaString;
}

QString labelMetaString(const QString &indentation, const QString &label)
{
    return indentation + "//" + MetaStrings::labelAnotation + ' ' + label + '\n';
}

QString idMetaString(const QString &indentation, const QString &id)
{
    return indentation + "//" + MetaStrings::extraAnotation + " meta-id " + id + '\n';
}

QRegularExpression idMetaStringRegex(const QString &id)
{
    const QString e = QRegularExpression::escape(id);
    const QString pat = QStringLiteral(R"(^[ \t]*//~ meta-id\s*%1[ \t]*(?:\r?\n|$))").arg(e);
    return QRegularExpression(pat, QRegularExpression::MultilineOption);
}

QString idBasedFunc(int trFunc, const QString &id, const QString &pluralArg)
{
    switch (trFunc) {
    case TrFunctionAliasManager::Function_trUtf8:
    case TrFunctionAliasManager::Function_tr:
    case TrFunctionAliasManager::Function_translate:
        return "qtTrId(\"" + id + "\"" + pluralArg + ")";
    case TrFunctionAliasManager::Function_qsTr:
    case TrFunctionAliasManager::Function_qsTranslate:
        return "qsTrId(\"" + id + "\"" + pluralArg + ")";
    case TrFunctionAliasManager::Function_QT_TR_NOOP:
    case TrFunctionAliasManager::Function_QT_TR_NOOP_UTF8:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP_UTF8:
        return "QT_TRID_NOOP(\"" + id + "\")";
    case TrFunctionAliasManager::Function_QT_TR_N_NOOP:
    case TrFunctionAliasManager::Function_QT_TRANSLATE_N_NOOP:
        return "QT_TRID_N_NOOP(\"" + id + "\")";
    default:
        return {};
    }
}

int getTrFunction(const QString &expr)
{
    int i = 0;
    int depth = 0;
    QString trFunc;
    while (i < expr.size()) {
        if (QChar c = expr[i++]; (c.isLetterOrNumber() || c == '_'_L1) && depth == 0)
            trFunc += c;
        else {
            while (c.isSpace() && i < expr.size())
                c = expr[i++];

            if (c == '(') {
                if (depth == 0)
                    if (int trFuncId = trFunctionAliasManager.trFunctionByName(trFunc);
                        trFuncId >= 0)
                        return trFuncId;
                depth++;
            } else if (c == ')'_L1)
                depth--;
            else if (c == "\""_L1 || c == "'"_L1) {
                const QChar quotation = c;
                while (i < expr.size()) {
                    c = expr[i++];
                    if (c == '\\'_L1)
                        i++;
                    else if (c == quotation)
                        break;
                }
            }
            trFunc.clear();
        }
    }
    return -1;
}

QString getPluralArg(const QString &fn, bool plural)
{
    if (!plural)
        return {};
    int paren = 1;
    int pos = fn.size() - 2;
    while (paren > 0 && pos >= 0) {
        if (fn[pos] == '('_L1)
            paren--;
        else if (fn[pos] == ')'_L1)
            paren++;
        if (paren == 1 && fn[pos] == ','_L1)
            return ' ' + fn.sliced(pos).removeLast().simplified();
        pos--;
    }
    return {};
}

void transformMessageNoLocation(TranslatorMessage &msg, const RecordDirectory &records,
                                QSet<QString> &ids, Translator &transformedTor, bool labels)
{
    if (msg.id().isEmpty()) {
        const QString id = records.calculateId(msg);
        if (labels)
            msg.setLabel(msg.context());
        msg.setId(id);
        msg.setContext({});
        msg.setComment({});
        ids.insert(id);
    }
    transformedTor.append(msg);
}

void transformMessageWithLocation(TranslatorMessage &msg, const RecordDirectory &records,
                                  QSet<QString> &ids, Translator &transformedTor, bool labels)
{
    TranslatorMessage::References normalRefs;
    TranslatorMessage::References nonsupportedRefs;
    for (const TranslatorMessage::Reference &r : msg.allReferences()) {
        TranslatorMessage::Reference ur{ r.fileName(),
                                         r.lineNumber()
                                                 + records.addedLines(r.fileName(), r.lineNumber()),
                                         r.startOffset(), r.endOffset() };
        if (records.isNonSupported(ur.fileName(), ur.lineNumber()))
            nonsupportedRefs.append(ur);
        else
            normalRefs.append(ur);
    }
    msg.clearReferences();
    if (!nonsupportedRefs.isEmpty()) {
        TranslatorMessage newMsg = msg;
        newMsg.setReferences(nonsupportedRefs);
        transformedTor.append(newMsg);
    }
    if (QString ctx = msg.context(); !ctx.isEmpty() && !normalRefs.empty()) {
        msg.setReferences(normalRefs);
        const QString id = records.id(msg);
        msg.setId(id);
        msg.setContext({});
        ids.insert(id);
        msg.setComment({});
        if (labels)
            msg.setLabel(ctx);
        transformedTor.append(msg);
    } else if (!normalRefs.empty()) {
        msg.setReferences(normalRefs);
        transformedTor.append(msg);
    }
}

bool makeFormIdBased(QStringList &lines, const QString &filename, const QString &label)
{
    auto itr = lines.begin();
    qsizetype pos;
    do // skip comment lines
        pos = itr++->indexOf(uiElement);
    while (pos < 0 && itr != lines.end());

    if (pos < 0) {
        printErr("ltext2id: no root element in the "
                 "ui file %1. Ignoring."_L1.arg(filename));
        return false;
    }
    itr--;

    if (!label.isEmpty() && itr->indexOf("label=") < 0)
        itr->insert(pos + uiElement.size(), "label=\"" + label + "\" ");

    if (itr->indexOf("idbasedtr") < 0)
        itr->insert(pos + uiElement.size(), "idbasedtr=\"true\" ");
    else
        itr->replace("idbasedtr=\"false\"", "idbasedtr=\"true\"");

    return true;
}
} // namespace

QT_BEGIN_NAMESPACE

// Static member definitions
const QSet<QString> FileTransformer::cppExtensions{ "c"_L1,   "c++"_L1, "cc"_L1, "cpp"_L1,
                                                    "cxx"_L1, "ch"_L1,  "h"_L1,  "h++"_L1,
                                                    "hh"_L1,  "hpp"_L1, "hxx"_L1 };

const QSet<QString> FileTransformer::otherExtensions{
    "jui"_L1, "ui"_L1, "js"_L1, "mjs"_L1, "qml"_L1, // python doesn't have id based
};

FileTransformer::FileTransformer(RecordDirectory &records, bool labels, bool quiet)
    : m_records(records), m_labels(labels), m_quiet(quiet)
{
}

void FileTransformer::transformUi()
{
    for (const auto &[filename, messages] : m_records.messageLocations().asKeyValueRange()) {
        if (filename.endsWith(".ui", Qt::CaseInsensitive)) {
            QStringList lines = readLines(filename);
            if (lines.empty())
                continue;

            if (!m_quiet)
                printOut("ltext2id: processing source file %1"_L1.arg(filename));
            if (m_labels)
                makeFormIdBased(lines, filename, (*messages.begin())->context);

            for (const std::shared_ptr<MessageItem> &msg : messages) {
                QString &line = lines[msg->lineNo - 1];
                qsizetype pos = line.indexOf(stringElement);
                qsizetype size = stringElement.size();
                if (pos < 0 || line.size() < pos + size
                    || (line.at(pos + size) != '>' && !line.at(pos + size).isSpace())) {
                    pos = line.indexOf(stringListElement);
                    size = stringListElement.size();
                }
                if (pos < 0 || line.size() < pos + size
                    || (line.at(pos + size) != '>' && !line.at(pos + size).isSpace())) {
                    printErr("ltext2id error: could not find the "
                             "expected translatable string in %1:%2.\n"_L1.arg(filename)
                                     .arg(msg->lineNo));
                    m_records.recordError(filename, msg->lineNo,
                                          "please use id %1"_L1.arg(msg->id));
                    continue;
                }
                pos += size;
                line.insert(pos, " id=\"" + msg->id + "\"");
            }
            writeLines(filename, lines);
        }
    }
}

void FileTransformer::transformSources()
{
    for (const auto &[filename, messages] : m_records.messageLocations().asKeyValueRange()) {
        if (!filename.endsWith(".ui", Qt::CaseInsensitive)) {
            if (!m_quiet)
                printOut("ltext2id: processing source file %1"_L1.arg(filename));

            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly)) {
                printErr("ltext2id error: failed to open file %1 for reading.\n"_L1.arg(filename));
                continue;
            }
            QTextStream in(&file);
            const QString code = in.readAll();
            file.close();

            QString newCode;
            newCode.reserve(code.size());
            qsizetype lastPos = 0;
            int addedLines = 0;
            for (const std::shared_ptr<MessageItem> &msg : messages) {
                if (msg->startOffset >= 0 && msg->endOffset > msg->startOffset
                    && msg->endOffset <= code.size() && msg->startOffset >= lastPos) {
                    int lastLinePos = code.lastIndexOf('\n', msg->startOffset) + 1;
                    if (lastLinePos < lastPos)
                        lastLinePos = msg->startOffset;
                    const QString lastLine =
                            code.sliced(lastLinePos, msg->startOffset - lastLinePos);

                    const QString indentation = getIndentation(lastLine);
                    QString fnId = textMetaString(indentation, msg->sourceText);
                    if (m_labels)
                        fnId += labelMetaString(indentation, msg->context);
                    QString fn = code.sliced(msg->startOffset, msg->endOffset - msg->startOffset);
                    int trFunc = getTrFunction(fn);
                    QString idBasedFn = idBasedFunc(trFunc, msg->id, getPluralArg(fn, msg->plural));
                    if (idBasedFn.isEmpty()) {
                        msg->lineNo += addedLines;
                        if (trFunc >= 0)
                            m_records.recordNonSupported(filename, msg->lineNo);
                        else
                            m_records.recordError(filename, msg->lineNo,
                                                  "Could not detect any translation calls here"_L1);
                        continue;
                    }
                    fnId += lastLine + std::move(idBasedFn);

                    QString codePiece = code.sliced(lastPos, lastLinePos - lastPos);
                    if (msg->hasMetaId) {
                        codePiece.remove(idMetaStringRegex(msg->id));
                        addedLines--;
                    }
                    newCode += codePiece;
                    newCode += fnId;

                    addedLines += fnId.count('\n');
                    m_records.recordAddedLines(filename, msg->lineNo, addedLines);
                    msg->lineNo += addedLines;
                    if (const int origMsgLines = fn.count('\n'); origMsgLines) {
                        addedLines -= origMsgLines;
                        m_records.recordAddedLines(filename, msg->lineNo - addedLines + 1,
                                                   addedLines);
                    }
                    lastPos = msg->endOffset;
                } else {
                    m_records.recordError(
                            filename, msg->lineNo,
                            QString("Invalid location offsets for the translation call: %1-%2")
                                    .arg(msg->startOffset)
                                    .arg(msg->endOffset));
                }
            }
            newCode += code.sliced(lastPos);

            QFile outFile(filename);
            if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                printErr("ltext2id error: failed to open file %1 for writing.\n"_L1.arg(filename));
                continue;
            }
            QTextStream out(&outFile);
            out << newCode;
            outFile.close();
        }
    }
}

bool FileTransformer::transformTsFiles(const QStringList &translations, bool sortMessages)
{
    ConversionData cd;
    cd.m_sortMessages = sortMessages;
    bool ok = true;
    for (const QString &tsFile : std::as_const(translations)) {
        if (!m_quiet)
            printOut("ltext2id: processing TS file %1"_L1.arg(tsFile));
        Translator tor;
        tor.load(tsFile, cd, "ts");
        if (!cd.errors().empty()) { // timestamp files
            cd.clearErrors();
            continue;
        }
        tor.makeFileNamesAbsolute(QFileInfo(tsFile).absoluteDir());

        Translator transformedTor;
        QSet<QString> ids;
        for (qsizetype i = 0; i < tor.messageCount(); i++) {
            TranslatorMessage &msg = tor.message(i);
            if (const QString filename = msg.fileName();
                filename.isEmpty() || !m_records.containsFile(filename))
                transformMessageNoLocation(msg, m_records, ids, transformedTor, m_labels);
            else
                transformMessageWithLocation(msg, m_records, ids, transformedTor, m_labels);
        }
        transformedTor.save(tsFile, cd, "ts");
        if (!cd.errors().empty()) {
            printErr(
                    "ltext2id error: error in processing translation files\n%1"_L1.arg(cd.error()));
            ok = false;
        }

        if (!m_quiet)
            printOut("ltext2id: verifying TS file %1"_L1.arg(tsFile));

        FileVerifier verifier(m_records, m_quiet);
        if (!verifier.verifyTs(tsFile, ids)) {
            printErr("ltext2id: verifying TS file %1 failed."_L1.arg(tsFile));
            ok = false;
        }
    }
    return ok;
}

void FileTransformer::generateMetaIds()
{
    for (const auto &[filename, messages] : m_records.messageLocations().asKeyValueRange()) {
        if (!filename.endsWith(".ui", Qt::CaseInsensitive)) {
            if (!m_quiet)
                printOut("ltext2id: processing source file %1"_L1.arg(filename));

            QStringList lines = readLines(filename);
            int addedLines = 0;
            for (auto itr = messages.cbegin(); itr != messages.cend(); itr++) {
                const MessageItem &m = **itr;
                QString &line = lines[m.lineNo - 1];
                const QString indentation = getIndentation(lines[m.lineNo - 1]);
                line = idMetaString(indentation, m.id) + line;
                m_records.recordAddedLines(filename, m.lineNo, ++addedLines);
            }
            writeLines(filename, lines);
        }
    }
}

bool FileTransformer::updateTsFiles(const QStringList &translations)
{
    ConversionData cd;
    bool ok = true;
    for (const QString &tsFile : std::as_const(translations)) {
        if (!m_quiet)
            printOut("ltext2id: processing TS file %1"_L1.arg(tsFile));
        Translator tor;
        tor.load(tsFile, cd, "ts");
        if (!cd.errors().empty()) { // timestamp files
            cd.clearErrors();
            continue;
        }
        tor.makeFileNamesAbsolute(QFileInfo(tsFile).absoluteDir());
        Translator transformedTor;

        for (qsizetype i = 0; i < tor.messageCount(); i++) {
            TranslatorMessage &msg = tor.message(i);
            TranslatorMessage::References refs;
            for (const TranslatorMessage::Reference &r : msg.allReferences()) {
                if (const QString filename = r.fileName();
                    !filename.isEmpty() && m_records.containsFile(filename)) {
                    TranslatorMessage::Reference ur{
                        filename, r.lineNumber() + m_records.addedLines(filename, r.lineNumber()),
                        r.startOffset(), r.endOffset()
                    };
                    refs.append(ur);
                }
            }
            msg.clearReferences();
            msg.setReferences(refs);
            msg.setExtra(meta_id_key, m_records.id(msg));
            transformedTor.append(msg);
        }

        transformedTor.save(tsFile, cd, "ts");
        if (!cd.errors().empty()) {
            printErr(
                    "ltext2id error: error in processing translation files\n%1"_L1.arg(cd.error()));
            ok = false;
        }

        if (!m_quiet)
            printOut("ltext2id: verifying TS file %1"_L1.arg(tsFile));
    }
    return ok;
}

QT_END_NAMESPACE
