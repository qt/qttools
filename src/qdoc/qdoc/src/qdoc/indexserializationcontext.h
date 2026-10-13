// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef INDEXSERIALIZATIONCONTEXT_H
#define INDEXSERIALIZATIONCONTEXT_H

#include "inclusionpolicy.h"

#include <QtCore/qstring.h>

#include <functional>

QT_BEGIN_NAMESPACE

class Node;

struct IndexSerializationContext
{
    enum class ExampleEntries { Inline, Separate };

    std::function<QString(const Node *)> documentLocation;
    std::function<QString(const QString &)> exampleFileLocation;

    InclusionPolicy inclusionPolicy;
    QString project;
    ExampleEntries exampleEntries = ExampleEntries::Inline;
    bool storeLocationInfo = false;
};

QT_END_NAMESPACE

#endif // INDEXSERIALIZATIONCONTEXT_H
