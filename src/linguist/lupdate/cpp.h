/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CPP_H
#define CPP_H

#include "lupdate.h"

#include <QtCore/QSet>

#include <iostream>

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

struct HashStringList {
    explicit HashStringList(const QList<HashString> &list) : m_list(list), m_hash(0x80000000) {}
    const QList<HashString> &value() const { return m_list; }
    bool operator==(const HashStringList &other) const { return m_list == other.m_list; }

    QList<HashString> m_list;
    mutable uint m_hash; // We use the highest bit as a validity indicator (set => invalid)
};

typedef QList<HashString> NamespaceList;

struct Namespace {

    Namespace() :
            classDef(this),
            hasTrFunctions(false), complained(false)
    {}
    ~Namespace()
    {
        qDeleteAll(children);
    }

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

typedef QHash<QString, IncludeCycle *> IncludeCycleHash;
typedef QHash<QString, const Translator *> TranslatorHash;

class CppFiles {

public:
    static QSet<const ParseResults *> getResults(const QString &cleanFile);
    static void setResults(const QString &cleanFile, const ParseResults *results);
    static const Translator *getTranslator(const QString &cleanFile);
    static void setTranslator(const QString &cleanFile, const Translator *results);
    static bool isBlacklisted(const QString &cleanFile);
    static void setBlacklisted(const QString &cleanFile);
    static void addIncludeCycle(const QSet<QString> &fileNames);

private:
    static IncludeCycleHash &includeCycles();
    static TranslatorHash &translatedFiles();
    static QSet<QString> &blacklistedFiles();
};

QT_END_NAMESPACE

#endif // CPP_H
