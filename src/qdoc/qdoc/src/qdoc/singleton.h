// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef SINGLETON_H
#define SINGLETON_H

QT_BEGIN_NAMESPACE

/*!
    \class Singleton
    \internal

    Class template for singleton objects in QDoc.
 */
template<typename T>
class Singleton
{
public:
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
    static T &instance()
    {
        static T s_instance {};
        return s_instance;
    }

protected:
    Singleton() = default;
};

QT_END_NAMESPACE

#endif // SINGLETON_H
