// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEMPLATE_SUGAR_H
#define TEMPLATE_SUGAR_H

template <typename T>
class MyContainer {
public:
    void append(const T &value);
    void prepend(T value);
    MyContainer<T> operator+(const MyContainer<T> &other) const;
    void replace(int index, const T &value);
};

template <typename T>
using ContainerAlias = MyContainer<T>;

class AliasUser {
public:
    void process(ContainerAlias<int> items);
    void transform(const ContainerAlias<double> &source, ContainerAlias<double> &dest);
};

class Connector {
public:
#ifdef Q_QDOC
    template <typename Functor>
    void invoke(Functor callback);
    template <typename PointerToMemberFunction>
    void invoke(PointerToMemberFunction callback, int priority);
#else
    template <typename F>
    void invoke(F) {}
    template <typename F>
    void invoke(F, int) {}
#endif
};

template <typename T>
class Dispatcher {
public:
    void byPointer(T *value);
    void byConstRef(const T &value);
    void nested(MyContainer<T> items);
};

#endif // TEMPLATE_SUGAR_H
