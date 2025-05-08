// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef NATIVEENUM_H
#define NATIVEENUM_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class EnumNode;

class NativeEnum
{
public:
    bool resolve(const QString &path, const QString &registeredQmlName);
    void setPrefix(const QString &prefix) { m_prefix = prefix; }

    [[nodiscard]] const EnumNode *enumNode() const { return m_enumNode; }
    [[nodiscard]] const QString &prefix() const { return m_prefix; }

private:
    EnumNode *m_enumNode { nullptr };
    QString m_prefix {};
};

class NativeEnumInterface
{
public:
    virtual ~NativeEnumInterface() = default;
    virtual NativeEnum *nativeEnum() = 0;
    virtual const NativeEnum *nativeEnum() const = 0;
};

QT_END_NAMESPACE

#endif // NATIVEENUM_H

