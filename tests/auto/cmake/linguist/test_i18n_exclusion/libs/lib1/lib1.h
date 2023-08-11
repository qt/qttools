// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _LIB1_H_
#define _LIB1_H_

#include <QObject>

class MyObject1 : public QObject
{
    Q_OBJECT
public:
    MyObject1(QObject *parent = nullptr);
    QString greeting() const;
};

#endif // _LIB1_H_
