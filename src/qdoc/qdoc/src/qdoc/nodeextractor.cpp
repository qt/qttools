// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nodeextractor.h"

#include "aggregate.h"
#include "atom.h"
#include "classnode.h"
#include "doc.h"
#include "enumnode.h"
#include "functionnode.h"
#include "ir/contentbuilder.h"
#include "pagenode.h"
#include "parameters.h"
#include "propertynode.h"
#include "qmltypenode.h"
#include "sections.h"
#include "sharedcommentnode.h"
#include "text.h"
#include "typedefnode.h"
#include "utilities.h"
#include "variablenode.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

namespace NodeExtractor {

/*!
    \internal
    Extract page-level metadata from a PageNode into a value-type struct.

    This function reads classification, identity, brief, and body fields
    from the given PageNode and returns them as an IR::PageMetadata value.
    Body content is populated via ContentBuilder, which transforms the
    atom chain into structured content blocks. Format-conditional atoms
    are skipped unconditionally since the template generator builds a
    format-agnostic IR.

    For aggregate pages (classes, QML types, namespaces), member listings
    are extracted via the Sections infrastructure and stored as frozen
    SectionIR values.

    The caller (TemplateGenerator) invokes this before passing the
    result to IR::Builder, ensuring Builder never includes PageNode
    or other Node subclass headers.
*/
IR::PageMetadata extractPageMetadata(const PageNode *pn)
{
    Q_ASSERT_X(pn, "NodeExtractor::extractPageMetadata",
               "PageNode pointer must be non-null");
    IR::PageMetadata pm;

    pm.nodeType = pn->nodeType();
    pm.genus = pn->genus();
    pm.status = pn->status();
    pm.access = pn->access();

    pm.title = pn->title();
    pm.fullTitle = pn->fullTitle();
    pm.url = pn->url();
    pm.since = pn->since();
    pm.deprecatedSince = pn->deprecatedSince();
    pm.brief = pn->doc().briefText().toString();

    const Text &bodyText = pn->doc().body();
    if (const Atom *firstAtom = bodyText.firstAtom()) {
        IR::ContentBuilder contentBuilder;
        pm.body = contentBuilder.build(firstAtom);
    }

    if (pn->isAggregate())
        pm.summarySections = extractSummarySections(static_cast<const Aggregate *>(pn));

    return pm;
}

/*!
    \internal
    Build categorized summary sections for an aggregate node.

    Delegates to the Sections class for member distribution, then extracts
    results into frozen SectionIR values. The section variant (C++ class,
    QML type, or generic) is chosen based on the aggregate's node type.
*/
QList<IR::SectionIR> extractSummarySections(const Aggregate *aggregate)
{
    // Sections constructor takes non-const Aggregate*. The internal mutation
    // (clearing/reducing static section vectors) is side-effect-free for the
    // aggregate itself. This const_cast is safe because Sections only reads
    // from the aggregate; it mutates its own static section vectors.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    Sections sections(const_cast<Aggregate *>(aggregate));

    const auto &sv = sections.summarySections();

    QList<IR::SectionIR> result;
    for (const auto &section : sv) {
        if (section.isEmpty())
            continue;

        IR::SectionIR irSection;
        irSection.title = section.title();
        irSection.id = Utilities::asAsciiPrintable(section.title());
        irSection.singular = section.singular();
        irSection.plural = section.plural();

        // SharedCommentNode groups several declarations under one doc comment.
        // Expand them into individual MemberIR entries so each function appears
        // in the member table. This means the IR member count may exceed the
        // Section::members() count.
        for (const auto *member : section.members()) {
            if (member->isSharedCommentNode()) {
                const auto *scn = static_cast<const SharedCommentNode *>(member);
                for (const auto *child : scn->collective())
                    irSection.members.append(extractMemberIR(child));
            } else {
                irSection.members.append(extractMemberIR(member));
            }
        }

        for (const auto *reimpl : section.reimplementedMembers())
            irSection.reimplementedMembers.append(extractMemberIR(reimpl));

        for (const auto &[base, count] : section.inheritedMembers()) {
            IR::InheritedMembersIR inherited;
            inherited.className = base->plainFullName();
            inherited.count = count;
            inherited.href = base->url();
            irSection.inheritedMembers.append(inherited);
        }

        result.append(irSection);
    }
    return result;
}

/*!
    \internal
    Build a MemberIR from a single Node.

    Extracts identity, classification, and type-specific data from the node.
    FunctionNode provides signatures, parameters, and overload metadata.
    EnumNode provides scoped/unscoped signature and enum value listings.
    PropertyNode provides a qualified data type signature.
*/
IR::MemberIR extractMemberIR(const Node *node)
{
    IR::MemberIR member;

    member.name = node->name();
    member.fullName = node->plainFullName();
    member.href = node->url();
    member.brief = node->doc().briefText().toString();

    member.nodeType = node->nodeType();
    member.access = node->access();
    member.status = node->status();

    if (node->isFunction()) {
        const auto *fn = static_cast<const FunctionNode *>(node);
        member.signature = fn->signature(
            Node::SignatureReturnType | Node::SignatureDefaultValues);
        member.isStatic = fn->isStatic();
        member.isConst = fn->isConst();
        member.isVirtual = !fn->isNonvirtual();
        member.isSignal = fn->isSignal();
        member.isSlot = fn->isSlot();
        member.overloadNumber = fn->overloadNumber();
        member.isPrimaryOverload = fn->isPrimaryOverload();

        const Parameters &params = fn->parameters();
        for (int i = 0; i < params.count(); ++i) {
            IR::ParameterIR param;
            param.type = params.at(i).type();
            param.name = params.at(i).name();
            param.defaultValue = params.at(i).defaultValue();
            member.parameters.append(param);
        }
    } else if (node->isEnumType()) {
        const auto *en = static_cast<const EnumNode *>(node);
        member.signature = en->isScoped()
            ? QStringLiteral("enum class %1").arg(en->name())
            : QStringLiteral("enum %1").arg(en->name());

        for (const auto &item : en->items()) {
            IR::EnumValueIR ev;
            ev.name = item.name();
            ev.value = item.value();
            ev.since = item.since();
            member.enumValues.append(ev);
        }
    } else if (node->isProperty()) {
        const auto *pn = static_cast<const PropertyNode *>(node);
        member.signature = pn->name() + " : "_L1 + pn->qualifiedDataType();
    } else if (node->isTypedef()) {
        const auto *td = static_cast<const TypedefNode *>(node);
        member.signature = td->associatedEnum()
            ? "flags "_L1 + td->name()
            : td->name();
    } else if (node->nodeType() == NodeType::Variable) {
        const auto *vn = static_cast<const VariableNode *>(node);
        member.signature = vn->leftType() + vn->name() + vn->rightType();
    } else {
        member.signature = node->name();
    }

    return member;
}

} // namespace NodeExtractor

QT_END_NAMESPACE
