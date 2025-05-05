// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INODE_H
#define INODE_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

enum class Genus : unsigned char;
enum class NodeType : unsigned char;

/*!
    \internal

    INode defines a minimal interface for QDoc's Node hierarchy of classes.
*/
class INode
{
public:
    virtual ~INode() = default;

    [[nodiscard]] virtual Genus genus() const = 0;
    [[nodiscard]] virtual NodeType nodeType() const = 0;
    [[nodiscard]] virtual const QString &name() const = 0;
    [[nodiscard]] virtual QString fullName() const = 0;
};

QT_END_NAMESPACE

#endif // INODE_H
