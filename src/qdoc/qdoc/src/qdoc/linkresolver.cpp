// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifdef QDOC_TEMPLATE_GENERATOR_ENABLED

#include "linkresolver.h"

#include "genustypes.h"
#include "hrefresolver.h"
#include "ir/contentblock.h"
#include "node.h"
#include "qdocdatabase.h"
#include "qdoclogging.h"

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

static Genus genusFromString(const QString &s)
{
    if (s == "cpp"_L1)
        return Genus::CPP;
    if (s == "qml"_L1)
        return Genus::QML;
    if (s == "doc"_L1)
        return Genus::DOC;
    if (s == "api"_L1)
        return Genus::API;
    return Genus::DontCare;
}

static bool isBrokenAutolink(const IR::InlineContent &inline_)
{
    return inline_.type == IR::InlineType::Link
            && inline_.link.has_value()
            && inline_.link->state == IR::LinkState::Broken
            && inline_.link->origin == IR::LinkOrigin::Auto;
}

/*!
    Constructs a LinkResolver that uses \a qdb for node lookup,
    \a hrefResolver for URL computation, and \a config for warning
    policy.
*/
LinkResolver::LinkResolver(QDocDatabase *qdb, const HrefResolver &hrefResolver,
                           const LinkResolverConfig &config)
    : m_qdb(qdb), m_hrefResolver(hrefResolver), m_config(config)
{
}

/*!
    Walks \a blocks and resolves all unresolved Link inlines in place.
    The \a relative node provides context for relative URL computation
    and warning emission.

    This must be called after ContentBuilder produces the block tree
    and before rendering.
*/
void LinkResolver::resolve(QList<IR::ContentBlock> &blocks, const Node *relative)
{
    for (auto &block : blocks)
        resolveBlock(block, relative);
}

/*!
    Resolves links within a single \a block by processing its inline
    content and recursing into child blocks.
*/
void LinkResolver::resolveBlock(IR::ContentBlock &block, const Node *relative)
{
    resolveInlines(block.inlineContent, relative);
    for (auto &child : block.children)
        resolveBlock(child, relative);
}

/*!
    Iterates \a inlines and resolves each Link element. Also recurses
    into children of all inline elements since Link inlines can contain
    nested formatting (such as Bold or Italic).
*/
void LinkResolver::resolveInlines(QList<IR::InlineContent> &inlines, const Node *relative)
{
    for (auto &inline_ : inlines) {
        if (inline_.type == IR::InlineType::Link && !inline_.href.isEmpty())
            resolveLink(inline_, relative);

        if (isBrokenAutolink(inline_)) {
            qCDebug(lcQdoc) << "Autolink degraded to text:" << inline_.href;
            inline_.type = IR::InlineType::Text;
            inline_.text = inline_.plainText();
            inline_.href.clear();
            inline_.children.clear();
            inline_.link.reset();
            inline_.attributes = QJsonObject();
            continue;
        }

        if (!inline_.children.isEmpty())
            resolveInlines(inline_.children, relative);
    }
}

/*!
    Core resolution logic for a single Link \a link. Mirrors the behavior
    of XmlGenerator::getLink() and XmlGenerator::getAutoLink():

    \list
    \li External URLs (http, https, ftp, file, mailto) are marked without
        node lookup.
    \li Node lookup uses genus and module metadata from ContentBuilder
        when available. Explicit links carry genus scoping (CPP, QML) and
        module names for tree-scoped search. Autolinks fall back to
        forest-wide search with DontCare genus.
    \li Deprecated node links are suppressed when the relative node isn't
        deprecated.
    \li HrefResolver returns an HrefResult variant: a URL on success, or
        an HrefSuppressReason (self-link, policy exclusion, etc.) that
        maps to LinkState::Suppressed.
    \li Unresolvable links emit warnings controlled by LinkResolverConfig.
    \endlist

    The \a relative node provides context for URL computation and
    warning source location.
*/
void LinkResolver::resolveLink(IR::InlineContent &link, const Node *relative)
{
    Q_ASSERT(link.link.has_value());

    const QString &target = link.href;

    // External URL -- no resolution needed.
    if (target.startsWith("http:"_L1) || target.startsWith("https:"_L1)
        || target.startsWith("ftp:"_L1) || target.startsWith("file:"_L1)
        || target.startsWith("mailto:"_L1)) {
        link.link->state = IR::LinkState::External;
        return;
    }

    // Use genus and module metadata from ContentBuilder when available.
    // Explicit links (LinkAtom) carry genus and module; autolinks don't.
    const Genus genus = genusFromString(
            link.attributes.value("linkGenus"_L1).toString());
    const QString &moduleName =
            link.attributes.value("linkModule"_L1).toString();
    const Node *targetNode =
            m_qdb->findNodeForTarget(target, relative, genus, moduleName);

    if (!targetNode) {
        if (link.link->origin == IR::LinkOrigin::Auto) {
            if (m_config.autolinkErrors && relative)
                relative->doc().location().warning(
                        u"Can't autolink to '%1'"_s.arg(target));
        } else {
            if (!m_config.noLinkErrors && relative)
                relative->doc().location().warning(
                        u"Can't link to '%1'"_s.arg(target));
        }

        link.link->state = IR::LinkState::Broken;
        return;
    }

    // Deprecated node suppression: don't link to deprecated nodes from
    // non-deprecated content, unless the link originates from within
    // the deprecated node's own documentation (parent check mirrors
    // HtmlGenerator::generateBody's inline deprecation logic).
    if (targetNode->isDeprecated() && relative
        && relative->parent() != targetNode && !relative->isDeprecated()) {
        link.href.clear();
        link.link->state = IR::LinkState::Suppressed;
        return;
    }

    // Compute URL via HrefResolver.
    QString url = targetNode->url();
    if (url.isNull()) {
        auto result = m_hrefResolver.hrefForNode(targetNode, relative);
        if (std::get_if<HrefSuppressReason>(&result)) {
            link.href.clear();
            link.link->state = IR::LinkState::Suppressed;
            return;
        }
        url = std::get<QString>(std::move(result));
    } else if (url.isEmpty()) {
        link.href.clear();
        link.link->state = IR::LinkState::Ignored;
        return;
    }

    link.href = url;
    link.link->state = IR::LinkState::Resolved;
}

QT_END_NAMESPACE

#endif // QDOC_TEMPLATE_GENERATOR_ENABLED
