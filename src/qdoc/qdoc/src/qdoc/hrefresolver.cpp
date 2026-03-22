// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "hrefresolver.h"

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "anchorid.h"
#include "inclusionfilter.h"
#include "qmltypenode.h"
#include "tree.h"
#include "utilities.h"

#include <QtCore/qfileinfo.h>

QT_BEGIN_NAMESPACE

/*!
    \struct HrefResolverConfig
    \internal
    \brief Configuration for HrefResolver URL computation.

    The \c outputPrefixFn and \c outputSuffixFn callbacks compute the
    output prefix and suffix for a given node. These correspond to
    Generator::outputPrefix() and Generator::outputSuffix() in the
    legacy path. Both should be set; an empty callback produces an
    empty prefix or suffix.

    The \c cleanRefFn callback sanitizes anchor references. It
    corresponds to Generator::cleanRef() in the legacy path.

    The \c qmlTypeContextFn callback returns the current QML type
    being documented, enabling property inheritance resolution for
    abstract QML types. An empty callback disables this resolution.
*/

/*!
    \class HrefResolver
    \internal
    \brief Computes URLs for documentation nodes without Generator dependencies.

    Not thread-safe: the mutable fileBase cache is written from const
    methods. Create one instance per thread or synchronize externally.
*/

HrefResolver::HrefResolver(const HrefResolverConfig &config)
    : m_config(config)
{
}

/*!
    Computes the filename stem for \a node without mutating the node.
    Results are cached in m_fileBaseCache to avoid recomputation.

    This mirrors Generator::fileBase() but uses an external QHash cache
    instead of const_cast on the Node.
*/
QString HrefResolver::fileBase(const Node *node) const
{
    if (!node->isPageNode() && !node->isCollectionNode()) {
        node = node->parent();
        if (!node)
            return {};
    }

    if (node->hasFileNameBase())
        return node->fileNameBase();

    auto it = m_fileBaseCache.constFind(node);
    if (it != m_fileBaseCache.constEnd())
        return it.value();

    QString result = Utilities::computeFileBase(
            node, m_config.project,
            m_config.outputPrefixFn,
            m_config.outputSuffixFn);

    m_fileBaseCache.insert(node, result);
    return result;
}

/*!
    Computes the output filename for \a node. Returns the node's
    explicit URL if one is set, otherwise constructs a filename
    from fileBase() and the configured file extension.

    This mirrors Generator::fileName() without depending on a
    Generator instance.
*/
QString HrefResolver::fileName(const Node *node) const
{
    if (!node->url().isEmpty())
        return node->url();

    const auto base = fileBase(node);
    if (base.isEmpty())
        return {};

    if (node->isTextPageNode() && !node->isCollectionNode()) {
        QFileInfo originalName(node->name());
        QString suffix = originalName.suffix();
        if (!suffix.isEmpty() && suffix != "html"_L1)
            return base + '.'_L1 + suffix;
    }

    return base + '.'_L1 + m_config.fileExtension;
}

/*!
    Computes the anchor fragment for \a node based on its type.
    Returns an empty string for page-level nodes that don't need
    anchors.

    Delegates to the free function computeAnchorId() for node-type
    dispatch, then applies the configured cleanRefFn for sanitization.
    This keeps the shared anchor logic generator-agnostic while
    allowing each caller to control cleanup policy.
*/
QString HrefResolver::anchorForNode(const Node *node) const
{
    const QString ref = computeAnchorId(node);
    if (ref.isEmpty())
        return ref;
    return m_config.cleanRefFn ? m_config.cleanRefFn(ref) : ref;
}

/*!
    Computes a relative URL for \a node, relative to \a relative.
    Returns the URL as a QString on success, or an HrefSuppressReason
    explaining why linking should be suppressed.

    This consolidates logic from XmlGenerator::linkForNode() without
    depending on a Generator instance. Cross-module prefixing is
    applied when useOutputSubdirs is enabled and the node lives in
    a different tree than the relative node.
*/
HrefResult HrefResolver::hrefForNode(const Node *node, const Node *relative) const
{
    if (node == nullptr)
        return HrefSuppressReason::NullNode;
    if (!node->url().isEmpty())
        return node->url();

    QString fn = fileName(node);
    if (fn.isEmpty())
        return HrefSuppressReason::NoFileBase;

    const NodeContext context = node->createContext();
    if (!InclusionFilter::isIncluded(m_config.inclusionPolicy, context))
        return HrefSuppressReason::ExcludedByPolicy;

    if (node->parent() && node->parent()->isQmlType() && node->parent()->isAbstract()) {
        const QmlTypeNode *qmlContext = m_config.qmlTypeContextFn
                ? m_config.qmlTypeContextFn() : nullptr;
        if (qmlContext) {
            if (qmlContext->inherits(node->parent())) {
                fn = fileName(qmlContext);
            } else if (node->parent()->isInternal() && !m_config.noLinkErrors) {
                node->doc().location().warning(
                        u"Cannot link to property in internal type '%1'"_s
                                .arg(node->parent()->name()));
                return HrefSuppressReason::InternalAbstractQml;
            }
        }
    }

    QString link = fn;

    if (!node->isPageNode() || node->isPropertyGroup()) {
        QString ref = anchorForNode(node);
        if (relative && fn == fileName(relative) && ref == anchorForNode(relative))
            return HrefSuppressReason::SameFileAnchor;

        link += '#'_L1;
        link += ref;
    }

    if (relative && (node != relative)) {
        if (m_config.useOutputSubdirs && !node->isExternalPage()
            && (node->isIndexNode() || node->tree() != relative->tree()))
            link.prepend("../%1/"_L1.arg(node->tree()->physicalModuleName()));
    }

    return link;
}

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
