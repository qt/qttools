// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "sections.h"

#include "aggregate.h"
#include "classnode.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "generator.h"
#include "genustypes.h"
#include "utilities.h"
#include "namespacenode.h"
#include "qmlpropertynode.h"
#include "qmltypenode.h"
#include "sharedcommentnode.h"
#include "typedefnode.h"
#include "variablenode.h"

#include <QtCore/qobjectdefs.h>

QT_BEGIN_NAMESPACE

QList<Section> Sections::s_stdSummarySections {
    { "Namespaces",       "namespace",       "namespaces",       "", Section::Summary },
    { "Classes",          "class",           "classes",          "", Section::Summary },
    { "Types",            "type",            "types",            "", Section::Summary },
    { "Variables",        "variable",        "variables",        "", Section::Summary },
    { "Static Variables", "static variable", "static variables", "", Section::Summary },
    { "Functions",        "function",        "functions",        "", Section::Summary },
    { "Macros",           "macro",           "macros",           "", Section::Summary },
};

QList<Section> Sections::s_stdDetailsSections {
    { "Namespaces",             "namespace",       "namespaces",       "nmspace", Section::Details },
    { "Classes",                "class",           "classes",          "classes", Section::Details },
    { "Type Documentation",     "type",            "types",            "types",   Section::Details },
    { "Variable Documentation", "variable",        "variables",        "vars",    Section::Details },
    { "Static Variables",       "static variable", "static variables", QString(), Section::Details },
    { "Function Documentation", "function",        "functions",        "func",    Section::Details },
    { "Macro Documentation",    "macro",           "macros",           "macros",  Section::Details },
};

QList<Section> Sections::s_stdCppClassSummarySections {
    { "Public Types",             "public type",             "public types",             "", Section::Summary },
    { "Properties",               "property",                "properties",               "", Section::Summary },
    { "Public Functions",         "public function",         "public functions",         "", Section::Summary },
    { "Public Slots",             "public slot",             "public slots",             "", Section::Summary },
    { "Signals",                  "signal",                  "signals",                  "", Section::Summary },
    { "Public Variables",         "public variable",         "public variables",         "", Section::Summary },
    { "Static Public Members",    "static public member",    "static public members",    "", Section::Summary },
    { "Protected Types",          "protected type",          "protected types",          "", Section::Summary },
    { "Protected Functions",      "protected function",      "protected functions",      "", Section::Summary },
    { "Protected Slots",          "protected slot",          "protected slots",          "", Section::Summary },
    { "Protected Variables",      "protected type",          "protected variables",      "", Section::Summary },
    { "Static Protected Members", "static protected member", "static protected members", "", Section::Summary },
    { "Private Types",            "private type",            "private types",            "", Section::Summary },
    { "Private Functions",        "private function",        "private functions",        "", Section::Summary },
    { "Private Slots",            "private slot",            "private slots",            "", Section::Summary },
    { "Static Private Members",   "static private member",   "static private members",   "", Section::Summary },
    { "Related Non-Members",      "related non-member",      "related non-members",      "", Section::Summary },
    { "Macros",                   "macro",                   "macros",                   "", Section::Summary },
};

QList<Section> Sections::s_stdCppClassDetailsSections {
    { "Member Type Documentation",     "member", "members", "types",     Section::Details },
    { "Property Documentation",        "member", "members", "prop",      Section::Details },
    { "Member Function Documentation", "member", "members", "func",      Section::Details },
    { "Member Variable Documentation", "member", "members", "vars",      Section::Details },
    { "Related Non-Members",           "member", "members", "relnonmem", Section::Details },
    { "Macro Documentation",           "member", "members", "macros",    Section::Details },
};

QList<Section> Sections::s_stdQmlTypeSummarySections {
    { "Enumerations",        "enumeration",       "enumerations",        "", Section::Summary },
    { "Properties",          "property",          "properties",          "", Section::Summary },
    { "Attached Properties", "attached property", "attached properties", "", Section::Summary },
    { "Signals",             "signal",            "signals",             "", Section::Summary },
    { "Signal Handlers",     "signal handler",    "signal handlers",     "", Section::Summary },
    { "Attached Signals",    "attached signal",   "attached signals",    "", Section::Summary },
    { "Methods",             "method",            "methods",             "", Section::Summary },
    { "Attached Methods",    "attached method",   "attached methods",    "", Section::Summary },
};

QList<Section> Sections::s_stdQmlTypeDetailsSections {
    { "Enumeration Documentation",       "member",         "members",         "qmlenum",    Section::Details },
    { "Property Documentation",          "member",         "members",         "qmlprop",    Section::Details },
    { "Attached Property Documentation", "member",         "members",         "qmlattprop", Section::Details },
    { "Signal Documentation",            "signal",         "signals",         "qmlsig",     Section::Details },
    { "Signal Handler Documentation",    "signal handler", "signal handlers", "qmlsighan",  Section::Details },
    { "Attached Signal Documentation",   "signal",         "signals",         "qmlattsig",  Section::Details },
    { "Method Documentation",            "member",         "members",         "qmlmeth",    Section::Details },
    { "Attached Method Documentation",   "member",         "members",         "qmlattmeth", Section::Details },
};

QList<Section> Sections::s_sinceSections {
    { "New Namespaces",              "", "", "", Section::Details },
    { "New Classes",                 "", "", "", Section::Details },
    { "New Member Functions",        "", "", "", Section::Details },
    { "New Functions in Namespaces", "", "", "", Section::Details },
    { "New Global Functions",        "", "", "", Section::Details },
    { "New Macros",                  "", "", "", Section::Details },
    { "New Enum Types",              "", "", "", Section::Details },
    { "New Enum Values",             "", "", "", Section::Details },
    { "New Type Aliases",            "", "", "", Section::Details },
    { "New Properties",              "", "", "", Section::Details },
    { "New Variables",               "", "", "", Section::Details },
    { "New QML Types",               "", "", "", Section::Details },
    { "New QML Enumeration Types",   "", "", "", Section::Details },
    { "New QML Properties",          "", "", "", Section::Details },
    { "New QML Signals",             "", "", "", Section::Details },
    { "New QML Signal Handlers",     "", "", "", Section::Details },
    { "New QML Methods",             "", "", "", Section::Details },
};

QList<Section> Sections::s_allMembers{ { "", "member", "members", "", Section::AllMembers } };

/*!
  \class Section
  \brief A class for containing the elements of one documentation section
 */

/*!
  The destructor must delete the members of collections
  when the members are allocated on the heap.
 */
Section::~Section()
{
    clear();
}

/*!
  A Section is now an element in a static vector, so we
  don't have to repeatedly construct and destroy them. But
  we do need to clear them before each call to build the
  sections for a C++ or QML entity.
 */
void Section::clear()
{
    m_reimplementedMemberMap.clear();
    m_members.clear();
    m_obsoleteMembers.clear();
    m_reimplementedMembers.clear();
    m_inheritedMembers.clear();
    m_classNodesList.clear();
    m_aggregate = nullptr;
}

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

    if (node->isPrivate() || node->isInternal()) {
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
                    std::pair<Aggregate *, int> p(node->parent(), 0);
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
    if (!node->isPrivate() && !node->isRelatedNonmember()) {
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
    // TODO:TEMPORARY:INTERMEDITATE: Section uses a series of maps
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
 */

/*!
  This constructor builds the vectors of sections based on the
  type of the \a aggregate node.
 */
Sections::Sections(Aggregate *aggregate) : m_aggregate(aggregate)
{
    initAggregate(s_allMembers, m_aggregate);
    switch (m_aggregate->nodeType()) {
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union:
        initAggregate(s_stdCppClassSummarySections, m_aggregate);
        initAggregate(s_stdCppClassDetailsSections, m_aggregate);
        buildStdCppClassRefPageSections();
        break;
    case NodeType::QmlType:
    case NodeType::QmlValueType:
        initAggregate(s_stdQmlTypeSummarySections, m_aggregate);
        initAggregate(s_stdQmlTypeDetailsSections, m_aggregate);
        buildStdQmlTypeRefPageSections();
        break;
    case NodeType::Namespace:
    case NodeType::HeaderFile:
    case NodeType::Proxy:
    default:
        initAggregate(s_stdSummarySections, m_aggregate);
        initAggregate(s_stdDetailsSections, m_aggregate);
        buildStdRefPageSections();
        break;
    }
}

/*!
  This constructor builds a vector of sections from the \e since
  node map, \a nsmap
 */
Sections::Sections(const NodeMultiMap &nsmap) : m_aggregate(nullptr)
{
    if (nsmap.isEmpty())
        return;
    SectionVector &sections = sinceSections();
    for (auto it = nsmap.constBegin(); it != nsmap.constEnd(); ++it) {
        Node *node = it.value();
        switch (node->nodeType()) {
        case NodeType::QmlType:
            sections[SinceQmlTypes].appendMember(node);
            break;
        case NodeType::Namespace:
            sections[SinceNamespaces].appendMember(node);
            break;
        case NodeType::Class:
        case NodeType::Struct:
        case NodeType::Union:
            sections[SinceClasses].appendMember(node);
            break;
        case NodeType::Enum: {
            // The map can contain an enum node with \since, or an enum node
            // with \value containing a since-clause. In the latter case,
            // key() is an empty string.
            if (!it.key().isEmpty())
                sections[SinceEnumTypes].appendMember(node);
            else
                sections[SinceEnumValues].appendMember(node);
            break;
        }
        case NodeType::Typedef:
        case NodeType::TypeAlias:
            sections[SinceTypeAliases].appendMember(node);
            break;
        case NodeType::Function: {
            const auto *fn = static_cast<const FunctionNode *>(node);
            switch (fn->metaness()) {
            case FunctionNode::QmlSignal:
                sections[SinceQmlSignals].appendMember(node);
                break;
            case FunctionNode::QmlSignalHandler:
                sections[SinceQmlSignalHandlers].appendMember(node);
                break;
            case FunctionNode::QmlMethod:
                sections[SinceQmlMethods].appendMember(node);
                break;
            default:
                if (fn->isMacro())
                    sections[SinceMacros].appendMember(node);
                else {
                    Node *p = fn->parent();
                    if (p) {
                        if (p->isClassNode())
                            sections[SinceMemberFunctions].appendMember(node);
                        else if (p->isNamespace()) {
                            if (p->name().isEmpty())
                                sections[SinceGlobalFunctions].appendMember(node);
                            else
                                sections[SinceNamespaceFunctions].appendMember(node);
                        } else
                            sections[SinceGlobalFunctions].appendMember(node);
                    } else
                        sections[SinceGlobalFunctions].appendMember(node);
                }
                break;
            }
            break;
        }
        case NodeType::Property:
            sections[SinceProperties].appendMember(node);
            break;
        case NodeType::Variable:
            sections[SinceVariables].appendMember(node);
            break;
        case NodeType::QmlProperty:
            sections[SinceQmlProperties].appendMember(node);
            break;
        case NodeType::QmlEnum:
            sections[SinceQmlEnumTypes].appendMember(node);
            break;
        default:
            break;
        }
    }
}

/*!
  The behavior of the destructor depends on the type of the
  Aggregate node that was passed to the constructor. If the
  constructor was passed a multimap, the destruction is a
  bit different because there was no Aggregate node.
 */
Sections::~Sections()
{
    if (m_aggregate) {
        switch (m_aggregate->nodeType()) {
        case NodeType::Class:
        case NodeType::Struct:
        case NodeType::Union:
            clear(stdCppClassSummarySections());
            clear(stdCppClassDetailsSections());
            allMembersSection().clear();
            break;
        case NodeType::QmlType:
        case NodeType::QmlValueType:
            clear(stdQmlTypeSummarySections());
            clear(stdQmlTypeDetailsSections());
            allMembersSection().clear();
            break;
        default:
            clear(stdSummarySections());
            clear(stdDetailsSections());
            allMembersSection().clear();
            break;
        }
        m_aggregate = nullptr;
    } else {
        clear(sinceSections());
    }
}

/*!
  Initialize the Aggregate in each Section of vector \a v with \a aggregate.
 */
void Sections::initAggregate(SectionVector &v, Aggregate *aggregate)
{
    for (Section &section : v)
        section.setAggregate(aggregate);
}

/*!
  Reset each Section in vector \a v to its initialized state.
 */
void Sections::clear(QList<Section> &v)
{
    for (Section &section : v)
        section.clear();
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
            stdRefPageSwitch(stdSummarySections(), n);
            if (!n->isSharingComment())
                stdRefPageSwitch(stdDetailsSections(), n);
        }
    }
    if (!m_aggregate->relatedByProxy().isEmpty()) {
        const QList<Node *> &relatedBy = m_aggregate->relatedByProxy();
        for (const auto &node : relatedBy)
            stdRefPageSwitch(stdSummarySections(), node);
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
                stdRefPageSwitch(stdSummarySections(), child);
        }
    }
    reduce(stdSummarySections());
    reduce(stdDetailsSections());
    allMembersSection().reduce();
}

/*!
  Inserts the node \a n in one of the entries in the vector \a v
  depending on the node's type, access attribute, and a few other
  attributes if the node is a signal, slot, or function.
 */
void Sections::distributeNodeInSummaryVector(SectionVector &sv, Node *n)
{
    if (n->isSharedCommentNode())
        return;
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
            if (fn->isPublic())
                sv[Signals].insert(fn);
        } else if (fn->isPublic()) {
            if (fn->isStatic())
                sv[StaticPublicMembers].insert(fn);
            else if (!sv[PublicFunctions].insertReimplementedMember(fn))
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
            else if (!n->isPrivate())
                sv[ProtectedVariables].insert(n);
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
    else if (n->isPublic())
        sv[PublicTypes].insert(n);
    else if (n->isPrivate())
        sv[PrivateTypes].insert(n);
    else
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

static void pushBaseClasses(QStack<ClassNode *> &stack, ClassNode *cn)
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
    SectionVector &summarySections = stdCppClassSummarySections();
    SectionVector &detailsSections = stdCppClassDetailsSections();
    Section &allMembers = allMembersSection();

    for (auto it = m_aggregate->constBegin(); it != m_aggregate->constEnd(); ++it) {
        Node *n = *it;
        if (!n->isPrivate() && !n->isProperty() && !n->isRelatedNonmember()
            && !n->isSharedCommentNode())
            allMembers.insert(n);

        distributeNodeInSummaryVector(summarySections, n);
        distributeNodeInDetailsVector(detailsSections, n);
    }
    if (!m_aggregate->relatedByProxy().isEmpty()) {
        const QList<Node *> relatedBy = m_aggregate->relatedByProxy();
        for (const auto &node : relatedBy)
            distributeNodeInSummaryVector(summarySections, node);
    }

    QStack<ClassNode *> stack;
    auto *cn = static_cast<ClassNode *>(m_aggregate);
    pushBaseClasses(stack, cn);
    while (!stack.isEmpty()) {
        ClassNode *cn = stack.pop();
        for (auto it = cn->constBegin(); it != cn->constEnd(); ++it) {
            Node *n = *it;
            if (!n->isPrivate() && !n->isProperty() && !n->isRelatedNonmember()
                && !n->isSharedCommentNode())
                allMembers.insert(n);
        }
        pushBaseClasses(stack, cn);
    }
    reduce(summarySections);
    reduce(detailsSections);
    allMembers.reduce();
}

/*!
  Build the section vectors for a standard reference page,
  when the aggregate node is a QML type.
 */
void Sections::buildStdQmlTypeRefPageSections()
{
    ClassNodes *classNodes = nullptr;
    SectionVector &summarySections = stdQmlTypeSummarySections();
    SectionVector &detailsSections = stdQmlTypeDetailsSections();
    Section &allMembers = allMembersSection();

    const Aggregate *qtn = m_aggregate;
    while (qtn) {
        if (!qtn->isAbstract() || !classNodes)
            classNodes = &allMembers.classNodesList().emplace_back(static_cast<const QmlTypeNode*>(qtn), NodeVector{});
        for (const auto n : qtn->childNodes()) {
            if (n->isInternal())
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
                allMembers.insert(n);
                classNodes->second.push_back(n);
            }


            if (qtn == m_aggregate || qtn->isAbstract()) {
                distributeQmlNodeInSummaryVector(summarySections, n);
                distributeQmlNodeInDetailsVector(detailsSections, n);
            }
        }
        if (qtn->qmlBaseNode() == qtn) {
            qCDebug(lcQdoc, "error: circular type definition: '%s' inherits itself",
                    qPrintable(qtn->name()));
            break;
        }
        qtn = qtn->qmlBaseNode();
    }

    reduce(summarySections);
    reduce(detailsSections);
    allMembers.reduce();
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
    const SectionVector *sections = nullptr;
    if (m_aggregate->isClassNode())
        sections = &stdCppClassSummarySections();
    else if (m_aggregate->isQmlType())
        sections = &stdQmlTypeSummarySections();
    else
        sections = &stdSummarySections();
    for (const auto &section : *sections) {
        if (!section.obsoleteMembers().isEmpty())
            summary_spv->append(&section);
    }
    if (m_aggregate->isClassNode())
        sections = &stdCppClassDetailsSections();
    else if (m_aggregate->isQmlType())
        sections = &stdQmlTypeDetailsSections();
    else
        sections = &stdDetailsSections();
    for (const auto &it : *sections) {
        if (!it.obsoleteMembers().isEmpty())
            details_spv->append(&it);
    }
    return !summary_spv->isEmpty();
}

QT_END_NAMESPACE
