// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "recorddirectory.h"
#include <translatormessage.h>

#include <QMap>

QT_BEGIN_NAMESPACE

const char *meta_id_key = "meta-id";

void RecordDirectory::recordMessage(const TranslatorMessage &msg)
{
    MessageItem mi{ msg.sourceText(), msg.context(),    msg.comment(),
                    QString(),        msg.lineNumber(), msg.startOffset(),
                    msg.endOffset(),  msg.isPlural(),   !msg.extra(meta_id_key).isEmpty() };
    if (const auto it = m_messages.find(mi); it != m_messages.end())
        mi.id = it->id;
    else if (QString id = msg.id(); id.isEmpty()) {
        mi.id = calculateId(msg);
        m_messages.emplace(mi);
    } else // this is only the case in verification
        mi.id = std::move(id);
    for (const TranslatorMessage::Reference &ref : msg.allReferences()) {
        auto ref_mi = std::make_shared<MessageItem>(mi);
        ref_mi->lineNo = ref.lineNumber();
        ref_mi->startOffset = ref.startOffset();
        ref_mi->endOffset = ref.endOffset();
        m_msgLocations[ref.fileName()].insert(std::move(ref_mi));
    }
}

void RecordDirectory::recordExistingId(const QString &id)
{
    m_existingIds.emplace(id);
}

void RecordDirectory::recordError(const QString &file, int line, const QString &error)
{
    if (auto &fileErrors = m_errors[file]; !fileErrors.contains(line))
        fileErrors[line] = "//ltext2id error: "_L1 + error;
}

void RecordDirectory::recordAddedLines(const QString &file, int fromLine, int addedLines)
{
    m_fileAddedLines[file].emplace(fromLine, addedLines);
}

int RecordDirectory::addedLines(const QString &file, int line) const
{
    int addedLines = 0;
    if (const auto fit = m_fileAddedLines.find(file); fit != m_fileAddedLines.cend()) {
        const auto &fileAddedLines = fit->second;
        auto itr = fileAddedLines.upper_bound(line);
        if (itr != fileAddedLines.cbegin()) {
            itr--;
            addedLines = itr->second;
        }
    }
    return addedLines;
}

bool RecordDirectory::containsFile(const QString &filename) const noexcept
{
    return m_msgLocations.contains(filename);
}

QString RecordDirectory::calculateId(const TranslatorMessage &msg) const
{
    QString id = msg.extra(meta_id_key);
    if (!id.isEmpty())
        return id;

    const QString &source = msg.sourceText();
    int words = 0;
    int index = 0;

    while (index < source.size()) {
        QChar c = source[index];
        bool word = false;
        while (!c.isLetterOrNumber()) {
            id += QChar('a' + c.unicode() % 32);
            word = true;
            if (++index == source.size())
                break;
            c = source[index];
        }
        if (word)
            words++;

        if (words == 2)
            break;
        id += c;
        index++;
    }

    if (!id.isEmpty() && id.front() == '-')
        id = msg.context() + id;
    else
        id = msg.context() + '-' + id;

    if (index < source.size() || !msg.comment().isEmpty())
        id += '-' + QString::number(qHash(source + msg.comment()) % 100000);

    if (m_existingIds.count(id))
        id += "_2"_L1;
    return id;
}

QString RecordDirectory::id(const TranslatorMessage &msg) const
{
    MessageItem mi{ msg.sourceText(), msg.context(), msg.comment(), QString() };
    if (const auto it = m_messages.find(mi); it != m_messages.end())
        return it->id;
    return {};
}

QT_END_NAMESPACE
