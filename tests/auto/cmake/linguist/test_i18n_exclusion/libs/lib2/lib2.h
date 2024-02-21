// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef _LIB2_H_
#define _LIB2_H_

#include <QObject>

class MyObject2 : public QObject
{
    Q_OBJECT
public:
    MyObject2(QObject *parent = nullptr);
    QString greeting() const;
};

#endif // _LIB2_H_
