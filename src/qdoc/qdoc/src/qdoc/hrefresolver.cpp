// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "hrefresolver.h"

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "anchorid.h"
#include "inclusionfilter.h"
#include "outputcontext.h"
#include "qmltypenode.h"
#include "tree.h"
#include "utilities.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>

QT_BEGIN_NAMESPACE

/*!
    \struct HrefResolverConfig
    \internal
    \brief Configuration for HrefResolver URL computation.

    The non-owning \c context pointer is the single source of truth for
    output layout (project name, file extension, per-genus prefixes and
    suffixes, subdir usage, diagnostic gating). It must outlive the
    HrefResolver.

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

    const OutputContext *ctx = m_config.context;
    QString result = Utilities::computeFileBase(
            node, ctx->project,
            [ctx](const Node *n) { return ctx->outputPrefix(n->genus()); },
            [ctx](const Node *n) { return ctx->outputSuffix(n->genus()); });

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

    return base + '.'_L1 + m_config.context->fileExtension;
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

    External URLs (carrying a scheme such as \c http://) pass through
    unchanged. Every other reference — including URLs loaded from a
    dependency's \c .index file — is reduced to a bare filename stem
    and then prefixed with whatever relative path reaches the target
    module's output directory from the current page's output
    directory. The prefix is derived from OutputContext and the two
    nodes' trees, not from a hardcoded \c ../<module>/ pattern, so it
    comes out correct regardless of whether the qdocconf uses
    per-module subdirs, a format subdirectory, or both.
*/
HrefResult HrefResolver::hrefForNode(const Node *node, const Node *relative) const
{
    if (node == nullptr)
        return HrefSuppressReason::NullNode;

    const bool hasExplicitUrl = !node->url().isEmpty();
    if (hasExplicitUrl && node->url().contains("://"_L1))
        return node->url();

    // An explicit URL arrives with a directory prefix baked in by the
    // reader that produced it (qdocindexfiles prepends ../<module>/).
    // That prefix encodes one specific layout's depth assumption and
    // is wrong for any other layout, so discard it and recompute from
    // the current output paths below.
    QString fn = hasExplicitUrl
            ? node->url().section('/'_L1, -1)
            : fileName(node);
    if (fn.isEmpty())
        return HrefSuppressReason::NoFileBase;

    const NodeContext context = node->createContext();
    if (!InclusionFilter::isIncluded(m_config.inclusionPolicy, context))
        return HrefSuppressReason::ExcludedByPolicy;

    if (!hasExplicitUrl && node->parent() && node->parent()->isQmlType()
        && node->parent()->isAbstract()) {
        const QmlTypeNode *qmlContext = m_config.qmlTypeContextFn
                ? m_config.qmlTypeContextFn() : nullptr;
        if (qmlContext) {
            if (qmlContext->inherits(node->parent())) {
                fn = fileName(qmlContext);
            } else if (node->parent()->isInternal() && !m_config.context->noLinkErrors) {
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

    if (relative && (node != relative) && !node->isExternalPage()
        && (node->isIndexNode() || node->tree() != relative->tree())) {
        link.prepend(crossModulePrefix(node, relative));
    }

    return link;
}

/*!
    \internal
    Returns a relative-path prefix (ending in \c /) that reaches \a
    target's output directory from \a source's output directory, or
    an empty string when the two share a directory.

    The current page's output directory is the OutputContext's \c
    outputDir. The target's output directory is derived by locating
    the current module's name as a segment in that path and swapping
    in the target module's name. If the current module's name isn't
    present — a flat layout where multiple modules render into the
    same directory — source and target share a directory and no
    prefix is needed.
*/
QString HrefResolver::crossModulePrefix(const Node *target, const Node *source) const
{
    const QString &sourceDir = m_config.context->outputDir.path();
    const QString currentSegment = '/'_L1 + source->tree()->physicalModuleName() + '/'_L1;
    const qsizetype moduleIndex = sourceDir.indexOf(currentSegment);
    if (moduleIndex < 0)
        return {};

    const QString targetSegment = '/'_L1 + target->tree()->physicalModuleName() + '/'_L1;
    QString targetDir = sourceDir;
    targetDir.replace(moduleIndex, currentSegment.size(), targetSegment);

    const QString relPath = QDir(sourceDir).relativeFilePath(targetDir);
    if (relPath.isEmpty() || relPath == "."_L1)
        return {};
    return relPath + '/'_L1;
}

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
