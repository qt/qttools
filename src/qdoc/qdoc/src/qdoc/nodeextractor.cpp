// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nodeextractor.h"

#include "aggregate.h"
#include "atom.h"
#include "classnode.h"
#include "collectionnode.h"
#include "config.h"
#include "doc.h"
#include "enumnode.h"
#include "functionnode.h"
#include "inclusionfilter.h"
#include "ir/contentbuilder.h"
#include "pagenode.h"
#include "parameters.h"
#include "propertynode.h"
#include "qmlpropertynode.h"
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

    if (pn->isQmlType()) {
        const auto *qcn = static_cast<const QmlTypeNode *>(pn);
        QString suffix = qcn->isQmlBasicType() ? " QML Value Type"_L1 : " QML Type"_L1;
        pm.title = pn->name() + suffix;
        pm.fullTitle = pm.title;
    } else {
        pm.title = pn->title();
        pm.fullTitle = pn->fullTitle();
    }

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

    if (pn->isQmlType()) {
        const auto *qcn = static_cast<const QmlTypeNode *>(pn);
        pm.qmlTypeData = extractQmlTypeData(qcn);
    }

    return pm;
}

/*!
    \internal
    Extract QML type metadata from a QmlTypeNode.

    Populates import statement, inheritance chain, inherited-by list,
    native C++ type link, and singleton/value-type flags. InclusionFilter
    is applied to match the legacy generator's visibility filtering.
*/
IR::QmlTypeData extractQmlTypeData(const QmlTypeNode *qcn)
{
    IR::QmlTypeData data;
    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    if (!qcn->logicalModuleName().isEmpty()) {
        bool includeImport = true;
        const CollectionNode *collection = qcn->logicalModule();
        if (collection) {
            const NodeContext context = collection->createContext();
            includeImport = InclusionFilter::isIncluded(policy, context);
        }
        if (includeImport) {
            QStringList parts = QStringList()
                << "import"_L1 << qcn->logicalModuleName() << qcn->logicalModuleVersion();
            data.importStatement = parts.join(' '_L1).trimmed();
        }
    }

    data.isSingleton = qcn->isSingleton();
    data.isValueType = qcn->isQmlBasicType();

    QmlTypeNode *base = qcn->qmlBaseNode();
    while (base) {
        const NodeContext context = base->createContext();
        if (InclusionFilter::isIncluded(policy, context))
            break;
        base = base->qmlBaseNode();
    }

    NodeList subs;
    QmlTypeNode::subclasses(qcn, subs, true);

    if (base) {
        IR::QmlTypeData::InheritsInfo inheritsInfo;
        inheritsInfo.name = base->name();
        inheritsInfo.href = base->url();
        const CollectionNode *baseModule = base->logicalModule();
        if (baseModule) {
            const NodeContext moduleContext = baseModule->createContext();
            if (InclusionFilter::isIncluded(policy, moduleContext))
                inheritsInfo.moduleName = base->logicalModuleName();
        }
        data.inherits = inheritsInfo;
    }

    if (!subs.isEmpty()) {
        QList<IR::QmlTypeData::InheritedByEntry> filteredSubs;
        for (const auto *sub : std::as_const(subs)) {
            const NodeContext context = sub->createContext();
            if (InclusionFilter::isIncluded(policy, context))
                filteredSubs.append({sub->name(), sub->url()});
        }
        std::sort(filteredSubs.begin(), filteredSubs.end(),
                  [](const IR::QmlTypeData::InheritedByEntry &a,
                     const IR::QmlTypeData::InheritedByEntry &b) {
                      return a.name < b.name;
                  });
        data.inheritedBy = filteredSubs;
    }

    ClassNode *cn = qcn->classNode();
    if (cn && cn->isQmlNativeType()) {
        const NodeContext context = cn->createContext();
        if (InclusionFilter::isIncluded(policy, context))
            data.nativeType = IR::QmlTypeData::NativeTypeInfo{cn->name(), cn->url()};
    }

    return data;
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
    Sections sections(aggregate);

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
    } else if (node->isQmlProperty()) {
        const auto *qpn = static_cast<const QmlPropertyNode *>(node);
        member.signature = qpn->name() + " : "_L1 + qpn->dataType();
        member.dataType = qpn->dataType();
        member.isAttached = qpn->isAttached();
        member.isDefault = qpn->isDefault();
        member.isReadOnly = qpn->isReadOnly();
        member.isRequired = qpn->isRequired();
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
