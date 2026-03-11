// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utilities.h"

#include "collectionnode.h"
#include "config.h"
#include "inclusionfilter.h"
#include "namespacenode.h"
#include "node.h"
#include "tree.h"

QT_BEGIN_NAMESPACE

namespace Utilities {

/*!
    \internal
    Computes the base filename for a documentation page node.

    This is the shared implementation used by both Generator::fileBase() and
    TemplateGenerator::fileBase(). The caller is responsible for navigating
    to the page/collection node and for caching the result; this function
    performs only the pure filename computation.

    The \a node must already be a page node or collection node.
    The \a project string is used for example page prefixes.
    The \a prefixFn and \a suffixFn callbacks supply the caller-specific
    output prefix and suffix (which differ between Generator and
    TemplateGenerator due to their different configuration mechanisms).

    \note This function lives in its own compilation unit (filebase.cpp)
    rather than utilities.cpp because it depends on Node, Config, and other
    heavyweight QDoc types. Keeping utilities.cpp free of these dependencies
    allows lightweight test targets to compile it without pulling in the
    full QDoc infrastructure.
*/
QString computeFileBase(
    const Node *node,
    const QString &project,
    const std::function<QString(const Node *)> &prefixFn,
    const std::function<QString(const Node *)> &suffixFn)
{
    QString base{node->name()};
    if (base.endsWith(".html"_L1))
        base.truncate(base.size() - 5);

    if (node->isCollectionNode()) {
        if (node->isQmlModule())
            base.append("-qmlmodule"_L1);
        else if (node->isModule())
            base.append("-module"_L1);
        base.append(suffixFn(node));
    } else if (node->isTextPageNode()) {
        if (node->isExample()) {
            base.prepend("%1-"_L1.arg(project.toLower()));
            base.append("-example"_L1);
        }
    } else if (node->isQmlType()) {
        /*
          To avoid file name conflicts in the html directory,
          we prepend a prefix (by default, "qml-") and an optional suffix
          to the file name. The suffix, if one exists, is appended to the
          module name.

          For historical reasons, skip the module name qualifier for QML value types
          in order to avoid excess redirects in the online docs. TODO: re-assess
        */
        if (!node->logicalModuleName().isEmpty() && !node->isQmlBasicType()) {
            const InclusionPolicy policy = Config::instance().createInclusionPolicy();
            const NodeContext context = node->logicalModule()->createContext();
            if (InclusionFilter::isIncluded(policy, context))
                base.prepend("%1%2-"_L1.arg(node->logicalModuleName(), suffixFn(node)));
        }
    } else if (node->isProxyNode()) {
        base.append("-%1-proxy"_L1.arg(node->tree()->physicalModuleName()));
    } else {
        base.clear();
        const Node *p = node;
        for (;;) {
            const Node *pp = p->parent();
            base.prepend(p->name());
            if (pp == nullptr || pp->name().isEmpty() || pp->isTextPageNode())
                break;
            base.prepend('-'_L1);
            p = pp;
        }
        if (node->isNamespace() && !node->name().isEmpty()) {
            const auto *ns = static_cast<const NamespaceNode *>(node);
            if (!ns->isDocumentedHere()) {
                base.append("-sub-"_L1);
                base.append(ns->tree()->camelCaseModuleName());
            }
        }
        base.append(suffixFn(node));
    }

    base.prepend(prefixFn(node));
    return asAsciiPrintable(base);
}

} // namespace Utilities

QT_END_NAMESPACE

