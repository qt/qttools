// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CPP_H
#define CPP_H

#include "lupdate.h"

#include <QtCore/QSet>
#include <QtCore/QStack>

QT_BEGIN_NAMESPACE

struct HashString {
    HashString() : m_hash(0x80000000) {}
    explicit HashString(const QString &str) : m_str(str), m_hash(0x80000000) {}
    void setValue(const QString &str) { m_str = str; m_hash = 0x80000000; }
    const QString &value() const { return m_str; }
    bool operator==(const HashString &other) const { return m_str == other.m_str; }
    QString m_str;

    mutable uint m_hash; // We use the highest bit as a validity indicator (set => invalid)
};

QDebug operator<<(QDebug debug, const HashString &s);

struct HashStringList {
    explicit HashStringList(const QList<HashString> &list) : m_list(list), m_hash(0x80000000) {}
    const QList<HashString> &value() const { return m_list; }
    bool operator==(const HashStringList &other) const { return m_list == other.m_list; }

    QList<HashString> m_list;
    mutable uint m_hash; // We use the highest bit as a validity indicator (set => invalid)
};

QDebug operator<<(QDebug debug, const HashStringList &lst);

typedef QList<HashString> NamespaceList;

struct Namespace {

    Namespace() : parent(nullptr), classDef(this), hasTrFunctions(false), complained(false) { }
    ~Namespace()
    {
        qDeleteAll(children);
    }

    Namespace *parent;
    QHash<HashString, Namespace *> children;
    QHash<HashString, NamespaceList> aliases;
    QList<HashStringList> usings;

    // Class declarations set no flags and create no namespaces, so they are ignored.
    // Class definitions may appear multiple times - but only because we are trying to
    // "compile" all sources irrespective of build configuration.
    // Nested classes may be forward-declared inside a definition, and defined in another file.
    // The latter will detach the class' child list, so clones need a backlink to the original
    // definition (either one in case of multiple definitions).
    // Namespaces can have tr() functions as well, so we need to track parent definitions for
    // them as well. The complication is that we may have to deal with a forrest instead of
    // a tree - in that case the parent will be arbitrary. However, it seem likely that
    // Q_DECLARE_TR_FUNCTIONS would be used either in "class-like" namespaces with a central
    // header or only locally in a file.
    Namespace *classDef;

    QString trQualification;

    bool hasTrFunctions;
    bool complained; // ... that tr functions are missing.
};

struct ParseResults {
    int fileId;
    Namespace rootNamespace;
    QSet<const ParseResults *> includes;
};

struct IncludeCycle {
    QSet<QString> fileNames;
    QSet<const ParseResults *> results;
};

struct CppParserState
{
    NamespaceList namespaces;
    QStack<qsizetype> namespaceDepths;
    NamespaceList functionContext;
    QString functionContextUnresolved;
    QString pendingContext;

    bool operator==(const CppParserState &other) const
    {
        return namespaces == other.namespaces
            && namespaceDepths == other.namespaceDepths
            && functionContext == other.functionContext
            && functionContextUnresolved == other.functionContextUnresolved
            && pendingContext == other.pendingContext;
    }
};

size_t qHash(const CppParserState &s, size_t seed);

struct ResultsCacheKey
{
    const QString cleanFile;
    const CppParserState parserState;

    ResultsCacheKey(const QString &filePath)
        : cleanFile(filePath)
    {
    }

    ResultsCacheKey(const QString &filePath, const CppParserState &state)
        : cleanFile(filePath),
          parserState(state)
    {
    }

    bool operator==(const ResultsCacheKey &other) const
    {
        return cleanFile == other.cleanFile
            && parserState == other.parserState;
    }
};

size_t qHash(const ResultsCacheKey &key, size_t seed);

typedef QHash<ResultsCacheKey, IncludeCycle *> IncludeCycleHash;
typedef QHash<QString, const Translator *> TranslatorHash;

class CppFiles {
public:
    static QSet<const ParseResults *> getResults(const ResultsCacheKey &key);
    static void setResults(const ResultsCacheKey &key, const ParseResults *results);
    static const Translator *getTranslator(const QString &cleanFile);
    static void setTranslator(const QString &cleanFile, const Translator *results);
    static bool isBlacklisted(const QString &cleanFile);
    static void setBlacklisted(const QString &cleanFile);
    static void addIncludeCycle(const QSet<QString> &fileNames, const CppParserState &parserState);

private:
    static IncludeCycleHash &includeCycles();
    static TranslatorHash &translatedFiles();
    static QSet<QString> &blacklistedFiles();
};

QT_END_NAMESPACE

#endif // CPP_H
