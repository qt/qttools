// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "sections.h"

#include "aggregate.h"
#include "classnode.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "genustypes.h"
#include "inclusionfilter.h"
#include "inclusionpolicy.h"
#include "namespacenode.h"
#include "node.h"
#include "qdoclogging.h"
#include "qmlpropertynode.h"
#include "qmltypenode.h"
#include "sharedcommentnode.h"
#include "typedefnode.h"
#include "variablenode.h"

#include <QtCore/qobjectdefs.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {

SectionVector makeStdSummarySections()
{
    return {
        { "Namespaces"_L1,       "namespace"_L1,       "namespaces"_L1,       ""_L1, Section::Summary },
        { "Classes"_L1,          "class"_L1,           "classes"_L1,          ""_L1, Section::Summary },
        { "Types"_L1,            "type"_L1,            "types"_L1,            ""_L1, Section::Summary },
        { "Variables"_L1,        "variable"_L1,        "variables"_L1,        ""_L1, Section::Summary },
        { "Static Variables"_L1, "static variable"_L1, "static variables"_L1, ""_L1, Section::Summary },
        { "Functions"_L1,        "function"_L1,        "functions"_L1,        ""_L1, Section::Summary },
        { "Macros"_L1,           "macro"_L1,           "macros"_L1,           ""_L1, Section::Summary },
    };
}

SectionVector makeStdDetailsSections()
{
    return {
        { "Namespaces"_L1,             "namespace"_L1,       "namespaces"_L1,       "nmspace"_L1, Section::Details },
        { "Classes"_L1,                "class"_L1,           "classes"_L1,          "classes"_L1, Section::Details },
        { "Type Documentation"_L1,     "type"_L1,            "types"_L1,            "types"_L1,   Section::Details },
        { "Variable Documentation"_L1, "variable"_L1,        "variables"_L1,        "vars"_L1,    Section::Details },
        { "Static Variables"_L1,       "static variable"_L1, "static variables"_L1, ""_L1,        Section::Details },
        { "Function Documentation"_L1, "function"_L1,        "functions"_L1,        "func"_L1,    Section::Details },
        { "Macro Documentation"_L1,    "macro"_L1,           "macros"_L1,           "macros"_L1,  Section::Details },
    };
}

SectionVector makeCppClassSummarySections()
{
    return {
        { "Public Types"_L1,             "public type"_L1,             "public types"_L1,             ""_L1, Section::Summary },
        { "Properties"_L1,               "property"_L1,                "properties"_L1,               ""_L1, Section::Summary },
        { "Public Functions"_L1,         "public function"_L1,         "public functions"_L1,         ""_L1, Section::Summary },
        { "Public Slots"_L1,             "public slot"_L1,             "public slots"_L1,             ""_L1, Section::Summary },
        { "Signals"_L1,                  "signal"_L1,                  "signals"_L1,                  ""_L1, Section::Summary },
        { "Public Variables"_L1,         "public variable"_L1,         "public variables"_L1,         ""_L1, Section::Summary },
        { "Static Public Members"_L1,    "static public member"_L1,    "static public members"_L1,    ""_L1, Section::Summary },
        { "Protected Types"_L1,          "protected type"_L1,          "protected types"_L1,          ""_L1, Section::Summary },
        { "Protected Functions"_L1,      "protected function"_L1,      "protected functions"_L1,      ""_L1, Section::Summary },
        { "Protected Slots"_L1,          "protected slot"_L1,          "protected slots"_L1,          ""_L1, Section::Summary },
        { "Protected Variables"_L1,      "protected type"_L1,          "protected variables"_L1,      ""_L1, Section::Summary },
        { "Static Protected Members"_L1, "static protected member"_L1, "static protected members"_L1, ""_L1, Section::Summary },
        { "Private Types"_L1,            "private type"_L1,            "private types"_L1,            ""_L1, Section::Summary },
        { "Private Functions"_L1,        "private function"_L1,        "private functions"_L1,        ""_L1, Section::Summary },
        { "Private Slots"_L1,            "private slot"_L1,            "private slots"_L1,            ""_L1, Section::Summary },
        { "Private Variables"_L1,        "private variable"_L1,        "private variables"_L1,        ""_L1, Section::Summary },
        { "Static Private Members"_L1,   "static private member"_L1,   "static private members"_L1,   ""_L1, Section::Summary },
        { "Related Non-Members"_L1,      "related non-member"_L1,      "related non-members"_L1,      ""_L1, Section::Summary },
        { "Macros"_L1,                   "macro"_L1,                   "macros"_L1,                   ""_L1, Section::Summary },
    };
}

SectionVector makeCppClassDetailsSections()
{
    return {
        { "Member Type Documentation"_L1,     "member"_L1, "members"_L1, "types"_L1,     Section::Details },
        { "Property Documentation"_L1,        "member"_L1, "members"_L1, "prop"_L1,      Section::Details },
        { "Member Function Documentation"_L1, "member"_L1, "members"_L1, "func"_L1,      Section::Details },
        { "Member Variable Documentation"_L1, "member"_L1, "members"_L1, "vars"_L1,      Section::Details },
        { "Related Non-Members"_L1,           "member"_L1, "members"_L1, "relnonmem"_L1, Section::Details },
        { "Macro Documentation"_L1,           "member"_L1, "members"_L1, "macros"_L1,    Section::Details },
    };
}

SectionVector makeQmlTypeSummarySections()
{
    return {
        { "Enumerations"_L1,        "enumeration"_L1,       "enumerations"_L1,        ""_L1, Section::Summary },
        { "Properties"_L1,          "property"_L1,          "properties"_L1,          ""_L1, Section::Summary },
        { "Attached Properties"_L1, "attached property"_L1, "attached properties"_L1, ""_L1, Section::Summary },
        { "Signals"_L1,             "signal"_L1,            "signals"_L1,             ""_L1, Section::Summary },
        { "Signal Handlers"_L1,     "signal handler"_L1,    "signal handlers"_L1,     ""_L1, Section::Summary },
        { "Attached Signals"_L1,    "attached signal"_L1,   "attached signals"_L1,    ""_L1, Section::Summary },
        { "Methods"_L1,             "method"_L1,            "methods"_L1,             ""_L1, Section::Summary },
        { "Attached Methods"_L1,    "attached method"_L1,   "attached methods"_L1,    ""_L1, Section::Summary },
    };
}

SectionVector makeQmlTypeDetailsSections()
{
    return {
        { "Enumeration Documentation"_L1,       "member"_L1,         "members"_L1,         "qmlenum"_L1,    Section::Details },
        { "Property Documentation"_L1,          "member"_L1,         "members"_L1,         "qmlprop"_L1,    Section::Details },
        { "Attached Property Documentation"_L1, "member"_L1,         "members"_L1,         "qmlattprop"_L1, Section::Details },
        { "Signal Documentation"_L1,            "signal"_L1,         "signals"_L1,         "qmlsig"_L1,     Section::Details },
        { "Signal Handler Documentation"_L1,    "signal handler"_L1, "signal handlers"_L1, "qmlsighan"_L1,  Section::Details },
        { "Attached Signal Documentation"_L1,   "signal"_L1,         "signals"_L1,         "qmlattsig"_L1,  Section::Details },
        { "Method Documentation"_L1,            "member"_L1,         "members"_L1,         "qmlmeth"_L1,    Section::Details },
        { "Attached Method Documentation"_L1,   "member"_L1,         "members"_L1,         "qmlattmeth"_L1, Section::Details },
    };
}

SectionVector makeSinceSections()
{
    return {
        { "New Namespaces"_L1,              ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Classes"_L1,                 ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Member Functions"_L1,        ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Functions in Namespaces"_L1, ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Global Functions"_L1,        ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Macros"_L1,                  ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Enum Types"_L1,              ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Enum Values"_L1,             ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Type Aliases"_L1,            ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Properties"_L1,              ""_L1, ""_L1, ""_L1, Section::Details },
        { "New Variables"_L1,               ""_L1, ""_L1, ""_L1, Section::Details },
        { "New QML Types"_L1,               ""_L1, ""_L1, ""_L1, Section::Details },
        { "New QML Enumeration Types"_L1,   ""_L1, ""_L1, ""_L1, Section::Details },
        { "New QML Properties"_L1,          ""_L1, ""_L1, ""_L1, Section::Details },
        { "New QML Signals"_L1,             ""_L1, ""_L1, ""_L1, Section::Details },
        { "New QML Signal Handlers"_L1,     ""_L1, ""_L1, ""_L1, Section::Details },
        { "New QML Methods"_L1,             ""_L1, ""_L1, ""_L1, Section::Details },
    };
}

} // anonymous namespace

/*!
  \class Section
  \brief A class for containing the elements of one documentation section
 */

/*!
  Construct a name for the \a node that can be used for sorting
  a set of nodes into equivalence classes.
 */
QString sortName(const Node *node)
{
    QString nodeName{node->name()};

    int numDigits = 0;
    for (qsizetype i = nodeName.size() - 1; i > 0; --i) {
        if (nodeName.at(i).digitValue() == -1)
            break;
        ++numDigits;
    }

    // we want 'qint8' to appear before 'qint16'
    if (numDigits > 0) {
        for (int i = 0; i < 4 - numDigits; ++i)
            nodeName.insert(nodeName.size() - numDigits - 1, QLatin1Char('0'));
    }

    if (node->isClassNode())
        return QLatin1Char('A') + nodeName;

    if (node->isFunction(Genus::CPP)) {
        const auto *fn = static_cast<const FunctionNode *>(node);

        QString sortNo;
        if (fn->isCtor())
            sortNo = QLatin1String("C");
        else if (fn->isCCtor())
            sortNo = QLatin1String("D");
        else if (fn->isMCtor())
            sortNo = QLatin1String("E");
        else if (fn->isDtor())
            sortNo = QLatin1String("F");
        else if (nodeName.startsWith(QLatin1String("operator")) && nodeName.size() > 8
                    && !nodeName[8].isLetterOrNumber())
            sortNo = QLatin1String("H");
        else
            sortNo = QLatin1String("G");

        return sortNo + nodeName + QLatin1Char(' ') + QString::number(fn->overloadNumber(), 36);
    }

    if (node->isFunction(Genus::QML))
        return QLatin1Char('E') + nodeName + QLatin1Char(' ') +
            QString::number(static_cast<const FunctionNode*>(node)->overloadNumber(), 36);

    if (node->isProperty() || node->isVariable())
        return QLatin1Char('G') + nodeName;

    return QLatin1Char('B') + nodeName;
}

/*!
  Inserts the \a node into this section if it is appropriate
  for this section.
 */
void Section::insert(Node *node)
{
    bool irrelevant = false;
    bool inherited = false;
    if (!node->isRelatedNonmember()) {
        Aggregate *p = node->parent();
        if (!p->isNamespace() && p != m_aggregate) {
            if (!p->isQmlType() || !p->isAbstract())
                inherited = true;
        }
    }

    const InclusionPolicy policy = Config::instance().createInclusionPolicy();
    const NodeContext context = node->createContext();

    if (!InclusionFilter::isIncluded(policy, context)) {
        irrelevant = true;
    } else if (node->isFunction()) {
        auto *func = static_cast<FunctionNode *>(node);
        irrelevant = (inherited && (func->isSomeCtor() || func->isDtor()));
    } else if (node->isClassNode() || node->isEnumType() || node->isTypedef()
               || node->isVariable()) {
        irrelevant = (inherited && m_style != AllMembers);
        if (!irrelevant && m_style == Details && node->isTypedef()) {
            const auto *tdn = static_cast<const TypedefNode *>(node);
            if (tdn->associatedEnum())
                irrelevant = true;
        }
    }

    if (!irrelevant) {
        QString key = sortName(node);
        if (node->isDeprecated()) {
            m_obsoleteMembers.push_back(node);
        } else {
            if (!inherited || m_style == AllMembers)
                m_members.push_back(node);

            if (inherited && (node->parent()->isClassNode() || node->parent()->isNamespace())) {
                if (m_inheritedMembers.isEmpty()
                    || m_inheritedMembers.last().first != node->parent()) {
                    std::pair<const Aggregate *, int> p(node->parent(), 0);
                    m_inheritedMembers.append(p);
                }
                m_inheritedMembers.last().second++;
            }
        }
    }
}

/*!
  Returns \c true if the \a node is a reimplemented member
  function of the current class. If true, the \a node is
  inserted into the reimplemented member map. True
  is returned only if \a node is inserted into the map.
  That is, false is returned if the \a node is already in
  the map.
 */
bool Section::insertReimplementedMember(Node *node)
{
    const InclusionPolicy policy = Config::instance().createInclusionPolicy();
    const NodeContext context = node->createContext();

    // Use specialized visibility check for documented reimplemented members
    if (node->isInAPI() && InclusionFilter::isReimplementedMemberVisible(policy, context) && !node->isRelatedNonmember()) {
        const auto *fn = static_cast<const FunctionNode *>(node);
        if (!fn->overridesThis().isEmpty()) {
            if (fn->parent() == m_aggregate) {
                QString key = sortName(fn);
                if (!m_reimplementedMemberMap.contains(key)) {
                    m_reimplementedMemberMap.insert(key, node);
                    return true;
                }
            }
        }
    }
    return false;
}

/*!
  If this section is not empty, convert its maps to sequential
  structures for better traversal during doc generation.
 */
void Section::reduce()
{
    // TODO:TEMPORARY:INTERMEDIATE: Section uses a series of maps
    // to internally manage the categorization of the various members
    // of an aggregate. It further uses a secondary "flattened"
    // (usually vector) version that is later used by consumers of a
    // Section content.
    //
    // One of the uses of those maps is that of ordering, by using
    // keys generated with `sortName`.
    // Nonetheless, this is the only usage that comes from the keys,
    // as they are neither necessary nor used outside of the internal
    // code for Section.
    //
    // Hence, the codebase is moving towards removing the maps in
    // favor of building a flattened, consumer ready, version of the
    // categorization directly, cutting the intermediate conversion
    // step.
    //
    // To do so while keeping as much of the old behavior as possible,
    // we provide a sorting for the flattened version that is based on
    // `sortName`, as the previous ordering was.
    //
    // This acts as a relatively heavy pessimization, as `sortName`,
    // used as a comparator, can be called multiple times for each
    // Node, while before it would have been called almost-once.
    //
    // Instead of fixing this issue, by for example caching the
    // sortName of each Node instance, we temporarily keep the
    // pessimization while the various maps are removed.
    //
    // When all the maps are removed, we can remove `sortName`, which
    // produces strings to use as key requiring a few allocations and
    // expensive operations, with an actual comparator function, which
    // should be more lightweight and more than offset the
    // multiple-calls.
    static auto node_less_than = [](const Node* left, const Node* right) {
        // For shared comment nodes, compare the names of the first child
        // nodes instead of the names of the nodes themselves, which are
        // usually empty.
        if (left->isSharedCommentNode())
            left = static_cast<const SharedCommentNode *>(left)->collective().first();
        if (right->isSharedCommentNode())
            right = static_cast<const SharedCommentNode *>(right)->collective().first();
        return sortName(left) < sortName(right);
    };

    std::stable_sort(m_members.begin(), m_members.end(), node_less_than);
    std::stable_sort(m_obsoleteMembers.begin(), m_obsoleteMembers.end(), node_less_than);

    m_reimplementedMembers = m_reimplementedMemberMap.values().toVector();

    for (auto &cn : m_classNodesList) {
        std::stable_sort(cn.second.begin(), cn.second.end(), node_less_than);
    }
}

/*!
  \class Sections
  \brief A class for creating vectors of collections for documentation

  Each element in a vector is an instance of Section, which
  contains all the elements that will be documented in one
  section of a reference documentation page.

  The constructor determines the appropriate section layout based on
  the aggregate's node type (C++ class, QML type, or generic
  namespace/header). Callers access the result through the unified
  summarySections() and detailsSections() accessors without needing
  to know which variant was selected.
 */

/*!
  This constructor builds the section vectors based on the
  type of the \a aggregate node.
 */
Sections::Sections(const Aggregate *aggregate) : m_aggregate(aggregate)
{
    m_allMembers.setAggregate(m_aggregate);
    switch (m_aggregate->nodeType()) {
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        m_summarySections = makeCppClassSummarySections();
        m_detailsSections = makeCppClassDetailsSections();
        initAggregate(m_summarySections, m_aggregate);
        initAggregate(m_detailsSections, m_aggregate);
        buildStdCppClassRefPageSections();
        break;
    case NodeType::QmlType:
    case NodeType::QmlValueType:
        m_summarySections = makeQmlTypeSummarySections();
        m_detailsSections = makeQmlTypeDetailsSections();
        initAggregate(m_summarySections, m_aggregate);
        initAggregate(m_detailsSections, m_aggregate);
        buildStdQmlTypeRefPageSections();
        break;
    case NodeType::Namespace:
    case NodeType::HeaderFile:
    case NodeType::Proxy:
    default:
        m_summarySections = makeStdSummarySections();
        m_detailsSections = makeStdDetailsSections();
        initAggregate(m_summarySections, m_aggregate);
        initAggregate(m_detailsSections, m_aggregate);
        buildStdRefPageSections();
        break;
    }
}

/*!
  This constructor builds the since sections from the \e since
  node map, \a nsmap
 */
Sections::Sections(const NodeMultiMap &nsmap)
    : m_aggregate(nullptr), m_sinceSections(makeSinceSections())
{
    if (nsmap.isEmpty())
        return;
    for (auto it = nsmap.constBegin(); it != nsmap.constEnd(); ++it) {
        Node *node = it.value();
        switch (node->nodeType()) {
        case NodeType::QmlType:
            m_sinceSections[SinceQmlTypes].appendMember(node);
            break;
        case NodeType::Namespace:
            m_sinceSections[SinceNamespaces].appendMember(node);
            break;
        case NodeType::Class:
        case NodeType::Struct:
        case NodeType::Union:
            m_sinceSections[SinceClasses].appendMember(node);
            break;
        case NodeType::Enum: {
            // The map can contain an enum node with \since, or an enum node
            // with \value containing a since-clause. In the latter case,
            // key() is an empty string.
            if (!it.key().isEmpty())
                m_sinceSections[SinceEnumTypes].appendMember(node);
            else
                m_sinceSections[SinceEnumValues].appendMember(node);
            break;
        }
        case NodeType::Typedef:
        case NodeType::TypeAlias:
            m_sinceSections[SinceTypeAliases].appendMember(node);
            break;
        case NodeType::Function: {
            const auto *fn = static_cast<const FunctionNode *>(node);
            switch (fn->metaness()) {
            case Metaness::QmlSignal:
                m_sinceSections[SinceQmlSignals].appendMember(node);
                break;
            case Metaness::QmlSignalHandler:
                m_sinceSections[SinceQmlSignalHandlers].appendMember(node);
                break;
            case Metaness::QmlMethod:
                m_sinceSections[SinceQmlMethods].appendMember(node);
                break;
            default:
                if (fn->isMacro())
                    m_sinceSections[SinceMacros].appendMember(node);
                else {
                    Node *p = fn->parent();
                    if (p) {
                        if (p->isClassNode())
                            m_sinceSections[SinceMemberFunctions].appendMember(node);
                        else if (p->isNamespace()) {
                            if (p->name().isEmpty())
                                m_sinceSections[SinceGlobalFunctions].appendMember(node);
                            else
                                m_sinceSections[SinceNamespaceFunctions].appendMember(node);
                        } else
                            m_sinceSections[SinceGlobalFunctions].appendMember(node);
                    } else
                        m_sinceSections[SinceGlobalFunctions].appendMember(node);
                }
                break;
            }
            break;
        }
        case NodeType::Property:
            m_sinceSections[SinceProperties].appendMember(node);
            break;
        case NodeType::SharedComment:
            if (node->isPropertyGroup())
                m_sinceSections[SinceQmlProperties].appendMember(node);
            break;
        case NodeType::Variable:
            m_sinceSections[SinceVariables].appendMember(node);
            break;
        case NodeType::QmlProperty:
            m_sinceSections[SinceQmlProperties].appendMember(node);
            break;
        case NodeType::QmlEnum:
            m_sinceSections[SinceQmlEnumTypes].appendMember(node);
            break;
        default:
            break;
        }
    }
}

/*!
  Initialize the Aggregate in each Section of vector \a v with \a aggregate.
 */
void Sections::initAggregate(SectionVector &v, const Aggregate *aggregate)
{
    for (Section &section : v)
        section.setAggregate(aggregate);
}

/*!
  Linearize the maps in each Section in \a v.
 */
void Sections::reduce(QList<Section> &v)
{
    for (Section &section : v)
        section.reduce();
}

/*!
  \internal

  Returns the node to test when distributing \a node based on
  Node::nodeType().

  It returns either \a node itself, or if \a node is a shared comment
  node, the first node in its collective.
*/
static Node *nodeToTestForDistribution(Node *node)
{
    if (node && node->isSharedCommentNode() && node->hasDoc()) {
        if (auto *scn = static_cast<SharedCommentNode *>(node); scn->collective().size())
            return scn->collective().first(); // TODO: warn about mixed node types in collective?
    }
    return node;
}

/*!
  This is a private helper function for buildStdRefPageSections().
 */
void Sections::stdRefPageSwitch(SectionVector &v, Node *n)
{
    auto *t = nodeToTestForDistribution(n);
    switch (t->nodeType()) {
    case NodeType::Namespace:
        v[StdNamespaces].insert(n);
        return;
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        v[StdClasses].insert(n);
        return;
    case NodeType::Enum:
    case NodeType::Typedef:
    case NodeType::TypeAlias:
        v[StdTypes].insert(n);
        return;
    case NodeType::Function: {
        auto *func = static_cast<FunctionNode *>(t);
        if (func->isMacro())
            v[StdMacros].insert(n);
        else
            v[StdFunctions].insert(n);
    }
        return;
    case NodeType::Variable: {
        const auto *var = static_cast<const VariableNode *>(t);
        if (!var->doc().isEmpty()) {
            if (var->isStatic())
                v[StdStaticVariables].insert(n);
            else
                v[StdVariables].insert(n);
        }
    }
        return;
    default:
        return;
    }
}

/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is not a C++ class or a QML type.

  If this is for a namespace page then if the namespace node
  itself does not have documentation, only its children that
  have documentation should be documented. In other words,
  there are cases where a namespace is declared but does not
  have documentation, but some of the elements declared in
  that namespace do have documentation.

  This special processing of namespaces that do not have a
  documentation comment is meant to allow documenting its
  members that do have documentation while avoiding posting
  error messages for its members that are not documented.
 */
void Sections::buildStdRefPageSections()
{
    const NamespaceNode *ns = nullptr;
    bool documentAll = true; // document all the children
    if (m_aggregate->isNamespace()) {
        ns = static_cast<const NamespaceNode *>(m_aggregate);
        if (!ns->hasDoc())
            documentAll = false; // only document children that have documentation
    }
    for (auto it = m_aggregate->constBegin(); it != m_aggregate->constEnd(); ++it) {
        Node *n = *it;
        if (documentAll || n->hasDoc()) {
            stdRefPageSwitch(m_summarySections, n);
            if (!n->isSharingComment())
                stdRefPageSwitch(m_detailsSections, n);
        }
    }
    if (!m_aggregate->relatedByProxy().isEmpty()) {
        const QList<Node *> &relatedBy = m_aggregate->relatedByProxy();
        for (const auto &node : relatedBy)
            stdRefPageSwitch(m_summarySections, node);
    }
    /*
      If we are building the sections for the reference page
      for a namespace node, include all the namespace node's
      included children in the sections.
     */
    if (ns && !ns->includedChildren().isEmpty()) {
        const QList<Node *> &children = ns->includedChildren();
        for (const auto &child : children) {
            if (documentAll || child->hasDoc())
                stdRefPageSwitch(m_summarySections, child);
        }
    }
    reduce(m_summarySections);
    reduce(m_detailsSections);
    m_allMembers.reduce();
}

/*!
  Inserts the node \a n in one of the entries in the vector \a v
  depending on the node's type, access attribute, and a few other
  attributes if the node is a signal, slot, or function.
 */
void Sections::distributeNodeInSummaryVector(SectionVector &sv, Node *n)
{
    if (n->isSharedCommentNode()) {
        static_cast<SharedCommentNode *>(n)->sort();
        return;
    }
    if (n->isFunction()) {
        auto *fn = static_cast<FunctionNode *>(n);
        if (fn->isRelatedNonmember()) {
            if (fn->isMacro())
                sv[Macros].insert(n);
            else
                sv[RelatedNonmembers].insert(n);
            return;
        }
        if (fn->isIgnored())
            return;
        if (fn->isSlot()) {
            if (fn->isPublic())
                sv[PublicSlots].insert(fn);
            else if (fn->isPrivate())
                sv[PrivateSlots].insert(fn);
            else
                sv[ProtectedSlots].insert(fn);
        } else if (fn->isSignal()) {
            if (fn->isPublic() && fn->isInAPI())
                sv[Signals].insert(fn);
        } else if (fn->isPublic()) {
            if (fn->isStatic() && fn->isInAPI())
                sv[StaticPublicMembers].insert(fn);
            else if (!sv[PublicFunctions].insertReimplementedMember(fn) && fn->isInAPI())
                sv[PublicFunctions].insert(fn);
        } else if (fn->isPrivate()) {
            if (fn->isStatic())
                sv[StaticPrivateMembers].insert(fn);
            else if (!sv[PrivateFunctions].insertReimplementedMember(fn))
                sv[PrivateFunctions].insert(fn);
        } else { // protected
            if (fn->isStatic())
                sv[StaticProtectedMembers].insert(fn);
            else if (!sv[ProtectedFunctions].insertReimplementedMember(fn))
                sv[ProtectedFunctions].insert(fn);
        }
        return;
    }
    if (n->isRelatedNonmember()) {
        sv[RelatedNonmembers].insert(n);
        return;
    }
    if (n->isVariable()) {
        if (n->isStatic()) {
            if (n->isPublic())
                sv[StaticPublicMembers].insert(n);
            else if (n->isPrivate())
                sv[StaticPrivateMembers].insert(n);
            else
                sv[StaticProtectedMembers].insert(n);
        } else {
            if (n->isPublic())
                sv[PublicVariables].insert(n);
            else if (n->isProtected())
                sv[ProtectedVariables].insert(n);
            else if (n->isPrivate())
                sv[PrivateVariables].insert(n);
        }
        return;
    }
    /*
      Getting this far means the node is either a property
      or some kind of type, like an enum or a typedef.
    */
    if (n->isTypedef() && (n->name() == QLatin1String("QtGadgetHelper")))
        return;
    if (n->isProperty())
        sv[Properties].insert(n);
    else if (n->isPublic() && n->isInAPI())
        sv[PublicTypes].insert(n);
    else if (n->isPrivate())
        sv[PrivateTypes].insert(n);
    else if (n->isProtected())
        sv[ProtectedTypes].insert(n);
}

/*!
  Inserts the node \a n in one of the entries in the vector \a v
  depending on the node's type, access attribute, and a few other
  attributes if the node is a signal, slot, or function.
 */
void Sections::distributeNodeInDetailsVector(SectionVector &dv, Node *n)
{
    if (n->isSharingComment())
        return;

    auto *t = nodeToTestForDistribution(n);
    if (t->isFunction()) {
        auto *fn = static_cast<FunctionNode *>(t);
        if (fn->isRelatedNonmember()) {
            if (fn->isMacro())
                dv[DetailsMacros].insert(n);
            else
                dv[DetailsRelatedNonmembers].insert(n);
            return;
        }
        if (fn->isIgnored())
            return;
        if (!fn->hasAssociatedProperties() || !fn->doc().isEmpty())
            dv[DetailsMemberFunctions].insert(n);
        return;
    }
    if (t->isRelatedNonmember()) {
        dv[DetailsRelatedNonmembers].insert(n);
        return;
    }
    if (t->isEnumType(Genus::CPP) || t->isTypedef()) {
        if (t->name() != QLatin1String("QtGadgetHelper"))
            dv[DetailsMemberTypes].insert(n);
        return;
    }
    if (t->isProperty())
        dv[DetailsProperties].insert(n);
    else if (t->isVariable() && !t->doc().isEmpty())
        dv[DetailsMemberVariables].insert(n);
}

void Sections::distributeQmlNodeInDetailsVector(SectionVector &dv, Node *n)
{
    if (n->isSharingComment())
        return;

    auto *t = nodeToTestForDistribution(n);
    if (t != n && n->isPropertyGroup()) {
        dv[QmlProperties].insert(n);
        return;
    }

    if (t->isQmlProperty()) {
        auto *pn = static_cast<QmlPropertyNode *>(t);
        if (pn->isAttached())
            dv[QmlAttachedProperties].insert(n);
        else
            dv[QmlProperties].insert(n);
    } else if (t->isEnumType(Genus::QML)) {
        dv[QmlEnumTypes].insert(n);
    } else if (t->isFunction()) {
        auto *fn = static_cast<FunctionNode *>(t);
        if (fn->isQmlSignal()) {
            if (fn->isAttached())
                dv[QmlAttachedSignals].insert(n);
            else
                dv[QmlSignals].insert(n);
        } else if (fn->isQmlSignalHandler()) {
            dv[QmlSignalHandlers].insert(n);
        } else if (fn->isQmlMethod()) {
            if (fn->isAttached())
                dv[QmlAttachedMethods].insert(n);
            else
                dv[QmlMethods].insert(n);
        }
    }
}

/*!
  Distributes a node \a n into the correct place in the summary section vector \a sv.
  Nodes that are sharing a comment are handled recursively - for recursion, the \a
  sharing parameter is set to \c true.
 */
void Sections::distributeQmlNodeInSummaryVector(SectionVector &sv, Node *n, bool sharing)
{
    if (n->isSharingComment() && !sharing)
        return;
    if (n->isQmlProperty()) {
        auto *pn = static_cast<QmlPropertyNode *>(n);
        if (pn->isAttached())
            sv[QmlAttachedProperties].insert(pn);
        else
            sv[QmlProperties].insert(pn);
    } else if (n->isEnumType(Genus::QML)) {
        sv[QmlEnumTypes].insert(n);
    } else if (n->isFunction()) {
        auto *fn = static_cast<FunctionNode *>(n);
        if (fn->isQmlSignal()) {
            if (fn->isAttached())
                sv[QmlAttachedSignals].insert(fn);
            else
                sv[QmlSignals].insert(fn);
        } else if (fn->isQmlSignalHandler()) {
            sv[QmlSignalHandlers].insert(fn);
        } else if (fn->isQmlMethod()) {
            if (fn->isAttached())
                sv[QmlAttachedMethods].insert(fn);
            else
                sv[QmlMethods].insert(fn);
        }
    } else if (n->isSharedCommentNode()) {
        auto *scn = static_cast<SharedCommentNode *>(n);
        if (scn->isPropertyGroup()) {
            sv[QmlProperties].insert(scn);
        } else {
            for (const auto &child : scn->collective())
                distributeQmlNodeInSummaryVector(sv, child, true);
        }
    }
}

static void pushBaseClasses(QStack<const ClassNode *> &stack, const ClassNode *cn)
{
    const QList<RelatedClass> baseClasses = cn->baseClasses();
    for (const auto &cls : baseClasses) {
        if (cls.m_node)
            stack.prepend(cls.m_node);
    }
}

/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is a C++.
 */
void Sections::buildStdCppClassRefPageSections()
{
    const InclusionPolicy policy = Config::instance().createInclusionPolicy();

    for (auto it = m_aggregate->constBegin(); it != m_aggregate->constEnd(); ++it) {
        Node *n = *it;
        const NodeContext context = n->createContext();
        if (InclusionFilter::isIncluded(policy, context) && !n->isProperty()
            && !n->isRelatedNonmember() && !n->isSharedCommentNode()) {
            m_allMembers.insert(n);
        }
        distributeNodeInSummaryVector(m_summarySections, n);
        distributeNodeInDetailsVector(m_detailsSections, n);
    }
    if (!m_aggregate->relatedByProxy().isEmpty()) {
        const QList<Node *> relatedBy = m_aggregate->relatedByProxy();
        for (const auto &node : relatedBy)
            distributeNodeInSummaryVector(m_summarySections, node);
    }

    QStack<const ClassNode *> stack;
    QSet<const ClassNode *> visited;
    auto *cn = static_cast<const ClassNode *>(m_aggregate);

    pushBaseClasses(stack, cn);
    while (!stack.isEmpty()) {
        const ClassNode *cur = stack.pop();
        if (visited.contains(cur))
            continue;
        visited.insert(cur);
        for (Node *n : cur->childNodes()) {
            const NodeContext context = n->createContext();
            if (InclusionFilter::isIncluded(policy, context) && !n->isProperty()
                && !n->isRelatedNonmember() && !n->isSharedCommentNode()) {
                m_allMembers.insert(n);
            }
        }
        pushBaseClasses(stack, cur);
    }
    reduce(m_summarySections);
    reduce(m_detailsSections);
    m_allMembers.reduce();
}

/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is a QML type.
 */
void Sections::buildStdQmlTypeRefPageSections()
{
    ClassNodes *classNodes = nullptr;

    const Aggregate *qtn = m_aggregate;
    while (qtn) {
        if (!qtn->isAbstract() || !classNodes)
            classNodes = &m_allMembers.classNodesList().emplace_back(static_cast<const QmlTypeNode*>(qtn), NodeVector{});
        const InclusionPolicy policy = Config::instance().createInclusionPolicy();
        for (const auto n : qtn->childNodes()) {
            const NodeContext context = n->createContext();
            if (!InclusionFilter::isIncluded(policy, context))
                continue;

            // Skip overridden property/function documentation from abstract base type
            if (qtn != m_aggregate && qtn->isAbstract()) {
                NodeList candidates;
                m_aggregate->findChildren(n->name(), candidates);
                if (std::any_of(candidates.cbegin(), candidates.cend(), [&n](const Node *c) {
                    if (c->nodeType() == n->nodeType()) {
                        if (!n->isFunction() ||
                                compare(static_cast<const FunctionNode *>(n),
                                        static_cast<const FunctionNode *>(c)) == 0)
                            return true;
                    }
                    return false;
                })) {
                    continue;
                }
            }

            if (!n->isSharedCommentNode() || n->isPropertyGroup()) {
                m_allMembers.insert(n);
                classNodes->second.push_back(n);
            }


            if (qtn == m_aggregate || qtn->isAbstract()) {
                distributeQmlNodeInSummaryVector(m_summarySections, n);
                distributeQmlNodeInDetailsVector(m_detailsSections, n);
            }
        }
        if (qtn->qmlBaseNode() == qtn) {
            qCDebug(lcQdoc, "error: circular type definition: '%s' inherits itself",
                    qPrintable(qtn->name()));
            break;
        }
        qtn = qtn->qmlBaseNode();
    }

    reduce(m_summarySections);
    reduce(m_detailsSections);
    m_allMembers.reduce();
}

/*!
  Returns true if any sections in this object contain obsolete
  members. If it returns false, then \a summary_spv and \a details_spv
  have not been modified. Otherwise, both vectors will contain pointers
  to the sections that contain obsolete members.
 */
bool Sections::hasObsoleteMembers(SectionPtrVector *summary_spv,
                                  SectionPtrVector *details_spv) const
{
    for (const auto &section : m_summarySections) {
        if (!section.obsoleteMembers().isEmpty())
            summary_spv->append(&section);
    }
    for (const auto &section : m_detailsSections) {
        if (!section.obsoleteMembers().isEmpty())
            details_spv->append(&section);
    }
    return !summary_spv->isEmpty();
}

QT_END_NAMESPACE
