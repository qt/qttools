/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

/*
  openedlist.h
*/

#ifndef OPENEDLIST_H
#define OPENEDLIST_H

#include "location.h"

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class OpenedList
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::OpenedList)

public:
    enum ListStyle { Bullet, Tag, Value, Numeric, UpperAlpha, LowerAlpha, UpperRoman, LowerRoman };

    OpenedList() : sty(Bullet), ini(1), nex(0) {}
    OpenedList(ListStyle style);
    OpenedList(const Location &location, const QString &hint);

    void next() { nex++; }

    bool isStarted() const { return nex >= ini; }
    ListStyle style() const { return sty; }
    QString styleString() const;
    int number() const { return nex; }
    QString numberString() const;
    QString prefix() const { return pref; }
    QString suffix() const { return suff; }

private:
    static QString toAlpha(int n);
    static int fromAlpha(const QString &str);
    static QString toRoman(int n);
    static int fromRoman(const QString &str);

    ListStyle sty;
    int ini;
    int nex;
    QString pref;
    QString suff;
};
Q_DECLARE_TYPEINFO(OpenedList, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

#endif
