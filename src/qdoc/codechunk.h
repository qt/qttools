/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

/*
  codechunk.h
*/

#ifndef CODECHUNK_H
#define CODECHUNK_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

// ### get rid of that class

class CodeChunk
{
public:
    CodeChunk() : hotspot(-1) {}
    CodeChunk(const QString &str) : s(str), hotspot(-1) {}

    void append(const QString &lexeme);
    void appendHotspot()
    {
        if (hotspot == -1)
            hotspot = s.length();
    }

    bool isEmpty() const { return s.isEmpty(); }
    void clear() { s.clear(); }
    QString toString() const { return s; }
    QStringList toPath() const;
    QString left() const { return s.left(hotspot == -1 ? s.length() : hotspot); }
    QString right() const { return s.mid(hotspot == -1 ? s.length() : hotspot); }

private:
    QString s;
    int hotspot;
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
