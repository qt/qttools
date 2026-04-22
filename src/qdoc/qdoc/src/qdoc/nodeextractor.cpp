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
#include "generator.h"
#include "hrefresolver.h"
#include "inclusionfilter.h"
#include "namespacenode.h"
#include "ir/contentbuilder.h"
#include "ir/signaturespan.h"
#include "pagenode.h"
#include "parameters.h"
#include "propertynode.h"
#include "qmlpropertynode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "sections.h"
#include "sharedcommentnode.h"
#include "template_declaration.h"
#include "text.h"
#include "tree.h"
#include "typedefnode.h"
#include "utilities.h"
#include "textutils.h"
#include "variablenode.h"

#include "location.h"

#include <QRegularExpression>

#include <deque>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

static IR::DiagnosticHandler diagnosticHandlerFor(const Node *node)
{
    const Location &loc = node->doc().location();
    return [loc](QtMsgType type, const QString &message) {
        switch (type) {
        case QtWarningMsg:
            loc.warning(message);
            break;
        default:
            loc.warning(message);
            break;
        }
    };
}

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
    } else if (pn->isClassNode() || pn->isNamespace() || pn->isHeader()) {
        // plainFullName() produces qualified names for nested aggregates
        // (e.g. "Outer::Inner"), matching the legacy generator behavior.
        // For top-level types and headers the result is the same as name().
        const auto *aggregate = static_cast<const Aggregate *>(pn);
        pm.fullTitle = aggregate->plainFullName();
        pm.title = pm.fullTitle;
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
        IR::ContentBuilder contentBuilder(IR::BriefHandling::Skip, headingOffset,
                                          diagnosticHandlerFor(pn));
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

    if (pn->isClassNode() || pn->isNamespace() || pn->isHeader()) {
        const auto *aggregate = static_cast<const Aggregate *>(pn);
        pm.cppReferenceData = extractCppReferenceData(aggregate, hrefResolver);
    }

    pm.navigationData = extractNavigationData(pn, hrefResolver);

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

static QList<IR::SignatureSpan> buildTemplateDeclSpans(const RelaxedTemplateDeclaration *templateDecl);

/*!
    \internal
    Extract C++ reference page metadata from a class, namespace, or header.

    Reads requisite table fields (header include, build-system snippets,
    status), inheritance hierarchies, template declarations, comparison
    operators, thread-safeness, and group associations. The result is a
    value-type struct that captures everything the template generator needs
    to render the requisites table and secondary sections without touching
    the Node tree at render time.

    All three aggregate page types (ClassNode, NamespaceNode, HeaderNode)
    are handled, with ClassNode-specific sections gated on isClassNode().
*/
IR::CppReferenceData extractCppReferenceData(const Aggregate *aggregate, const HrefResolver *hrefResolver)
{
    IR::CppReferenceData data;
    QDocDatabase *qdb = QDocDatabase::qdocDB();

    data.isNamespace = aggregate->isNamespace();
    data.isHeader = aggregate->isHeader();
    data.isInnerClass = aggregate->parent() && aggregate->parent()->isClassNode();
    data.typeWord = aggregate->typeWord(false);
    data.hasObsoleteMembers = aggregate->hasObsoleteMembers();

    auto ancestors = aggregate->plainFullName().split("::"_L1);
    ancestors.pop_back();
    data.ancestorNames = ancestors;

    if (aggregate->includeFile())
        data.headerInclude = *aggregate->includeFile();

    if (!aggregate->physicalModuleName().isEmpty()) {
        const CollectionNode *cn =
            qdb->getCollectionNode(aggregate->physicalModuleName(), NodeType::Module);
        if (cn && (!cn->cmakeComponent().isEmpty() || !cn->cmakePackage().isEmpty())) {
            const QString package = cn->cmakePackage().isEmpty()
                ? "Qt"_L1 + QString::number(QT_VERSION_MAJOR)
                : cn->cmakePackage();
            QString findPkg;
            if (cn->cmakeComponent().isEmpty())
                findPkg = "find_package("_L1 + package + " REQUIRED)"_L1;
            else
                findPkg = "find_package("_L1 + package + " REQUIRED COMPONENTS "_L1
                    + cn->cmakeComponent() + ")"_L1;

            QString target;
            if (!cn->cmakeTargetItem().isEmpty()) {
                target = cn->cmakeTargetItem();
            } else if (cn->cmakeComponent().isEmpty()) {
                target = package + "::"_L1 + package;
            } else {
                target = package + "::"_L1 + cn->cmakeComponent();
            }

            data.cmakeFindPackage = findPkg;
            data.cmakeTargetLinkLibraries =
                "target_link_libraries(mytarget PRIVATE "_L1 + target + ")"_L1;
        }
        if (cn && !cn->qtVariable().isEmpty())
            data.qmakeVariable = "QT += "_L1 + cn->qtVariable();
    }

    auto statusOpt = formatStatus(aggregate, qdb);
    if (statusOpt) {
        data.statusText = *statusOpt;
        if (aggregate->status() == Status::Deprecated)
            data.statusCssClass = "deprecated"_L1;
        else if (!aggregate->deprecatedSince().isEmpty())
            data.statusCssClass = "pending-deprecation"_L1;
        else if (aggregate->status() == Status::Preliminary)
            data.statusCssClass = "preliminary"_L1;
        else
            data.statusCssClass = "status"_L1;
    }

    if (aggregate->isClassNode()) {
        auto *classNode = const_cast<ClassNode *>(static_cast<const ClassNode *>(aggregate));

        if (classNode->isQmlNativeType()) {
            const InclusionPolicy policy = Config::instance().createInclusionPolicy();
            const NodeContext context = classNode->createContext();
            if (InclusionFilter::isIncluded(policy, context)) {
                QList<QmlTypeNode *> nativeTypes{classNode->qmlNativeTypes().cbegin(),
                                                  classNode->qmlNativeTypes().cend()};
                if (!nativeTypes.isEmpty()) {
                    std::sort(nativeTypes.begin(), nativeTypes.end(), Node::nodeNameLessThan);
                    data.qmlNativeType = IR::CppReferenceData::QmlNativeTypeLink{
                        nativeTypes.first()->name(),
                        resolveHref(hrefResolver, nativeTypes.first(), aggregate)
                    };
                }
            }
        }

        const auto *metaTags = classNode->doc().metaTagMap();
        if (metaTags && metaTags->contains(u"qdoc-suppress-inheritance"_s))
            data.suppressInheritance = true;

        if (!data.suppressInheritance) {
            const auto &baseClasses = classNode->baseClasses();
            for (const auto &bc : baseClasses) {
                if (bc.m_node) {
                    data.baseClasses.append({
                        bc.m_node->plainFullName(),
                        resolveHref(hrefResolver, bc.m_node, aggregate),
                        bc.m_access
                    });
                }
            }

            const auto &derivedClasses = classNode->derivedClasses();
            for (const auto &dc : derivedClasses) {
                if (dc.m_node) {
                    data.derivedClasses.append({
                        dc.m_node->plainFullName(),
                        resolveHref(hrefResolver, dc.m_node, aggregate)
                    });
                }
            }
            std::sort(data.derivedClasses.begin(), data.derivedClasses.end(),
                      [](const IR::CppReferenceData::DerivedClassEntry &a,
                         const IR::CppReferenceData::DerivedClassEntry &b) {
                          return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
                      });
        }
    }

    if (aggregate->templateDecl()) {
        data.templateDeclSpans = buildTemplateDeclSpans(&*aggregate->templateDecl());
    }

    const auto selfCategory = aggregate->comparisonCategory();
    if (selfCategory != ComparisonCategory::None)
        data.selfComparisonCategory = QString::fromStdString(comparisonCategoryAsString(selfCategory));

    const auto *comparesMap = aggregate->doc().comparesWithMap();
    if (comparesMap && !comparesMap->isEmpty()) {
        for (auto [key, description] : comparesMap->asKeyValueRange()) {
            IR::CppReferenceData::ComparisonEntry entry;
            entry.category = QString::fromStdString(comparisonCategoryAsString(key));

            const QStringList types{description.firstAtom()->string().split(';'_L1)};
            entry.comparableTypes = types;

            if (description.firstAtom()->next() != description.lastAtom()) {
                Text descText = Text::subText(description.firstAtom()->next(),
                                              description.lastAtom());
                entry.description = descText.toString();
            }
            data.comparisonEntries.append(entry);
        }
    }

    Node::ThreadSafeness ts = aggregate->threadSafeness();
    if (ts != Node::UnspecifiedSafeness) {
        IR::CppReferenceData::ThreadSafetyInfo tsInfo;
        switch (ts) {
        case Node::NonReentrant:
            tsInfo.level = "non-reentrant"_L1;
            break;
        case Node::Reentrant:
            tsInfo.level = "reentrant"_L1;
            break;
        case Node::ThreadSafe:
            tsInfo.level = "thread-safe"_L1;
            break;
        default:
            break;
        }

        NodeList reentrant, threadsafe, nonreentrant;
        bool hasExceptions = false;
        for (const auto *child : aggregate->childNodes()) {
            if (!child->isDeprecated()) {
                switch (child->threadSafeness()) {
                case Node::Reentrant:
                    reentrant.append(const_cast<Node *>(child));
                    if (ts == Node::ThreadSafe) hasExceptions = true;
                    break;
                case Node::ThreadSafe:
                    threadsafe.append(const_cast<Node *>(child));
                    if (ts == Node::Reentrant) hasExceptions = true;
                    break;
                case Node::NonReentrant:
                    nonreentrant.append(const_cast<Node *>(child));
                    hasExceptions = true;
                    break;
                default:
                    break;
                }
            }
        }
        if (hasExceptions) {
            for (const auto *node : std::as_const(reentrant)) {
                tsInfo.reentrantExceptions.append({
                    node->plainFullName(),
                    resolveHref(hrefResolver, node, aggregate)
                });
            }
            for (const auto *node : std::as_const(threadsafe)) {
                tsInfo.threadSafeExceptions.append({
                    node->plainFullName(),
                    resolveHref(hrefResolver, node, aggregate)
                });
            }
            for (const auto *node : std::as_const(nonreentrant)) {
                tsInfo.nonReentrantExceptions.append({
                    node->plainFullName(),
                    resolveHref(hrefResolver, node, aggregate)
                });
            }
        }
        data.threadSafety = std::move(tsInfo);
    }

    const QStringList &groupNames = aggregate->groupNames();
    if (!groupNames.isEmpty()) {
        const auto &groupMap = qdb->groups();
        for (const auto &groupName : groupNames) {
            auto it = groupMap.find(groupName);
            if (it == groupMap.end() || !*it)
                continue;
            CollectionNode *group = *it;
            // TODO: mergeCollections() mutates the node tree during
            // extraction, violating the principle that the new pipeline
            // reads without side effects. Replace with an eager merge
            // pass that runs before generation begins.
            qdb->mergeCollections(group);
            if (group->wasSeen()) {
                data.groups.append({
                    group->fullTitle(),
                    resolveHref(hrefResolver, group, aggregate)
                });
            }
        }
    }

    if (aggregate->isNamespace()) {
        const auto *ns = static_cast<const NamespaceNode *>(aggregate);
        if (!ns->hasDoc() && ns->docNode()) {
            data.isPartialNamespace = true;
            data.fullNamespaceHref = resolveHref(hrefResolver, ns->docNode(), aggregate);
            data.fullNamespaceModuleName = ns->docNode()->tree()->camelCaseModuleName();
        }
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
        irSection.id = TextUtils::asAsciiPrintable(section.title());
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
        irSection.id = TextUtils::asAsciiPrintable(section.title());
        irSection.singular = section.singular();
        irSection.plural = section.plural();

        for (const auto *member : section.members()) {
            if (member->isSharedCommentNode()) {
                const auto *scn = static_cast<const SharedCommentNode *>(member);

                QList<IR::ContentBlock> sharedBody;
                const Text &bodyText = scn->doc().body();
                if (const Atom *firstAtom = bodyText.firstAtom()) {
                    IR::ContentBuilder contentBuilder(IR::BriefHandling::Include, 0,
                                                      diagnosticHandlerFor(scn));
                    sharedBody = contentBuilder.build(firstAtom);
                }

                QList<IR::ContentBlock> sharedAlso;
                const QList<Text> &alsoTexts = scn->doc().alsoList();
                for (const Text &alsoText : alsoTexts) {
                    if (const Atom *firstAtom = alsoText.firstAtom()) {
                        IR::ContentBuilder contentBuilder(IR::BriefHandling::Include, 0,
                                                          diagnosticHandlerFor(scn));
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
            IR::ContentBuilder contentBuilder(IR::BriefHandling::Include, 0,
                                              diagnosticHandlerFor(node));
            member.body = contentBuilder.build(firstAtom);
        }

        const QList<Text> &alsoTexts = node->doc().alsoList();
        for (const Text &alsoText : alsoTexts) {
            if (const Atom *firstAtom = alsoText.firstAtom()) {
                IR::ContentBuilder contentBuilder(IR::BriefHandling::Include, 0,
                                                  diagnosticHandlerFor(node));
                QList<IR::ContentBlock> blocks = contentBuilder.build(firstAtom);
                member.alsoList.append(blocks);
            }
        }
    }

    Section::Style spanStyle = includeDetail ? Section::Details : Section::Summary;
    member.signatureSpans = buildSignatureSpans(node, hrefResolver, relative, spanStyle);

    return member;
}

static QList<IR::SignatureSpan> buildTypeSpans(const QString &typeString)
{
    QList<IR::SignatureSpan> spans;
    QString pendingWord;

    for (int i = 0; i <= typeString.size(); ++i) {
        QChar ch;
        if (i != typeString.size())
            ch = typeString.at(i);

        QChar lower = ch.toLower();
        if ((lower >= 'a'_L1 && lower <= 'z'_L1) || ch.digitValue() >= 0
            || ch == '_'_L1 || ch == ':'_L1) {
            pendingWord += ch;
        } else {
            if (!pendingWord.isEmpty()) {
                bool isProbablyType = (pendingWord != "const"_L1);
                IR::SignatureSpan span;
                span.role = isProbablyType ? IR::SpanRole::Type : IR::SpanRole::Text;
                span.text = pendingWord;
                spans.append(span);
            }
            pendingWord.clear();

            if (!ch.isNull()) {
                IR::SignatureSpan span;
                span.role = IR::SpanRole::Text;
                span.text = QString(ch);
                spans.append(span);
            }
        }
    }
    return spans;
}

static QList<IR::SignatureSpan> buildExtraSpans(const Node *node, Section::Style style)
{
    QString extraStr = CodeMarker::extraSynopsis(node, style);
    if (extraStr.isEmpty())
        return {};

    // extraSynopsis may contain <@extref target="...">text</@extref> tags for
    // cppreference links. Parse those into ExternalRef spans; everything else
    // becomes Extra spans.
    static const QRegularExpression extrefRegex(
        u"<@extref target=\"([^\"]+)\">([^<]+)</@extref>"_s);

    QList<IR::SignatureSpan> spans;
    IR::SignatureSpan wrapper;
    wrapper.role = IR::SpanRole::Extra;

    qsizetype pos = 0;
    auto it = extrefRegex.globalMatch(extraStr);
    while (it.hasNext()) {
        auto match = it.next();
        if (match.capturedStart() > pos) {
            IR::SignatureSpan textSpan;
            textSpan.role = IR::SpanRole::Text;
            textSpan.text = extraStr.mid(pos, match.capturedStart() - pos);
            wrapper.children.append(textSpan);
        }
        IR::SignatureSpan ref;
        ref.role = IR::SpanRole::ExternalRef;
        ref.text = match.captured(2);
        ref.href = "https://en.cppreference.com/w/cpp/language/"_L1 + match.captured(1);
        wrapper.children.append(ref);
        pos = match.capturedEnd();
    }
    if (pos < extraStr.size()) {
        IR::SignatureSpan textSpan;
        textSpan.role = IR::SpanRole::Text;
        textSpan.text = extraStr.mid(pos);
        wrapper.children.append(textSpan);
    }

    if (wrapper.children.isEmpty()) {
        wrapper.text = extraStr;
    }
    spans.append(wrapper);
    return spans;
}

static QList<IR::SignatureSpan> buildTemplateDeclSpans(const RelaxedTemplateDeclaration *templateDecl)
{
    if (!templateDecl)
        return {};

    IR::SignatureSpan declSpan;
    declSpan.role = IR::SpanRole::TemplateDecl;
    declSpan.text = "template"_L1;

    IR::SignatureSpan open;
    open.role = IR::SpanRole::Text;
    open.text = "<"_L1;
    declSpan.children.append(open);

    bool first = true;
    for (const auto &param : templateDecl->parameters) {
        if (param.sfinae_constraint)
            continue;
        if (!first) {
            IR::SignatureSpan comma;
            comma.role = IR::SpanRole::Text;
            comma.text = ", "_L1;
            declSpan.children.append(comma);
        }

        switch (param.kind) {
        case RelaxedTemplateParameter::Kind::TypeTemplateParameter:
        case RelaxedTemplateParameter::Kind::TemplateTemplateParameter: {
            IR::SignatureSpan kw;
            kw.role = IR::SpanRole::Text;
            kw.text = "typename"_L1;
            declSpan.children.append(kw);
            break;
        }
        case RelaxedTemplateParameter::Kind::NonTypeTemplateParameter: {
            if (!param.valued_declaration.type.empty()) {
                auto typeSpans = buildTypeSpans(QString::fromStdString(param.valued_declaration.type));
                declSpan.children.append(typeSpans);
            }
            break;
        }
        }

        if (param.is_parameter_pack) {
            IR::SignatureSpan dots;
            dots.role = IR::SpanRole::Text;
            dots.text = "..."_L1;
            declSpan.children.append(dots);
        }

        if (!param.valued_declaration.name.empty()) {
            IR::SignatureSpan space;
            space.role = IR::SpanRole::Text;
            space.text = " "_L1;
            declSpan.children.append(space);

            IR::SignatureSpan nameSpan;
            nameSpan.role = IR::SpanRole::Parameter;
            nameSpan.text = QString::fromStdString(param.valued_declaration.name);
            declSpan.children.append(nameSpan);
        }

        if (!param.valued_declaration.initializer.empty()) {
            IR::SignatureSpan eq;
            eq.role = IR::SpanRole::Text;
            eq.text = " = "_L1;
            declSpan.children.append(eq);

            if (param.kind == RelaxedTemplateParameter::Kind::TypeTemplateParameter
                || param.kind == RelaxedTemplateParameter::Kind::TemplateTemplateParameter) {
                auto typeSpans = buildTypeSpans(QString::fromStdString(param.valued_declaration.initializer));
                declSpan.children.append(typeSpans);
            } else {
                IR::SignatureSpan val;
                val.role = IR::SpanRole::Text;
                val.text = QString::fromStdString(param.valued_declaration.initializer);
                declSpan.children.append(val);
            }
        }

        first = false;
    }

    IR::SignatureSpan close;
    close.role = IR::SpanRole::Text;
    close.text = ">"_L1;
    declSpan.children.append(close);

    if (templateDecl->requires_clause && !templateDecl->requires_clause->empty()) {
        IR::SignatureSpan req;
        req.role = IR::SpanRole::Text;
        req.text = " requires "_L1 + QString::fromStdString(*templateDecl->requires_clause);
        declSpan.children.append(req);
    }

    return { declSpan };
}

static QList<IR::SignatureSpan> buildCppSynopsisSpans(const Node *node,
                                                       const HrefResolver *hrefResolver,
                                                       const Node *relative,
                                                       Section::Style style)
{
    Q_UNUSED(hrefResolver);
    QList<IR::SignatureSpan> spans;

    auto appendText = [&spans](const QString &text) {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Text;
        span.text = text;
        spans.append(span);
    };

    auto appendName = [&spans, node, hrefResolver, relative](const QString &name) {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Name;
        span.text = name;
        span.href = resolveHref(hrefResolver, node, relative);
        spans.append(span);
    };

    auto appendTypeSpans = [&spans](const QString &type, bool trailingSpace) {
        auto typeSpans = buildTypeSpans(type);
        spans.append(typeSpans);
        if (trailingSpace && !type.isEmpty()
            && !type.endsWith('*'_L1) && !type.endsWith('&'_L1)) {
            IR::SignatureSpan space;
            space.role = IR::SpanRole::Text;
            space.text = " "_L1;
            spans.append(space);
        }
    };

    // Extra qualifiers go first (prepended in CppCodeMarker)
    if (style != Section::AllMembers) {
        auto extras = buildExtraSpans(node, style);
        if (!extras.isEmpty()) {
            spans.append(extras);
            appendText(" "_L1);
        }
    }

    // Name with parent prefix for Details style
    QString nameText = node->name();
    bool linkName = (style != Section::Details);

    if (style == Section::Details) {
        if (!node->isRelatedNonmember() && !node->isProxyNode()
            && !node->parent()->name().isEmpty()
            && !node->parent()->isHeader() && !node->isProperty() && !node->isQmlNode()) {
            nameText = node->parent()->name() + "::"_L1 + nameText;
        }
    }

    switch (node->nodeType()) {
    case NodeType::Namespace:
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        appendText(Node::nodeTypeString(node->nodeType()) + " "_L1);
        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }
        break;
    case NodeType::Function: {
        const auto *func = static_cast<const FunctionNode *>(node);

        if (style == Section::Details) {
            if (auto templateDecl = node->templateDecl()) {
                auto tmplSpans = buildTemplateDeclSpans(&*templateDecl);
                spans.append(tmplSpans);
                appendText(" "_L1);
            }
        }

        if (style == Section::Summary || style == Section::Accessors) {
            if (!func->isNonvirtual())
                appendText("virtual "_L1);
        }

        if (style != Section::AllMembers && !func->returnType().isEmpty())
            appendTypeSpans(func->returnTypeString(), true);

        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }

        if (!func->isMacroWithoutParams()) {
            appendText("("_L1);
            if (!func->parameters().isEmpty()) {
                const Parameters &parameters = func->parameters();
                for (int i = 0; i < parameters.count(); ++i) {
                    if (i > 0)
                        appendText(", "_L1);
                    const Parameter &param = parameters.at(i);
                    QString pName = param.name();
                    QString type = param.type();
                    QString value = param.defaultValue();
                    qsizetype insertPos = param.nameInsertionPoint();
                    if (insertPos >= 0 && style != Section::AllMembers && !pName.isEmpty()) {
                        appendTypeSpans(type.left(insertPos), false);
                        IR::SignatureSpan paramSpan;
                        paramSpan.role = IR::SpanRole::Parameter;
                        paramSpan.text = pName;
                        spans.append(paramSpan);
                        appendTypeSpans(type.mid(insertPos), false);
                    } else {
                        bool trailingSpace = style != Section::AllMembers && !pName.isEmpty();
                        appendTypeSpans(type, trailingSpace);
                        if (style != Section::AllMembers && !pName.isEmpty()) {
                            IR::SignatureSpan paramSpan;
                            paramSpan.role = IR::SpanRole::Parameter;
                            paramSpan.text = pName;
                            spans.append(paramSpan);
                        }
                    }
                    if (style != Section::AllMembers && !value.isEmpty())
                        appendText(" = "_L1 + value);
                }
            }
            appendText(")"_L1);
        }

        if (func->isConst())
            appendText(" const"_L1);

        if (style == Section::Summary || style == Section::Accessors) {
            if (func->isFinal())
                appendText(" final"_L1);
            if (func->isOverride())
                appendText(" override"_L1);
            if (func->isPureVirtual())
                appendText(" = 0"_L1);
            if (func->isRef())
                appendText(" &"_L1);
            else if (func->isRefRef())
                appendText(" &&"_L1);
        } else if (style == Section::AllMembers) {
            if (!func->returnType().isEmpty() && func->returnType() != "void"_L1) {
                appendText(" : "_L1);
                appendTypeSpans(func->returnTypeString(), false);
            }
        } else {
            if (func->isRef())
                appendText(" &"_L1);
            else if (func->isRefRef())
                appendText(" &&"_L1);
            if (const auto &req = func->trailingRequiresClause(); req && !req->isEmpty())
                appendText(" requires "_L1 + *req);
        }
        break;
    }
    case NodeType::Enum: {
        const auto *enume = static_cast<const EnumNode *>(node);
        appendText("enum"_L1);
        if (enume->isScoped())
            appendText(" class"_L1);
        if (!enume->isAnonymous()) {
            appendText(" "_L1);
            if (linkName) {
                appendName(nameText);
            } else {
                IR::SignatureSpan span;
                span.role = IR::SpanRole::Name;
                span.text = nameText;
                spans.append(span);
            }
        }
        if (style == Section::Summary) {
            appendText(" { "_L1);
            const int MaxEnumValues = 6;
            QStringList documentedItems = enume->doc().enumItemNames();
            if (documentedItems.isEmpty()) {
                const auto &enumItems = enume->items();
                for (const auto &item : enumItems)
                    documentedItems << item.name();
            }
            const QStringList omitItems = enume->doc().omitEnumItemNames();
            for (const auto &item : omitItems)
                documentedItems.removeAll(item);

            if (documentedItems.size() > MaxEnumValues) {
                const QString last = documentedItems.last();
                documentedItems = documentedItems.mid(0, MaxEnumValues - 1);
                documentedItems += "..."_L1;
                documentedItems += last;
            }
            appendText(documentedItems.join(", "_L1));
            if (!documentedItems.isEmpty())
                appendText(" "_L1);
            appendText("}"_L1);
        }
        break;
    }
    case NodeType::TypeAlias: {
        if (style == Section::Details) {
            if (auto templateDecl = node->templateDecl()) {
                auto tmplSpans = buildTemplateDeclSpans(&*templateDecl);
                spans.append(tmplSpans);
                appendText(" "_L1);
            }
        }
        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }
        break;
    }
    case NodeType::Typedef: {
        if (static_cast<const TypedefNode *>(node)->associatedEnum())
            appendText("flags "_L1);
        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }
        break;
    }
    case NodeType::Property: {
        const auto *property = static_cast<const PropertyNode *>(node);
        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }
        appendText(" : "_L1);
        appendTypeSpans(property->qualifiedDataType(), false);
        break;
    }
    case NodeType::QmlProperty: {
        const auto *property = static_cast<const QmlPropertyNode *>(node);
        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }
        appendText(" : "_L1);
        appendTypeSpans(property->dataType(), false);
        break;
    }
    case NodeType::Variable: {
        const auto *variable = static_cast<const VariableNode *>(node);
        if (style == Section::AllMembers) {
            if (linkName) {
                appendName(nameText);
            } else {
                IR::SignatureSpan span;
                span.role = IR::SpanRole::Name;
                span.text = nameText;
                spans.append(span);
            }
            appendText(" : "_L1);
            appendTypeSpans(variable->dataType(), false);
        } else {
            appendTypeSpans(variable->leftType(), true);
            if (linkName) {
                appendName(nameText);
            } else {
                IR::SignatureSpan span;
                span.role = IR::SpanRole::Name;
                span.text = nameText;
                spans.append(span);
            }
            appendText(variable->rightType());
        }
        break;
    }
    default:
        if (linkName) {
            appendName(nameText);
        } else {
            IR::SignatureSpan span;
            span.role = IR::SpanRole::Name;
            span.text = nameText;
            spans.append(span);
        }
        break;
    }

    return spans;
}

static QList<IR::SignatureSpan> buildQmlItemSpans(const Node *node,
                                                   const HrefResolver *hrefResolver)
{
    QList<IR::SignatureSpan> spans;

    auto appendText = [&spans](const QString &text) {
        IR::SignatureSpan span;
        span.role = IR::SpanRole::Text;
        span.text = text;
        spans.append(span);
    };

    auto appendTypeSpans = [&spans](const QString &type, bool trailingSpace) {
        auto typeSpans = buildTypeSpans(type);
        spans.append(typeSpans);
        if (trailingSpace && !type.isEmpty()
            && !type.endsWith('*'_L1) && !type.endsWith('&'_L1)) {
            IR::SignatureSpan space;
            space.role = IR::SpanRole::Text;
            space.text = " "_L1;
            spans.append(space);
        }
    };

    IR::SignatureSpan nameSpan;
    nameSpan.role = IR::SpanRole::Name;
    nameSpan.text = node->name();
    nameSpan.href = resolveHref(hrefResolver, node, node->parent());

    if (node->isQmlProperty()) {
        const auto *pn = static_cast<const QmlPropertyNode *>(node);
        spans.append(nameSpan);
        appendText(" : "_L1);
        appendTypeSpans(pn->dataType(), false);
    } else if (node->isFunction(Genus::QML)) {
        const auto *func = static_cast<const FunctionNode *>(node);
        if (!func->returnType().isEmpty())
            appendTypeSpans(func->returnTypeString(), true);
        spans.append(nameSpan);
        appendText("("_L1);
        if (!func->parameters().isEmpty()) {
            const Parameters &parameters = func->parameters();
            for (int i = 0; i < parameters.count(); ++i) {
                if (i > 0)
                    appendText(", "_L1);
                QString pName = parameters.at(i).name();
                QString type = parameters.at(i).type();
                if (!pName.isEmpty()) {
                    appendTypeSpans(type, true);
                    IR::SignatureSpan paramSpan;
                    paramSpan.role = IR::SpanRole::Parameter;
                    paramSpan.text = pName;
                    spans.append(paramSpan);
                } else {
                    IR::SignatureSpan paramSpan;
                    paramSpan.role = IR::SpanRole::Parameter;
                    paramSpan.text = type;
                    spans.append(paramSpan);
                }
            }
        }
        appendText(")"_L1);
    } else {
        spans.append(nameSpan);
    }

    auto extras = buildExtraSpans(node, Section::Summary);
    if (!extras.isEmpty()) {
        appendText(" "_L1);
        spans.append(extras);
    }

    return spans;
}

static QString plainTextFromSpans(const QList<IR::SignatureSpan> &spans)
{
    QString result;
    for (const auto &span : spans)
        result += span.plainText();
    return result;
}

/*!
    \internal
    Build structured signature spans from Node data.

    This function produces a QList of SignatureSpan values that carry
    semantic roles (Type, Name, Parameter, Extra, and so on) for each
    element of a member's synopsis. It parallels what CppCodeMarker's
    markedUpSynopsis() and markedUpQmlItem() produce as tagged strings,
    but outputs structured IR spans instead.

    The \a style parameter controls level of detail: Summary includes
    virtual/override qualifiers, Details adds template declarations and
    parent prefixes, AllMembers uses a condensed format.
*/
QList<IR::SignatureSpan> buildSignatureSpans(const Node *node,
                                             const HrefResolver *hrefResolver,
                                             const Node *relative,
                                             Section::Style style)
{
    if (node->isQmlNode() && !node->isEnumType())
        return buildQmlItemSpans(node, hrefResolver);
    return buildCppSynopsisSpans(node, hrefResolver, relative, style);
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

    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    std::function<IR::AllMemberEntry(Node *)> buildEntry = [&](Node *node) -> IR::AllMemberEntry {
        IR::AllMemberEntry entry;
        entry.signatureSpans = buildQmlItemSpans(node, hrefResolver);
        entry.signature = plainTextFromSpans(entry.signatureSpans);
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

    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    for (const auto *node : allMembers.members()) {
        if (node->name().isEmpty())
            continue;
        const NodeContext context = node->createContext();
        if (!InclusionFilter::isIncluded(policy, context))
            continue;

        IR::AllMemberEntry entry;
        entry.signatureSpans = buildSignatureSpans(node, hrefResolver, aggregate, Section::AllMembers);
        entry.signature = plainTextFromSpans(entry.signatureSpans);
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

/*!
    \internal
    Extract navigation metadata from a PageNode.

    Reads navigation configuration values (homepage, landingpage,
    cppclassespage, qmltypespage) and the node's position in the
    documentation tree to produce a breadcrumb chain, sequential
    links (previous/next/start), and the configured TOC depth.

    The breadcrumb chain follows page-type-specific logic ported
    from HtmlGenerator::generateNavigationBar(): static chain
    entries for API reference pages (class, QML type), a
    navigationParent() walk for generic pages with a 16-item
    circular reference cutoff, and a fallback to the first group
    page when no navigation parent exists.

    Sequential links come from the node's pre-populated link map,
    set by QDocDatabase::updateNavigation() before generation runs.
*/
IR::NavigationData extractNavigationData(const PageNode *pn, const HrefResolver *hrefResolver)
{
    IR::NavigationData nav;
    const Config &config = Config::instance();
    QDocDatabase *qdb = QDocDatabase::qdocDB();

    const QString navDot = CONFIG_NAVIGATION + Config::dot;
    const QString homepage = config.get(navDot + CONFIG_HOMEPAGE).asString();
    const QString hometitle = config.get(navDot + CONFIG_HOMETITLE).asString(homepage);
    const QString landingpage = config.get(navDot + CONFIG_LANDINGPAGE).asString();
    const QString landingtitle = config.get(navDot + CONFIG_LANDINGTITLE).asString(landingpage);
    const QString cppclassespage = config.get(navDot + CONFIG_CPPCLASSESPAGE).asString();
    const QString cppclassestitle = config.get(navDot + CONFIG_CPPCLASSESTITLE).asString("C++ Classes"_L1);
    const QString qmltypespage = config.get(navDot + CONFIG_QMLTYPESPAGE).asString();
    const QString qmltypestitle = config.get(navDot + CONFIG_QMLTYPESTITLE).asString("QML Types"_L1);

    const QString pageTitle = pn->title();
    using CrumbState = IR::NavigationData::CrumbState;

    auto resolveCrumb = [&](const QString &targetName)
            -> std::pair<QString, CrumbState> {
        const Node *target = qdb->findNodeForTarget(targetName, pn);
        if (!target)
            return {{}, CrumbState::Unresolved};
        if (target == pn)
            return {{}, CrumbState::Current};
        return {resolveHref(hrefResolver, target, pn), CrumbState::Link};
    };

    if (!homepage.isEmpty()) {
        auto [href, state] = resolveCrumb(homepage);
        if (state == CrumbState::Current)
            return nav;
        nav.breadcrumbs.append({hometitle, std::move(href), state});
    }

    if (!landingpage.isEmpty()) {
        auto [href, state] = resolveCrumb(landingpage);
        if (state != CrumbState::Current)
            nav.breadcrumbs.append({landingtitle, std::move(href), state});
    }

    if (pn->isClassNode()) {
        if (!cppclassespage.isEmpty() && !cppclassestitle.isEmpty()) {
            auto [href, state] = resolveCrumb(cppclassespage);
            nav.breadcrumbs.append({cppclassestitle, std::move(href), state});
        }

        const auto *moduleNode = qdb->getModuleNode(pn);
        QString moduleState;
        if (moduleNode && !moduleNode->state().isEmpty())
            moduleState = QStringLiteral(" (%1)").arg(moduleNode->state());

        if (!pn->physicalModuleName().isEmpty() && moduleNode
            && (!moduleState.isEmpty() || moduleNode->title() != cppclassespage)) {
            nav.breadcrumbs.append({moduleNode->name() + moduleState,
                                    resolveHref(hrefResolver, moduleNode, pn),
                                    CrumbState::Link});
        }
        nav.breadcrumbs.append({pn->name(), {}, CrumbState::Current});
    } else if (pn->isQmlType()) {
        if (!qmltypespage.isEmpty() && !qmltypestitle.isEmpty()) {
            auto [href, state] = resolveCrumb(qmltypespage);
            nav.breadcrumbs.append({qmltypestitle, std::move(href), state});
        }

        const auto *moduleNode = qdb->getModuleNode(pn);
        QString moduleState;
        if (moduleNode && !moduleNode->state().isEmpty())
            moduleState = QStringLiteral(" (%1)").arg(moduleNode->state());

        if (moduleNode
            && (!moduleState.isEmpty() || moduleNode->title() != qmltypespage)) {
            nav.breadcrumbs.append({moduleNode->name() + moduleState,
                                    resolveHref(hrefResolver, moduleNode, pn),
                                    CrumbState::Link});
        }
        nav.breadcrumbs.append({pn->name(), {}, CrumbState::Current});
    } else {
        auto currentNode = pn;
        std::deque<const Node *> navNodes;
        qsizetype navItems = 0;
        while (currentNode->navigationParent() && ++navItems < 16) {
            if (std::find(navNodes.cbegin(), navNodes.cend(),
                          currentNode->navigationParent()) == navNodes.cend())
                navNodes.push_front(currentNode->navigationParent());
            currentNode = currentNode->navigationParent();
        }
        if (navNodes.empty()) {
            const QStringList groups = pn->groupNames();
            for (const auto &groupName : groups) {
                const auto *groupNode = qdb->findNodeByNameAndType(
                        QStringList{groupName}, &Node::isGroup);
                if (groupNode && !groupNode->title().isEmpty()) {
                    navNodes.push_front(groupNode);
                    break;
                }
            }
        }
        for (const auto *navNode : navNodes) {
            if (navNode->isPageNode())
                nav.breadcrumbs.append({navNode->title(),
                                        resolveHref(hrefResolver, navNode, pn),
                                        CrumbState::Link});
        }
        if (!nav.breadcrumbs.isEmpty())
            nav.breadcrumbs.append({pageTitle, {}, CrumbState::Current});
    }

    const auto &linkMap = pn->links();
    if (linkMap.contains(Node::PreviousLink)) {
        const auto &linkPair = linkMap[Node::PreviousLink];
        const Node *target = qdb->findNodeForTarget(linkPair.first, pn);
        QString href;
        QString title;
        if (target && target != pn) {
            href = resolveHref(hrefResolver, target, pn);
            title = (linkPair.first == linkPair.second && !target->title().isEmpty())
                    ? target->title() : linkPair.second;
        } else {
            href = linkPair.first;
            title = linkPair.second;
        }
        nav.previousLink = IR::NavigationData::LinkEntry{title, href};
    }
    if (linkMap.contains(Node::NextLink)) {
        const auto &linkPair = linkMap[Node::NextLink];
        const Node *target = qdb->findNodeForTarget(linkPair.first, pn);
        QString href;
        QString title;
        if (target && target != pn) {
            href = resolveHref(hrefResolver, target, pn);
            title = (linkPair.first == linkPair.second && !target->title().isEmpty())
                    ? target->title() : linkPair.second;
        } else {
            href = linkPair.first;
            title = linkPair.second;
        }
        nav.nextLink = IR::NavigationData::LinkEntry{title, href};
    }
    if (linkMap.contains(Node::StartLink)) {
        const auto &linkPair = linkMap[Node::StartLink];
        const Node *target = qdb->findNodeForTarget(linkPair.first, pn);
        QString href;
        QString title;
        if (target && target != pn) {
            href = resolveHref(hrefResolver, target, pn);
            title = (linkPair.first == linkPair.second && !target->title().isEmpty())
                    ? target->title() : linkPair.second;
        } else {
            href = linkPair.first;
            title = linkPair.second;
        }
        nav.startLink = IR::NavigationData::LinkEntry{title, href};
    }

    const QString formatDot = "HTML"_L1 + Config::dot;
    nav.tocDepth = config.get(formatDot + "tocdepth"_L1).asInt();

    return nav;
}

} // namespace NodeExtractor

QT_END_NAMESPACE
