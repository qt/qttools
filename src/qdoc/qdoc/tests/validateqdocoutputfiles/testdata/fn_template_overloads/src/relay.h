// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

class ForwardRelay
{
public:
#ifdef Q_QDOC
    template <typename Member>
    static void connect(Member method);
    template <typename Functor>
    static void connect(Functor functor);
#else
    template <typename Callable>
    static void connect(Callable callable);
#endif
};

class ReverseRelay
{
public:
#ifdef Q_QDOC
    template <typename Functor>
    static void connect(Functor functor);
    template <typename Member>
    static void connect(Member method);
#else
    template <typename Callable>
    static void connect(Callable callable);
#endif
};
