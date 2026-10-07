// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOC_LINKRESOLVER_H
#define QDOC_LINKRESOLVER_H

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class HrefResolver;
class Node;
class QDocDatabase;

namespace IR {
struct ContentBlock;
struct InlineContent;
} // namespace IR

struct LinkResolverConfig
{
    bool autolinkErrors{false};
    bool noLinkErrors{false};
};

/*!
    \class LinkResolver
    \internal
    \brief Walks ContentBlock trees and resolves unresolved Link inlines to URLs.

    LinkResolver sits between ContentBuilder and rendering. ContentBuilder
    produces Link inlines with raw target strings (such as "QWidget" or
    "QString::size"). LinkResolver resolves these targets to actual URLs
    using QDocDatabase for node lookup and HrefResolver for URL computation.

    External URLs (http, https, ftp, file, mailto) are detected and marked
    without node lookup. Autolinks to deprecated nodes from non-deprecated
    content are suppressed; explicit \\l and \\sa links keep the author's
    reference, matching legacy HTML. Suppressed links carry a diagnostic
    linkState string identifying the reason (policy exclusion,
    self-reference, etc.).
    Unresolvable links emit warnings controlled by the injected
    LinkResolverConfig.

    \sa HrefResolver, IR::ContentBuilder
*/
class LinkResolver
{
public:
    /*!
        Context for a resolve() pass.

        \c semantic is the node whose documentation the content was
        authored in; \c page is the output page node that renders the
        content. See resolve() for the role each plays.
    */
    struct Context
    {
        const Node *semantic { nullptr };
        const Node *page { nullptr };
    };

    LinkResolver(QDocDatabase *qdb, const HrefResolver &hrefResolver,
                 const LinkResolverConfig &config);

    void resolve(QList<IR::ContentBlock> &blocks, Context context);

private:
    void resolveBlock(IR::ContentBlock &block, const Context &context);
    void resolveInlines(QList<IR::InlineContent> &inlines, const Context &context);
    void resolveLink(IR::InlineContent &link, const Context &context);

    QDocDatabase *m_qdb;
    const HrefResolver &m_hrefResolver;
    LinkResolverConfig m_config;
};

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED

#endif // QDOC_LINKRESOLVER_H
