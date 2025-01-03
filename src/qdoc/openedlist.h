// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
