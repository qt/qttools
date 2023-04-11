// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CODEMARKER_H
#define CODEMARKER_H

#include "atom.h"
#include "sections.h"

QT_BEGIN_NAMESPACE

class CodeMarker
{
public:
    CodeMarker();
    virtual ~CodeMarker();

    virtual void initializeMarker();
    virtual void terminateMarker();
    virtual bool recognizeCode(const QString & /*code*/) { return true; }
    virtual bool recognizeExtension(const QString & /*extension*/) { return true; }
    virtual bool recognizeLanguage(const QString & /*language*/) { return false; }
    [[nodiscard]] virtual Atom::AtomType atomType() const { return Atom::Code; }
    virtual QString markedUpCode(const QString &code, const Node * /*relative*/,
                                 const Location & /*location*/)
    {
        return protect(code);
    }
    virtual QString markedUpSynopsis(const Node * /*node*/, const Node * /*relative*/,
                                     Section::Style /*style*/)
    {
        return QString();
    }
    virtual QString markedUpQmlItem(const Node *, bool) { return QString(); }
    virtual QString markedUpName(const Node * /*node*/) { return QString(); }
    virtual QString markedUpEnumValue(const QString & /*enumValue*/, const Node * /*relative*/)
    {
        return QString();
    }
    virtual QString markedUpInclude(const QString & /*include*/) { return QString(); }

    static void initialize();
    static void terminate();
    static CodeMarker *markerForCode(const QString &code);
    static CodeMarker *markerForFileName(const QString &fileName);
    static CodeMarker *markerForLanguage(const QString &lang);
    static const Node *nodeForString(const QString &string);
    static QString stringForNode(const Node *node);
    static QString extraSynopsis(const Node *node, Section::Style style);

    QString typified(const QString &string, bool trailingSpace = false);

protected:
    static QString protect(const QString &string);
    static void appendProtectedString(QString *output, QStringView str);
    QString taggedNode(const Node *node);
    QString taggedQmlNode(const Node *node);
    QString linkTag(const Node *node, const QString &body);

private:
    static QString s_defaultLang;
    static QList<CodeMarker *> s_markers;
};

QT_END_NAMESPACE

#endif
