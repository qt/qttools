// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ATOM_H
#define ATOM_H

#include "genustypes.h"
#include "location.h"

#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class Tree;
class LinkAtom;

class Atom
{
public:
    enum AtomType {
        AnnotatedList,
        AutoLink,
        BaseName,
        BR,
        BriefLeft,
        BriefRight,
        C,
        CaptionLeft,
        CaptionRight,
        Code,
        CodeBad,
        CodeQuoteArgument,
        CodeQuoteCommand,
        ComparesLeft,
        ComparesRight,
        DetailsLeft,
        DetailsRight,
        DivLeft,
        DivRight,
        ExampleFileLink,
        ExampleImageLink,
        FootnoteLeft,
        FootnoteRight,
        FormatElse,
        FormatEndif,
        FormatIf,
        FormattingLeft,
        FormattingRight,
        GeneratedList,
        HR,
        Image,
        ImageText,
        ImportantLeft,
        ImportantRight,
        InlineImage,
        Keyword,
        LegaleseLeft,
        LegaleseRight,
        LineBreak,
        Link,
        LinkNode,
        ListLeft,
        ListItemNumber,
        ListTagLeft,
        ListTagRight,
        ListItemLeft,
        ListItemRight,
        ListRight,
        NavAutoLink,
        NavLink,
        Nop,
        NoteLeft,
        NoteRight,
        ParaLeft,
        ParaRight,
        Qml,
        QuotationLeft,
        QuotationRight,
        RawString,
        SectionLeft,
        SectionRight,
        SectionHeadingLeft,
        SectionHeadingRight,
        SidebarLeft,
        SidebarRight,
        SinceList,
        SinceTagLeft,
        SinceTagRight,
        SnippetCommand,
        SnippetIdentifier,
        SnippetLocation,
        String,
        TableLeft,
        TableRight,
        TableHeaderLeft,
        TableHeaderRight,
        TableRowLeft,
        TableRowRight,
        TableItemLeft,
        TableItemRight,
        TableOfContents,
        Target,
        UnhandledFormat,
        WarningLeft,
        WarningRight,
        UnknownCommand,
        Last = UnknownCommand
    };

    friend class LinkAtom;

    explicit Atom(AtomType type, const QString &string = "") : m_type(type), m_strs(string) { }

    Atom(AtomType type, const QString &p1, const QString &p2) : m_type(type), m_strs(p1)
    {
        if (!p2.isEmpty())
            m_strs << p2;
    }

    Atom(Atom *previous, AtomType type, const QString &string)
        : m_next(previous->m_next), m_type(type), m_strs(string)
    {
        previous->m_next = this;
    }

    Atom(Atom *previous, AtomType type, const QString &p1, const QString &p2)
        : m_next(previous->m_next), m_type(type), m_strs(p1)
    {
        if (!p2.isEmpty())
            m_strs << p2;
        previous->m_next = this;
    }

    virtual ~Atom() = default;

    void appendChar(QChar ch) { m_strs[0] += ch; }
    void concatenateString(const QString &string) { m_strs[0] += string; }
    void append(const QString &string) { m_strs << string; }
    void chopString() { m_strs[0].chop(1); }
    void setString(const QString &string) { m_strs[0] = string; }
    Atom *next() { return m_next; }
    void setNext(Atom *newNext) { m_next = newNext; }

    [[nodiscard]] const Atom *find(AtomType t) const;
    [[nodiscard]] const Atom *find(AtomType t, const QString &s) const;
    [[nodiscard]] const Atom *next() const { return m_next; }
    [[nodiscard]] const Atom *next(AtomType t) const;
    [[nodiscard]] const Atom *next(AtomType t, const QString &s) const;
    [[nodiscard]] AtomType type() const { return m_type; }
    [[nodiscard]] QString typeString() const;
    [[nodiscard]] const QString &string() const { return m_strs[0]; }
    [[nodiscard]] const QString &string(int i) const { return m_strs[i]; }
    [[nodiscard]] qsizetype count() const { return m_strs.size(); }
    [[nodiscard]] QString linkText() const;
    [[nodiscard]] const QStringList &strings() const { return m_strs; }

    [[nodiscard]] virtual bool isLinkAtom() const { return false; }
    virtual Genus genus() { return Genus::DontCare; }
    virtual Tree *domain() { return nullptr; }
    virtual void resolveSquareBracketParams() {}

protected:
    Atom *m_next = nullptr;
    AtomType m_type {};
    QStringList m_strs {};
};

class LinkAtom : public Atom
{
public:
    LinkAtom(const QString &p1, const QString &p2, Location location = Location());
    LinkAtom(const LinkAtom &t);
    LinkAtom(Atom *previous, const LinkAtom &t);
    ~LinkAtom() override = default;

    [[nodiscard]] bool isLinkAtom() const override { return true; }
    Genus genus() override
    {
        resolveSquareBracketParams();
        return m_genus;
    }
    Tree *domain() override
    {
        resolveSquareBracketParams();
        return m_domain;
    }
    void resolveSquareBracketParams() override;

public:
    Location location;

protected:
    bool m_resolved {};
    Genus m_genus {};
    Tree *m_domain {};
    QString m_squareBracketParams {};
};

#define ATOM_FORMATTING_BOLD "bold"
#define ATOM_FORMATTING_INDEX "index"
#define ATOM_FORMATTING_ITALIC "italic"
#define ATOM_FORMATTING_LINK "link"
#define ATOM_FORMATTING_NOTRANSLATE "notranslate"
#define ATOM_FORMATTING_PARAMETER "parameter"
#define ATOM_FORMATTING_SPAN "span "
#define ATOM_FORMATTING_SUBSCRIPT "subscript"
#define ATOM_FORMATTING_SUPERSCRIPT "superscript"
#define ATOM_FORMATTING_TELETYPE "teletype"
#define ATOM_FORMATTING_TRADEMARK "trademark"
#define ATOM_FORMATTING_UICONTROL "uicontrol"
#define ATOM_FORMATTING_UNDERLINE "underline"

#define ATOM_LIST_BULLET "bullet"
#define ATOM_LIST_TAG "tag"
#define ATOM_LIST_VALUE "value"
#define ATOM_LIST_LOWERALPHA "loweralpha"
#define ATOM_LIST_LOWERROMAN "lowerroman"
#define ATOM_LIST_NUMERIC "numeric"
#define ATOM_LIST_UPPERALPHA "upperalpha"
#define ATOM_LIST_UPPERROMAN "upperroman"

QT_END_NAMESPACE

#endif
