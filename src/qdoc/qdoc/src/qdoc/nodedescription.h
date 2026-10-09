#ifndef NODEDESCRIPTION_H
#define NODEDESCRIPTION_H

// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QString>

QT_BEGIN_NAMESPACE

class Node;

namespace NodeText
{

enum class GrammaticalNumber
{
    Singular,
    Plural
};

QString nodeTypeDescription(const Node *node, GrammaticalNumber number = GrammaticalNumber::Singular);

} // namespace NodeText

QT_END_NAMESPACE

#endif // NODEDESCRIPTION_H
