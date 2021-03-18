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

#ifndef NODE_H
#define NODE_H

#include "doc.h"
#include "parameters.h"

#include <QtCore/qdir.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class Node;
class Tree;
class EnumNode;
class PageNode;
class ClassNode;
class Aggregate;
class ExampleNode;
class TypedefNode;
class QmlTypeNode;
class QDocDatabase;
class FunctionNode;
class PropertyNode;
class CollectionNode;
class QmlPropertyNode;
class SharedCommentNode;

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
typedef QVector<CollectionNode *> CollectionList;

class Node
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::Node)

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

    enum Access : unsigned char { Public, Protected, Private };

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

    virtual ~Node() {}
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
    QString pageTypeString() const;
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
    bool isPrivate() const { return access_ == Private; }
    bool isProperty() const { return nodeType_ == Property; }
    bool isProxyNode() const { return nodeType_ == Proxy; }
    bool isPublic() const { return access_ == Public; }
    bool isProtected() const { return access_ == Protected; }
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
        setAccess(Private);
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
    QString nodeSubtypeString() const;

    bool isSharingComment() const { return (sharedCommentNode_ != nullptr); }
    bool hasSharedDoc() const;
    void setSharedCommentNode(SharedCommentNode *t) { sharedCommentNode_ = t; }
    SharedCommentNode *sharedCommentNode() { return sharedCommentNode_; }

    // QString guid() const;
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

    static QString cleanId(const QString &str);
    static FlagValue toFlagValue(bool b);
    static bool fromFlagValue(FlagValue fv, bool defaultValue);
    static QString pageTypeString(PageType t);
    static QString nodeTypeString(NodeType t);
    static QString nodeSubtypeString(unsigned char t);
    static void initialize();
    static NodeType goal(const QString &t) { return goals_.value(t); }
    static bool nodeNameLessThan(const Node *first, const Node *second);

protected:
    Node(NodeType type, Aggregate *parent, const QString &name);

private:
    NodeType nodeType_;
    Genus genus_;
    Access access_;
    ThreadSafeness safeness_;
    PageType pageType_;
    Status status_;
    bool indexNodeFlag_ : 1;
    bool relatedNonmember_ : 1;
    bool hadDoc_ : 1;

    Aggregate *parent_;
    SharedCommentNode *sharedCommentNode_;
    QString name_;
    Location declLocation_;
    Location defLocation_;
    Doc doc_;
    QMap<LinkType, QPair<QString, QString>> linkMap_;
    QString fileNameBase_;
    QString physicalModuleName_;
    QString url_;
    QString since_;
    QString templateDecl_;
    QString reconstitutedBrief_;
    // mutable QString uuid_;
    QString outSubDir_;
    static QStringMap operators_;
    static int propertyGroupCount_;
    static QMap<QString, Node::NodeType> goals_;
};

class PageNode : public Node
{
public:
    PageNode(Aggregate *parent, const QString &name) : Node(Page, parent, name), noAutoList_(false)
    {
    }
    PageNode(NodeType type, Aggregate *parent, const QString &name)
        : Node(type, parent, name), noAutoList_(false)
    {
    }
    PageNode(Aggregate *parent, const QString &name, PageType ptype)
        : Node(Page, parent, name), noAutoList_(false)
    {
        setPageType(ptype);
    }

    bool isPageNode() const override { return true; }
    bool isTextPageNode() const override { return !isAggregate(); } // PageNode but not Aggregate

    QString title() const override { return title_; }
    QString subtitle() const override { return subtitle_; }
    QString fullTitle() const override;
    bool setTitle(const QString &title) override;
    bool setSubtitle(const QString &subtitle) override
    {
        subtitle_ = subtitle;
        return true;
    }
    QString nameForLists() const override { return title(); }

    virtual QString imageFileName() const { return QString(); }
    virtual void setImageFileName(const QString &) {}

    bool noAutoList() const { return noAutoList_; }
    void setNoAutoList(bool b) override { noAutoList_ = b; }
    const QStringList &groupNames() const { return groupNames_; }
    void appendGroupName(const QString &t) override { groupNames_.append(t); }

    void setOutputFileName(const QString &f) override { outputFileName_ = f; }
    QString outputFileName() const override { return outputFileName_; }

protected:
    friend class Node;

protected:
    bool noAutoList_;
    QString title_;
    QString subtitle_;
    QString outputFileName_;
    QStringList groupNames_;
};

class ExternalPageNode : public PageNode
{
public:
    ExternalPageNode(Aggregate *parent, const QString &url)
        : PageNode(Node::ExternalPage, parent, url)
    {
        setPageType(Node::ArticlePage);
        setUrl(url);
    }
};

class Aggregate : public PageNode
{
public:
    Node *findChildNode(const QString &name, Node::Genus genus, int findFlags = 0) const;
    Node *findNonfunctionChild(const QString &name, bool (Node::*)() const);
    void findChildren(const QString &name, NodeVector &nodes) const;
    FunctionNode *findFunctionChild(const QString &name, const Parameters &parameters);
    FunctionNode *findFunctionChild(const FunctionNode *clone);

    void normalizeOverloads();
    void markUndocumentedChildrenInternal();

    bool isAggregate() const override { return true; }
    const EnumNode *findEnumNodeForValue(const QString &enumValue) const;

    int count() const { return children_.size(); }
    const NodeList &childNodes() const { return children_; }
    const NodeList &nonfunctionList();
    NodeList::ConstIterator constBegin() const { return children_.constBegin(); }
    NodeList::ConstIterator constEnd() const { return children_.constEnd(); }

    void addIncludeFile(const QString &includeFile);
    void setIncludeFiles(const QStringList &includeFiles);
    const QStringList &includeFiles() const { return includeFiles_; }

    QStringList primaryKeys();
    QmlPropertyNode *hasQmlProperty(const QString &) const;
    QmlPropertyNode *hasQmlProperty(const QString &, bool attached) const;
    virtual QmlTypeNode *qmlBaseNode() const { return nullptr; }
    void addChildByTitle(Node *child, const QString &title);
    void printChildren(const QString &title);
    void addChild(Node *child);
    void adoptChild(Node *child);
    void setOutputSubdirectory(const QString &t) override;

    FunctionMap &functionMap() { return functionMap_; }
    void findAllFunctions(NodeMapMap &functionIndex);
    void findAllNamespaces(NodeMultiMap &namespaces);
    void findAllAttributions(NodeMultiMap &attributions);
    bool hasObsoleteMembers() const;
    void findAllObsoleteThings();
    void findAllClasses();
    void findAllSince();
    void resolveQmlInheritance();
    bool hasOverloads(const FunctionNode *fn) const;
    void appendToRelatedByProxy(const NodeList &t) { relatedByProxy_.append(t); }
    NodeList &relatedByProxy() { return relatedByProxy_; }
    QString typeWord(bool cap) const;

protected:
    Aggregate(NodeType type, Aggregate *parent, const QString &name)
        : PageNode(type, parent, name), functionCount_(0)
    {
    }
    ~Aggregate() override;
    void removeFunctionNode(FunctionNode *fn);

private:
    friend class Node;
    void addFunction(FunctionNode *fn);
    void adoptFunction(FunctionNode *fn);
    static bool isSameSignature(const FunctionNode *f1, const FunctionNode *f2);

protected:
    NodeList children_;
    NodeList relatedByProxy_;

private:
    QStringList includeFiles_;
    NodeList enumChildren_;
    NodeMultiMap nonfunctionMap_;
    NodeList nonfunctionList_;

protected:
    int functionCount_;
    FunctionMap functionMap_;
};

class ProxyNode : public Aggregate
{
public:
    ProxyNode(Aggregate *parent, const QString &name);
    bool docMustBeGenerated() const override { return true; }
    bool isRelatableType() const override { return true; }
};

class NamespaceNode : public Aggregate
{
public:
    NamespaceNode(Aggregate *parent, const QString &name)
        : Aggregate(Namespace, parent, name), seen_(false), tree_(nullptr), docNode_(nullptr)
    {
    }
    ~NamespaceNode() override;
    Tree *tree() const override { return (parent() ? parent()->tree() : tree_); }

    bool isFirstClassAggregate() const override { return true; }
    bool isRelatableType() const override { return true; }
    bool wasSeen() const override { return seen_; }
    void markSeen() { seen_ = true; }
    void markNotSeen() { seen_ = false; }
    void setTree(Tree *t) { tree_ = t; }
    const NodeList &includedChildren() const;
    void includeChild(Node *child);
    QString whereDocumented() const { return whereDocumented_; }
    void setWhereDocumented(const QString &t) { whereDocumented_ = t; }
    bool isDocumentedHere() const;
    bool hasDocumentedChildren() const;
    void reportDocumentedChildrenInUndocumentedNamespace() const;
    bool docMustBeGenerated() const override;
    void setDocNode(NamespaceNode *ns) { docNode_ = ns; }
    NamespaceNode *docNode() const { return docNode_; }

private:
    bool seen_;
    Tree *tree_;
    QString whereDocumented_;
    NamespaceNode *docNode_;
    NodeList includedChildren_;
};

struct RelatedClass
{
    RelatedClass() {}
    // constructor for resolved base class
    RelatedClass(Node::Access access, ClassNode *node) : access_(access), node_(node) {}
    // constructor for unresolved base class
    RelatedClass(Node::Access access, const QStringList &path, const QString &signature)
        : access_(access), node_(nullptr), path_(path), signature_(signature)
    {
    }
    QString accessString() const;
    bool isPrivate() const { return (access_ == Node::Private); }

    Node::Access access_;
    ClassNode *node_;
    QStringList path_;
    QString signature_;
};

struct UsingClause
{
    UsingClause() {}
    UsingClause(const QString &signature) : node_(nullptr), signature_(signature) {}
    const QString &signature() const { return signature_; }
    const Node *node() { return node_; }
    void setNode(const Node *n) { node_ = n; }

    const Node *node_;
    QString signature_;
};

class ClassNode : public Aggregate
{
public:
    ClassNode(NodeType type, Aggregate *parent, const QString &name)
        : Aggregate(type, parent, name), abstract_(false), wrapper_(false), qmlelement(nullptr)
    {
    }
    bool isFirstClassAggregate() const override { return true; }
    bool isClassNode() const override { return true; }
    bool isRelatableType() const override { return true; }
    bool isWrapper() const override { return wrapper_; }
    QString obsoleteLink() const override { return obsoleteLink_; }
    void setObsoleteLink(const QString &t) override { obsoleteLink_ = t; }
    void setWrapper() override { wrapper_ = true; }

    void addResolvedBaseClass(Access access, ClassNode *node);
    void addDerivedClass(Access access, ClassNode *node);
    void addUnresolvedBaseClass(Access access, const QStringList &path, const QString &signature);
    void addUnresolvedUsingClause(const QString &signature);
    void removePrivateAndInternalBases();
    void resolvePropertyOverriddenFromPtrs(PropertyNode *pn);

    QVector<RelatedClass> &baseClasses() { return bases_; }
    QVector<RelatedClass> &derivedClasses() { return derived_; }
    QVector<RelatedClass> &ignoredBaseClasses() { return ignoredBases_; }
    QVector<UsingClause> &usingClauses() { return usingClauses_; }

    const QVector<RelatedClass> &baseClasses() const { return bases_; }
    const QVector<RelatedClass> &derivedClasses() const { return derived_; }
    const QVector<RelatedClass> &ignoredBaseClasses() const { return ignoredBases_; }
    const QVector<UsingClause> &usingClauses() const { return usingClauses_; }

    QmlTypeNode *qmlElement() { return qmlelement; }
    void setQmlElement(QmlTypeNode *qcn) { qmlelement = qcn; }
    bool isAbstract() const override { return abstract_; }
    void setAbstract(bool b) override { abstract_ = b; }
    PropertyNode *findPropertyNode(const QString &name);
    QmlTypeNode *findQmlBaseNode();
    FunctionNode *findOverriddenFunction(const FunctionNode *fn);
    PropertyNode *findOverriddenProperty(const FunctionNode *fn);
    bool docMustBeGenerated() const override;

private:
    void promotePublicBases(const QVector<RelatedClass> &bases);

private:
    QVector<RelatedClass> bases_;
    QVector<RelatedClass> derived_;
    QVector<RelatedClass> ignoredBases_;
    QVector<UsingClause> usingClauses_;
    bool abstract_;
    bool wrapper_;
    QString obsoleteLink_;
    QmlTypeNode *qmlelement;
};

class HeaderNode : public Aggregate
{
public:
    HeaderNode(Aggregate *parent, const QString &name);
    bool docMustBeGenerated() const override;
    bool isFirstClassAggregate() const override { return true; }
    bool isRelatableType() const override { return true; }
    QString title() const override { return (title_.isEmpty() ? name() : title_); }
    QString subtitle() const override { return subtitle_; }
    QString fullTitle() const override
    {
        return (title_.isEmpty() ? name() : name() + " - " + title_);
    }
    bool setTitle(const QString &title) override
    {
        title_ = title;
        return true;
    }
    bool setSubtitle(const QString &subtitle) override
    {
        subtitle_ = subtitle;
        return true;
    }
    QString nameForLists() const override { return title(); }
    bool hasDocumentedChildren() const;

private:
    QString title_;
    QString subtitle_;
};

class ExampleNode : public PageNode
{
public:
    ExampleNode(Aggregate *parent, const QString &name) : PageNode(Node::Example, parent, name) {}
    QString imageFileName() const override { return imageFileName_; }
    void setImageFileName(const QString &ifn) override { imageFileName_ = ifn; }
    const QStringList &files() const { return files_; }
    const QStringList &images() const { return images_; }
    const QString &projectFile() const { return projectFile_; }
    void setFiles(const QStringList files, const QString &projectFile)
    {
        files_ = files;
        projectFile_ = projectFile;
    }
    void setImages(const QStringList images) { images_ = images; }
    void appendFile(QString &file) { files_.append(file); }
    void appendImage(QString &image) { images_.append(image); }

private:
    QString imageFileName_;
    QString projectFile_;
    QStringList files_;
    QStringList images_;
};

struct ImportRec
{
    QString name_; // module name
    QString version_; // <major> . <minor>
    QString importId_; // "as" name
    QString importUri_; // subdirectory of module directory

    ImportRec(const QString &name, const QString &version, const QString &importId,
              const QString &importUri)
        : name_(name), version_(version), importId_(importId), importUri_(importUri)
    {
    }
    QString &name() { return name_; }
    QString &version() { return version_; }
    QString &importId() { return importId_; }
    QString &importUri() { return importUri_; }
    bool isEmpty() const { return name_.isEmpty(); }
};

typedef QVector<ImportRec> ImportList;

class QmlTypeNode : public Aggregate
{
public:
    QmlTypeNode(Aggregate *parent, const QString &name, NodeType type = QmlType);
    bool isFirstClassAggregate() const override { return true; }
    bool isQtQuickNode() const override
    {
        return (logicalModuleName() == QLatin1String("QtQuick"));
    }
    ClassNode *classNode() override { return cnode_; }
    void setClassNode(ClassNode *cn) override { cnode_ = cn; }
    bool isAbstract() const override { return abstract_; }
    bool isWrapper() const override { return wrapper_; }
    void setAbstract(bool b) override { abstract_ = b; }
    void setWrapper() override { wrapper_ = true; }
    bool isInternal() const override { return (status() == Internal); }
    QString qmlFullBaseName() const override;
    QString obsoleteLink() const override { return obsoleteLink_; }
    void setObsoleteLink(const QString &t) override { obsoleteLink_ = t; }
    QString logicalModuleName() const override;
    QString logicalModuleVersion() const override;
    QString logicalModuleIdentifier() const override;
    CollectionNode *logicalModule() const override { return logicalModule_; }
    void setQmlModule(CollectionNode *t) override { logicalModule_ = t; }

    const ImportList &importList() const { return importList_; }
    void setImportList(const ImportList &il) { importList_ = il; }
    const QString &qmlBaseName() const { return qmlBaseName_; }
    void setQmlBaseName(const QString &name) { qmlBaseName_ = name; }
    bool qmlBaseNodeNotSet() const { return (qmlBaseNode_ == nullptr); }
    QmlTypeNode *qmlBaseNode() const override { return qmlBaseNode_; }
    void setQmlBaseNode(QmlTypeNode *b) { qmlBaseNode_ = b; }
    void requireCppClass() { cnodeRequired_ = true; }
    bool cppClassRequired() const { return cnodeRequired_; }
    static void addInheritedBy(const Node *base, Node *sub);
    static void subclasses(const Node *base, NodeList &subs);
    static void terminate();
    bool inherits(Aggregate *type);

public:
    static bool qmlOnly;
    static QMultiMap<const Node *, Node *> inheritedBy;

private:
    bool abstract_;
    bool cnodeRequired_;
    bool wrapper_;
    ClassNode *cnode_;
    QString qmlBaseName_;
    QString obsoleteLink_;
    CollectionNode *logicalModule_;
    QmlTypeNode *qmlBaseNode_;
    ImportList importList_;
};

class QmlBasicTypeNode : public Aggregate
{
public:
    QmlBasicTypeNode(Aggregate *parent, const QString &name, NodeType type = QmlBasicType);
    bool isFirstClassAggregate() const override { return true; }
};

class QmlPropertyNode : public Node
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::QmlPropertyNode)

public:
    QmlPropertyNode(Aggregate *parent, const QString &name, const QString &type, bool attached);

    void setDataType(const QString &dataType) override { type_ = dataType; }
    void setStored(bool stored) { stored_ = toFlagValue(stored); }
    void setDesignable(bool designable) { designable_ = toFlagValue(designable); }

    const QString &dataType() const { return type_; }
    QString qualifiedDataType() const { return type_; }
    bool isReadOnlySet() const { return (readOnly_ != FlagValueDefault); }
    bool isStored() const { return fromFlagValue(stored_, true); }
    bool isDesignable() const { return fromFlagValue(designable_, false); }
    bool isWritable();
    bool isDefault() const override { return isdefault_; }
    bool isReadOnly() const override { return fromFlagValue(readOnly_, false); }
    bool isAlias() const override { return isAlias_; }
    bool isAttached() const override { return attached_; }
    bool isQtQuickNode() const override { return parent()->isQtQuickNode(); }
    QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    QString logicalModuleName() const override { return parent()->logicalModuleName(); }
    QString logicalModuleVersion() const override { return parent()->logicalModuleVersion(); }
    QString logicalModuleIdentifier() const override { return parent()->logicalModuleIdentifier(); }
    QString element() const override { return parent()->name(); }

    void markDefault() override { isdefault_ = true; }
    void markReadOnly(bool flag) override { readOnly_ = toFlagValue(flag); }

private:
    PropertyNode *findCorrespondingCppProperty();

private:
    QString type_;
    FlagValue stored_;
    FlagValue designable_;
    bool isAlias_;
    bool isdefault_;
    bool attached_;
    FlagValue readOnly_;
};

class EnumItem
{
public:
    EnumItem() {}
    EnumItem(const QString &name, const QString &value) : name_(name), value_(value) {}

    const QString &name() const { return name_; }
    const QString &value() const { return value_; }

private:
    QString name_;
    QString value_;
};

class EnumNode : public Node
{
public:
    EnumNode(Aggregate *parent, const QString &name, bool isScoped = false)
        : Node(Enum, parent, name), flagsType_(nullptr), isScoped_(isScoped)
    {
    }

    void addItem(const EnumItem &item);
    void setFlagsType(TypedefNode *typedeff);
    bool hasItem(const QString &name) const { return names_.contains(name); }
    bool isScoped() const { return isScoped_; }

    const QVector<EnumItem> &items() const { return items_; }
    Access itemAccess(const QString &name) const;
    const TypedefNode *flagsType() const { return flagsType_; }
    QString itemValue(const QString &name) const;
    Node *clone(Aggregate *parent) override;

private:
    QVector<EnumItem> items_;
    QSet<QString> names_;
    const TypedefNode *flagsType_;
    bool isScoped_;
};

class TypedefNode : public Node
{
public:
    TypedefNode(Aggregate *parent, const QString &name, NodeType type = Typedef)
        : Node(type, parent, name), associatedEnum_(nullptr)
    {
    }

    bool hasAssociatedEnum() const { return associatedEnum_ != nullptr; }
    const EnumNode *associatedEnum() const { return associatedEnum_; }
    Node *clone(Aggregate *parent) override;

private:
    void setAssociatedEnum(const EnumNode *t);

    friend class EnumNode;

    const EnumNode *associatedEnum_;
};

class TypeAliasNode : public TypedefNode
{
public:
    TypeAliasNode(Aggregate *parent,
                  const QString &name,
                  const QString &aliasedType)
        : TypedefNode(parent, name, NodeType::TypeAlias), aliasedType_(aliasedType)
    {
    }

    const QString &aliasedType() const { return aliasedType_; }
    const Node *aliasedNode() const { return aliasedNode_; }
    void setAliasedNode(const Node *node) { aliasedNode_ = node; }
    Node *clone(Aggregate *parent) override;

private:
    QString aliasedType_;
    const Node *aliasedNode_ {};
};

inline void EnumNode::setFlagsType(TypedefNode *t)
{
    flagsType_ = t;
    t->setAssociatedEnum(this);
}

class SharedCommentNode : public Node
{
public:
    SharedCommentNode(Node *n) : Node(Node::SharedComment, n->parent(), QString())
    {
        collective_.reserve(1);
        append(n);
    }
    SharedCommentNode(QmlTypeNode *parent, int count, QString &group)
        : Node(Node::SharedComment, parent, group)
    {
        collective_.reserve(count);
    }
    ~SharedCommentNode() override { collective_.clear(); }

    bool isPropertyGroup() const override
    {
        return !name().isEmpty() && !collective_.isEmpty()
                && (collective_.at(0)->isQmlProperty() || collective_.at(0)->isJsProperty());
    }
    int count() const { return collective_.size(); }
    void append(Node *n)
    {
        collective_.append(n);
        n->setSharedCommentNode(this);
        setGenus(n->genus());
    }
    void sort() { std::sort(collective_.begin(), collective_.end(), Node::nodeNameLessThan); }
    const QVector<Node *> &collective() const { return collective_; }
    void setOverloadFlags();
    void setRelatedNonmember(bool b) override;
    Node *clone(Aggregate *parent) override;

private:
    QVector<Node *> collective_;
};

class FunctionNode : public Node
{
public:
    enum Virtualness { NonVirtual, NormalVirtual, PureVirtual };

    enum Metaness {
        Plain,
        Signal,
        Slot,
        Ctor,
        Dtor,
        CCtor, // copy constructor
        MCtor, // move-copy constructor
        MacroWithParams,
        MacroWithoutParams,
        Native,
        CAssign, // copy-assignment operator
        MAssign, // move-assignment operator
        QmlSignal,
        QmlSignalHandler,
        QmlMethod,
        JsSignal,
        JsSignalHandler,
        JsMethod
    };

    FunctionNode(Aggregate *parent, const QString &name); // C++ function (Plain)
    FunctionNode(Metaness type, Aggregate *parent, const QString &name, bool attached = false);

    Node *clone(Aggregate *parent) override;
    Metaness metaness() const { return metaness_; }
    QString metanessString() const;
    bool changeMetaness(Metaness from, Metaness to);
    void setMetaness(Metaness t) { metaness_ = t; }
    Metaness setMetaness(const QString &t);
    QString kindString() const;
    static Metaness getMetaness(const QString &t);
    static Metaness getMetanessFromTopic(const QString &t);
    static Genus getGenus(Metaness t);

    void setReturnType(const QString &t) { returnType_ = t; }
    void setParentPath(const QStringList &p) { parentPath_ = p; }
    void setVirtualness(const QString &t);
    void setVirtualness(Virtualness v) { virtualness_ = v; }
    void setVirtual() { virtualness_ = NormalVirtual; }
    void setConst(bool b) { const_ = b; }
    void setStatic(bool b) { static_ = b; }
    void setReimpFlag() { reimpFlag_ = true; }
    void setOverridesThis(const QString &path) { overridesThis_ = path; }

    const QString &returnType() const { return returnType_; }
    QString virtualness() const;
    bool isConst() const { return const_; }
    bool isStatic() const override { return static_; }
    bool isOverload() const { return overloadFlag_; }
    bool isMarkedReimp() const override { return reimpFlag_; }
    bool isSomeCtor() const { return isCtor() || isCCtor() || isMCtor(); }
    bool isMacroWithParams() const { return (metaness_ == MacroWithParams); }
    bool isMacroWithoutParams() const { return (metaness_ == MacroWithoutParams); }
    bool isMacro() const override { return (isMacroWithParams() || isMacroWithoutParams()); }
    bool isObsolete() const override;

    bool isCppFunction() const { return metaness_ == Plain; } // Is this correct?
    bool isSignal() const { return (metaness_ == Signal); }
    bool isSlot() const { return (metaness_ == Slot); }
    bool isCtor() const { return (metaness_ == Ctor); }
    bool isDtor() const { return (metaness_ == Dtor); }
    bool isCCtor() const { return (metaness_ == CCtor); }
    bool isMCtor() const { return (metaness_ == MCtor); }
    bool isCAssign() const { return (metaness_ == CAssign); }
    bool isMAssign() const { return (metaness_ == MAssign); }

    bool isJsMethod() const { return (metaness_ == JsMethod); }
    bool isJsSignal() const { return (metaness_ == JsSignal); }
    bool isJsSignalHandler() const { return (metaness_ == JsSignalHandler); }

    bool isQmlMethod() const { return (metaness_ == QmlMethod); }
    bool isQmlSignal() const { return (metaness_ == QmlSignal); }
    bool isQmlSignalHandler() const { return (metaness_ == QmlSignalHandler); }

    bool isSpecialMemberFunction() const
    {
        return (isDtor() || isCCtor() || isMCtor() || isCAssign() || isMAssign());
    }
    bool isNonvirtual() const { return (virtualness_ == NonVirtual); }
    bool isVirtual() const { return (virtualness_ == NormalVirtual); }
    bool isPureVirtual() const { return (virtualness_ == PureVirtual); }
    bool returnsBool() const { return (returnType_ == QLatin1String("bool")); }

    Parameters &parameters() { return parameters_; }
    const Parameters &parameters() const { return parameters_; }
    bool isPrivateSignal() const { return parameters_.isPrivateSignal(); }
    void setParameters(const QString &signature) { parameters_.set(signature); }
    QString signature(bool values, bool noReturnType, bool templateParams = false) const override;

    const QString &overridesThis() const { return overridesThis_; }
    const NodeList &associatedProperties() const { return associatedProperties_; }
    const QStringList &parentPath() const { return parentPath_; }
    bool hasAssociatedProperties() const { return !associatedProperties_.isEmpty(); }
    bool hasOneAssociatedProperty() const { return (associatedProperties_.size() == 1); }
    Node *firstAssociatedProperty() const { return associatedProperties_[0]; }

    QString element() const override { return parent()->name(); }
    bool isAttached() const override { return attached_; }
    bool isQtQuickNode() const override { return parent()->isQtQuickNode(); }
    QString qmlTypeName() const override { return parent()->qmlTypeName(); }
    QString logicalModuleName() const override { return parent()->logicalModuleName(); }
    QString logicalModuleVersion() const override { return parent()->logicalModuleVersion(); }
    QString logicalModuleIdentifier() const override { return parent()->logicalModuleIdentifier(); }

    void debug() const;

    void setFinal(bool b) { isFinal_ = b; }
    bool isFinal() const { return isFinal_; }

    void setOverride(bool b) { isOverride_ = b; }
    bool isOverride() const { return isOverride_; }

    void setRef(bool b) { isRef_ = b; }
    bool isRef() const { return isRef_; }

    void setRefRef(bool b) { isRefRef_ = b; }
    bool isRefRef() const { return isRefRef_; }

    void setInvokable(bool b) { isInvokable_ = b; }
    bool isInvokable() const { return isInvokable_; }

    bool hasTag(const QString &t) const override { return (tag_ == t); }
    void setTag(const QString &t) { tag_ = t; }
    const QString &tag() const { return tag_; }
    bool compare(const FunctionNode *fn) const;
    bool isIgnored() const;
    bool hasOverloads() const;
    void clearOverloadFlag() { overloadFlag_ = false; }
    void setOverloadFlag() { overloadFlag_ = true; }
    void setOverloadNumber(signed short n);
    void appendOverload(FunctionNode *fn);
    signed short overloadNumber() const { return overloadNumber_; }
    FunctionNode *nextOverload() { return nextOverload_; }
    void setNextOverload(FunctionNode *fn) { nextOverload_ = fn; }
    FunctionNode *findPrimaryFunction();

private:
    void addAssociatedProperty(PropertyNode *property);

    friend class Aggregate;
    friend class PropertyNode;

    bool const_ : 1;
    bool static_ : 1;
    bool reimpFlag_ : 1;
    bool attached_ : 1;
    bool overloadFlag_ : 1;
    bool isFinal_ : 1;
    bool isOverride_ : 1;
    bool isRef_ : 1;
    bool isRefRef_ : 1;
    bool isInvokable_ : 1;
    Metaness metaness_;
    Virtualness virtualness_;
    signed short overloadNumber_;
    FunctionNode *nextOverload_;
    QString returnType_;
    QStringList parentPath_;
    QString overridesThis_;
    QString tag_;
    NodeList associatedProperties_;
    Parameters parameters_;
};

class PropertyNode : public Node
{
public:
    enum FunctionRole { Getter, Setter, Resetter, Notifier };
    enum { NumFunctionRoles = Notifier + 1 };

    PropertyNode(Aggregate *parent, const QString &name);

    void setDataType(const QString &dataType) override { type_ = dataType; }
    void addFunction(FunctionNode *function, FunctionRole role);
    void addSignal(FunctionNode *function, FunctionRole role);
    void setStored(bool stored) { stored_ = toFlagValue(stored); }
    void setDesignable(bool designable) { designable_ = toFlagValue(designable); }
    void setScriptable(bool scriptable) { scriptable_ = toFlagValue(scriptable); }
    void setWritable(bool writable) { writable_ = toFlagValue(writable); }
    void setUser(bool user) { user_ = toFlagValue(user); }
    void setOverriddenFrom(const PropertyNode *baseProperty);
    void setRuntimeDesFunc(const QString &rdf) { runtimeDesFunc_ = rdf; }
    void setRuntimeScrFunc(const QString &scrf) { runtimeScrFunc_ = scrf; }
    void setConstant() { const_ = true; }
    void setFinal() { final_ = true; }
    void setRevision(int revision) { revision_ = revision; }

    const QString &dataType() const { return type_; }
    QString qualifiedDataType() const;
    NodeList functions() const;
    const NodeList &functions(FunctionRole role) const { return functions_[(int)role]; }
    const NodeList &getters() const { return functions(Getter); }
    const NodeList &setters() const { return functions(Setter); }
    const NodeList &resetters() const { return functions(Resetter); }
    const NodeList &notifiers() const { return functions(Notifier); }
    bool hasAccessFunction(const QString &name) const;
    FunctionRole role(const FunctionNode *fn) const;
    bool isStored() const { return fromFlagValue(stored_, storedDefault()); }
    bool isDesignable() const { return fromFlagValue(designable_, designableDefault()); }
    bool isScriptable() const { return fromFlagValue(scriptable_, scriptableDefault()); }
    const QString &runtimeDesignabilityFunction() const { return runtimeDesFunc_; }
    const QString &runtimeScriptabilityFunction() const { return runtimeScrFunc_; }
    bool isWritable() const { return fromFlagValue(writable_, writableDefault()); }
    bool isUser() const { return fromFlagValue(user_, userDefault()); }
    bool isConstant() const { return const_; }
    bool isFinal() const { return final_; }
    const PropertyNode *overriddenFrom() const { return overrides_; }

    bool storedDefault() const { return true; }
    bool userDefault() const { return false; }
    bool designableDefault() const { return !setters().isEmpty(); }
    bool scriptableDefault() const { return true; }
    bool writableDefault() const { return !setters().isEmpty(); }

private:
    QString type_;
    QString runtimeDesFunc_;
    QString runtimeScrFunc_;
    NodeList functions_[NumFunctionRoles];
    FlagValue stored_;
    FlagValue designable_;
    FlagValue scriptable_;
    FlagValue writable_;
    FlagValue user_;
    bool const_;
    bool final_;
    int revision_;
    const PropertyNode *overrides_;
};

inline void PropertyNode::addFunction(FunctionNode *function, FunctionRole role)
{
    functions_[(int)role].append(function);
    function->addAssociatedProperty(this);
}

inline void PropertyNode::addSignal(FunctionNode *function, FunctionRole role)
{
    functions_[(int)role].append(function);
    function->addAssociatedProperty(this);
}

inline NodeList PropertyNode::functions() const
{
    NodeList list;
    for (int i = 0; i < NumFunctionRoles; ++i)
        list += functions_[i];
    return list;
}

class VariableNode : public Node
{
public:
    VariableNode(Aggregate *parent, const QString &name);

    void setLeftType(const QString &leftType) { leftType_ = leftType; }
    void setRightType(const QString &rightType) { rightType_ = rightType; }
    void setStatic(bool b) { static_ = b; }

    const QString &leftType() const { return leftType_; }
    const QString &rightType() const { return rightType_; }
    QString dataType() const { return leftType_ + rightType_; }
    bool isStatic() const override { return static_; }
    Node *clone(Aggregate *parent) override;

private:
    QString leftType_;
    QString rightType_;
    bool static_;
};

inline VariableNode::VariableNode(Aggregate *parent, const QString &name)
    : Node(Variable, parent, name), static_(false)
{
    setGenus(Node::CPP);
}

class CollectionNode : public PageNode
{
public:
    CollectionNode(NodeType type, Aggregate *parent, const QString &name)
        : PageNode(type, parent, name), seen_(false)
    {
    }

    bool isCollectionNode() const override { return true; }
    QString qtVariable() const override { return qtVariable_; }
    void setQtVariable(const QString &v) override { qtVariable_ = v; }
    void addMember(Node *node) override;
    bool hasMembers() const override;
    bool hasNamespaces() const override;
    bool hasClasses() const override;
    void getMemberNamespaces(NodeMap &out) override;
    void getMemberClasses(NodeMap &out) const override;
    bool wasSeen() const override { return seen_; }

    QString fullTitle() const override { return title(); }
    QString logicalModuleName() const override { return logicalModuleName_; }
    QString logicalModuleVersion() const override
    {
        return logicalModuleVersionMajor_ + QLatin1Char('.') + logicalModuleVersionMinor_;
    }
    QString logicalModuleIdentifier() const override
    {
        return logicalModuleName_ + logicalModuleVersionMajor_;
    }
    void setLogicalModuleInfo(const QString &arg) override;
    void setLogicalModuleInfo(const QStringList &info) override;

    const NodeList &members() const { return members_; }
    void printMembers(const QString &title);

    void markSeen() { seen_ = true; }
    void markNotSeen() { seen_ = false; }

private:
    bool seen_;
    NodeList members_;
    QString logicalModuleName_;
    QString logicalModuleVersionMajor_;
    QString logicalModuleVersionMinor_;
    QString qtVariable_;
};

QT_END_NAMESPACE

#endif
