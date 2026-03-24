// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nodeextractor.h"

#include "aggregate.h"
#include "atom.h"
#include "classnode.h"
#include "codemarker.h"
#include "collectionnode.h"
#include "comparisoncategory.h"
#include "config.h"
#include "doc.h"
#include "enumnode.h"
#include "functionnode.h"
#include "hrefresolver.h"
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

#include <QRegularExpression>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

static QString resolveHref(const HrefResolver *resolver, const Node *target, const Node *relative)
{
    if (!resolver)
        return target->url();
    auto result = resolver->hrefForNode(target, relative);
    if (const auto *href = std::get_if<QString>(&result))
        return *href;
    return {};
}

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
IR::PageMetadata extractPageMetadata(const PageNode *pn, const HrefResolver *hrefResolver)
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
        // Offset section heading levels to account for page structure.
        // QDoc's \section1 maps to level 1, but pages already use <h1>
        // for the title. The legacy generators apply a node-type-dependent
        // offset; we replicate the same mapping here.
        const int headingOffset = [&] {
            switch (pn->nodeType()) {
            case NodeType::Namespace:
            case NodeType::Class:
            case NodeType::Struct:
            case NodeType::Union:
            case NodeType::Module:
                return 2;
            case NodeType::QmlModule:
            case NodeType::QmlValueType:
            case NodeType::QmlType:
            case NodeType::Page:
            case NodeType::Group:
                return 1;
            default:
                return 3;
            }
        }();
        IR::ContentBuilder contentBuilder(IR::BriefHandling::Skip, headingOffset);
        pm.body = contentBuilder.build(firstAtom);
    }

    if (pn->isAggregate()) {
        const auto *aggregate = static_cast<const Aggregate *>(pn);
        pm.summarySections = extractSummarySections(aggregate, hrefResolver);
        pm.detailSections = extractDetailSections(aggregate, hrefResolver);
    }

    if (pn->isQmlType()) {
        const auto *qcn = static_cast<const QmlTypeNode *>(pn);
        pm.qmlTypeData = extractQmlTypeData(qcn, hrefResolver);
    }

    if (pn->isCollectionNode()) {
        const auto *cn = static_cast<const CollectionNode *>(pn);
        pm.collectionData = extractCollectionData(cn, hrefResolver);
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
IR::QmlTypeData extractQmlTypeData(const QmlTypeNode *qcn, const HrefResolver *hrefResolver)
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
        inheritsInfo.href = resolveHref(hrefResolver, base, qcn);
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
                filteredSubs.append({sub->name(), resolveHref(hrefResolver, sub, qcn)});
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
            data.nativeType = IR::QmlTypeData::NativeTypeInfo{cn->name(), resolveHref(hrefResolver, cn, qcn)};
    }

    return data;
}

/*!
    \internal
    Extract collection metadata from a CollectionNode.

    Populates module identity, CMake/qmake build variables, technology
    preview state, and pre-sorted member listings. For C++ modules,
    members are categorized into separate namespace and class lists.
    For groups and QML modules, a single flat member list is produced.

    All member lists are filtered through InclusionFilter (excluding
    internal entries) and exclude deprecated nodes, then sorted
    alphabetically by name (case-insensitive).
*/
IR::CollectionData extractCollectionData(const CollectionNode *cn, const HrefResolver *hrefResolver)
{
    IR::CollectionData data;

    data.logicalModuleName = cn->logicalModuleName();
    data.logicalModuleVersion = cn->logicalModuleVersion();
    data.qtVariable = cn->qtVariable();
    data.cmakePackage = cn->cmakePackage();
    data.cmakeComponent = cn->cmakeComponent();
    data.cmakeTargetItem = cn->cmakeTargetItem();
    data.state = cn->state();

    data.isModule = cn->isModule();
    data.isQmlModule = cn->isQmlModule();
    data.isGroup = cn->isGroup();
    data.noAutoList = cn->noAutoList();

    if (cn->noAutoList())
        return data;

    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    auto makeMemberEntry = [hrefResolver, cn](const Node *node) -> IR::CollectionData::MemberEntry {
        return { node->name(), resolveHref(hrefResolver, node, cn), node->doc().briefText().toString() };
    };

    auto sortEntries = [](QList<IR::CollectionData::MemberEntry> &entries) {
        std::sort(entries.begin(), entries.end(),
                  [](const IR::CollectionData::MemberEntry &a,
                     const IR::CollectionData::MemberEntry &b) {
                      return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
                  });
    };

    if (cn->isModule()) {
        const NodeMap nsMap = cn->getMembers(NodeType::Namespace);
        for (auto *node : nsMap.values()) {
            const NodeContext context = node->createContext();
            if (InclusionFilter::isIncluded(policy, context) && !node->isDeprecated())
                data.namespaces.append(makeMemberEntry(node));
        }
        sortEntries(data.namespaces);

        const NodeMap classMap = cn->getMembers([](const Node *n) { return n->isClassNode(); });
        for (auto *node : classMap.values()) {
            const NodeContext context = node->createContext();
            if (InclusionFilter::isIncluded(policy, context) && !node->isDeprecated())
                data.classes.append(makeMemberEntry(node));
        }
        sortEntries(data.classes);
    } else {
        for (const auto *node : cn->members()) {
            if (!node->isInAPI())
                continue;
            const NodeContext context = node->createContext();
            if (InclusionFilter::isIncluded(policy, context) && !node->isDeprecated())
                data.members.append(makeMemberEntry(node));
        }
        sortEntries(data.members);
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
QList<IR::SectionIR> extractSummarySections(const Aggregate *aggregate, const HrefResolver *hrefResolver)
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
                for (const auto *child : scn->collective()) {
                    IR::MemberIR irMember = extractMemberIR(child, hrefResolver, aggregate);
                    irMember.href = "#"_L1 + hrefResolver->anchorForNode(child);
                    irSection.members.append(irMember);
                }
            } else {
                IR::MemberIR irMember = extractMemberIR(member, hrefResolver, aggregate);
                irMember.href = "#"_L1 + hrefResolver->anchorForNode(member);
                irSection.members.append(irMember);
            }
        }

        for (const auto *reimpl : section.reimplementedMembers())
            irSection.reimplementedMembers.append(extractMemberIR(reimpl, hrefResolver, aggregate));

        for (const auto &[base, count] : section.inheritedMembers()) {
            IR::InheritedMembersIR inherited;
            inherited.className = base->plainFullName();
            inherited.count = count;
            inherited.href = resolveHref(hrefResolver, base, aggregate);
            irSection.inheritedMembers.append(inherited);
        }

        result.append(irSection);
    }
    return result;
}

/*!
    \internal
    Build categorized detail sections for an aggregate node.

    Iterates Sections::detailsSections() and extracts full member
    documentation including body content, anchor IDs, and metadata.
    SharedCommentNode groups share a single documentation body across
    their children, with each child getting its own anchorId and synopsis.
*/
QList<IR::SectionIR> extractDetailSections(const Aggregate *aggregate, const HrefResolver *hrefResolver)
{
    Sections sections(aggregate);
    const auto &sv = sections.detailsSections();

    QList<IR::SectionIR> result;
    for (const auto &section : sv) {
        if (section.isEmpty())
            continue;

        IR::SectionIR irSection;
        irSection.title = section.title();
        irSection.id = Utilities::asAsciiPrintable(section.title());
        irSection.singular = section.singular();
        irSection.plural = section.plural();

        for (const auto *member : section.members()) {
            if (member->isSharedCommentNode()) {
                const auto *scn = static_cast<const SharedCommentNode *>(member);

                QList<IR::ContentBlock> sharedBody;
                const Text &bodyText = scn->doc().body();
                if (const Atom *firstAtom = bodyText.firstAtom()) {
                    IR::ContentBuilder contentBuilder(IR::BriefHandling::Include);
                    sharedBody = contentBuilder.build(firstAtom);
                }

                QList<IR::ContentBlock> sharedAlso;
                const QList<Text> &alsoTexts = scn->doc().alsoList();
                for (const Text &alsoText : alsoTexts) {
                    if (const Atom *firstAtom = alsoText.firstAtom()) {
                        IR::ContentBuilder contentBuilder(IR::BriefHandling::Include);
                        sharedAlso.append(contentBuilder.build(firstAtom));
                    }
                }

                for (const auto *child : scn->collective()) {
                    IR::MemberIR irMember = extractMemberIR(child, hrefResolver, aggregate, MemberExtractionLevel::Detail);
                    irMember.body = sharedBody;
                    irMember.alsoList = sharedAlso;
                    irSection.members.append(irMember);
                }
            } else {
                irSection.members.append(extractMemberIR(member, hrefResolver, aggregate, MemberExtractionLevel::Detail));
            }
        }

        result.append(irSection);
    }
    return result;
}

static QString threadSafenessString(Node::ThreadSafeness ts)
{
    switch (ts) {
    case Node::Reentrant:
        return "reentrant"_L1;
    case Node::ThreadSafe:
        return "thread-safe"_L1;
    default:
        return {};
    }
}

/*!
    \internal
    Build a MemberIR from a single Node.

    Extracts identity, classification, and type-specific data from the node.
    FunctionNode provides signatures, parameters, and overload metadata.
    EnumNode provides scoped/unscoped signature and enum value listings.
    PropertyNode provides a qualified data type signature.

    When \a level is MemberExtractionLevel::Detail, also populates
    detail documentation fields: anchorId, synopsis, since,
    threadSafety, comparisonCategory, noexcept metadata, body (via
    ContentBuilder), and alsoList.
*/
IR::MemberIR extractMemberIR(const Node *node, const HrefResolver *hrefResolver, const Node *relative, MemberExtractionLevel level)
{
    const bool includeDetail = (level == MemberExtractionLevel::Detail);
    IR::MemberIR member;

    member.name = node->name();
    member.fullName = node->plainFullName();
    member.href = resolveHref(hrefResolver, node, relative);
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

    if (includeDetail) {
        member.anchorId = hrefResolver->anchorForNode(node);
        member.synopsis = member.signature;
        member.since = node->since();
        member.threadSafety = threadSafenessString(node->threadSafeness());

        const std::string catStr = comparisonCategoryAsString(node->comparisonCategory());
        if (!catStr.empty())
            member.comparisonCategory = QString::fromStdString(catStr);

        if (node->isFunction()) {
            const auto *fn = static_cast<const FunctionNode *>(node);
            const auto &noexcept_ = fn->getNoexcept();
            if (noexcept_) {
                member.isNoexcept = true;
                member.noexceptNote = *noexcept_;
            }
        }

        const Text &bodyText = node->doc().body();
        if (const Atom *firstAtom = bodyText.firstAtom()) {
            IR::ContentBuilder contentBuilder(IR::BriefHandling::Include);
            member.body = contentBuilder.build(firstAtom);
        }

        const QList<Text> &alsoTexts = node->doc().alsoList();
        for (const Text &alsoText : alsoTexts) {
            if (const Atom *firstAtom = alsoText.firstAtom()) {
                IR::ContentBuilder contentBuilder(IR::BriefHandling::Include);
                QList<IR::ContentBlock> blocks = contentBuilder.build(firstAtom);
                member.alsoList.append(blocks);
            }
        }
    }

    return member;
}

static QString stripCodeMarkerTags(const QString &marked)
{
    static const QRegularExpression tagRegex(
        "<@[a-z]+[^>]*>|</@[a-z]+>"_L1,
        QRegularExpression::InvertedGreedinessOption);
    QString result = marked;
    result.remove(tagRegex);
    return result;
}

/*!
    \internal
    Extract a grouped all-members listing for a QML type.

    Constructs a Sections object from the QmlTypeNode, extracts
    allMembersSection().classNodesList() to group members by
    originating QML type, and builds AllMemberEntry items with
    QML-specific hints and property group nesting.
*/
static IR::AllMembersIR extractQmlAllMembersIR(const QmlTypeNode *qcn, const HrefResolver *hrefResolver)
{
    IR::AllMembersIR result;
    result.typeName = qcn->name();
    result.typeHref = resolveHref(hrefResolver, qcn, qcn);
    result.isQmlType = true;

    Sections sections(qcn);
    ClassNodesList &groupedMembers = sections.allMembersSection().classNodesList();
    if (groupedMembers.isEmpty())
        return result;

    CodeMarker *marker = CodeMarker::markerForLanguage("QML"_L1);
    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    std::function<IR::AllMemberEntry(Node *)> buildEntry = [&](Node *node) -> IR::AllMemberEntry {
        IR::AllMemberEntry entry;
        entry.signature = stripCodeMarkerTags(
            marker->markedUpQmlItem(node, true));
        entry.href = resolveHref(hrefResolver, node, qcn);

        if (node->isQmlProperty()) {
            auto *qpn = static_cast<QmlPropertyNode *>(node);
            QStringList qmlHints = qpn->hints();
            if (qpn->isAttached() && !qmlHints.contains("attached"_L1))
                qmlHints << "attached"_L1;
            for (const auto &h : std::as_const(qmlHints))
                entry.hints.append(h);
        } else if (node->isAttached()) {
            entry.hints.append("attached"_L1);
        }

        if (node->isPropertyGroup()) {
            entry.isPropertyGroup = true;
            const auto *scn = static_cast<SharedCommentNode *>(node);
            for (auto *child : scn->collective()) {
                const NodeContext childContext = child->createContext();
                if (!InclusionFilter::isIncluded(policy, childContext))
                    continue;
                entry.children.append(buildEntry(child));
            }
        }

        return entry;
    };

    auto isVisible = [&policy](Node *node) {
        const NodeContext context = node->createContext();
        return InclusionFilter::isIncluded(policy, context)
            && !(node->isSharingComment() && node->sharedCommentNode()->isPropertyGroup());
    };

    for (const auto &[originType, nodes] : groupedMembers) {
        Q_ASSERT(originType);
        if (nodes.isEmpty())
            continue;

        IR::MemberGroup group;
        if (originType != qcn) {
            group.typeName = originType->name();
            group.typeHref = resolveHref(hrefResolver, originType, qcn);
        }

        for (auto *node : nodes) {
            if (isVisible(node))
                group.members.append(buildEntry(node));
        }

        result.memberGroups.append(group);
    }

    return result;
}

/*!
    \internal
    Extract a flat all-members listing for a C++ class or namespace.

    Constructs a Sections object from the aggregate, extracts
    allMembersSection().members(), builds AllMemberEntry for each
    visible member, and returns an AllMembersIR with isQmlType=false.
*/
static IR::AllMembersIR extractCppAllMembersIR(const Aggregate *aggregate, const HrefResolver *hrefResolver)
{
    IR::AllMembersIR result;
    result.typeName = aggregate->plainFullName();
    result.typeHref = resolveHref(hrefResolver, aggregate, aggregate);
    result.isQmlType = false;

    Sections sections(aggregate);
    const Section &allMembers = sections.allMembersSection();
    if (allMembers.isEmpty())
        return result;

    CodeMarker *marker = CodeMarker::markerForCode(QString());
    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    for (const auto *node : allMembers.members()) {
        if (node->name().isEmpty())
            continue;
        const NodeContext context = node->createContext();
        if (!InclusionFilter::isIncluded(policy, context))
            continue;

        IR::AllMemberEntry entry;
        entry.signature = stripCodeMarkerTags(
            marker->markedUpSynopsis(node, aggregate, Section::AllMembers));
        entry.href = resolveHref(hrefResolver, node, aggregate);
        result.members.append(entry);
    }

    return result;
}

/*!
    \internal
    Extract all-members IR for a page node.

    Dispatches to the QML or C++ extraction function based on the page
    type. Returns std::nullopt for page types that don't have member
    listing pages (generic pages, QML basic types) or when the listing
    would be empty.
*/
std::optional<IR::AllMembersIR> extractAllMembersIR(const PageNode *pn, const HrefResolver *hrefResolver)
{
    if (pn->isQmlType()) {
        const auto *qcn = static_cast<const QmlTypeNode *>(pn);
        if (qcn->isQmlBasicType())
            return std::nullopt;
        auto result = extractQmlAllMembersIR(qcn, hrefResolver);
        bool hasMember = false;
        for (const auto &group : std::as_const(result.memberGroups)) {
            if (!group.members.isEmpty()) {
                hasMember = true;
                break;
            }
        }
        if (!hasMember)
            return std::nullopt;
        return result;
    }

    if (pn->isAggregate() && (pn->isClassNode() || pn->isNamespace())) {
        const auto *aggregate = static_cast<const Aggregate *>(pn);
        auto result = extractCppAllMembersIR(aggregate, hrefResolver);
        if (result.members.isEmpty())
            return std::nullopt;
        return result;
    }

    return std::nullopt;
}

} // namespace NodeExtractor

QT_END_NAMESPACE
