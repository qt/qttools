/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
            m_hotspot = m_str.length();
    }

    [[nodiscard]] bool isEmpty() const { return m_str.isEmpty(); }
    void clear() { m_str.clear(); }
    [[nodiscard]] QString toString() const { return m_str; }
    [[nodiscard]] QString left() const
    {
        return m_str.left(m_hotspot == -1 ? m_str.length() : m_hotspot);
    }
    [[nodiscard]] QString right() const
    {
        return m_str.mid(m_hotspot == -1 ? m_str.length() : m_hotspot);
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
