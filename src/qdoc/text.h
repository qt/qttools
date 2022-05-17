// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEXT_H
#define TEXT_H

#include "atom.h"

QT_BEGIN_NAMESPACE

class Text
{
public:
    Text();
    explicit Text(const QString &str);
    Text(const Text &text);
    ~Text();

    Text &operator=(const Text &text);

    Atom *firstAtom() { return m_first; }
    Atom *lastAtom() { return m_last; }
    Text &operator<<(Atom::AtomType atomType);
    Text &operator<<(const QString &string);
    Text &operator<<(const Atom &atom);
    Text &operator<<(const LinkAtom &atom);
    Text &operator<<(const Text &text);
    void stripFirstAtom();
    void stripLastAtom();

    [[nodiscard]] bool isEmpty() const { return m_first == nullptr; }
    [[nodiscard]] bool contains(const QString &str) const;
    [[nodiscard]] QString toString() const;
    [[nodiscard]] const Atom *firstAtom() const { return m_first; }
    [[nodiscard]] const Atom *lastAtom() const { return m_last; }
    Text subText(Atom::AtomType left, Atom::AtomType right, const Atom *from = nullptr,
                 bool inclusive = false) const;
    void dump() const;
    void clear();

    static Text subText(const Atom *begin, const Atom *end = nullptr);
    static Text sectionHeading(const Atom *sectionBegin);
    static int compare(const Text &text1, const Text &text2);

private:
    Atom *m_first { nullptr };
    Atom *m_last { nullptr };
};

inline bool operator==(const Text &text1, const Text &text2)
{
    return Text::compare(text1, text2) == 0;
}
inline bool operator!=(const Text &text1, const Text &text2)
{
    return Text::compare(text1, text2) != 0;
}
inline bool operator<(const Text &text1, const Text &text2)
{
    return Text::compare(text1, text2) < 0;
}
inline bool operator<=(const Text &text1, const Text &text2)
{
    return Text::compare(text1, text2) <= 0;
}
inline bool operator>(const Text &text1, const Text &text2)
{
    return Text::compare(text1, text2) > 0;
}
inline bool operator>=(const Text &text1, const Text &text2)
{
    return Text::compare(text1, text2) >= 0;
}

QT_END_NAMESPACE

#endif
