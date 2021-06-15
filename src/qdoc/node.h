/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef NODE_H
#define NODE_H

#include "access.h"
#include "doc.h"
#include "enumitem.h"
#include "importrec.h"
#include "parameters.h"
#include "relatedclass.h"
#include "usingclause.h"

#include <QtCore/qdir.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class Aggregate;
class ClassNode;
class CollectionNode;
class EnumNode;
class ExampleNode;
class FunctionNode;
class Node;
class QDocDatabase;
class QmlTypeNode;
class PageNode;
class PropertyNode;
class QmlPropertyNode;
class SharedCommentNode;
class Tree;
class TypedefNode;

typedef QMap<QString, FunctionNode *> FunctionMap;
typedef QList<Node *> NodeList;
typedef QList<ClassNode *> ClassList;
typedef QList<Node *> NodeVector;
typedef QMap<QString, Node *> NodeMap;
typedef QMap<QString, NodeMap> NodeMapMap;
typedef QMultiMap<QString, Node *> NodeMultiMap;
typedef QMap<QString, NodeMultiMap> NodeMultiMapMap;
typedef QMap<QString, CollectionNode *> CNMap;
typedef QMultiMap<QString, CollectionNode *> CNMultiMap;

class Node
{
public:
    enum NodeType : unsigned char {
        NoType,
        Namespace,
        Class,
        Struct,
        Union,
        HeaderFile,
        Page,
        Enum,
        Example,
        ExternalPage,
        Function,
        Typedef,
        TypeAlias,
        Property,
        Variable,
        Group,
        Module,
        QmlType,
        QmlModule,
        QmlProperty,
        QmlBasicType,
        JsType,
        JsModule,
        JsProperty,
        JsBasicType,
        SharedComment,
        Collection,
        Proxy,
        LastType
    };

    enum Genus : unsigned char {
        DontCare = 0x0,
        CPP = 0x1,
        JS = 0x2,
        QML = 0x4,
        DOC = 0x8,
        API = CPP | JS | QML
        };

    enum Status : unsigned char {
        Obsolete,
        Deprecated,
        Preliminary,
        Active,
        Internal,
        DontDocument
    }; // don't reorder this enum

    enum ThreadSafeness : unsigned char {
        UnspecifiedSafeness,
        NonReentrant,
        Reentrant,
        ThreadSafe
    };

    enum LinkType : unsigned char { StartLink, NextLink, PreviousLink, ContentsLink };

    enum PageType : unsigned char {
        NoPageType,
        AttributionPage,
        ApiPage,
        ArticlePage,
        ExamplePage,
        HowToPage,
        OverviewPage,
        TutorialPage,
        FAQPage,
        OnBeyondZebra
    };

    enum FlagValue { FlagValueDefault = -1, FlagValueFalse = 0, FlagValueTrue = 1 };

    virtual ~Node() = default;
    virtual Node *clone(Aggregate *) { return nullptr; } // currently only FunctionNode
    virtual Tree *tree() const;
    Aggregate *root() const;

    NodeType nodeType() const { return m_nodeType; }
    QString nodeTypeString() const;
    bool changeType(NodeType from, NodeType to);

    Genus genus() const { return m_genus; }
    void setGenus(Genus t) { m_genus = t; }
    static Genus getGenus(NodeType t);

    PageType pageType() const { return m_pageType; }
    void setPageType(PageType t) { m_pageType = t; }
    void setPageType(const QString &t);
    static PageType getPageType(NodeType t);

    bool isActive() const { return m_status == Active; }
    bool isAnyType() const { return true; }
    bool isClass() const { return m_nodeType == Class; }
    bool isCppNode() const { return genus() == CPP; }
    bool isDeprecated() const { return (m_status == Deprecated); }
    bool isDontDocument() const { return (m_status == DontDocument); }
    bool isEnumType() const { return m_nodeType == Enum; }
    bool isExample() const { return m_nodeType == Example; }
    bool isExternalPage() const { return m_nodeType == ExternalPage; }
    bool isFunction(Genus g = DontCare) const
    {
        return (m_nodeType != Function ? false : (genus() == g ? true : g == DontCare));
    }
    bool isGroup() const { return m_nodeType == Group; }
    bool isHeader() const { return m_nodeType == HeaderFile; }
    bool isIndexNode() const { return m_indexNodeFlag; }
    bool isJsBasicType() const { return m_nodeType == JsBasicType; }
    bool isJsModule() const { return m_nodeType == JsModule; }
    bool isJsNode() const { return genus() == JS; }
    bool isJsProperty() const { return m_nodeType == JsProperty; }
    bool isJsType() const { return m_nodeType == JsType; }
    bool isModule() const { return m_nodeType == Module; }
    bool isNamespace() const { return m_nodeType == Namespace; }
    bool isPage() const { return m_nodeType == Page; }
    bool isPreliminary() const { return (m_status == Preliminary); }
    bool isPrivate() const { return m_access == Access::Private; }
    bool isProperty() const { return m_nodeType == Property; }
    bool isProxyNode() const { return m_nodeType == Proxy; }
    bool isPublic() const { return m_access == Access::Public; }
    bool isProtected() const { return m_access == Access::Protected; }
    bool isQmlBasicType() const { return m_nodeType == QmlBasicType; }
    bool isQmlModule() const { return m_nodeType == QmlModule; }
    bool isQmlNode() const { return genus() == QML; }
    bool isQmlProperty() const { return m_nodeType == QmlProperty; }
    bool isQmlType() const { return m_nodeType == QmlType; }
    bool isRelatedNonmember() const { return m_relatedNonmember; }
    bool isStruct() const { return m_nodeType == Struct; }
    bool isSharedCommentNode() const { return m_nodeType == SharedComment; }
    bool isTypeAlias() const { return m_nodeType == TypeAlias; }
    bool isTypedef() const { return m_nodeType == Typedef || m_nodeType == TypeAlias; }
    bool isUnion() const { return m_nodeType == Union; }
    bool isVariable() const { return m_nodeType == Variable; }
    bool isGenericCollection() const { return (m_nodeType == Node::Collection); }

    virtual bool isObsolete() const { return (m_status == Obsolete); }
    virtual bool isAbstract() const { return false; }
    virtual bool isAggregate() const { return false; } // means "can have children"
    virtual bool isFirstClassAggregate() const
    {
        return false;
    } // Aggregate but not proxy or prop group"
    virtual bool isAlias() const { return false; }
    virtual bool isAttached() const { return false; }
    virtual bool isClassNode() const { return false; }
    virtual bool isCollectionNode() const { return false; }
    virtual bool isDefault() const { return false; }
    virtual bool isInternal() const;
    virtual bool isMacro() const { return false; }
    virtual bool isPageNode() const { return false; } // means "generates a doc page"
    virtual bool isQtQuickNode() const { return false; }
    virtual bool isReadOnly() const { return false; }
    virtual bool isRelatableType() const { return false; }
    virtual bool isMarkedReimp() const { return false; }
    virtual bool isPropertyGroup() const { return false; }
    virtual bool isStatic() const { return false; }
    virtual bool isTextPageNode() const { return false; } // means PageNode but not Aggregate
    virtual bool isWrapper() const;

    QString plainName() const;
    QString plainFullName(const Node *relative = nullptr) const;
    QString plainSignature() const;
    QString fullName(const Node *relative = nullptr) const;
    virtual QString signature(bool, bool, bool = false) const { return plainName(); }

    const QString &fileNameBase() const { return m_fileNameBase; }
    bool hasFileNameBase() const { return !m_fileNameBase.isEmpty(); }
    void setFileNameBase(const QString &t) { m_fileNameBase = t; }

    void setAccess(Access t) { m_access = t; }
    void setLocation(const Location &t);
    void setDoc(const Doc &doc, bool replace = false);
    void setStatus(Status t);
    void setThreadSafeness(ThreadSafeness t) { m_safeness = t; }
    void setSince(const QString &since);
    void setPhysicalModuleName(const QString &name) { m_physicalModuleName = name; }
    void setUrl(const QString &url) { m_url = url; }
    void setTemplateDecl(const QString &t) { m_templateDecl = t; }
    void setReconstitutedBrief(const QString &t) { m_reconstitutedBrief = t; }
    void setParent(Aggregate *n) { m_parent = n; }
    void setIndexNodeFlag(bool isIndexNode = true) { m_indexNodeFlag = isIndexNode; }
    void setHadDoc() { m_hadDoc = true; }
    virtual void setRelatedNonmember(bool b) { m_relatedNonmember = b; }
    virtual void setOutputFileName(const QString &) {}
    virtual void addMember(Node *) {}
    virtual bool hasMembers() const { return false; }
    virtual bool hasNamespaces() const { return false; }
    virtual bool hasClasses() const { return false; }
    virtual void setAbstract(bool) {}
    virtual void setWrapper() {}
    virtual void getMemberNamespaces(NodeMap &) {}
    virtual void getMemberClasses(NodeMap &) const {}
    virtual void setDataType(const QString &) {}
    virtual bool wasSeen() const { return false; }
    virtual void appendGroupName(const QString &) {}
    virtual QString element() const { return QString(); }
    virtual void setNoAutoList(bool) {}
    virtual bool docMustBeGenerated() const { return false; }

    virtual QString title() const { return name(); }
    virtual QString subtitle() const { return QString(); }
    virtual QString fullTitle() const { return name(); }
    virtual bool setTitle(const QString &) { return false; }
    virtual bool setSubtitle(const QString &) { return false; }

    void markInternal()
    {
        setAccess(Access::Private);
        setStatus(Internal);
    }
    virtual void markDefault() {}
    virtual void markReadOnly(bool) {}

    bool match(const QList<int> &types) const;
    Aggregate *parent() const { return m_parent; }
    const QString &name() const { return m_name; }
    QString physicalModuleName() const;
    QString url() const { return m_url; }
    virtual QString nameForLists() const { return m_name; }
    virtual QString outputFileName() const { return QString(); }
    virtual QString obsoleteLink() const { return QString(); }
    virtual void setObsoleteLink(const QString &) {}
    virtual void setQtVariable(const QString &) {}
    virtual QString qtVariable() const { return QString(); }
    virtual void setQtCMakeComponent(const QString &) {}
    virtual QString qtCMakeComponent() const { return QString(); }
    virtual bool hasTag(const QString &) const { return false; }

    const QMap<LinkType, QPair<QString, QString>> &links() const { return m_linkMap; }
    void setLink(LinkType linkType, const QString &link, const QString &desc);

    Access access() const { return m_access; }
    QString accessString() const;
    const Location &declLocation() const { return m_declLocation; }
    const Location &defLocation() const { return m_defLocation; }
    const Location &location() const
    {
        return (m_defLocation.isEmpty() ? m_declLocation : m_defLocation);
    }
    const Doc &doc() const { return m_doc; }
    bool isInAPI() const
    {
        return !isPrivate() && !isInternal() && !isDontDocument() && hasDoc();
    }
    bool hasDoc() const { return (m_hadDoc || !m_doc.isEmpty()); }
    bool hadDoc() const { return m_hadDoc; }
    Status status() const { return m_status; }
    ThreadSafeness threadSafeness() const;
    ThreadSafeness inheritedThreadSafeness() const;
    QString since() const { return m_since; }
    const QString &templateDecl() const { return m_templateDecl; }
    const QString &reconstitutedBrief() const { return m_reconstitutedBrief; }

    bool isSharingComment() const { return (m_sharedCommentNode != nullptr); }
    bool hasSharedDoc() const;
    void setSharedCommentNode(SharedCommentNode *t) { m_sharedCommentNode = t; }
    SharedCommentNode *sharedCommentNode() { return m_sharedCommentNode; }

    QString extractClassName(const QString &string) const;
    virtual QString qmlTypeName() const { return m_name; }
    virtual QString qmlFullBaseName() const { return QString(); }
    virtual QString logicalModuleName() const { return QString(); }
    virtual QString logicalModuleVersion() const { return QString(); }
    virtual QString logicalModuleIdentifier() const { return QString(); }
    virtual void setLogicalModuleInfo(const QString &) {}
    virtual void setLogicalModuleInfo(const QStringList &) {}
    virtual CollectionNode *logicalModule() const { return nullptr; }
    virtual void setQmlModule(CollectionNode *) {}
    virtual ClassNode *classNode() { return nullptr; }
    virtual void setClassNode(ClassNode *) {}
    QmlTypeNode *qmlTypeNode();
    ClassNode *declarativeCppNode();
    const QString &outputSubdirectory() const { return m_outSubDir; }
    virtual void setOutputSubdirectory(const QString &t) { m_outSubDir = t; }
    QString fullDocumentName() const;
    QString qualifyCppName();
    QString qualifyQmlName();
    QString unqualifyQmlName();
    QString qualifyWithParentName();

    static FlagValue toFlagValue(bool b);
    static bool fromFlagValue(FlagValue fv, bool defaultValue);
    static QString nodeTypeString(NodeType t);
    static void initialize();
    static NodeType goal(const QString &t) { return goals.value(t); }
    static bool nodeNameLessThan(const Node *first, const Node *second);

protected:
    Node(NodeType type, Aggregate *parent, const QString &name);

private:
    NodeType m_nodeType {};
    Genus m_genus {};
    Access m_access { Access::Public };
    ThreadSafeness m_safeness { UnspecifiedSafeness };
    PageType m_pageType { NoPageType };
    Status m_status { Active };
    bool m_indexNodeFlag : 1;
    bool m_relatedNonmember : 1;
    bool m_hadDoc : 1;

    Aggregate *m_parent { nullptr };
    SharedCommentNode *m_sharedCommentNode { nullptr };
    QString m_name {};
    Location m_declLocation {};
    Location m_defLocation {};
    Doc m_doc {};
    QMap<LinkType, QPair<QString, QString>> m_linkMap {};
    QString m_fileNameBase {};
    QString m_physicalModuleName {};
    QString m_url {};
    QString m_since {};
    QString m_templateDecl {};
    QString m_reconstitutedBrief {};
    QString m_outSubDir {};
    static QStringMap operators;
    static QMap<QString, Node::NodeType> goals;
};

QT_END_NAMESPACE

#endif
