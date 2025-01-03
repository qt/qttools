// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CODECHUNK_H
#define CODECHUNK_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

// ### get rid of that class

class CodeChunk
{
public:
    CodeChunk() : m_hotspot(-1) { }

    void append(const QString &lexeme);
    void appendHotspot()
    {
        if (m_hotspot == -1)
            m_hotspot = m_str.size();
    }

    [[nodiscard]] bool isEmpty() const { return m_str.isEmpty(); }
    void clear() { m_str.clear(); }
    [[nodiscard]] QString toString() const { return m_str; }
    [[nodiscard]] QString left() const
    {
        return m_str.left(m_hotspot == -1 ? m_str.size() : m_hotspot);
    }
    [[nodiscard]] QString right() const
    {
        return m_str.mid(m_hotspot == -1 ? m_str.size() : m_hotspot);
    }

private:
    QString m_str {};
    qsizetype m_hotspot {};
};

inline bool operator==(const CodeChunk &c, const CodeChunk &d)
{
    return c.toString() == d.toString();
}

inline bool operator!=(const CodeChunk &c, const CodeChunk &d)
{
    return !(c == d);
}

inline bool operator<(const CodeChunk &c, const CodeChunk &d)
{
    return c.toString() < d.toString();
}

inline bool operator>(const CodeChunk &c, const CodeChunk &d)
{
    return d < c;
}

inline bool operator<=(const CodeChunk &c, const CodeChunk &d)
{
    return !(c > d);
}

inline bool operator>=(const CodeChunk &c, const CodeChunk &d)
{
    return !(c < d);
}

QT_END_NAMESPACE

#endif
