// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "fileverifier.h"
#include "utils.h"
#include "recorddirectory.h"
#include <translator.h>
#include <trlib/trparser.h>

#include <QFileInfo>
#include <QMap>

using namespace Qt::StringLiterals;
using namespace Utils;

QT_BEGIN_NAMESPACE

FileVerifier::FileVerifier(const RecordDirectory &records, bool quiet)
    : m_records(records), m_quiet(quiet)
{
}

bool FileVerifier::verifyTs(const QString &tsFile, QSet<QString> &transformedIds)
{
    bool verifyFail = false;
    ConversionData cd;
    Translator verifyTor;
    verifyTor.load(tsFile, cd, "ts");
    verifyTor.makeFileNamesAbsolute(QFileInfo(tsFile).absoluteDir());

    if (!cd.errors().empty()) {
        printErr("ltext2id error: cannot load the TS file %1 after transformation. "
                 "Manual fix is necessary."_L1.arg(tsFile));
        verifyFail = true;
    }

    for (qsizetype i = 0; i < verifyTor.messageCount(); i++) {
        const TranslatorMessage &msg = verifyTor.message(i);
        if (!msg.context().isEmpty()
            && !m_records.isNonSupported(msg.fileName(), msg.lineNumber())) {
            qWarning()
                    << "ltext2id error: failed to transform message with context '%1' and source '%2' in TS file %3"_L1
                               .arg(msg.context(), msg.sourceText(), tsFile);
            verifyFail = true;
        } else
            transformedIds.remove(msg.id());
    }
    for (const QString &missingId : transformedIds) {
        printErr("ltext2id error: missing id '%1' from the TS file %2."_L1.arg(missingId).arg(
                tsFile));
        verifyFail = true;
    }
    return !verifyFail;
}

void FileVerifier::verifySources(const QStringList &sources, ConversionData &cd)
{
    Translator verifyTor;
    processSources(verifyTor, sources, cd);

    RecordDirectory verifyRecords;
    for (const TranslatorMessage &msg : verifyTor.messages())
        verifyRecords.recordMessage(msg);

    for (const auto &[filename, messages] : m_records.messageLocations().asKeyValueRange()) {
        if (!m_quiet)
            printOut("ltext2id: verifying source file %1"_L1.arg(filename));
        const auto &verifyMessages = verifyRecords.messageLocations()[filename];
        auto transformedItr = verifyMessages.cbegin();
        for (const std::shared_ptr<MessageItem> &original : messages) {
            if (m_records.isNonSupported(filename, original->lineNo)
                && transformedItr != verifyMessages.cend()) {
                transformedItr++;
                continue;
            }
            // fast forward the ignored messages or the ones that were
            // already id based before transformation
            while (transformedItr != verifyMessages.cend()
                   && transformedItr->get()->lineNo < original->lineNo)
                transformedItr++;

            if (transformedItr == verifyMessages.cend())
                const_cast<RecordDirectory &>(m_records).recordError(
                        filename, original->lineNo,
                        "Missing translation call with the expected id '%1' after transformation"_L1
                                .arg(original->id));
            else {
                const MessageItem &transformed = *transformedItr->get();
                if (original->sourceText != transformed.sourceText || original->id != transformed.id
                    || original->lineNo != transformed.lineNo
                    || original->plural != transformed.plural) {
                    const_cast<RecordDirectory &>(m_records).recordError(filename, original->lineNo,
                                                                         original->id);
                } else
                    transformedItr++;
            }
        }
    }

    for (const auto &[fileName, fileErrors] : m_records.errors().asKeyValueRange()) {
        QStringList lines = readLines(fileName);
        for (const auto &[lineNo, error] : fileErrors.asKeyValueRange()) {
            auto &line = lines[lineNo - 1];
            const QString indentation = getIndentation(line);
            line = indentation + error + '\n' + line;
        }
        writeLines(fileName, lines);
    }
}

QT_END_NAMESPACE
