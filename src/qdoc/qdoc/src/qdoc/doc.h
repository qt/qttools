// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef DOC_H
#define DOC_H

#include "location.h"
#include "docutilities.h"
#include "topic.h"

#include "filesystem/fileresolver.h"
#include "boundaries/filesystem/resolvedfile.h"

#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Atom;
class CodeMarker;
class DocPrivate;
class Quoter;
class Text;

typedef std::pair<QString, QString> ArgPair;
typedef QList<ArgPair> ArgList;
typedef QMultiMap<QString, QString> QStringMultiMap;

class Doc
{
public:
    // the order is important
    enum Sections {
        NoSection = -1,
        Section1 = 1,
        Section2 = 2,
        Section3 = 3,
        Section4 = 4
    };

    Doc() = default;
    Doc(const Location &start_loc, const Location &end_loc, const QString &source,
        const QSet<QString> &metaCommandSet, const QSet<QString> &topics);
    Doc(const Doc &doc);
    ~Doc();

    Doc &operator=(const Doc &doc);

    [[nodiscard]] const Location &location() const;
    [[nodiscard]] const Location &startLocation() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] const QString &source() const;
    [[nodiscard]] const Text &body() const;
    [[nodiscard]] Text briefText(bool inclusive = false) const;
    [[nodiscard]] Text trimmedBriefText(const QString &className) const;
    [[nodiscard]] Text legaleseText() const;
    [[nodiscard]] QSet<QString> parameterNames() const;
    [[nodiscard]] QStringList enumItemNames() const;
    [[nodiscard]] QStringList omitEnumItemNames() const;
    [[nodiscard]] QSet<QString> metaCommandsUsed() const;
    [[nodiscard]] TopicList topicsUsed() const;
    [[nodiscard]] ArgList metaCommandArgs(const QString &metaCommand) const;
    [[nodiscard]] QList<Text> alsoList() const;
    [[nodiscard]] bool hasTableOfContents() const;
    [[nodiscard]] bool hasKeywords() const;
    [[nodiscard]] bool hasTargets() const;
    [[nodiscard]] bool isInternal() const;
    [[nodiscard]] bool isMarkedReimp() const;
    [[nodiscard]] const QList<Atom *> &tableOfContents() const;
    [[nodiscard]] const QList<int> &tableOfContentsLevels() const;
    [[nodiscard]] const QList<Atom *> &keywords() const;
    [[nodiscard]] const QList<Atom *> &targets() const;
    [[nodiscard]] QStringMultiMap *metaTagMap() const;

    static void initialize(FileResolver& file_resolver);
    static void terminate();
    static void trimCStyleComment(Location &location, QString &str);
    static CodeMarker *quoteFromFile(const Location &location, Quoter &quoter,
                                     ResolvedFile resolved_file);

private:
    void detach();
    DocPrivate *m_priv { nullptr };
    static DocUtilities &m_utilities;
};
Q_DECLARE_TYPEINFO(Doc, Q_RELOCATABLE_TYPE);
typedef QList<Doc> DocList;

QT_END_NAMESPACE

#endif
