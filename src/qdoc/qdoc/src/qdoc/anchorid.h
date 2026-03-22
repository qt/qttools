// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_ANCHORID_H
#define QDOC_ANCHORID_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Node;

QString computeAnchorId(const Node *node);

QT_END_NAMESPACE

#endif // QDOC_ANCHORID_H
