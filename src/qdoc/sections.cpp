/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include <qobjectdefs.h>
#include "sections.h"
#include "config.h"
#include <qdebug.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

const Aggregate *Sections::aggregate_ = 0;

static FastSection privateFuncs("Private Functions", "private function", "private functions");
static FastSection privateSlots("Private Slots", "private slot", "private slots");
static FastSection privateTypes("Private Types", "private type", "private types");
static FastSection protectedFuncs("Protected Functions", "protected function", "protected functions");
static FastSection protectedSlots("Protected Slots", "protected slot", "protected slots");
static FastSection protectedTypes("Protected Types", "protected type", "protected types");
static FastSection protectedVars("Protected Variables", "protected type", "protected variables");
static FastSection publicFuncs("Public Functions", "public function", "public functions");
static FastSection publicSignals("Signals", "signal", "signals");
static FastSection publicSlots("Public Slots", "public slot", "public slots");
static FastSection publicTypes("Public Types", "public type", "public types");
static FastSection publicVars("Public Variables", "public variable", "public variables");
static FastSection properties("Properties", "property", "properties");
static FastSection relatedNonMembs("Related Non-Members", "related non-member", "related non-members");
static FastSection staticPrivMembs("Static Private Members", "static private member", "static private members");
static FastSection staticProtMembs("Static Protected Members", "static protected member", "static protected members");
static FastSection staticPubMembs("Static Public Members", "static public member", "static public members");
static FastSection macros("Macros", "macro", "macros");

static FastSection detMemberFuncs("Member Function Documentation", "func", "member", "members");
static FastSection detMemberTypes("Member Type Documentation", "types", "member", "members");
static FastSection detMemberVars("Member Variable Documentation", "vars", "member", "members");
static FastSection detProperties("Property Documentation", "prop", "member", "members");
static FastSection detRelatedNonMembs("Related Non-Members", "relnonmem", "member", "members");
static FastSection detMacros("Macro Documentation", "macros", "member", "members");

/*!
  The destructor must delete each member of the
  list of QML class lists, if it is not empty;
 */
Section::~Section()
{
    if (!classKeysNodesList_.isEmpty()) {
        for (int i=0; i<classKeysNodesList_.size(); i++) {
            ClassKeysNodes* classKeysNodes = classKeysNodesList_[i];
            classKeysNodesList_[i] = 0;
            delete classKeysNodes;
        }
    }
}

/*!
  The destructor must delete the QML class maps in the class
  map list, if the class map list is not empty.
 */
FastSection::~FastSection()
{
    clear();
}

/*!
  The FastSection variables are now static variables so we
  don't have to repeatedly construct and destroy them. But
  we do need to clear them for each call to sections() or
  qmlSections().
 */
void FastSection::clear()
{
    memberMap_.clear();
    reimpMemberMap_.clear();
    if (!classMapList_.isEmpty()) {
        for (int i=0; i<classMapList_.size(); i++) {
            ClassMap* classMap = classMapList_[i];
            classMapList_[i] = 0;
            delete classMap;
        }
    }
    inherited_.clear();
}

/*!
  \class Sections
  \brief A class for creating a list of collections for documentation

  Each element in the list is a Section struct, which contains all the
  elements that will be documented in a section of a reference page.
 */

/*!
  Construct a name for the \a node that can be used for sorting
  a set of nodes into equivalence classes. If \a name is provided,
  start with that name. Itherwise start with the name in \a node.
 */
QString Sections::sortName(const Node *node, const QString* name)
{
    QString nodeName;
    if (name != 0)
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
            nodeName.insert(nodeName.size()-numDigits-1, QLatin1Char('0'));
    }

    if (node->isFunction()) {
        const FunctionNode *func = static_cast<const FunctionNode *>(node);
        QString sortNo;
        if (func->isSomeCtor())
            sortNo = QLatin1String("C");
        else if (func->isDtor())
            sortNo = QLatin1String("D");
        else if (nodeName.startsWith(QLatin1String("operator"))
                 && nodeName.length() > 8
                 && !nodeName[8].isLetterOrNumber())
            sortNo = QLatin1String("F");
        else
            sortNo = QLatin1String("E");
        return sortNo + nodeName + QLatin1Char(' ') + QString::number(func->overloadNumber(), 36);
    }

    if (node->isClass())
        return QLatin1Char('A') + nodeName;

    if (node->isProperty() || node->isVariable())
        return QLatin1Char('E') + nodeName;

    if (node->isQmlMethod() || node->isQmlSignal() || node->isQmlSignalHandler())
         return QLatin1Char('E') + nodeName;

    return QLatin1Char('B') + nodeName;
}

/*!
  Insert the \a node into the temporary section \a fs. Whether
  the \a node is actually inserted can depend on the \a style
  and the \a status.
 */
void Sections::insert(FastSection &fs, Node *node, Style style, Status status)
{
    bool irrelevant = false;
    bool inheritedMember = false;
    if (!node->relates()) {
        Aggregate* p = node->parent();
        if (p->isQmlPropertyGroup())
            p = p->parent();
        if (!p->isNamespace() && p != aggregate_) {
            if ((!p->isQmlType() && !p->isJsType()) || !p->isAbstract())
                inheritedMember = true;
        }
    }

    if (node->isPrivate()) {
        irrelevant = true;
    }
    else if (node->isFunction()) {
        FunctionNode *func = (FunctionNode *) node;
        irrelevant = (inheritedMember && (func->isSomeCtor() || func->isDtor()));
    }
    else if (node->isClass() || node->isEnumType() || node->isTypedef()) {
        irrelevant = (inheritedMember && style != Subpage);
        if (!irrelevant && style == Detailed && node->isTypedef()) {
            const TypedefNode* tdn = static_cast<const TypedefNode*>(node);
            if (tdn->associatedEnum())
                irrelevant = true;
        }
    }

    if (!irrelevant) {
        if (status == Compat) {
            irrelevant = (node->status() != Node::Compat);
        }
        else if (status == Obsolete) {
            irrelevant = (node->status() != Node::Obsolete);
        }
        else {
            irrelevant = (node->status() == Node::Compat ||
                          node->status() == Node::Obsolete);
        }
    }

    if (!irrelevant) {
        if (!inheritedMember || style == Subpage) {
            QString key = sortName(node);
            fs.memberMap_.insertMulti(key, node);
        }
        else {
            if (node->parent()->isClass() || node->parent()->isNamespace()) {
                if (fs.inherited_.isEmpty() || fs.inherited_.last().first != node->parent()) {
                    QPair<Aggregate *, int> p(node->parent(), 0);
                    fs.inherited_.append(p);
                }
                fs.inherited_.last().second++;
            }
        }
    }
}

/*!
  Returns \c true if \a node represents a reimplemented member
  function in the class of the FastSection \a fs. If it is
  a reimplemented function, then it is inserted into the
  reimplemented member map in \a fs. The test is performed
  only if \a status is \e OK. True is returned if \a node
  is inserted into the map. Otherwise, false is returned.
 */
bool Sections::insertReimpFunc(FastSection& fs, Node* node, Status status)
{
    if (!node->isPrivate() && (node->relates() == 0)) {
        const FunctionNode* fn = static_cast<const FunctionNode*>(node);
        if (!fn->reimplementedFrom().isEmpty() && (status == Okay)) {
            if (fn->parent() == aggregate_) {
                QString key = sortName(fn);
                if (!fs.reimpMemberMap_.contains(key)) {
                    fs.reimpMemberMap_.insert(key,node);
                    return true;
                }
            }
        }
    }
    return false;
}

/*!
  If \a fs is not empty, convert it to a Section and append
  the new Section to \a sectionList.
 */
void Sections::append(QList<Section>& sectionList, const FastSection& fs, bool includeKeys)
{
    if (!fs.isEmpty()) {
        Section section(fs.name_, fs.divClass_, fs.singular_, fs.plural_);
        sectionList.append(section);
        Section* s = &sectionList[sectionList.size()-1];
        if (fs.classMapList_.isEmpty()) {
            Section section(fs.name_, fs.divClass_, fs.singular_, fs.plural_);
            if (includeKeys)
                s->keys_ = fs.memberMap_.keys();
            s->members_ = fs.memberMap_.values();
            s->reimpMembers_ = fs.reimpMemberMap_.values();
            s->inherited_ = fs.inherited_;
        }
        else {
            for (int i=0; i<fs.classMapList_.size(); i++) {
                ClassMap* classMap = fs.classMapList_[i];
                ClassKeysNodes* ckn = new ClassKeysNodes;
                ckn->first = classMap->first;
                ckn->second.second = classMap->second.values();
                ckn->second.first = classMap->second.keys();
                s->classKeysNodesList_.append(ckn);
             }
        }
    }
}

static void clearClassSummaryVars()
{
    privateFuncs.clear();
    privateSlots.clear();
    privateTypes.clear();
    protectedFuncs.clear();
    protectedSlots.clear();
    protectedTypes.clear();
    protectedVars.clear();
    publicFuncs.clear();
    publicSignals.clear();
    publicSlots.clear();
    publicTypes.clear();
    publicVars.clear();
    properties.clear();
    relatedNonMembs.clear();
    staticPrivMembs.clear();
    staticProtMembs.clear();
    staticPubMembs.clear();
    macros.clear();
}

static void clearClassDetailedVars()
{
    detMemberFuncs.clear();
    detMemberTypes.clear();
    detMemberVars.clear();
    detProperties.clear();
    detRelatedNonMembs.clear();
    detMacros.clear();
}

QList<Section> Sections::getStdCppSections(const Aggregate *aggregate, Style style, Status status)
{
    QList<Section> sections;
    setCurrentNode(aggregate);
    if (aggregate->isClass()) {
        if (style == Summary)
            getCppClassStdSummarySections(sections, style, status);
        else if (style == Detailed)
            getCppClassStdDetailedSections(sections, style, status);
        else
            getAllCppClassMembers(sections, style, status);
    } else if (style == Summary || style == Detailed) {
        getAllStdCppSections(sections, style, status);
    }

    return sections;
}

void Sections::getAllStdCppSections(QList<Section> &sections, Style style, Status status)
{
    FastSection namespaces("Namespaces",
                           style == Detailed ? "nmspace" : QString(),
                           "namespace",
                           "namespaces");
    FastSection classes("Classes",
                        style == Detailed ? "classes" : QString(),
                        "class",
                        "classes");
    FastSection types(style == Summary ? "Types" : "Type Documentation",
                      style == Detailed ? "types" : QString(),
                      "type",
                      "types");
    FastSection variables(style == Summary ? "Variables" : "Variable Documentation",
                          style == Detailed ? "vars" : QString(),
                          "variable",
                          "variables");
    FastSection staticVars("Static Variables",
                           QString(),
                           "static variable",
                           "static variables");
    FastSection functions(style == Summary ? "Functions" : "Function Documentation",
                          style == Detailed ? "func" : QString(),
                          "function",
                          "functions");
    FastSection macros(style == Summary ? "Macros" : "Macro Documentation",
                       style == Detailed ? "macros" : QString(),
                       "macro",
                       "macros");

    bool documentAll = true;
    NodeList nodeList = aggregate_->childNodes();
    nodeList += aggregate_->relatedNodes();
    if (aggregate_->isNamespace()) {
        const NamespaceNode* ns = static_cast<const NamespaceNode*>(aggregate_);
        if (!ns->hasDoc())
            documentAll = false;
        if (style == Summary) {
            if (!ns->orphans().isEmpty())
                nodeList += ns->orphans();
        }
    }
    NodeList::ConstIterator c = nodeList.constBegin();
    while (c != nodeList.constEnd()) {
        Node *n = *c;
        if (documentAll || n->hasDoc()) {
            switch (n->type()) {
            case Node::Namespace:
                insert(namespaces, n, style, status);
                break;
            case Node::Class:
                insert(classes, n, style, status);
                break;
            case Node::Enum:
            case Node::Typedef:
                insert(types, n, style, status);
                break;
            case Node::Function:
                {
                    FunctionNode *func = static_cast<FunctionNode *>(n);
                    if (func->isMacro())
                        insert(macros, n, style, status);
                    else
                        insert(functions, n, style, status);
                }
                break;
            case Node::Variable:
                {
                    const VariableNode* var = static_cast<const VariableNode*>(n);
                    if (!var->doc().isEmpty()) {
                        if (var->isStatic())
                            insert(staticVars, n, style, status);
                        else
                            insert(variables, n, style, status);
                    }
                }
                break;
            case Node::SharedComment:
                {
                    SharedCommentNode *scn = static_cast<SharedCommentNode *>(n);
                    if (!scn->doc().isEmpty())
                        insert(functions, scn, style, status);
                }
                break;
            default:
                break;
            }
        }
        ++c;
    }
    append(sections, namespaces);
    append(sections, classes);
    append(sections, types);
    append(sections, variables);
    append(sections, staticVars);
    append(sections, functions);
    append(sections, macros);
}

void Sections::getCppClassStdSummarySections(QList<Section> &sections, Style style, Status status)
{
    clearClassSummaryVars();
    NodeList::ConstIterator r = aggregate_->relatedNodes().constBegin();
    while (r != aggregate_->relatedNodes().constEnd()) {
        if ((*r)->isFunction()) {
            FunctionNode *func = static_cast<FunctionNode *>(*r);
            if (func->isMacro())
                insert(macros, *r, style, status);
            else
                insert(relatedNonMembs, *r, style, status);
        } else {
            insert(relatedNonMembs, *r, style, status);
        }
        ++r;
    }

    bool documentAll = true;
    if (aggregate_->parent() && !aggregate_->name().isEmpty() && !aggregate_->hasDoc())
        documentAll = false;
    QStack<const Aggregate *> stack;
    stack.push(aggregate_);
    while (!stack.isEmpty()) {
        const Aggregate* ancestor = stack.pop();
        NodeList::ConstIterator c = ancestor->childNodes().constBegin();
        while (c != ancestor->childNodes().constEnd()) {
            Node* n = *c;
            if (!documentAll && !n->hasDoc()) {
                ++c;
                continue;
            }
            bool isSlot = false;
            bool isSignal = false;
            bool isStatic = false;
            if (n->isFunction()) {
                const FunctionNode *func = (const FunctionNode *) n;
                isSlot = (func->isSlot());
                isSignal = (func->isSignal());
                isStatic = func->isStatic();
                if (func->hasAssociatedProperties() && !func->hasActiveAssociatedProperty()) {
                    ++c;
                    continue;
                } else if (func->isIgnored()) {
                    ++c;
                    continue;
                }
            }
            else if (n->isVariable()) {
                const VariableNode *var = static_cast<const VariableNode *>(n);
                isStatic = var->isStatic();
            } else if (n->isTypedef()) {
                if (n->name() == QLatin1String("QtGadgetHelper")) {
                    ++c;
                    continue;
                }
            }

            switch (n->access()) {
            case Node::Public:
                if (isSlot) {
                    insert(publicSlots, n, style, status);
                }
                else if (isSignal) {
                    insert(publicSignals, n, style, status);
                } else if (isStatic) {
                    if (!n->isVariable() || !n->doc().isEmpty())
                        insert(staticPubMembs, n, style, status);
                } else if (n->isProperty()) {
                    insert(properties, n, style, status);
                } else if (n->isVariable()) {
                    if (!n->doc().isEmpty())
                        insert(publicVars, n, style, status);
                } else if (n->isFunction()) {
                    if (!insertReimpFunc(publicFuncs,n,status))
                        insert(publicFuncs, n, style, status);
                } else if (!n->isSharedCommentNode()) {
                    insert(publicTypes, n, style, status);
                }
                break;
            case Node::Protected:
                if (isSlot) {
                    insert(protectedSlots, n, style, status);
                } else if (isStatic) {
                    if (!n->isVariable() || !n->doc().isEmpty())
                        insert(staticProtMembs, n, style, status);
                } else if (n->isVariable()) {
                    if (!n->doc().isEmpty())
                        insert(protectedVars, n, style, status);
                } else if (n->isFunction()) {
                    if (!insertReimpFunc(protectedFuncs, n, status))
                        insert(protectedFuncs, n, style, status);
                } else {
                    insert(protectedTypes, n, style, status);
                }
                break;
            case Node::Private:
                if (isSlot) {
                    insert(privateSlots, n, style, status);
                } else if (isStatic) {
                    if (!n->isVariable() || !n->doc().isEmpty())
                        insert(staticPrivMembs, n, style, status);
                } else if (n->isFunction()) {
                    if (!insertReimpFunc(privateFuncs, n, status))
                        insert(privateFuncs, n, style, status);
                } else {
                    insert(privateTypes, n, style, status);
                }
            }
            ++c;
        }

        if (ancestor->isClass()) {
            const ClassNode* cn = static_cast<const ClassNode*>(ancestor);
            QList<RelatedClass>::ConstIterator r = cn->baseClasses().constBegin();
            while (r != cn->baseClasses().constEnd()) {
                if (r->node_)
                    stack.prepend(r->node_);
                ++r;
            }
        }
    }
    append(sections, publicTypes);
    append(sections, properties);
    append(sections, publicFuncs);
    append(sections, publicSlots);
    append(sections, publicSignals);
    append(sections, publicVars);
    append(sections, staticPubMembs);
    append(sections, protectedTypes);
    append(sections, protectedFuncs);
    append(sections, protectedSlots);
    append(sections, protectedVars);
    append(sections, staticProtMembs);
    append(sections, privateTypes);
    append(sections, privateFuncs);
    append(sections, privateSlots);
    append(sections, staticPrivMembs);
    append(sections, relatedNonMembs);
    append(sections, macros);
}

void Sections::getCppClassStdDetailedSections(QList<Section> &sections, Style style, Status status)
{
    clearClassDetailedVars();
    NodeList::ConstIterator r = aggregate_->relatedNodes().constBegin();
    while (r != aggregate_->relatedNodes().constEnd()) {
        Node* n = *r;
        if (n->isFunction()) {
            FunctionNode *func = static_cast<FunctionNode *>(n);
            if (func->isMacro())
                insert(detMacros, n, style, status);
            else if (!func->isSharingComment())
                insert(detRelatedNonMembs, n, style, status);
        } else {
            insert(detRelatedNonMembs, n, style, status);
        }
        ++r;
    }

    bool documentAll = true;
    if (aggregate_->parent() && !aggregate_->name().isEmpty() && !aggregate_->hasDoc())
        documentAll = false;
    NodeList::ConstIterator c = aggregate_->childNodes().constBegin();
    while (c != aggregate_->childNodes().constEnd()) {
        Node* n = *c;
        if (n->isSharingComment()) {
            // do nothing
        } else if (!documentAll && !n->hasDoc()) {
            ++c;
            continue;
        } else if (n->isEnumType() || n->isTypedef()) {
            if (n->name() == QLatin1String("QtGadgetHelper")) {
                ++c;
                continue;
            }
            insert(detMemberTypes, *c, style, status);
        } else if (n->isProperty()) {
            insert(detProperties, *c, style, status);
        } else if (n->isVariable()) {
            if (!n->doc().isEmpty())
                insert(detMemberVars, *c, style, status);
        } else if (n->isFunction()) {
            FunctionNode *function = static_cast<FunctionNode *>(n);
            if (function->isIgnored()) {
                ++c;
                continue;
            }
            if (!function->isSharingComment()) {
                if (!function->hasAssociatedProperties() || !function->doc().isEmpty())
                    insert(detMemberFuncs, function, style, status);
            }
        } else if (n->isSharedCommentNode()) {
            SharedCommentNode *scn = static_cast<SharedCommentNode *>(n);
            if (!scn->doc().isEmpty())
                insert(detMemberFuncs, scn, style, status);
        }
        ++c;
    }

    append(sections, detMemberTypes);
    append(sections, detProperties);
    append(sections, detMemberFuncs);
    append(sections, detMemberVars);
    append(sections, detRelatedNonMembs);
    append(sections, detMacros);
}

/*!
  Build the "all members" list for a C++ class.
 */
void Sections::getAllCppClassMembers(QList<Section> &sections, Style style, Status status)
{
    FastSection all(QString(), QString(), "member", "members");
    QStack<const Aggregate*> stack;
    stack.push(aggregate_);
    while (!stack.isEmpty()) {
        const Aggregate* ancestor = stack.pop();
        NodeList::ConstIterator c = ancestor->childNodes().constBegin();
        while (c != ancestor->childNodes().constEnd()) {
            Node *n = *c;
            if (!n->isPrivate() && !n->isProperty())
                insert(all, n, style, status);
            ++c;
        }
        if (ancestor->isClass()) {
            const ClassNode* cn = static_cast<const ClassNode*>(ancestor);
            QList<RelatedClass>::ConstIterator r = cn->baseClasses().constBegin();
            while (r != cn->baseClasses().constEnd()) {
                if (r->node_)
                    stack.prepend(r->node_);
                ++r;
            }
        }
    }
    append(sections, all);
}

/*!
  This function is for documenting QML properties. It returns
  the list of documentation sections for the children of the
  \a aggregate.
 */
QList<Section> Sections::getStdQmlSections(Aggregate* aggregate, Style style, Status status)
{
    QList<Section> sections;
    setCurrentNode(aggregate);
    if (aggregate) {
        if (style == Summary)
            getQmlTypeStdSummarySections(sections, style, status);
        else if (style == Detailed)
            getQmlTypeStdDetailedSections(sections, style, status);
        else
            getAllQmlTypeMembers(sections);
    }
    return sections;
}

void Sections::getQmlTypeStdSummarySections(QList<Section> &sections, Style style, Status status)
{
    FastSection qmlproperties("Properties", QString(), "property", "properties");
    FastSection qmlattachedproperties("Attached Properties", QString(), "attached property", "attached properties");
    FastSection qmlsignals("Signals", QString(), "signal", "signals");
    FastSection qmlsignalhandlers("Signal Handlers", QString(), "signal handler", "signal handlers");
    FastSection qmlattachedsignals("Attached Signals", QString(), "attached signal", "attached signals");
    FastSection qmlmethods("Methods", QString(), "method", "methods");
    FastSection qmlattachedmethods("Attached Methods", QString(), "attached method", "attached methods");
    const Aggregate* qcn = aggregate_;
    while (qcn != 0) {
        NodeList::ConstIterator c = qcn->childNodes().constBegin();
        while (c != qcn->childNodes().constEnd()) {
            if ((*c)->status() == Node::Internal) {
                ++c;
                continue;
            }
            if ((*c)->isQmlPropertyGroup() || (*c)->isJsPropertyGroup()) {
                insert(qmlproperties, *c, style, status);
            }
            else if ((*c)->isQmlProperty() || (*c)->isJsProperty()) {
                const QmlPropertyNode* pn = static_cast<const QmlPropertyNode*>(*c);
                if (pn->isAttached())
                    insert(qmlattachedproperties,*c,style, status);
                else {
                    insert(qmlproperties,*c,style, status);
                }
            }
            else if ((*c)->isQmlSignal() || (*c)->isJsSignal()) {
                const FunctionNode* sn = static_cast<const FunctionNode*>(*c);
                if (sn->isAttached())
                    insert(qmlattachedsignals,*c,style, status);
                else
                    insert(qmlsignals,*c,style, status);
            }
            else if ((*c)->isQmlSignalHandler() || (*c)->isJsSignalHandler()) {
                insert(qmlsignalhandlers,*c,style, status);
            }
            else if ((*c)->isQmlMethod() || (*c)->isJsMethod()) {
                const FunctionNode* mn = static_cast<const FunctionNode*>(*c);
                if (mn->isAttached())
                    insert(qmlattachedmethods,*c,style, status);
                else
                    insert(qmlmethods,*c,style, status);
            }
            ++c;
        }
        if (qcn->qmlBaseNode() != 0) {
            qcn = static_cast<QmlTypeNode*>(qcn->qmlBaseNode());
            if (!qcn->isAbstract())
                qcn = 0;
        }
        else
            qcn = 0;
    }
    append(sections,qmlproperties);
    append(sections,qmlattachedproperties);
    append(sections,qmlsignals);
    append(sections,qmlsignalhandlers);
    append(sections,qmlattachedsignals);
    append(sections,qmlmethods);
    append(sections,qmlattachedmethods);
}

void Sections::getQmlTypeStdDetailedSections(QList<Section> &sections, Style style, Status status)
{
    FastSection qmlproperties( "Property Documentation","qmlprop","member","members");
    FastSection qmlattachedproperties("Attached Property Documentation","qmlattprop",
                                      "member","members");
    FastSection qmlsignals("Signal Documentation","qmlsig","signal","signals");
    FastSection qmlsignalhandlers("Signal Handler Documentation","qmlsighan","signal handler","signal handlers");
    FastSection qmlattachedsignals("Attached Signal Documentation","qmlattsig",
                                   "signal","signals");
    FastSection qmlmethods("Method Documentation","qmlmeth","member","members");
    FastSection qmlattachedmethods("Attached Method Documentation","qmlattmeth",
                                   "member","members");
    const Aggregate* qcn = aggregate_;
    while (qcn != 0) {
        NodeList::ConstIterator c = qcn->childNodes().constBegin();
        while (c != qcn->childNodes().constEnd()) {
            if ((*c)->status() == Node::Internal) {
                ++c;
                continue;
            }
            if ((*c)->isQmlPropertyGroup() || (*c)->isJsPropertyGroup()) {
                insert(qmlproperties,*c,style, status);
            }
            else if ((*c)->isQmlProperty() || (*c)->isJsProperty()) {
                const QmlPropertyNode* pn = static_cast<const QmlPropertyNode*>(*c);
                if (pn->isAttached())
                    insert(qmlattachedproperties,*c,style, status);
                else
                    insert(qmlproperties,*c,style, status);
            }
            else if ((*c)->isQmlSignal() || (*c)->isJsSignal()) {
                const FunctionNode* sn = static_cast<const FunctionNode*>(*c);
                if (sn->isAttached())
                    insert(qmlattachedsignals,*c,style, status);
                else
                    insert(qmlsignals,*c,style, status);
            }
            else if ((*c)->isQmlSignalHandler() || (*c)->isJsSignalHandler()) {
                insert(qmlsignalhandlers,*c,style, status);
            }
            else if ((*c)->isQmlMethod() || (*c)->isJsMethod()) {
                const FunctionNode* mn = static_cast<const FunctionNode*>(*c);
                if (mn->isAttached())
                    insert(qmlattachedmethods,*c,style, status);
                else
                    insert(qmlmethods,*c,style, status);
            }
            ++c;
        }
        if (qcn->qmlBaseNode() != 0) {
            qcn = static_cast<QmlTypeNode*>(qcn->qmlBaseNode());
            if (!qcn->isAbstract())
                qcn = 0;
        }
        else
            qcn = 0;
    }
    append(sections,qmlproperties);
    append(sections,qmlattachedproperties);
    append(sections,qmlsignals);
    append(sections,qmlsignalhandlers);
    append(sections,qmlattachedsignals);
    append(sections,qmlmethods);
    append(sections,qmlattachedmethods);
}

/*!
  This is where the list of all members including inherited
  members is prepared.
*/
void Sections::getAllQmlTypeMembers(QList<Section> &sections)
{
    ClassMap* classMap = 0;
    FastSection all(QString(), QString(), "member", "members");
    const Aggregate* current = aggregate_;
    while (current != 0) {
        /*
          If the QML type is abstract, do not create
          a new entry in the list for it. Instead,
          add its members to the current entry.

          However, if the first class is abstract,
          there is no current entry. In that case,
          create a new entry in the list anyway.
          I'm not sure that is correct, but it at
          least can prevent a crash.
        */
        if (!current->isAbstract() || !classMap) {
            classMap = new ClassMap;
            classMap->first = static_cast<const QmlTypeNode*>(current);
            all.classMapList_.append(classMap);
        }
        NodeList::ConstIterator c = current->childNodes().constBegin();
        while (c != current->childNodes().constEnd()) {
            if ((*c)->isQmlPropertyGroup() || (*c)->isJsPropertyGroup()) {
                const QmlPropertyGroupNode* qpgn = static_cast<const QmlPropertyGroupNode*>(*c);
                NodeList::ConstIterator p = qpgn->childNodes().constBegin();
                while (p != qpgn->childNodes().constEnd()) {
                    if ((*p)->isQmlProperty() || (*c)->isJsProperty()) {
                        QString key = (*p)->name();
                        key = sortName(*p, &key);
                        all.memberMap_.insert(key,*p);
                        classMap->second.insert(key,*p);
                    }
                    ++p;
                }
            }
            else {
                QString key = (*c)->name();
                key = sortName(*c, &key);
                all.memberMap_.insert(key,*c);
                classMap->second.insert(key,*c);
            }
            ++c;
        }
        if (current->qmlBaseNode() == current) {
            qDebug() << "qdoc internal error: circular type definition."
                     << "QML type" << current->name()
                     << "can't be its own base type";
            break;
        }
        current = current->qmlBaseNode();
        while (current) {
            if (current->isAbstract())
                break;
            if (current->isInternal())
                current = current->qmlBaseNode();
            else
                break;
        }
    }
    append(sections, all, true);
}

QT_END_NAMESPACE
