#ifndef NODEDESCRIPTION_H
#define NODEDESCRIPTION_H

// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QString>

QT_BEGIN_NAMESPACE

class Node;

namespace NodeText
{

QString typeString(const Node *, bool plural = false);

} // namespace NodeText

QT_END_NAMESPACE

#endif // NODEDESCRIPTION_H

