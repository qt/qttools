/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "text.h"

#include <QtCore/qregularexpression.h>

#include <cstdio>

QT_BEGIN_NAMESPACE

Text::Text() : m_first(nullptr), m_last(nullptr) { }

Text::Text(const QString &str) : m_first(nullptr), m_last(nullptr)
{
    operator<<(str);
}

Text::Text(const Text &text) : m_first(nullptr), m_last(nullptr)
{
    operator=(text);
}

Text::~Text()
{
    clear();
}

Text &Text::operator=(const Text &text)
{
    if (this != &text) {
        clear();
        operator<<(text);
    }
    return *this;
}

Text &Text::operator<<(Atom::AtomType atomType)
{
    return operator<<(Atom(atomType));
}

Text &Text::operator<<(const QString &string)
{
    return string.isEmpty() ? *this : operator<<(Atom(Atom::String, string));
}

Text &Text::operator<<(const Atom &atom)
{
    if (atom.count() < 2) {
        if (m_first == nullptr) {
            m_first = new Atom(atom.type(), atom.string());
            m_last = m_first;
        } else
            m_last = new Atom(m_last, atom.type(), atom.string());
    } else {
        if (m_first == nullptr) {
            m_first = new Atom(atom.type(), atom.string(), atom.string(1));
            m_last = m_first;
        } else
            m_last = new Atom(m_last, atom.type(), atom.string(), atom.string(1));
    }
    return *this;
}

/*!
  Special output operator for LinkAtom. It makes a copy of
  the LinkAtom \a atom and connects the cop;y to the list
  in this Text.
 */
Text &Text::operator<<(const LinkAtom &atom)
{
    if (m_first == nullptr) {
        m_first = new LinkAtom(atom);
        m_last = m_first;
    } else
        m_last = new LinkAtom(m_last, atom);
    return *this;
}

Text &Text::operator<<(const Text &text)
{
    const Atom *atom = text.firstAtom();
    while (atom != nullptr) {
        operator<<(*atom);
        atom = atom->next();
    }
    return *this;
}

void Text::stripFirstAtom()
{
    if (m_first != nullptr) {
        if (m_first == m_last)
            m_last = nullptr;
        Atom *oldFirst = m_first;
        m_first = m_first->next();
        delete oldFirst;
    }
}

void Text::stripLastAtom()
{
    if (m_last != nullptr) {
        Atom *oldLast = m_last;
        if (m_first == m_last) {
            m_first = nullptr;
            m_last = nullptr;
        } else {
            m_last = m_first;
            while (m_last->next() != oldLast)
                m_last = m_last->next();
            m_last->setNext(nullptr);
        }
        delete oldLast;
    }
}

/*!
  This function traverses the atom list of the Text object,
  extracting all the string parts. It concatenates them to
  a result string and returns it.
 */
QString Text::toString() const
{
    QString str;
    const Atom *atom = firstAtom();
    while (atom != nullptr) {
        if (atom->type() == Atom::String || atom->type() == Atom::AutoLink
            || atom->type() == Atom::C)
            str += atom->string();
        atom = atom->next();
    }
    return str;
}

/*!
  Returns true if this Text contains the substring \a str.
 */
bool Text::contains(const QString &str) const
{
    const Atom *atom = firstAtom();
    while (atom != nullptr) {
        if (atom->type() == Atom::String || atom->type() == Atom::AutoLink
            || atom->type() == Atom::C)
            if (atom->string().contains(str, Qt::CaseInsensitive))
                return true;
        atom = atom->next();
    }
    return false;
}

Text Text::subText(Atom::AtomType left, Atom::AtomType right, const Atom *from,
                   bool inclusive) const
{
    const Atom *begin = from ? from : firstAtom();
    const Atom *end;

    while (begin != nullptr && begin->type() != left)
        begin = begin->next();
    if (begin != nullptr) {
        if (!inclusive)
            begin = begin->next();
    }

    end = begin;
    while (end != nullptr && end->type() != right)
        end = end->next();
    if (end == nullptr)
        begin = nullptr;
    else if (inclusive)
        end = end->next();
    return subText(begin, end);
}

Text Text::sectionHeading(const Atom *sectionLeft)
{
    if (sectionLeft != nullptr) {
        const Atom *begin = sectionLeft;
        while (begin != nullptr && begin->type() != Atom::SectionHeadingLeft)
            begin = begin->next();
        if (begin != nullptr)
            begin = begin->next();

        const Atom *end = begin;
        while (end != nullptr && end->type() != Atom::SectionHeadingRight)
            end = end->next();

        if (end != nullptr)
            return subText(begin, end);
    }
    return Text();
}

void Text::dump() const
{
    const Atom *atom = firstAtom();
    while (atom != nullptr) {
        QString str = atom->string();
        str.replace("\\", "\\\\");
        str.replace("\"", "\\\"");
        str.replace("\n", "\\n");
        str.replace(QRegularExpression(R"([^ -~])"), "?");
        if (!str.isEmpty())
            str = " \"" + str + QLatin1Char('"');
        fprintf(stderr, "    %-15s%s\n", atom->typeString().toLatin1().data(),
                str.toLatin1().data());
        atom = atom->next();
    }
}

Text Text::subText(const Atom *begin, const Atom *end)
{
    Text text;
    if (begin != nullptr) {
        while (begin != end) {
            text << *begin;
            begin = begin->next();
        }
    }
    return text;
}

void Text::clear()
{
    while (m_first != nullptr) {
        Atom *atom = m_first;
        m_first = m_first->next();
        delete atom;
    }
    m_first = nullptr;
    m_last = nullptr;
}

int Text::compare(const Text &text1, const Text &text2)
{
    if (text1.isEmpty())
        return text2.isEmpty() ? 0 : -1;
    if (text2.isEmpty())
        return 1;

    const Atom *atom1 = text1.firstAtom();
    const Atom *atom2 = text2.firstAtom();

    for (;;) {
        if (atom1->type() != atom2->type())
            return (int)atom1->type() - (int)atom2->type();
        int cmp = QString::compare(atom1->string(), atom2->string());
        if (cmp != 0)
            return cmp;

        if (atom1 == text1.lastAtom())
            return atom2 == text2.lastAtom() ? 0 : -1;
        if (atom2 == text2.lastAtom())
            return 1;
        atom1 = atom1->next();
        atom2 = atom2->next();
    }
}

QT_END_NAMESPACE
