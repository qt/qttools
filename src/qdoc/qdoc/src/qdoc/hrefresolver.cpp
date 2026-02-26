// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "hrefresolver.h"

#include "config.h"
#include "functionnode.h"
#include "generator.h"
#include "inclusionfilter.h"
#include "inclusionpolicy.h"
#include "node.h"
#include "tree.h"
#include "typedefnode.h"
#include "utilities.h"

#include <QtCore/qfileinfo.h>

using namespace Qt::Literals::StringLiterals;

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

    This extracts the anchor computation from XmlGenerator::refForNode()
    and Generator::fullDocumentLocation().
*/
QString HrefResolver::anchorForNode(const Node *node) const
{
    QString ref;

    switch (node->nodeType()) {
    case NodeType::Enum:
    case NodeType::QmlEnum:
        ref = node->name() + "-enum"_L1;
        break;
    case NodeType::Typedef: {
        const auto *tdf = static_cast<const TypedefNode *>(node);
        if (tdf->associatedEnum())
            return anchorForNode(tdf->associatedEnum());
    } Q_FALLTHROUGH();
    case NodeType::TypeAlias:
        ref = node->name() + "-typedef"_L1;
        break;
    case NodeType::Function: {
        const auto *fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::QmlSignal:
            ref = fn->name() + "-signal"_L1;
            break;
        case FunctionNode::QmlSignalHandler:
            ref = fn->name() + "-signal-handler"_L1;
            break;
        case FunctionNode::QmlMethod:
            ref = fn->name() + "-method"_L1;
            if (fn->overloadNumber() != 0)
                ref += '-'_L1 + QString::number(fn->overloadNumber());
            break;
        default:
            if (const auto *p = fn->primaryAssociatedProperty(); p && fn->doc().isEmpty()) {
                return anchorForNode(p);
            } else {
                ref = fn->name();
                if (fn->overloadNumber() != 0)
                    ref += '-'_L1 + QString::number(fn->overloadNumber());
            }
            break;
        }
    } break;
    case NodeType::SharedComment: {
        if (!node->isPropertyGroup())
            break;
    } Q_FALLTHROUGH();
    case NodeType::QmlProperty:
        if (node->isAttached())
            ref = node->name() + "-attached-prop"_L1;
        else
            ref = node->name() + "-prop"_L1;
        break;
    case NodeType::Property:
        ref = node->name() + "-prop"_L1;
        break;
    case NodeType::Variable:
        ref = node->name() + "-var"_L1;
        break;
    default:
        break;
    }

    if (ref.isEmpty())
        return ref;

    return Generator::cleanRef(ref);
}

/*!
    Computes a relative URL for \a node, relative to \a relative.
    Returns an empty string for null nodes, self-links (where the
    target is the same file and anchor as the context), and nodes
    excluded by inclusion policy.

    This consolidates logic from XmlGenerator::linkForNode() without
    depending on a Generator instance. Cross-module prefixing is
    applied when useOutputSubdirs is enabled and the node lives in
    a different tree than the relative node.
*/
QString HrefResolver::hrefForNode(const Node *node, const Node *relative) const
{
    if (node == nullptr)
        return QString();
    if (!node->url().isEmpty())
        return node->url();

    QString fn = fileName(node);
    if (fn.isEmpty())
        return {};

    const InclusionPolicy policy = Config::instance().createInclusionPolicy();
    const NodeContext context = node->createContext();
    if (!InclusionFilter::isIncluded(policy, context))
        return QString();

    if (node->parent() && node->parent()->isQmlType() && node->parent()->isAbstract()) {
        if (Generator::qmlTypeContext()) {
            if (Generator::qmlTypeContext()->inherits(node->parent())) {
                fn = fileName(Generator::qmlTypeContext());
            } else if (node->parent()->isInternal() && !Generator::noLinkErrors()) {
                node->doc().location().warning(
                        u"Cannot link to property in internal type '%1'"_s
                                .arg(node->parent()->name()));
                return QString();
            }
        }
    }

    QString link = fn;

    if (!node->isPageNode() || node->isPropertyGroup()) {
        QString ref = anchorForNode(node);
        if (relative && fn == fileName(relative) && ref == anchorForNode(relative))
            return QString();

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
