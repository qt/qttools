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

#ifndef OPENEDLIST_H
#define OPENEDLIST_H

#include "location.h"

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class OpenedList
{
public:
    enum ListStyle { Bullet, Tag, Value, Numeric, UpperAlpha, LowerAlpha, UpperRoman, LowerRoman };

    OpenedList() : sty(Bullet), ini(1), nex(0) {}
    explicit OpenedList(ListStyle style);
    OpenedList(const Location &location, const QString &hint);

    void next() { nex++; }

    [[nodiscard]] bool isStarted() const { return nex >= ini; }
    [[nodiscard]] ListStyle style() const { return sty; }
    [[nodiscard]] QString styleString() const;
    [[nodiscard]] int number() const { return nex; }
    [[nodiscard]] QString numberString() const;
    [[nodiscard]] QString prefix() const { return pref; }
    [[nodiscard]] QString suffix() const { return suff; }

private:
    static int fromAlpha(const QString &str);
    static QString toRoman(int n);
    static int fromRoman(const QString &str);

    ListStyle sty;
    int ini;
    int nex;
    QString pref;
    QString suff;
};
Q_DECLARE_TYPEINFO(OpenedList, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

#endif
