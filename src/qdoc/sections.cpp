// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "sections.h"

#include "aggregate.h"
#include "classnode.h"
#include "config.h"
#include "enumnode.h"
#include "functionnode.h"
#include "generator.h"
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
    { "Namespaces",       "namespace",       "namespaces",       "", Section::Summary, Section::Active },
    { "Classes",          "class",           "classes",          "", Section::Summary, Section::Active },
    { "Types",            "type",            "types",            "", Section::Summary, Section::Active },
    { "Variables",        "variable",        "variables",        "", Section::Summary, Section::Active },
    { "Static Variables", "static variable", "static variables", "", Section::Summary, Section::Active },
    { "Functions",        "function",        "functions",        "", Section::Summary, Section::Active },
    { "Macros",           "macro",           "macros",           "", Section::Summary, Section::Active },
};

QList<Section> Sections::s_stdDetailsSections {
    { "Namespaces",             "namespace",       "namespaces",       "nmspace", Section::Details, Section::Active },
    { "Classes",                "class",           "classes",          "classes", Section::Details, Section::Active },
    { "Type Documentation",     "type",            "types",            "types",   Section::Details, Section::Active },
    { "Variable Documentation", "variable",        "variables",        "vars",    Section::Details, Section::Active },
    { "Static Variables",       "static variable", "static variables", QString(), Section::Details, Section::Active },
    { "Function Documentation", "function",        "functions",        "func",    Section::Details, Section::Active },
    { "Macro Documentation",    "macro",           "macros",           "macros",  Section::Details, Section::Active },
};

QList<Section> Sections::s_stdCppClassSummarySections {
    { "Public Types",             "public type",             "public types",             "", Section::Summary, Section::Active },
    { "Properties",               "property",                "properties",               "", Section::Summary, Section::Active },
    { "Public Functions",         "public function",         "public functions",         "", Section::Summary, Section::Active },
    { "Public Slots",             "public slot",             "public slots",             "", Section::Summary, Section::Active },
    { "Signals",                  "signal",                  "signals",                  "", Section::Summary, Section::Active },
    { "Public Variables",         "public variable",         "public variables",         "", Section::Summary, Section::Active },
    { "Static Public Members",    "static public member",    "static public members",    "", Section::Summary, Section::Active },
    { "Protected Types",          "protected type",          "protected types",          "", Section::Summary, Section::Active },
    { "Protected Functions",      "protected function",      "protected functions",      "", Section::Summary, Section::Active },
    { "Protected Slots",          "protected slot",          "protected slots",          "", Section::Summary, Section::Active },
    { "Protected Variables",      "protected type",          "protected variables",      "", Section::Summary, Section::Active },
    { "Static Protected Members", "static protected member", "static protected members", "", Section::Summary, Section::Active },
    { "Private Types",            "private type",            "private types",            "", Section::Summary, Section::Active },
    { "Private Functions",        "private function",        "private functions",        "", Section::Summary, Section::Active },
    { "Private Slots",            "private slot",            "private slots",            "", Section::Summary, Section::Active },
    { "Static Private Members",   "static private member",   "static private members",   "", Section::Summary, Section::Active },
    { "Related Non-Members",      "related non-member",      "related non-members",      "", Section::Summary, Section::Active },
    { "Macros",                   "macro",                   "macros",                   "", Section::Summary, Section::Active },
};

QList<Section> Sections::s_stdCppClassDetailsSections {
    { "Member Type Documentation",     "member", "members", "types",     Section::Details, Section::Active },
    { "Property Documentation",        "member", "members", "prop",      Section::Details, Section::Active },
    { "Member Function Documentation", "member", "members", "func",      Section::Details, Section::Active },
    { "Member Variable Documentation", "member", "members", "vars",      Section::Details, Section::Active },
    { "Related Non-Members",           "member", "members", "relnonmem", Section::Details, Section::Active },
    { "Macro Documentation",           "member", "members", "macros",    Section::Details, Section::Active },
};

QList<Section> Sections::s_stdQmlTypeSummarySections {
    { "Properties",          "property",          "properties",          "", Section::Summary, Section::Active },
    { "Attached Properties", "attached property", "attached properties", "", Section::Summary, Section::Active },
    { "Signals",             "signal",            "signals",             "", Section::Summary, Section::Active },
    { "Signal Handlers",     "signal handler",    "signal handlers",     "", Section::Summary, Section::Active },
    { "Attached Signals",    "attached signal",   "attached signals",    "", Section::Summary, Section::Active },
    { "Methods",             "method",            "methods",             "", Section::Summary, Section::Active },
    { "Attached Methods",    "attached method",   "attached methods",    "", Section::Summary, Section::Active },
};

QList<Section> Sections::s_stdQmlTypeDetailsSections {
    { "Property Documentation",          "member",         "members",         "qmlprop",    Section::Details, Section::Active },
    { "Attached Property Documentation", "member",         "members",         "qmlattprop", Section::Details, Section::Active },
    { "Signal Documentation",            "signal",         "signals",         "qmlsig",     Section::Details, Section::Active },
    { "Signal Handler Documentation",    "signal handler", "signal handlers", "qmlsighan",  Section::Details, Section::Active },
    { "Attached Signal Documentation",   "signal",         "signals",         "qmlattsig",  Section::Details, Section::Active },
    { "Method Documentation",            "member",         "members",         "qmlmeth",    Section::Details, Section::Active },
    { "Attached Method Documentation",   "member",         "members",         "qmlattmeth", Section::Details, Section::Active },
};

QList<Section> Sections::s_sinceSections {
    { "    New Namespaces",              "", "", "", Section::Details, Section::Active },
    { "    New Classes",                 "", "", "", Section::Details, Section::Active },
    { "    New Member Functions",        "", "", "", Section::Details, Section::Active },
    { "    New Functions in Namespaces", "", "", "", Section::Details, Section::Active },
    { "    New Global Functions",        "", "", "", Section::Details, Section::Active },
    { "    New Macros",                  "", "", "", Section::Details, Section::Active },
    { "    New Enum Types",              "", "", "", Section::Details, Section::Active },
    { "    New Type Aliases",            "", "", "", Section::Details, Section::Active },
    { "    New Properties",              "", "", "", Section::Details, Section::Active },
    { "    New Variables",               "", "", "", Section::Details, Section::Active },
    { "    New QML Types",               "", "", "", Section::Details, Section::Active },
    { "    New QML Properties",          "", "", "", Section::Details, Section::Active },
    { "    New QML Signals",             "", "", "", Section::Details, Section::Active },
    { "    New QML Signal Handlers",     "", "", "", Section::Details, Section::Active },
    { "    New QML Methods",             "", "", "", Section::Details, Section::Active },
};

QList<Section> Sections::s_allMembers{ { "", "member", "members", "", Section::AllMembers, Section::Active } };

/*!
  \class Section
  \brief A class for containing the elements of one documentation section
 */

/*!
  \fn Section::Section(Style style, Status status)

  The constructor used when the \a style and \a status must
  be provided.
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
    qDeleteAll(m_classMapList);
    qDeleteAll(m_classKeysNodesList);
    m_memberMap.clear();
    m_obsoleteMemberMap.clear();
    m_reimplementedMemberMap.clear();
    m_classMapList.clear();
    m_members.clear();
    m_obsoleteMembers.clear();
    m_reimplementedMembers.clear();
    m_inheritedMembers.clear();
    m_classKeysNodesList.clear();
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

    if (node->isFunction()) {
        const auto *fn = static_cast<const FunctionNode *>(node);
        if (fn->isCppFunction()) {
            QString sortNo;
            if (fn->isSomeCtor())
                sortNo = QLatin1String("C");
            else if (fn->isDtor())
                sortNo = QLatin1String("D");
            else if (nodeName.startsWith(QLatin1String("operator")) && nodeName.length() > 8
                     && !nodeName[8].isLetterOrNumber())
                sortNo = QLatin1String("F");
            else
                sortNo = QLatin1String("E");
            return sortNo + nodeName + QLatin1Char(' ') + QString::number(fn->overloadNumber(), 36);
        }
        if (fn->isQmlMethod() || fn->isQmlSignal() || fn->isQmlSignalHandler())
            return QLatin1Char('E') + nodeName;
    }
    if (node->isClassNode())
        return QLatin1Char('A') + nodeName;

    if (node->isProperty() || node->isVariable())
        return QLatin1Char('E') + nodeName;

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
            m_obsoleteMemberMap.insert(key, node);
        } else {
            if (!inherited || m_style == AllMembers)
                m_memberMap.insert(key, node);

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
  inserted into the reimplemented member map. The test is
  performed only when the section status is \e Active. True
  is returned only if \a node is inserted into the map.
  That is, false is returned if the \a node is already in
  the map.
 */
bool Section::insertReimplementedMember(Node *node)
{
    if (!node->isPrivate() && !node->isRelatedNonmember()) {
        const auto *fn = static_cast<const FunctionNode *>(node);
        if (!fn->overridesThis().isEmpty() && (m_status == Active)) {
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
  Allocate a new ClassMap on the heap for the \a aggregate
  node, append it to the list of class maps, and return a
  pointer to the new class map.
 */
ClassMap *Section::newClassMap(const Aggregate *aggregate)
{
    auto *classMap = new ClassMap;
    classMap->first = static_cast<const QmlTypeNode *>(aggregate);
    m_classMapList.append(classMap);
    return classMap;
}

/*!
  Add node \a n to the \a classMap and to the member map.
 */
void Section::add(ClassMap *classMap, Node *n)
{
    const QString key = sortName(n);
    m_memberMap.insert(key, n);
    classMap->second.insert(key, n);
}

/*!
  If this section is not empty, convert its maps to sequential
  structures for better traversal during doc generation.
 */
void Section::reduce()
{
    if (!isEmpty()) {
        m_members = m_memberMap.values().toVector();
        m_reimplementedMembers = m_reimplementedMemberMap.values().toVector();
        for (const auto &cm : m_classMapList) {
            auto *ckn = new ClassKeysNodes;
            ckn->first = cm->first;
            ckn->second.second = cm->second.values().toVector();
            ckn->second.first = cm->second.keys();
            m_classKeysNodesList.append(ckn);
        }
    }
    if (!m_obsoleteMemberMap.isEmpty()) {
        m_obsoleteMembers = m_obsoleteMemberMap.values().toVector();
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
    case Node::Class:
    case Node::Struct:
    case Node::Union:
        initAggregate(s_stdCppClassSummarySections, m_aggregate);
        initAggregate(s_stdCppClassDetailsSections, m_aggregate);
        buildStdCppClassRefPageSections();
        break;
    case Node::QmlType:
    case Node::QmlValueType:
        initAggregate(s_stdQmlTypeSummarySections, m_aggregate);
        initAggregate(s_stdQmlTypeDetailsSections, m_aggregate);
        buildStdQmlTypeRefPageSections();
        break;
    case Node::Namespace:
    case Node::HeaderFile:
    case Node::Proxy:
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
        case Node::QmlType:
            sections[SinceQmlTypes].appendMember(node);
            break;
        case Node::Namespace:
            sections[SinceNamespaces].appendMember(node);
            break;
        case Node::Class:
        case Node::Struct:
        case Node::Union:
            sections[SinceClasses].appendMember(node);
            break;
        case Node::Enum:
            sections[SinceEnumTypes].appendMember(node);
            break;
        case Node::Typedef:
        case Node::TypeAlias:
            sections[SinceTypeAliases].appendMember(node);
            break;
        case Node::Function: {
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
        case Node::Property:
            sections[SinceProperties].appendMember(node);
            break;
        case Node::Variable:
            sections[SinceVariables].appendMember(node);
            break;
        case Node::QmlProperty:
            sections[SinceQmlProperties].appendMember(node);
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
        case Node::Class:
        case Node::Struct:
        case Node::Union:
            clear(stdCppClassSummarySections());
            clear(stdCppClassDetailsSections());
            allMembersSection().clear();
            break;
        case Node::QmlType:
        case Node::QmlValueType:
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
  This is a private helper function for buildStdRefPageSections().
 */
void Sections::stdRefPageSwitch(SectionVector &v, Node *n, Node *t)
{
    // t is the reference node to be tested, n is the node to be distributed.
    // t differs from n only for shared comment nodes.
    if (!t)
        t = n;

    switch (t->nodeType()) {
    case Node::Namespace:
        v[StdNamespaces].insert(n);
        return;
    case Node::Class:
    case Node::Struct:
    case Node::Union:
        v[StdClasses].insert(n);
        return;
    case Node::Enum:
    case Node::Typedef:
    case Node::TypeAlias:
        v[StdTypes].insert(n);
        return;
    case Node::Function: {
        auto *func = static_cast<FunctionNode *>(t);
        if (func->isMacro())
            v[StdMacros].insert(n);
        else
            v[StdFunctions].insert(n);
    }
        return;
    case Node::Variable: {
        const auto *var = static_cast<const VariableNode *>(t);
        if (!var->doc().isEmpty()) {
            if (var->isStatic())
                v[StdStaticVariables].insert(n);
            else
                v[StdVariables].insert(n);
        }
    }
        return;
    case Node::SharedComment: {
        auto *scn = static_cast<SharedCommentNode *>(t);
        if (!scn->doc().isEmpty() && scn->collective().count())
            stdRefPageSwitch(
                    v, scn,
                    scn->collective().first()); // TODO: warn about mixed node types in collective?
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

    // t is the reference node to be tested - typically it's this node (n), but for
    // shared comment nodes we need to distribute based on the nodes in its collective.
    Node *t = n;

    if (n->isSharedCommentNode() && n->hasDoc()) {
        auto *scn = static_cast<SharedCommentNode *>(n);
        if (scn->collective().count())
            t = scn->collective().first(); // TODO: warn about mixed node types in collective?
    }

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
    if (t->isEnumType() || t->isTypedef()) {
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

    // t is the reference node to be tested - typically it's this node (n), but for
    // shared comment nodes we need to distribute based on the nodes in its collective.
    Node *t = n;

    if (n->isSharedCommentNode() && n->hasDoc()) {
        if (n->isPropertyGroup()) {
            dv[QmlProperties].insert(n);
            return;
        }
        auto *scn = static_cast<SharedCommentNode *>(n);
        if (scn->collective().count())
            t = scn->collective().first(); // TODO: warn about mixed node types in collective?
    }

    if (t->isQmlProperty()) {
        auto *pn = static_cast<QmlPropertyNode *>(t);
        if (pn->isAttached())
            dv[QmlAttachedProperties].insert(n);
        else
            dv[QmlProperties].insert(n);
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
    bool documentAll = true;
    if (m_aggregate->parent() && !m_aggregate->name().isEmpty() && !m_aggregate->hasDoc())
        documentAll = false;
    for (auto it = m_aggregate->constBegin(); it != m_aggregate->constEnd(); ++it) {
        Node *n = *it;
        if (!n->isPrivate() && !n->isProperty() && !n->isRelatedNonmember()
            && !n->isSharedCommentNode())
            allMembers.insert(n);
        if (!documentAll && !n->hasDoc())
            continue;

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
            if (!documentAll && !n->hasDoc())
                continue;
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
    ClassMap *classMap = nullptr;
    SectionVector &summarySections = stdQmlTypeSummarySections();
    SectionVector &detailsSections = stdQmlTypeDetailsSections();
    Section &allMembers = allMembersSection();

    const Aggregate *qtn = m_aggregate;
    while (qtn) {
        if (!qtn->isAbstract() || !classMap)
            classMap = allMembers.newClassMap(qtn);
        for (const auto n : qtn->childNodes()) {
            if (n->isInternal())
                continue;

            // Skip overridden property/function documentation from abstract base type
            if (qtn != m_aggregate && qtn->isAbstract()) {
                NodeList candidates;
                m_aggregate->findChildren(n->name(), candidates);
                if (std::any_of(candidates.cbegin(), candidates.cend(), [&n](const Node *c) {
                    if (c->nodeType() == n->nodeType()) {
                        if (!n->isFunction() || static_cast<const FunctionNode*>(n)->compare(c, false))
                            return true;
                    }
                    return false;
                })) {
                    continue;
                }
            }

            if (!n->isSharedCommentNode() || n->isPropertyGroup())
                allMembers.add(classMap, n);

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
        qtn = static_cast<QmlTypeNode *>(qtn->qmlBaseNode());
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
    else if (m_aggregate->isQmlType() || m_aggregate->isQmlBasicType())
        sections = &stdQmlTypeSummarySections();
    else
        sections = &stdSummarySections();
    for (const auto &section : *sections) {
        if (!section.obsoleteMembers().isEmpty())
            summary_spv->append(&section);
    }
    if (m_aggregate->isClassNode())
        sections = &stdCppClassDetailsSections();
    else if (m_aggregate->isQmlType() || m_aggregate->isQmlBasicType())
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
