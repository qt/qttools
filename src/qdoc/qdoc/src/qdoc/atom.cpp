// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "atom.h"

#include "location.h"
#include "qdocdatabase.h"

#include <QtCore/qregularexpression.h>

#include <cstdio>

QT_BEGIN_NAMESPACE

/*! \class Atom
    \brief The Atom class is the fundamental unit for representing
    documents internally.

  Atoms have a \i type and are completed by a \i string whose
  meaning depends on the \i type. For example, the string
  \quotation
      \i italic text looks nicer than \bold bold text
  \endquotation
  is represented by the following atoms:
  \quotation
      (FormattingLeft, ATOM_FORMATTING_ITALIC)
      (String, "italic")
      (FormattingRight, ATOM_FORMATTING_ITALIC)
      (String, " text is more attractive than ")
      (FormattingLeft, ATOM_FORMATTING_BOLD)
      (String, "bold")
      (FormattingRight, ATOM_FORMATTING_BOLD)
      (String, " text")
  \endquotation

  \also Text
*/

/*! \enum Atom::AtomType

  \value AnnotatedList
  \value AutoLink
  \value BaseName
  \value BriefLeft
  \value BriefRight
  \value C
  \value CaptionLeft
  \value CaptionRight
  \value Code
  \value CodeBad
  \value CodeQuoteArgument
  \value CodeQuoteCommand
  \value DetailsLeft
  \value DetailsRight
  \value DivLeft
  \value DivRight
  \value ExampleFileLink
  \value ExampleImageLink
  \value FormatElse
  \value FormatEndif
  \value FormatIf
  \value FootnoteLeft
  \value FootnoteRight
  \value FormattingLeft
  \value FormattingRight
  \value GeneratedList
  \value Image
  \value ImageText
  \value ImportantNote
  \value InlineImage
  \value Keyword
  \value LineBreak
  \value Link
  \value LinkNode
  \value ListLeft
  \value ListItemNumber
  \value ListTagLeft
  \value ListTagRight
  \value ListItemLeft
  \value ListItemRight
  \value ListRight
  \value NavAutoLink
  \value NavLink
  \value Nop
  \value Note
  \value ParaLeft
  \value ParaRight
  \value Qml
  \value QuotationLeft
  \value QuotationRight
  \value RawString
  \value SectionLeft
  \value SectionRight
  \value SectionHeadingLeft
  \value SectionHeadingRight
  \value SidebarLeft
  \value SidebarRight
  \value SinceList
  \value SinceTagLeft
  \value SinceTagRight
  \value String
  \value TableLeft
  \value TableRight
  \value TableHeaderLeft
  \value TableHeaderRight
  \value TableRowLeft
  \value TableRowRight
  \value TableItemLeft
  \value TableItemRight
  \value TableOfContents
  \value Target
  \value UnhandledFormat
  \value UnknownCommand
*/

QString Atom::s_noError = QString();

static const struct
{
    const char *english;
    int no;
} atms[] = { { "AnnotatedList", Atom::AnnotatedList },
             { "AutoLink", Atom::AutoLink },
             { "BaseName", Atom::BaseName },
             { "br", Atom::BR },
             { "BriefLeft", Atom::BriefLeft },
             { "BriefRight", Atom::BriefRight },
             { "C", Atom::C },
             { "CaptionLeft", Atom::CaptionLeft },
             { "CaptionRight", Atom::CaptionRight },
             { "Code", Atom::Code },
             { "CodeBad", Atom::CodeBad },
             { "CodeQuoteArgument", Atom::CodeQuoteArgument },
             { "CodeQuoteCommand", Atom::CodeQuoteCommand },
             { "DetailsLeft", Atom::DetailsLeft },
             { "DetailsRight", Atom::DetailsRight },
             { "DivLeft", Atom::DivLeft },
             { "DivRight", Atom::DivRight },
             { "ExampleFileLink", Atom::ExampleFileLink },
             { "ExampleImageLink", Atom::ExampleImageLink },
             { "FootnoteLeft", Atom::FootnoteLeft },
             { "FootnoteRight", Atom::FootnoteRight },
             { "FormatElse", Atom::FormatElse },
             { "FormatEndif", Atom::FormatEndif },
             { "FormatIf", Atom::FormatIf },
             { "FormattingLeft", Atom::FormattingLeft },
             { "FormattingRight", Atom::FormattingRight },
             { "GeneratedList", Atom::GeneratedList },
             { "hr", Atom::HR },
             { "Image", Atom::Image },
             { "ImageText", Atom::ImageText },
             { "ImportantLeft", Atom::ImportantLeft },
             { "ImportantRight", Atom::ImportantRight },
             { "InlineImage", Atom::InlineImage },
             { "Keyword", Atom::Keyword },
             { "LegaleseLeft", Atom::LegaleseLeft },
             { "LegaleseRight", Atom::LegaleseRight },
             { "LineBreak", Atom::LineBreak },
             { "Link", Atom::Link },
             { "LinkNode", Atom::LinkNode },
             { "ListLeft", Atom::ListLeft },
             { "ListItemNumber", Atom::ListItemNumber },
             { "ListTagLeft", Atom::ListTagLeft },
             { "ListTagRight", Atom::ListTagRight },
             { "ListItemLeft", Atom::ListItemLeft },
             { "ListItemRight", Atom::ListItemRight },
             { "ListRight", Atom::ListRight },
             { "NavAutoLink", Atom::NavAutoLink },
             { "NavLink", Atom::NavLink },
             { "Nop", Atom::Nop },
             { "NoteLeft", Atom::NoteLeft },
             { "NoteRight", Atom::NoteRight },
             { "ParaLeft", Atom::ParaLeft },
             { "ParaRight", Atom::ParaRight },
             { "Qml", Atom::Qml },
             { "QuotationLeft", Atom::QuotationLeft },
             { "QuotationRight", Atom::QuotationRight },
             { "RawString", Atom::RawString },
             { "SectionLeft", Atom::SectionLeft },
             { "SectionRight", Atom::SectionRight },
             { "SectionHeadingLeft", Atom::SectionHeadingLeft },
             { "SectionHeadingRight", Atom::SectionHeadingRight },
             { "SidebarLeft", Atom::SidebarLeft },
             { "SidebarRight", Atom::SidebarRight },
             { "SinceList", Atom::SinceList },
             { "SinceTagLeft", Atom::SinceTagLeft },
             { "SinceTagRight", Atom::SinceTagRight },
             { "SnippetCommand", Atom::SnippetCommand },
             { "SnippetIdentifier", Atom::SnippetIdentifier },
             { "SnippetLocation", Atom::SnippetLocation },
             { "String", Atom::String },
             { "TableLeft", Atom::TableLeft },
             { "TableRight", Atom::TableRight },
             { "TableHeaderLeft", Atom::TableHeaderLeft },
             { "TableHeaderRight", Atom::TableHeaderRight },
             { "TableRowLeft", Atom::TableRowLeft },
             { "TableRowRight", Atom::TableRowRight },
             { "TableItemLeft", Atom::TableItemLeft },
             { "TableItemRight", Atom::TableItemRight },
             { "TableOfContents", Atom::TableOfContents },
             { "Target", Atom::Target },
             { "UnhandledFormat", Atom::UnhandledFormat },
             { "WarningLeft", Atom::WarningLeft },
             { "WarningRight", Atom::WarningRight },
             { "UnknownCommand", Atom::UnknownCommand },
             { nullptr, 0 } };

/*! \fn Atom::Atom(AtomType type, const QString &string)

  Constructs an atom of the specified \a type with the single
  parameter \a string and does not put the new atom in a list.
*/

/*! \fn Atom::Atom(AtomType type, const QString &p1, const QString &p2)

  Constructs an atom of the specified \a type with the two
  parameters \a p1 and \a p2 and does not put the new atom
  in a list.
*/

/*! \fn Atom(Atom *previous, AtomType type, const QString &string)

  Constructs an atom of the specified \a type with the single
  parameter \a string and inserts the new atom into the list
  after the \a previous atom.
*/

/*! \fn Atom::Atom(Atom *previous, AtomType type, const QString &p1, const QString &p2)

  Constructs an atom of the specified \a type with the two
  parameters \a p1 and \a p2 and inserts the new atom into
  the list after the \a previous atom.
*/

/*! \fn void Atom::appendChar(QChar ch)

  Appends \a ch to the string parameter of this atom.

  \also string()
*/

/*! \fn void Atom::appendString(const QString &string)

  Appends \a string to the string parameter of this atom.

  \also string()
*/

/*! \fn void Atom::chopString()

  \also string()
*/

/*! \fn Atom *Atom::next()
  Return the next atom in the atom list.
  \also type(), string()
*/

/*!
  Return the next Atom in the list if it is of AtomType \a t.
  Otherwise return 0.
 */
const Atom *Atom::next(AtomType t) const
{
    return (m_next && (m_next->type() == t)) ? m_next : nullptr;
}

/*!
  Return the next Atom in the list if it is of AtomType \a t
  and its string part is \a s. Otherwise return 0.
 */
const Atom *Atom::next(AtomType t, const QString &s) const
{
    return (m_next && (m_next->type() == t) && (m_next->string() == s)) ? m_next : nullptr;
}

/*! \fn const Atom *Atom::next() const
  Return the next atom in the atom list.
  \also type(), string()
*/

/*! \fn AtomType Atom::type() const
  Return the type of this atom.
  \also string(), next()
*/

/*!
  Return the type of this atom as a string. Return "Invalid" if
  type() returns an impossible value.

  This is only useful for debugging.

  \also type()
*/
QString Atom::typeString() const
{
    static bool deja = false;

    if (!deja) {
        int i = 0;
        while (atms[i].english != nullptr) {
            if (atms[i].no != i)
                Location::internalError(QStringLiteral("QDoc::Atom: atom %1 missing").arg(i));
            ++i;
        }
        deja = true;
    }

    int i = static_cast<int>(type());
    if (i < 0 || i > static_cast<int>(Last))
        return QLatin1String("Invalid");
    return QLatin1String(atms[i].english);
}

/*! \fn const QString &Atom::string() const

  Returns the string parameter that together with the type
  characterizes this atom.

  \also type(), next()
*/

/*!
    For a link atom, returns the string representing the link text
    if one exist in the list of atoms.
*/
QString Atom::linkText() const
{
    Q_ASSERT(m_type == Atom::Link);
    QString result;

    if (next() &&  next()->string() == ATOM_FORMATTING_LINK) {
        auto *atom = next()->next();
        while (atom && atom->type() != Atom::FormattingRight) {
            result += atom->string();
            atom = atom->next();
        }
        return result;
    }

    return string();
}

/*!
  The only constructor for LinkAtom. It creates an Atom of
  type Atom::Link. \a p1 being the link target. \a p2 is the
  parameters in square brackets. Normally there is just one
  word in the square brackets, but there can be up to three
  words separated by spaces. The constructor splits \a p2 on
  the space character.
 */
LinkAtom::LinkAtom(const QString &p1, const QString &p2)
    : Atom(Atom::Link, p1),
      m_resolved(false),
      m_genus(Node::DontCare),
      m_domain(nullptr),
      m_squareBracketParams(p2)
{
    // nada.
}

/*!
  This function resolves the parameters that were enclosed in
  square brackets. If the parameters have already been resolved,
  it does nothing and returns immediately.
 */
void LinkAtom::resolveSquareBracketParams()
{
    if (m_resolved)
        return;
    const QStringList params = m_squareBracketParams.toLower().split(QLatin1Char(' '));
    for (const auto &param : params) {
        if (!m_domain) {
            m_domain = QDocDatabase::qdocDB()->findTree(param);
            if (m_domain) {
                continue;
            }
        }

        if (param == "qml") {
            m_genus = Node::QML;
            continue;
        }
        if (param == "cpp") {
            m_genus = Node::CPP;
            continue;
        }
        if (param == "doc") {
            m_genus = Node::DOC;
            continue;
        }
        if (param == "api") {
            m_genus = Node::API;
            continue;
        }
        m_error = m_squareBracketParams;
        break;
    }
    m_resolved = true;
}

/*!
  Standard copy constructor of LinkAtom \a t.
 */
LinkAtom::LinkAtom(const LinkAtom &t)
    : Atom(Link, t.string()),
      m_resolved(t.m_resolved),
      m_genus(t.m_genus),
      m_domain(t.m_domain),
      m_error(t.m_error),
      m_squareBracketParams(t.m_squareBracketParams)
{
    // nothing
}

/*!
  Special copy constructor of LinkAtom \a t, where
  where the new LinkAtom will not be the first one
  in the list.
 */
LinkAtom::LinkAtom(Atom *previous, const LinkAtom &t)
    : Atom(previous, Link, t.string()),
      m_resolved(t.m_resolved),
      m_genus(t.m_genus),
      m_domain(t.m_domain),
      m_error(t.m_error),
      m_squareBracketParams(t.m_squareBracketParams)
{
    previous->m_next = this;
}

QT_END_NAMESPACE
