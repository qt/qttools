/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "sections.h"

#include "config.h"
#include "generator.h"
#include "loggingcategory.h"

#include <QtCore/qdebug.h>
#include <QtCore/qobjectdefs.h>

#include <stdio.h>

QT_BEGIN_NAMESPACE

// Aggregate *Sections::aggregate_ = nullptr;

static bool sectionsInitialized = false;
QVector<Section> Sections::stdSummarySections_(7, Section(Section::Summary, Section::Active));
QVector<Section> Sections::stdDetailsSections_(7, Section(Section::Details, Section::Active));
QVector<Section> Sections::stdCppClassSummarySections_(18,
                                                       Section(Section::Summary, Section::Active));
QVector<Section> Sections::stdCppClassDetailsSections_(6,
                                                       Section(Section::Details, Section::Active));
QVector<Section> Sections::sinceSections_(15, Section(Section::Details, Section::Active));
QVector<Section> Sections::allMembers_(1, Section(Section::AllMembers, Section::Active));
QVector<Section> Sections::stdQmlTypeSummarySections_(7,
                                                      Section(Section::Summary, Section::Active));
QVector<Section> Sections::stdQmlTypeDetailsSections_(7,
                                                      Section(Section::Details, Section::Active));

/*!
  \class Section
  \brief A class for containing the elements of one documentation section
 */

/*!
  The constructor used when the \a style and \a status must
  be provided.
 */
Section::Section(Style style, Status status) : style_(style), status_(status), aggregate_(nullptr)
{
    // members_.reserve(100);
    // obsoleteMembers_.reserve(50);
    // reimplementedMembers_.reserve(50);
}

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
    memberMap_.clear();
    obsoleteMemberMap_.clear();
    reimplementedMemberMap_.clear();
    if (!classMapList_.isEmpty()) {
        for (int i = 0; i < classMapList_.size(); i++) {
            ClassMap *cm = classMapList_[i];
            classMapList_[i] = nullptr;
            delete cm;
        }
        classMapList_.clear();
    }
    keys_.clear();
    obsoleteKeys_.clear();
    members_.clear();
    obsoleteMembers_.clear();
    reimplementedMembers_.clear();
    inheritedMembers_.clear();
    if (!classKeysNodesList_.isEmpty()) {
        for (int i = 0; i < classKeysNodesList_.size(); i++) {
            ClassKeysNodes *ckn = classKeysNodesList_[i];
            classKeysNodesList_[i] = nullptr;
            delete ckn;
        }
        classKeysNodesList_.clear();
    }
    aggregate_ = nullptr;
}

/*!
  Construct a name for the \a node that can be used for sorting
  a set of nodes into equivalence classes. If \a name is provided,
  start with that name. Itherwise start with the name in \a node.
 */
QString Section::sortName(const Node *node, const QString *name)
{
    QString nodeName;
    if (name != nullptr)
        nodeName = *name;
    else
        nodeName = node->name();
    int numDigits = 0;
    for (int i = nodeName.size() - 1; i > 0; --i) {
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
        const FunctionNode *fn = static_cast<const FunctionNode *>(node);
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

        if (fn->isJsMethod() || fn->isJsSignal() || fn->isJsSignalHandler())
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
        if (!p->isNamespace() && p != aggregate_) {
            if ((!p->isQmlType() && !p->isJsType()) || !p->isAbstract())
                inherited = true;
        }
    }

    if (node->isPrivate() || node->isInternal()) {
        irrelevant = true;
    } else if (node->isFunction()) {
        FunctionNode *func = static_cast<FunctionNode *>(node);
        irrelevant = (inherited && (func->isSomeCtor() || func->isDtor()));
    } else if (node->isClassNode() || node->isEnumType() || node->isTypedef()
               || node->isVariable()) {
        irrelevant = (inherited && style_ != AllMembers);
        if (!irrelevant && style_ == Details && node->isTypedef()) {
            const TypedefNode *tdn = static_cast<const TypedefNode *>(node);
            if (tdn->associatedEnum())
                irrelevant = true;
        }
    }

    if (!irrelevant) {
        QString key = sortName(node);
        if (node->isObsolete()) {
            obsoleteMemberMap_.insert(key, node);
        } else {
            if (!inherited)
                memberMap_.insert(key, node);
            else if (style_ == AllMembers) {
                if (!memberMap_.contains(key))
                    memberMap_.insert(key, node);
            }
            if (inherited && (node->parent()->isClassNode() || node->parent()->isNamespace())) {
                if (inheritedMembers_.isEmpty()
                    || inheritedMembers_.last().first != node->parent()) {
                    QPair<Aggregate *, int> p(node->parent(), 0);
                    inheritedMembers_.append(p);
                }
                inheritedMembers_.last().second++;
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
        const FunctionNode *fn = static_cast<const FunctionNode *>(node);
        if (!fn->overridesThis().isEmpty() && (status_ == Active)) {
            if (fn->parent() == aggregate_) {
                QString key = sortName(fn);
                if (!reimplementedMemberMap_.contains(key)) {
                    reimplementedMemberMap_.insert(key, node);
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
    ClassMap *classMap = new ClassMap;
    classMap->first = static_cast<const QmlTypeNode *>(aggregate);
    classMapList_.append(classMap);
    return classMap;
}

/*!
  Add node \a n to the \a classMap and to the member map.
 */
void Section::add(ClassMap *classMap, Node *n)
{
    QString key = n->name();
    key = sortName(n, &key);
    memberMap_.insert(key, n);
    classMap->second.insert(key, n);
}

/*!
  If this section is not empty, convert its maps to sequential
  structures for better traversal during doc generation.
 */
void Section::reduce()
{
    if (!isEmpty()) {
        keys_ = memberMap_.keys();
        obsoleteKeys_ = obsoleteMemberMap_.keys();
        members_ = memberMap_.values().toVector();
        obsoleteMembers_ = obsoleteMemberMap_.values().toVector();
        reimplementedMembers_ = reimplementedMemberMap_.values().toVector();
        for (int i = 0; i < classMapList_.size(); i++) {
            ClassMap *cm = classMapList_[i];
            ClassKeysNodes *ckn = new ClassKeysNodes;
            ckn->first = cm->first;
            ckn->second.second = cm->second.values().toVector();
            ckn->second.first = cm->second.keys();
            classKeysNodesList_.append(ckn);
        }
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
Sections::Sections(Aggregate *aggregate) : aggregate_(aggregate)
{
    initSections();
    initAggregate(allMembers_, aggregate_);
    switch (aggregate_->nodeType()) {
    case Node::Class:
    case Node::Struct:
    case Node::Union:
        initAggregate(stdCppClassSummarySections_, aggregate_);
        initAggregate(stdCppClassDetailsSections_, aggregate_);
        buildStdCppClassRefPageSections();
        break;
    case Node::JsType:
    case Node::JsBasicType:
    case Node::QmlType:
    case Node::QmlBasicType:
        initAggregate(stdQmlTypeSummarySections_, aggregate_);
        initAggregate(stdQmlTypeDetailsSections_, aggregate_);
        buildStdQmlTypeRefPageSections();
        break;
    case Node::Namespace:
    case Node::HeaderFile:
    case Node::Proxy:
    default:
        initAggregate(stdSummarySections_, aggregate_);
        initAggregate(stdDetailsSections_, aggregate_);
        buildStdRefPageSections();
        break;
    }
}

/*!
  This constructor builds a vector of sections from the \e since
  node map, \a nsmap
 */
Sections::Sections(const NodeMultiMap &nsmap) : aggregate_(nullptr)
{
    initSections();
    if (nsmap.isEmpty())
        return;
    SectionVector &sections = sinceSections();
    for (auto it = nsmap.constBegin(); it != nsmap.constEnd(); ++it) {
        Node *node = it.value();
        switch (node->nodeType()) {
        case Node::JsType:
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
            sections[SinceTypedefs].appendMember(node);
            break;
        case Node::TypeAlias:
            sections[SinceTypeAliases].appendMember(node);
            break;
        case Node::Function: {
            const FunctionNode *fn = static_cast<const FunctionNode *>(node);
            switch (fn->metaness()) {
            case FunctionNode::JsSignal:
            case FunctionNode::QmlSignal:
                sections[SinceQmlSignals].appendMember(node);
                break;
            case FunctionNode::JsSignalHandler:
            case FunctionNode::QmlSignalHandler:
                sections[SinceQmlSignalHandlers].appendMember(node);
                break;
            case FunctionNode::JsMethod:
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
        case Node::JsProperty:
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
    if (aggregate_) {
        switch (aggregate_->nodeType()) {
        case Node::Class:
        case Node::Struct:
        case Node::Union:
            clear(stdCppClassSummarySections());
            clear(stdCppClassDetailsSections());
            allMembersSection().clear();
            break;
        case Node::JsType:
        case Node::JsBasicType:
        case Node::QmlType:
        case Node::QmlBasicType:
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
        aggregate_ = nullptr;
    } else {
        clear(sinceSections());
    }
}

/*!
  Initialize the Aggregate in each Section of vector \a v with \a aggregate.
 */
void Sections::initAggregate(SectionVector &v, Aggregate *aggregate)
{
    for (int i = 0; i < v.size(); ++i)
        v[i].setAggregate(aggregate);
}

/*!
  This function is called once to initialize all the instances
  of QVector<Section>. The vectors have already been constructed
  with the correct number of Section entries in each. Each Section
  entry has already been constructed with the correct values of
  Style and Status for the vector it is in. This function adds the
  correct text strings to each section in each vector.
 */
void Sections::initSections()
{
    if (sectionsInitialized)
        return;
    sectionsInitialized = true;

    allMembers_[0].init("member", "members");
    {
        QVector<Section> &v = stdCppClassSummarySections_;
        v[0].init("Public Types", "public type", "public types");
        v[1].init("Properties", "property", "properties");
        v[2].init("Public Functions", "public function", "public functions");
        v[3].init("Public Slots", "public slot", "public slots");
        v[4].init("Signals", "signal", "signals");
        v[5].init("Public Variables", "public variable", "public variables");
        v[6].init("Static Public Members", "static public member", "static public members");
        v[7].init("Protected Types", "protected type", "protected types");
        v[8].init("Protected Functions", "protected function", "protected functions");
        v[9].init("Protected Slots", "protected slot", "protected slots");
        v[10].init("Protected Variables", "protected type", "protected variables");
        v[11].init("Static Protected Members", "static protected member",
                   "static protected members");
        v[12].init("Private Types", "private type", "private types");
        v[13].init("Private Functions", "private function", "private functions");
        v[14].init("Private Slots", "private slot", "private slots");
        v[15].init("Static Private Members", "static private member", "static private members");
        v[16].init("Related Non-Members", "related non-member", "related non-members");
        v[17].init("Macros", "macro", "macros");
    }

    {
        QVector<Section> &v = stdCppClassDetailsSections_;
        v[0].init("Member Type Documentation", "types", "member", "members");
        v[1].init("Property Documentation", "prop", "member", "members");
        v[2].init("Member Function Documentation", "func", "member", "members");
        v[3].init("Member Variable Documentation", "vars", "member", "members");
        v[4].init("Related Non-Members", "relnonmem", "member", "members");
        v[5].init("Macro Documentation", "macros", "member", "members");
    }

    {
        QVector<Section> &v = stdSummarySections_;
        v[0].init("Namespaces", "namespace", "namespaces");
        v[1].init("Classes", "class", "classes");
        v[2].init("Types", "type", "types");
        v[3].init("Variables", "variable", "variables");
        v[4].init("Static Variables", "static variable", "static variables");
        v[5].init("Functions", "function", "functions");
        v[6].init("Macros", "macro", "macros");
    }

    {
        QVector<Section> &v = stdDetailsSections_;
        v[0].init("Namespaces", "nmspace", "namespace", "namespaces");
        v[1].init("Classes", "classes", "class", "classes");
        v[2].init("Type Documentation", "types", "type", "types");
        v[3].init("Variable Documentation", "vars", "variable", "variables");
        v[4].init("Static Variables", QString(), "static variable", "static variables");
        v[5].init("Function Documentation", "func", "function", "functions");
        v[6].init("Macro Documentation", "macros", "macro", "macros");
    }

    {
        QVector<Section> &v = sinceSections_;
        v[SinceNamespaces].init("    New Namespaces");
        v[SinceClasses].init("    New Classes");
        v[SinceMemberFunctions].init("    New Member Functions");
        v[SinceNamespaceFunctions].init("    New Functions in Namespaces");
        v[SinceGlobalFunctions].init("    New Global Functions");
        v[SinceMacros].init("    New Macros");
        v[SinceEnumTypes].init("    New Enum Types");
        v[SinceTypedefs].init("    New Typedefs");
        v[SinceTypeAliases].init("    New Type Aliases");
        v[SinceProperties].init("    New Properties");
        v[SinceVariables].init("    New Variables");
        v[SinceQmlTypes].init("    New QML Types");
        v[SinceQmlProperties].init("    New QML Properties");
        v[SinceQmlSignals].init("    New QML Signals");
        v[SinceQmlSignalHandlers].init("    New QML Signal Handlers");
        v[SinceQmlMethods].init("    New QML Methods");
    }

    {
        QVector<Section> &v = stdQmlTypeSummarySections_;
        v[0].init("Properties", "property", "properties");
        v[1].init("Attached Properties", "attached property", "attached properties");
        v[2].init("Signals", "signal", "signals");
        v[3].init("Signal Handlers", "signal handler", "signal handlers");
        v[4].init("Attached Signals", "attached signal", "attached signals");
        v[5].init("Methods", "method", "methods");
        v[6].init("Attached Methods", "attached method", "attached methods");
    }

    {
        QVector<Section> &v = stdQmlTypeDetailsSections_;
        v[0].init("Property Documentation", "qmlprop", "member", "members");
        v[1].init("Attached Property Documentation", "qmlattprop", "member", "members");
        v[2].init("Signal Documentation", "qmlsig", "signal", "signals");
        v[3].init("Signal Handler Documentation", "qmlsighan", "signal handler", "signal handlers");
        v[4].init("Attached Signal Documentation", "qmlattsig", "signal", "signals");
        v[5].init("Method Documentation", "qmlmeth", "member", "members");
        v[6].init("Attached Method Documentation", "qmlattmeth", "member", "members");
    }
}

/*!
  Reset each Section in vector \a v to its initialized state.
 */
void Sections::clear(QVector<Section> &v)
{
    for (int i = 0; i < v.size(); ++i)
        v[i].clear();
}

/*!
  Linearize the maps in each Section in \a v.
 */
void Sections::reduce(QVector<Section> &v)
{
    for (int i = 0; i < v.size(); ++i)
        v[i].reduce();
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
        FunctionNode *func = static_cast<FunctionNode *>(t);
        if (func->isMacro())
            v[StdMacros].insert(n);
        else
            v[StdFunctions].insert(n);
    }
        return;
    case Node::Variable: {
        const VariableNode *var = static_cast<const VariableNode *>(t);
        if (!var->doc().isEmpty()) {
            if (var->isStatic())
                v[StdStaticVariables].insert(n);
            else
                v[StdVariables].insert(n);
        }
    }
        return;
    case Node::SharedComment: {
        SharedCommentNode *scn = static_cast<SharedCommentNode *>(t);
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
    if (aggregate_->isNamespace()) {
        ns = static_cast<const NamespaceNode *>(aggregate_);
        if (!ns->hasDoc())
            documentAll = false; // only document children that have documentation
    }
    for (auto it = aggregate_->constBegin(); it != aggregate_->constEnd(); ++it) {
        Node *n = *it;
        if (documentAll || n->hasDoc()) {
            stdRefPageSwitch(stdSummarySections(), n);
            stdRefPageSwitch(stdDetailsSections(), n);
        }
    }
    if (!aggregate_->relatedByProxy().isEmpty()) {
        const QList<Node *> &relatedBy = aggregate_->relatedByProxy();
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
        FunctionNode *fn = static_cast<FunctionNode *>(n);
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
        SharedCommentNode *scn = static_cast<SharedCommentNode *>(n);
        if (scn->collective().count())
            t = scn->collective().first(); // TODO: warn about mixed node types in collective?
    }

    if (t->isFunction()) {
        FunctionNode *fn = static_cast<FunctionNode *>(t);
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
        SharedCommentNode *scn = static_cast<SharedCommentNode *>(n);
        if (scn->collective().count())
            t = scn->collective().first(); // TODO: warn about mixed node types in collective?
    }

    if (t->isQmlProperty() || t->isJsProperty()) {
        QmlPropertyNode *pn = static_cast<QmlPropertyNode *>(t);
        if (pn->isAttached())
            dv[QmlAttachedProperties].insert(n);
        else
            dv[QmlProperties].insert(n);
    } else if (t->isFunction()) {
        FunctionNode *fn = static_cast<FunctionNode *>(t);
        if (fn->isQmlSignal() || fn->isJsSignal()) {
            if (fn->isAttached())
                dv[QmlAttachedSignals].insert(n);
            else
                dv[QmlSignals].insert(n);
        } else if (fn->isQmlSignalHandler() || fn->isJsSignalHandler()) {
            dv[QmlSignalHandlers].insert(n);
        } else if (fn->isQmlMethod() || fn->isJsMethod()) {
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
    if (n->isQmlProperty() || n->isJsProperty()) {
        QmlPropertyNode *pn = static_cast<QmlPropertyNode *>(n);
        if (pn->isAttached())
            sv[QmlAttachedProperties].insert(pn);
        else
            sv[QmlProperties].insert(pn);
    } else if (n->isFunction()) {
        FunctionNode *fn = static_cast<FunctionNode *>(n);
        if (fn->isQmlSignal() || fn->isJsSignal()) {
            if (fn->isAttached())
                sv[QmlAttachedSignals].insert(fn);
            else
                sv[QmlSignals].insert(fn);
        } else if (fn->isQmlSignalHandler() || fn->isJsSignalHandler()) {
            sv[QmlSignalHandlers].insert(fn);
        } else if (fn->isQmlMethod() || fn->isJsMethod()) {
            if (fn->isAttached())
                sv[QmlAttachedMethods].insert(fn);
            else
                sv[QmlMethods].insert(fn);
        }
    } else if (n->isSharedCommentNode()) {
        SharedCommentNode *scn = static_cast<SharedCommentNode *>(n);
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
    const QVector<RelatedClass> baseClasses = cn->baseClasses();
    for (const auto &cls : baseClasses) {
        if (cls.node_)
            stack.prepend(cls.node_);
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
    if (aggregate_->parent() && !aggregate_->name().isEmpty() && !aggregate_->hasDoc())
        documentAll = false;
    for (auto it = aggregate_->constBegin(); it != aggregate_->constEnd(); ++it) {
        Node *n = *it;
        if (!n->isPrivate() && !n->isProperty() && !n->isRelatedNonmember()
            && !n->isSharedCommentNode())
            allMembers.insert(n);
        if (!documentAll && !n->hasDoc())
            continue;

        distributeNodeInSummaryVector(summarySections, n);
        distributeNodeInDetailsVector(detailsSections, n);
    }
    if (!aggregate_->relatedByProxy().isEmpty()) {
        const QList<Node *> relatedBy = aggregate_->relatedByProxy();
        for (const auto &node : relatedBy)
            distributeNodeInSummaryVector(summarySections, node);
    }

    QStack<ClassNode *> stack;
    ClassNode *cn = static_cast<ClassNode *>(aggregate_);
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

    const Aggregate *qtn = aggregate_;
    while (qtn) {
        if (!qtn->isAbstract() || !classMap)
            classMap = allMembers.newClassMap(qtn);
        for (const auto n : qtn->childNodes()) {
            if (n->isInternal())
                continue;

            if (!n->isSharedCommentNode() || n->isPropertyGroup())
                allMembers.add(classMap, n);

            if (qtn == aggregate_ || qtn->isAbstract()) {
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
    if (aggregate_->isClassNode())
        sections = &stdCppClassSummarySections();
    else if (aggregate_->isQmlType() || aggregate_->isQmlBasicType())
        sections = &stdQmlTypeSummarySections();
    else
        sections = &stdSummarySections();
    for (const auto &section : *sections) {
        if (!section.obsoleteMembers().isEmpty())
            summary_spv->append(&section);
    }
    if (aggregate_->isClassNode())
        sections = &stdCppClassDetailsSections();
    else if (aggregate_->isQmlType() || aggregate_->isQmlBasicType())
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
