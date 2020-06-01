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
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

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
typedef QVector<ClassNode *> ClassList;
typedef QVector<Node *> NodeVector;
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

    enum Genus : unsigned char { DontCare, CPP, JS, QML, DOC };

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

    NodeType nodeType() const { return nodeType_; }
    QString nodeTypeString() const;
    bool changeType(NodeType from, NodeType to);

    Genus genus() const { return genus_; }
    void setGenus(Genus t) { genus_ = t; }
    static Genus getGenus(NodeType t);

    PageType pageType() const { return pageType_; }
    void setPageType(PageType t) { pageType_ = t; }
    void setPageType(const QString &t);
    static PageType getPageType(NodeType t);

    bool isActive() const { return status_ == Active; }
    bool isAnyType() const { return true; }
    bool isClass() const { return nodeType_ == Class; }
    bool isCppNode() const { return genus() == CPP; }
    bool isDeprecated() const { return (status_ == Deprecated); }
    bool isDontDocument() const { return (status_ == DontDocument); }
    bool isEnumType() const { return nodeType_ == Enum; }
    bool isExample() const { return nodeType_ == Example; }
    bool isExternalPage() const { return nodeType_ == ExternalPage; }
    bool isFunction(Genus g = DontCare) const
    {
        return (nodeType_ != Function ? false : (genus() == g ? true : g == DontCare));
    }
    bool isGroup() const { return nodeType_ == Group; }
    bool isHeader() const { return nodeType_ == HeaderFile; }
    bool isIndexNode() const { return indexNodeFlag_; }
    bool isJsBasicType() const { return nodeType_ == JsBasicType; }
    bool isJsModule() const { return nodeType_ == JsModule; }
    bool isJsNode() const { return genus() == JS; }
    bool isJsProperty() const { return nodeType_ == JsProperty; }
    bool isJsType() const { return nodeType_ == JsType; }
    bool isModule() const { return nodeType_ == Module; }
    bool isNamespace() const { return nodeType_ == Namespace; }
    bool isPage() const { return nodeType_ == Page; }
    bool isPreliminary() const { return (status_ == Preliminary); }
    bool isPrivate() const { return access_ == Access::Private; }
    bool isProperty() const { return nodeType_ == Property; }
    bool isProxyNode() const { return nodeType_ == Proxy; }
    bool isPublic() const { return access_ == Access::Public; }
    bool isProtected() const { return access_ == Access::Protected; }
    bool isQmlBasicType() const { return nodeType_ == QmlBasicType; }
    bool isQmlModule() const { return nodeType_ == QmlModule; }
    bool isQmlNode() const { return genus() == QML; }
    bool isQmlProperty() const { return nodeType_ == QmlProperty; }
    bool isQmlType() const { return nodeType_ == QmlType; }
    bool isRelatedNonmember() const { return relatedNonmember_; }
    bool isStruct() const { return nodeType_ == Struct; }
    bool isSharedCommentNode() const { return nodeType_ == SharedComment; }
    bool isTypeAlias() const { return nodeType_ == TypeAlias; }
    bool isTypedef() const { return nodeType_ == Typedef || nodeType_ == TypeAlias; }
    bool isUnion() const { return nodeType_ == Union; }
    bool isVariable() const { return nodeType_ == Variable; }
    bool isGenericCollection() const { return (nodeType_ == Node::Collection); }

    virtual bool isObsolete() const { return (status_ == Obsolete); }
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

    const QString &fileNameBase() const { return fileNameBase_; }
    bool hasFileNameBase() const { return !fileNameBase_.isEmpty(); }
    void setFileNameBase(const QString &t) { fileNameBase_ = t; }

    void setAccess(Access t) { access_ = t; }
    void setLocation(const Location &t);
    void setDoc(const Doc &doc, bool replace = false);
    void setStatus(Status t)
    {
        if (status_ == Obsolete && t == Deprecated)
            return;
        status_ = t;
    }
    void setThreadSafeness(ThreadSafeness t) { safeness_ = t; }
    void setSince(const QString &since);
    void setPhysicalModuleName(const QString &name) { physicalModuleName_ = name; }
    void setUrl(const QString &url) { url_ = url; }
    void setTemplateDecl(const QString &t) { templateDecl_ = t; }
    void setReconstitutedBrief(const QString &t) { reconstitutedBrief_ = t; }
    void setParent(Aggregate *n) { parent_ = n; }
    void setIndexNodeFlag(bool isIndexNode = true) { indexNodeFlag_ = isIndexNode; }
    void setHadDoc() { hadDoc_ = true; }
    virtual void setRelatedNonmember(bool b) { relatedNonmember_ = b; }
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

    bool match(const QVector<int> &types) const;
    Aggregate *parent() const { return parent_; }
    const QString &name() const { return name_; }
    QString physicalModuleName() const;
    QString url() const { return url_; }
    virtual QString nameForLists() const { return name_; }
    virtual QString outputFileName() const { return QString(); }
    virtual QString obsoleteLink() const { return QString(); }
    virtual void setObsoleteLink(const QString &) {}
    virtual void setQtVariable(const QString &) {}
    virtual QString qtVariable() const { return QString(); }
    virtual void setQtCMakeComponent(const QString &) {}
    virtual QString qtCMakeComponent() const { return QString(); }
    virtual bool hasTag(const QString &) const { return false; }

    const QMap<LinkType, QPair<QString, QString>> &links() const { return linkMap_; }
    void setLink(LinkType linkType, const QString &link, const QString &desc);

    Access access() const { return access_; }
    QString accessString() const;
    const Location &declLocation() const { return declLocation_; }
    const Location &defLocation() const { return defLocation_; }
    const Location &location() const
    {
        return (defLocation_.isEmpty() ? declLocation_ : defLocation_);
    }
    const Doc &doc() const { return doc_; }
    bool isInAPI() const { return !isPrivate() && !isInternal() && hasDoc(); }
    bool hasDoc() const { return (hadDoc_ || !doc_.isEmpty()); }
    bool hadDoc() const { return hadDoc_; }
    Status status() const { return status_; }
    ThreadSafeness threadSafeness() const;
    ThreadSafeness inheritedThreadSafeness() const;
    QString since() const { return since_; }
    const QString &templateDecl() const { return templateDecl_; }
    const QString &reconstitutedBrief() const { return reconstitutedBrief_; }

    bool isSharingComment() const { return (sharedCommentNode_ != nullptr); }
    bool hasSharedDoc() const;
    void setSharedCommentNode(SharedCommentNode *t) { sharedCommentNode_ = t; }
    SharedCommentNode *sharedCommentNode() { return sharedCommentNode_; }

    QString extractClassName(const QString &string) const;
    virtual QString qmlTypeName() const { return name_; }
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
    const QString &outputSubdirectory() const { return outSubDir_; }
    virtual void setOutputSubdirectory(const QString &t) { outSubDir_ = t; }
    QString fullDocumentName() const;
    QString qualifyCppName();
    QString qualifyQmlName();
    QString unqualifyQmlName();
    QString qualifyWithParentName();

    static FlagValue toFlagValue(bool b);
    static bool fromFlagValue(FlagValue fv, bool defaultValue);
    static QString nodeTypeString(NodeType t);
    static void initialize();
    static NodeType goal(const QString &t) { return goals_.value(t); }
    static bool nodeNameLessThan(const Node *first, const Node *second);

protected:
    Node(NodeType type, Aggregate *parent, const QString &name);

private:
    NodeType nodeType_ {};
    Genus genus_ {};
    Access access_ { Access::Public };
    ThreadSafeness safeness_ { UnspecifiedSafeness };
    PageType pageType_ { NoPageType };
    Status status_ { Active };
    bool indexNodeFlag_ : 1;
    bool relatedNonmember_ : 1;
    bool hadDoc_ : 1;

    Aggregate *parent_ { nullptr };
    SharedCommentNode *sharedCommentNode_ { nullptr };
    QString name_ {};
    Location declLocation_ {};
    Location defLocation_ {};
    Doc doc_ {};
    QMap<LinkType, QPair<QString, QString>> linkMap_ {};
    QString fileNameBase_ {};
    QString physicalModuleName_ {};
    QString url_ {};
    QString since_ {};
    QString templateDecl_ {};
    QString reconstitutedBrief_ {};
    QString outSubDir_ {};
    static QStringMap operators_;
    static QMap<QString, Node::NodeType> goals_;
};

QT_END_NAMESPACE

#endif
