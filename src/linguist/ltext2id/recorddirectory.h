// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RECORDDIRECTORY_H
#define RECORDDIRECTORY_H

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>

#include <map>
#include <memory>
#include <unordered_set>
#include <set>

QT_BEGIN_NAMESPACE

class TranslatorMessage;

using namespace Qt::StringLiterals;

extern const char *meta_id_key;
struct MessageItem
{
    QString sourceText;
    QString context;
    QString comment;
    QString id;
    int lineNo = 0;
    int startOffset = 0;
    int endOffset = 0;
    bool plural = 0;
    bool hasMetaId = 0;
    bool operator==(const MessageItem &mi) const noexcept
    {
        return sourceText == mi.sourceText && context == mi.context && comment == mi.comment;
    }
};

struct MsgPtrComp
{
    bool operator()(const std::shared_ptr<MessageItem> &lhs,
                    const std::shared_ptr<MessageItem> &rhs) const noexcept
    {
        if (lhs->lineNo != rhs->lineNo)
            return lhs->lineNo < rhs->lineNo;
        else
            return lhs->startOffset < rhs->endOffset;
    }
};

QT_END_NAMESPACE

namespace std {
template <>
struct hash<MessageItem>
{
    std::size_t operator()(const MessageItem &key) const
    {
        std::size_t h1 = qHash(key.context);
        std::size_t h2 = qHash(key.sourceText);
        std::size_t h3 = qHash(key.comment);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};
} // namespace std

QT_BEGIN_NAMESPACE

class RecordDirectory
{
public:
    using MsgLocations = QHash<QString, std::set<std::shared_ptr<MessageItem>, MsgPtrComp>>;
    using ErrorLocations = QHash<QString, QMap<int, QString>>;

    void recordMessage(const TranslatorMessage &msg);
    QString id(const TranslatorMessage &msg) const;
    void recordExistingId(const QString &id);
    void recordError(const QString &file, int line, const QString &error);
    void recordNonSupported(const QString &file, int line) { m_nonSupporteds[file].insert(line); }
    void recordAddedLines(const QString &file, int fromLine, int addedLines);
    bool isNonSupported(const QString &file, int line) const
    {
        return m_nonSupporteds[file].contains(line);
    }
    int addedLines(const QString &file, int line) const;
    bool containsFile(const QString &filename) const noexcept;
    const MsgLocations &messageLocations() const noexcept { return m_msgLocations; }
    const ErrorLocations &errors() const noexcept { return m_errors; }
    QString calculateId(const TranslatorMessage &msg) const;
    ;

private:
    std::unordered_set<MessageItem> m_messages;
    MsgLocations m_msgLocations;
    QHash<QString, QSet<int>> m_nonSupporteds;
    ErrorLocations m_errors;
    std::unordered_map<QString, std::map<int, int>> m_fileAddedLines;
    std::unordered_set<QString> m_existingIds;
};

QT_END_NAMESPACE

#endif // RECORDDIRECTORY_H
