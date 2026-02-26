// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_HREFRESOLVER_H
#define QDOC_HREFRESOLVER_H

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

#include <functional>

QT_BEGIN_NAMESPACE

class Node;

struct HrefResolverConfig
{
    QString project;
    QString fileExtension;
    bool useOutputSubdirs{false};
    std::function<QString(const Node *)> outputPrefixFn;
    std::function<QString(const Node *)> outputSuffixFn;
};

class HrefResolver
{
public:
    explicit HrefResolver(const HrefResolverConfig &config);

    [[nodiscard]] QString hrefForNode(const Node *node, const Node *relative) const;
    [[nodiscard]] QString anchorForNode(const Node *node) const;
    [[nodiscard]] QString fileName(const Node *node) const;

private:
    [[nodiscard]] QString fileBase(const Node *node) const;

    HrefResolverConfig m_config;
    mutable QHash<const Node *, QString> m_fileBaseCache;
};

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED

#endif // QDOC_HREFRESOLVER_H
