// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdocindexfiles.h"

#include "access.h"
#include "atom.h"
#include "classnode.h"
#include "collectionnode.h"
#include "comparisoncategory.h"
#include "config.h"
#include "enumnode.h"
#include "qmlenumnode.h"
#include "examplenode.h"
#include "externalpagenode.h"
#include "functionnode.h"
#include "generator.h"
#include "genustypes.h"
#include "headernode.h"
#include "location.h"
#include "utilities.h"
#include "propertynode.h"
#include "qdocdatabase.h"
#include "qmlpropertynode.h"
#include "typedefnode.h"
#include "variablenode.h"

#include <QtCore/qxmlstream.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

enum QDocAttr {
    QDocAttrNone,
    QDocAttrExample,
    QDocAttrFile,
    QDocAttrImage,
    QDocAttrDocument,
    QDocAttrExternalPage,
    QDocAttrAttribution
};

static Node *root_ = nullptr;
static IndexSectionWriter *post_ = nullptr;

/*!
  \class QDocIndexFiles

  This class handles qdoc index files.
 */

QDocIndexFiles *QDocIndexFiles::s_qdocIndexFiles = nullptr;

/*!
  Constructs the singleton QDocIndexFiles.
 */
QDocIndexFiles::QDocIndexFiles() : m_gen(nullptr)
{
    m_qdb = QDocDatabase::qdocDB();
    m_storeLocationInfo = Config::instance().get(CONFIG_LOCATIONINFO).asBool();
}

/*!
  Destroys the singleton QDocIndexFiles.
 */
QDocIndexFiles::~QDocIndexFiles()
{
    m_qdb = nullptr;
    m_gen = nullptr;
}

/*!
  Creates the singleton. Allows only one instance of the class
  to be created. Returns a pointer to the singleton.
 */
QDocIndexFiles *QDocIndexFiles::qdocIndexFiles()
{
    if (s_qdocIndexFiles == nullptr)
        s_qdocIndexFiles = new QDocIndexFiles;
    return s_qdocIndexFiles;
}

/*!
  Destroys the singleton.
 */
void QDocIndexFiles::destroyQDocIndexFiles()
{
    if (s_qdocIndexFiles != nullptr) {
        delete s_qdocIndexFiles;
        s_qdocIndexFiles = nullptr;
    }
}

/*!
  Reads and parses the list of index files in \a indexFiles.
 */
void QDocIndexFiles::readIndexes(const QStringList &indexFiles)
{
    for (const QString &file : indexFiles) {
        qCDebug(lcQdoc) << "Loading index file: " << file;
        readIndexFile(file);
    }
}

/*!
  Reads and parses the index file at \a path.
 */
void QDocIndexFiles::readIndexFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Could not read index file" << path;
        return;
    }

    QXmlStreamReader reader(&file);
    reader.setNamespaceProcessing(false);

    if (!reader.readNextStartElement())
        return;

    if (reader.name() != QLatin1String("INDEX"))
        return;

    QXmlStreamAttributes attrs = reader.attributes();

    QString indexUrl {attrs.value(QLatin1String("url")).toString()};

    // Decide how we link to nodes loaded from this index file:
    // If building a set that will be installed AND the URL of
    // the dependency is identical to ours, assume that also
    // the dependent html files are available under the same
    // directory tree. Otherwise, link using the full index URL.
    if (!Config::installDir.isEmpty() && indexUrl == Config::instance().get(CONFIG_URL).asString()) {
        // Generate a relative URL between the install dir and the index file
        // when the -installdir command line option is set.
        QDir installDir(path.section('/', 0, -3) + '/' + Generator::outputSubdir());
        indexUrl = installDir.relativeFilePath(path).section('/', 0, -2);
    }
    m_project = attrs.value(QLatin1String("project")).toString();
    QString indexTitle = attrs.value(QLatin1String("indexTitle")).toString();
    m_basesList.clear();
    m_relatedNodes.clear();

    NamespaceNode *root = m_qdb->newIndexTree(m_project);
    if (!root) {
        qWarning() << "Issue parsing index tree" << path;
        return;
    }

    root->tree()->setIndexTitle(indexTitle);

    // Scan all elements in the XML file, constructing a map that contains
    // base classes for each class found.
    while (reader.readNextStartElement()) {
        readIndexSection(reader, root, indexUrl);
    }

    // Now that all the base classes have been found for this index,
    // arrange them into an inheritance hierarchy.
    resolveIndex();
}

/*!
  Read a <section> element from the index file and create the
  appropriate node(s).
 */
void QDocIndexFiles::readIndexSection(QXmlStreamReader &reader, Node *current,
                                      const QString &indexUrl)
{
    QXmlStreamAttributes attributes = reader.attributes();
    QStringView elementName = reader.name();

    QString name = attributes.value(QLatin1String("name")).toString();
    QString href = attributes.value(QLatin1String("href")).toString();
    Node *node;
    Location location;
    Aggregate *parent = nullptr;
    bool hasReadChildren = false;

    if (current->isAggregate())
        parent = static_cast<Aggregate *>(current);

    if (attributes.hasAttribute(QLatin1String("related"))) {
        bool isIntTypeRelatedValue = false;
        int relatedIndex = attributes.value(QLatin1String("related")).toInt(&isIntTypeRelatedValue);
        if (isIntTypeRelatedValue) {
            if (adoptRelatedNode(parent, relatedIndex)) {
                reader.skipCurrentElement();
                return;
            }
        } else {
            QList<Node *>::iterator nodeIterator =
                    std::find_if(m_relatedNodes.begin(), m_relatedNodes.end(), [&](const Node *relatedNode) {
                        return (name == relatedNode->name() &&  href == relatedNode->url().section(QLatin1Char('/'), -1));
                    });

            if (nodeIterator != m_relatedNodes.end() && parent) {
                parent->adoptChild(*nodeIterator);
                reader.skipCurrentElement();
                return;
            }
        }
    }

    QString filePath;
    int lineNo = 0;
    if (attributes.hasAttribute(QLatin1String("filepath"))) {
        filePath = attributes.value(QLatin1String("filepath")).toString();
        lineNo = attributes.value("lineno").toInt();
    }
    if (elementName == QLatin1String("namespace")) {
        auto *namespaceNode = new NamespaceNode(parent, name);
        node = namespaceNode;
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name.toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(name.toLower() + ".html");
    } else if (elementName == QLatin1String("class") || elementName == QLatin1String("struct")
               || elementName == QLatin1String("union")) {
        NodeType type = NodeType::Class;
        if (elementName == QLatin1String("class"))
            type = NodeType::Class;
        else if (elementName == QLatin1String("struct"))
            type = NodeType::Struct;
        else if (elementName == QLatin1String("union"))
            type = NodeType::Union;
        node = new ClassNode(type, parent, name);
        if (attributes.hasAttribute(QLatin1String("bases"))) {
            QString bases = attributes.value(QLatin1String("bases")).toString();
            if (!bases.isEmpty())
                m_basesList.append(
                        std::pair<ClassNode *, QString>(static_cast<ClassNode *>(node), bases));
        }
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name.toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(name.toLower() + ".html");
        bool abstract = false;
        if (attributes.value(QLatin1String("abstract")) == QLatin1String("true"))
            abstract = true;
        node->setAbstract(abstract);
    } else if (elementName == QLatin1String("header")) {
        node = new HeaderNode(parent, name);

        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value(QLatin1String("location")).toString();

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);
    } else if (parent && ((elementName == QLatin1String("qmlclass") || elementName == QLatin1String("qmlvaluetype")
                           || elementName == QLatin1String("qmlbasictype")))) {
        auto *qmlTypeNode = new QmlTypeNode(parent, name,
                    elementName == QLatin1String("qmlclass") ? NodeType::QmlType : NodeType::QmlValueType);
        qmlTypeNode->setTitle(attributes.value(QLatin1String("title")).toString());
        QString logicalModuleName = attributes.value(QLatin1String("qml-module-name")).toString();
        if (!logicalModuleName.isEmpty())
            m_qdb->addToQmlModule(logicalModuleName, qmlTypeNode);
        bool abstract = false;
        if (attributes.value(QLatin1String("abstract")) == QLatin1String("true"))
            abstract = true;
        qmlTypeNode->setAbstract(abstract);
        QString qmlFullBaseName = attributes.value(QLatin1String("qml-base-type")).toString();
        if (!qmlFullBaseName.isEmpty()) {
            qmlTypeNode->setQmlBaseName(qmlFullBaseName);
        }
        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value("location").toString();
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);
        node = qmlTypeNode;
    } else if (parent && elementName == QLatin1String("qmlproperty")) {
        QString type = attributes.value(QLatin1String("type")).toString();
        bool attached = false;
        if (attributes.value(QLatin1String("attached")) == QLatin1String("true"))
            attached = true;
        bool readonly = false;
        if (attributes.value(QLatin1String("writable")) == QLatin1String("false"))
            readonly = true;
        auto *qmlPropertyNode = new QmlPropertyNode(parent, name, std::move(type), attached);
        qmlPropertyNode->markReadOnly(readonly);
        if (attributes.value(QLatin1String("required")) == QLatin1String("true"))
            qmlPropertyNode->setRequired();
        node = qmlPropertyNode;
    } else if (elementName == QLatin1String("group")) {
        auto *collectionNode = m_qdb->addGroup(name);
        collectionNode->setTitle(attributes.value(QLatin1String("title")).toString());
        collectionNode->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            collectionNode->markSeen();
        node = collectionNode;
    } else if (elementName == QLatin1String("module")) {
        auto *collectionNode = m_qdb->addModule(name);
        collectionNode->setTitle(attributes.value(QLatin1String("title")).toString());
        collectionNode->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            collectionNode->markSeen();
        node = collectionNode;
    } else if (elementName == QLatin1String("qmlmodule")) {
        auto *collectionNode = m_qdb->addQmlModule(name);
        const QStringList info = QStringList()
                << name
                << QString(attributes.value(QLatin1String("qml-module-version")).toString());
        collectionNode->setLogicalModuleInfo(info);
        collectionNode->setTitle(attributes.value(QLatin1String("title")).toString());
        collectionNode->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            collectionNode->markSeen();
        node = collectionNode;
    } else if (elementName == QLatin1String("page")) {
        QDocAttr subtype = QDocAttrNone;
        QString attr = attributes.value(QLatin1String("subtype")).toString();
        if (attr == QLatin1String("attribution")) {
            subtype = QDocAttrAttribution;
        } else if (attr == QLatin1String("example")) {
            subtype = QDocAttrExample;
        } else if (attr == QLatin1String("file")) {
            subtype = QDocAttrFile;
        } else if (attr == QLatin1String("image")) {
            subtype = QDocAttrImage;
        } else if (attr == QLatin1String("page")) {
            subtype = QDocAttrDocument;
        } else if (attr == QLatin1String("externalpage")) {
            subtype = QDocAttrExternalPage;
        } else
            goto done;

        if (current->isExample()) {
            auto *exampleNode = static_cast<ExampleNode *>(current);
            if (subtype == QDocAttrFile) {
                exampleNode->appendFile(name);
                goto done;
            } else if (subtype == QDocAttrImage) {
                exampleNode->appendImage(name);
                goto done;
            }
        }
        PageNode *pageNode = nullptr;
        if (subtype == QDocAttrExample)
            pageNode = new ExampleNode(parent, name);
        else if (subtype == QDocAttrExternalPage)
            pageNode = new ExternalPageNode(parent, name);
        else {
            pageNode = new PageNode(parent, name);
            if (subtype == QDocAttrAttribution) pageNode->markAttribution();
        }

        pageNode->setTitle(attributes.value(QLatin1String("title")).toString());

        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value(QLatin1String("location")).toString();

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);

        node = pageNode;

    } else if (elementName == QLatin1String("enum") || elementName == QLatin1String("qmlenum")) {
        EnumNode *enumNode;
        if (elementName == QLatin1String("enum"))
            enumNode = new EnumNode(parent, name, attributes.hasAttribute("scoped"));
        else
            enumNode = new QmlEnumNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

        while (reader.readNextStartElement()) {
            QXmlStreamAttributes childAttributes = reader.attributes();
            if (reader.name() == QLatin1String("value")) {
                EnumItem item(childAttributes.value(QLatin1String("name")).toString(),
                              childAttributes.value(QLatin1String("value")).toString(),
                              childAttributes.value(QLatin1String("since")).toString()
                              );
                enumNode->addItem(item);
            } else if (reader.name() == QLatin1String("keyword")) {
                insertTarget(TargetRec::Keyword, childAttributes, enumNode);
            } else if (reader.name() == QLatin1String("target")) {
                insertTarget(TargetRec::Target, childAttributes, enumNode);
            }
            reader.skipCurrentElement();
        }

        node = enumNode;

        hasReadChildren = true;
    } else if (elementName == QLatin1String("typedef")) {
        if (attributes.hasAttribute("aliasedtype"))
            node = new TypeAliasNode(parent, name, attributes.value(QLatin1String("aliasedtype")).toString());
        else
            node = new TypedefNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");
    } else if (elementName == QLatin1String("property")) {
        auto *propNode = new PropertyNode(parent, name);
        node = propNode;
        if (attributes.value(QLatin1String("bindable")) == QLatin1String("true"))
            propNode->setPropertyType(PropertyNode::PropertyType::BindableProperty);

        propNode->setWritable(attributes.value(QLatin1String("writable")) != QLatin1String("false"));

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (elementName == QLatin1String("function")) {
        QString t = attributes.value(QLatin1String("meta")).toString();
        bool attached = false;
        FunctionNode::Metaness metaness = FunctionNode::Plain;
        if (!t.isEmpty())
            metaness = FunctionNode::getMetaness(t);
        if (attributes.value(QLatin1String("attached")) == QLatin1String("true"))
            attached = true;
        auto *fn = new FunctionNode(metaness, parent, name, attached);

        fn->setReturnType(attributes.value(QLatin1String("type")).toString());

        const auto &declaredTypeAttr = attributes.value(QLatin1String("declaredtype"));
        if (!declaredTypeAttr.isEmpty())
            fn->setDeclaredReturnType(declaredTypeAttr.toString());

        if (fn->isCppNode()) {
            fn->setVirtualness(attributes.value(QLatin1String("virtual")).toString());
            fn->setConst(attributes.value(QLatin1String("const")) == QLatin1String("true"));
            fn->setStatic(attributes.value(QLatin1String("static")) == QLatin1String("true"));
            fn->setFinal(attributes.value(QLatin1String("final")) == QLatin1String("true"));
            fn->setOverride(attributes.value(QLatin1String("override")) == QLatin1String("true"));

            if (attributes.value(QLatin1String("explicit")) == QLatin1String("true"))
                fn->markExplicit();

            if (attributes.value(QLatin1String("constexpr")) == QLatin1String("true"))
                fn->markConstexpr();

            if (attributes.value(QLatin1String("noexcept")) == QLatin1String("true")) {
                fn->markNoexcept(attributes.value("noexcept_expression").toString());
            }

            qsizetype refness = attributes.value(QLatin1String("refness")).toUInt();
            if (refness == 1)
                fn->setRef(true);
            else if (refness == 2)
                fn->setRefRef(true);
            /*
              Theoretically, this should ensure that each function
              node receives the same overload number and overload
              flag it was written with, and it should be unnecessary
              to call normalizeOverloads() for index nodes.
             */
            if (attributes.value(QLatin1String("overload")) == QLatin1String("true"))
                fn->setOverloadNumber(attributes.value(QLatin1String("overload-number")).toUInt());
            else
                fn->setOverloadNumber(0);
        }

        /*
            Note: The "signature" attribute was written to the
            index file, but it is not read back in. That is ok
            because we reconstruct the parameter list and the
            return type, from which the signature was built in
            the first place and from which it can be rebuilt.
        */
        while (reader.readNextStartElement()) {
            QXmlStreamAttributes childAttributes = reader.attributes();
            if (reader.name() == QLatin1String("parameter")) {
                // Do not use the default value for the parameter; it is not
                // required, and has been known to cause problems.
                QString type = childAttributes.value(QLatin1String("type")).toString();
                QString name = childAttributes.value(QLatin1String("name")).toString();
                fn->parameters().append(type, name);
            } else if (reader.name() == QLatin1String("keyword")) {
                insertTarget(TargetRec::Keyword, childAttributes, fn);
            } else if (reader.name() == QLatin1String("target")) {
                insertTarget(TargetRec::Target, childAttributes, fn);
            }
            reader.skipCurrentElement();
        }

        node = fn;
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

        hasReadChildren = true;
    } else if (elementName == QLatin1String("variable")) {
        auto *varNode = new VariableNode(parent, name);
        varNode->setLeftType(attributes.value("type").toString());
        varNode->setStatic((attributes.value("static").toString() == "true") ? true : false);
        node = varNode;
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");
    } else if (elementName == QLatin1String("keyword")) {
        insertTarget(TargetRec::Keyword, attributes, current);
        goto done;
    } else if (elementName == QLatin1String("target")) {
        insertTarget(TargetRec::Target, attributes, current);
        goto done;
    } else if (elementName == QLatin1String("contents")) {
        insertTarget(TargetRec::Contents, attributes, current);
        goto done;
    } else if (elementName == QLatin1String("proxy")) {
        node = new ProxyNode(parent, name);
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name.toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(name.toLower() + ".html");
    } else {
        goto done;
    }

    {
        if (!href.isEmpty()) {
            node->setUrl(href);
            // Include the index URL if it exists
            if (!node->isExternalPage() && !indexUrl.isEmpty())
                node->setUrl(indexUrl + QLatin1Char('/') + href);
        }

        const QString access = attributes.value(QLatin1String("access")).toString();
        if (access == "protected")
            node->setAccess(Access::Protected);
        else if ((access == "private") || (access == "internal"))
            node->setAccess(Access::Private);
        else
            node->setAccess(Access::Public);

        if (attributes.hasAttribute(QLatin1String("related"))) {
            node->setRelatedNonmember(true);
            m_relatedNodes << node;
        }

        if (attributes.hasAttribute(QLatin1String("threadsafety"))) {
            QString threadSafety = attributes.value(QLatin1String("threadsafety")).toString();
            if (threadSafety == QLatin1String("non-reentrant"))
                node->setThreadSafeness(Node::NonReentrant);
            else if (threadSafety == QLatin1String("reentrant"))
                node->setThreadSafeness(Node::Reentrant);
            else if (threadSafety == QLatin1String("thread safe"))
                node->setThreadSafeness(Node::ThreadSafe);
            else
                node->setThreadSafeness(Node::UnspecifiedSafeness);
        } else
            node->setThreadSafeness(Node::UnspecifiedSafeness);

        const QString category = attributes.value(QLatin1String("comparison_category")).toString();
        node->setComparisonCategory(comparisonCategoryFromString(category.toStdString()));

        QString status = attributes.value(QLatin1String("status")).toString();
        // TODO: "obsolete" is kept for backward compatibility, remove in the near future
        if (status == QLatin1String("obsolete") || status == QLatin1String("deprecated"))
            node->setStatus(Node::Deprecated);
        else if (status == QLatin1String("preliminary"))
            node->setStatus(Node::Preliminary);
        else if (status == QLatin1String("internal"))
            node->setStatus(Node::Internal);
        else if (status == QLatin1String("ignored"))
            node->setStatus(Node::DontDocument);
        else
            node->setStatus(Node::Active);

        QString physicalModuleName = attributes.value(QLatin1String("module")).toString();
        if (!physicalModuleName.isEmpty())
            m_qdb->addToModule(physicalModuleName, node);

        QString since = attributes.value(QLatin1String("since")).toString();
        if (!since.isEmpty()) {
            node->setSince(since);
        }

        if (attributes.hasAttribute(QLatin1String("documented"))) {
            if (attributes.value(QLatin1String("documented")) == QLatin1String("true"))
                node->setHadDoc();
        }

        QString groupsAttr = attributes.value(QLatin1String("groups")).toString();
        if (!groupsAttr.isEmpty()) {
            const QStringList groupNames = groupsAttr.split(QLatin1Char(','));
            for (const auto &group : groupNames) {
                m_qdb->addToGroup(group, node);
            }
        }

        // Create some content for the node.
        QSet<QString> emptySet;
        Location t(filePath);
        if (!filePath.isEmpty()) {
            t.setLineNo(lineNo);
            node->setLocation(t);
            location = t;
        }
        Doc doc(location, location, QString(), emptySet, emptySet); // placeholder
        node->setDoc(doc);
        node->setIndexNodeFlag(); // Important: This node came from an index file.
        QString briefAttr = attributes.value(QLatin1String("brief")).toString();
        if (!briefAttr.isEmpty()) {
            node->setReconstitutedBrief(briefAttr);
        }

        if (const auto sortKey = attributes.value(QLatin1String("sortkey")).toString(); !sortKey.isEmpty()) {
            node->doc().constructExtra();
            if (auto *metaMap = node->doc().metaTagMap())
                metaMap->insert("sortkey", sortKey);
        }
        if (!hasReadChildren) {
            bool useParent = (elementName == QLatin1String("namespace") && name.isEmpty());
            while (reader.readNextStartElement()) {
                if (useParent)
                    readIndexSection(reader, parent, indexUrl);
                else
                    readIndexSection(reader, node, indexUrl);
            }
        }
    }

done:
    while (!reader.isEndElement()) {
        if (reader.readNext() == QXmlStreamReader::Invalid) {
            break;
        }
    }
}

void QDocIndexFiles::insertTarget(TargetRec::TargetType type,
                                  const QXmlStreamAttributes &attributes, Node *node)
{
    int priority;
    switch (type) {
    case TargetRec::Keyword:
        priority = 1;
        break;
    case TargetRec::Target:
        priority = 2;
        break;
    case TargetRec::Contents:
        priority = 3;
        break;
    default:
        return;
    }

    QString name = attributes.value(QLatin1String("name")).toString();
    QString title = attributes.value(QLatin1String("title")).toString();
    m_qdb->insertTarget(name, title, type, node, priority);
}

/*!
  This function tries to resolve class inheritance immediately
  after the index file is read. It is not always possible to
  resolve a class inheritance at this point, because the base
  class might be in an index file that hasn't been read yet, or
  it might be in one of the header files that will be read for
  the current module. These cases will be resolved after all
  the index files and header and source files have been read,
  just prior to beginning the generate phase for the current
  module.

  I don't think this is completely correct because it always
  sets the access to public.
 */
void QDocIndexFiles::resolveIndex()
{
    for (const auto &pair : std::as_const(m_basesList)) {
        const QStringList bases = pair.second.split(QLatin1Char(','));
        for (const auto &base : bases) {
            QStringList basePath = base.split(QString("::"));
            Node *n = m_qdb->findClassNode(basePath);
            if (n)
                pair.first->addResolvedBaseClass(Access::Public, static_cast<ClassNode *>(n));
            else
                pair.first->addUnresolvedBaseClass(Access::Public, basePath);
        }
    }
    // No longer needed.
    m_basesList.clear();
}

static QString getAccessString(Access t)
{

    switch (t) {
    case Access::Public:
        return QLatin1String("public");
    case Access::Protected:
        return QLatin1String("protected");
    case Access::Private:
        return QLatin1String("private");
    default:
        break;
    }
    return QLatin1String("public");
}

static QString getStatusString(Node::Status t)
{
    switch (t) {
    case Node::Deprecated:
        return QLatin1String("deprecated");
    case Node::Preliminary:
        return QLatin1String("preliminary");
    case Node::Active:
        return QLatin1String("active");
    case Node::Internal:
        return QLatin1String("internal");
    case Node::DontDocument:
        return QLatin1String("ignored");
    default:
        break;
    }
    return QLatin1String("active");
}

static QString getThreadSafenessString(Node::ThreadSafeness t)
{
    switch (t) {
    case Node::NonReentrant:
        return QLatin1String("non-reentrant");
    case Node::Reentrant:
        return QLatin1String("reentrant");
    case Node::ThreadSafe:
        return QLatin1String("thread safe");
    case Node::UnspecifiedSafeness:
    default:
        break;
    }
    return QLatin1String("unspecified");
}

/*!
    Returns the index of \a node in the list of related non-member nodes.
*/
int QDocIndexFiles::indexForNode(Node *node)
{
    qsizetype i = m_relatedNodes.indexOf(node);
    if (i == -1) {
        i = m_relatedNodes.size();
        m_relatedNodes << node;
    }
    return i;
}

/*!
    Adopts the related non-member node identified by \a index to the
    parent \a adoptiveParent. Returns \c true if successful.
*/
bool QDocIndexFiles::adoptRelatedNode(Aggregate *adoptiveParent, int index)
{
    Node *related = m_relatedNodes.value(index);

    if (adoptiveParent && related) {
        adoptiveParent->adoptChild(related);
        return true;
    }

    return false;
}

/*!
    Write canonicalized versions of \\target and \\keyword identifiers
    that appear in the documentation of \a node into the index using
    \a writer, so that they can be used as link targets in external
    documentation sets.
*/
void QDocIndexFiles::writeTargets(QXmlStreamWriter &writer, Node *node)
{
    if (node->doc().hasTargets()) {
        for (const Atom *target : std::as_const(node->doc().targets())) {
            const QString &title = target->string();
            const QString &name{Utilities::asAsciiPrintable(title)};
            writer.writeStartElement("target");
            writer.writeAttribute("name", node->isExternalPage() ? title : name);
            if (name != title)
                writer.writeAttribute("title", title);
            writer.writeEndElement(); // target
        }
    }
    if (node->doc().hasKeywords()) {
        for (const Atom *keyword : std::as_const(node->doc().keywords())) {
            const QString &title = keyword->string();
            const QString &name{Utilities::asAsciiPrintable(title)};
            writer.writeStartElement("keyword");
            writer.writeAttribute("name", name);
            if (name != title)
                writer.writeAttribute("title", title);
            writer.writeEndElement(); // keyword
        }
    }
}

/*!
  Generate the index section with the given \a writer for the \a node
  specified, returning true if an element was written, and returning
  false if an element is not written.

  \note Function nodes are processed in generateFunctionSection()
 */
bool QDocIndexFiles::generateIndexSection(QXmlStreamWriter &writer, Node *node,
                                          IndexSectionWriter *post)
{
    if (m_gen == nullptr)
        m_gen = Generator::currentGenerator();

    Q_ASSERT(m_gen);

    post_ = nullptr;
    /*
      Don't include index nodes in a new index file.
     */
    if (node->isIndexNode())
        return false;

    QString nodeName;
    QString logicalModuleName;
    QString logicalModuleVersion;
    QString qmlFullBaseName;
    QString baseNameAttr;
    QString moduleNameAttr;
    QString moduleVerAttr;

    switch (node->nodeType()) {
    case NodeType::Namespace:
        nodeName = "namespace";
        break;
    case NodeType::Class:
        nodeName = "class";
        break;
    case NodeType::Struct:
        nodeName = "struct";
        break;
    case NodeType::Union:
        nodeName = "union";
        break;
    case NodeType::HeaderFile:
        nodeName = "header";
        break;
    case NodeType::QmlEnum:
        nodeName = "qmlenum";
        break;
    case NodeType::QmlType:
    case NodeType::QmlValueType:
        nodeName = (node->nodeType() == NodeType::QmlType) ? "qmlclass" : "qmlvaluetype";
        logicalModuleName = node->logicalModuleName();
        baseNameAttr = "qml-base-type";
        moduleNameAttr = "qml-module-name";
        moduleVerAttr = "qml-module-version";
        qmlFullBaseName = node->qmlFullBaseName();
        break;
    case NodeType::Page:
    case NodeType::Example:
    case NodeType::ExternalPage:
        nodeName = "page";
        break;
    case NodeType::Group:
        nodeName = "group";
        break;
    case NodeType::Module:
        nodeName = "module";
        break;
    case NodeType::QmlModule:
        nodeName = "qmlmodule";
        moduleNameAttr = "qml-module-name";
        moduleVerAttr = "qml-module-version";
        logicalModuleName = node->logicalModuleName();
        logicalModuleVersion = node->logicalModuleVersion();
        break;
    case NodeType::Enum:
        nodeName = "enum";
        break;
    case NodeType::TypeAlias:
    case NodeType::Typedef:
        nodeName = "typedef";
        break;
    case NodeType::Property:
        nodeName = "property";
        break;
    case NodeType::Variable:
        nodeName = "variable";
        break;
    case NodeType::SharedComment:
        if (!node->isPropertyGroup())
            return false;
        // Add an entry for property groups so that they can be linked to
        nodeName = "qmlproperty";
        break;
    case NodeType::QmlProperty:
        nodeName = "qmlproperty";
        break;
    case NodeType::Proxy:
        nodeName = "proxy";
        break;
    case NodeType::Function: // Now processed in generateFunctionSection()
    default:
        return false;
    }

    QString objName = node->name();
    // Special case: only the root node should have an empty name.
    if (objName.isEmpty() && node != m_qdb->primaryTreeRoot())
        return false;

    writer.writeStartElement(nodeName);

    if (!node->isTextPageNode() && !node->isCollectionNode() && !node->isHeader()) {
        if (node->threadSafeness() != Node::UnspecifiedSafeness)
            writer.writeAttribute("threadsafety", getThreadSafenessString(node->threadSafeness()));
    }

    writer.writeAttribute("name", objName);

    // Write module and base type info for QML types
    if (!moduleNameAttr.isEmpty()) {
        if (!logicalModuleName.isEmpty())
            writer.writeAttribute(moduleNameAttr, logicalModuleName);
        if (!logicalModuleVersion.isEmpty())
            writer.writeAttribute(moduleVerAttr, logicalModuleVersion);
    }
    if (!baseNameAttr.isEmpty() && !qmlFullBaseName.isEmpty())
        writer.writeAttribute(baseNameAttr, qmlFullBaseName);

    QString href;
    if (!node->isExternalPage()) {
        QString fullName = node->fullDocumentName();
        if (fullName != objName)
            writer.writeAttribute("fullname", fullName);
        href = m_gen->fullDocumentLocation(node);
    } else
        href = node->name();
    if (node->isQmlNode()) {
        Aggregate *p = node->parent();
        if (p && p->isQmlType() && p->isAbstract())
            href.clear();
    }
    if (!href.isEmpty())
        writer.writeAttribute("href", href);

    writer.writeAttribute("status", getStatusString(node->status()));
    if (!node->isTextPageNode() && !node->isCollectionNode() && !node->isHeader()) {
        writer.writeAttribute("access", getAccessString(node->access()));
        if (node->isAbstract())
            writer.writeAttribute("abstract", "true");
    }
    const Location &declLocation = node->declLocation();
    if (!declLocation.fileName().isEmpty())
        writer.writeAttribute("location", declLocation.fileName());
    if (m_storeLocationInfo && !declLocation.filePath().isEmpty()) {
        writer.writeAttribute("filepath", declLocation.filePath());
        writer.writeAttribute("lineno", QString("%1").arg(declLocation.lineNo()));
    }

    if (node->isRelatedNonmember())
        writer.writeAttribute("related", QString::number(indexForNode(node)));

    if (!node->since().isEmpty())
        writer.writeAttribute("since", node->since());

    if (node->hasDoc())
        writer.writeAttribute("documented", "true");

    QStringList groups = m_qdb->groupNamesForNode(node);
    if (!groups.isEmpty())
        writer.writeAttribute("groups", groups.join(QLatin1Char(',')));

   if (const auto *metamap = node->doc().metaTagMap(); metamap)
        if (const auto sortKey = metamap->value("sortkey"); !sortKey.isEmpty())
            writer.writeAttribute("sortkey", sortKey);

    QString brief = node->doc().trimmedBriefText(node->name()).toString();
    switch (node->nodeType()) {
    case NodeType::Class:
    case NodeType::Struct:
    case NodeType::Union: {
        // Classes contain information about their base classes.
        const auto *classNode = static_cast<const ClassNode *>(node);
        const QList<RelatedClass> &bases = classNode->baseClasses();
        QSet<QString> baseStrings;
        for (const auto &related : bases) {
            ClassNode *n = related.m_node;
            if (n)
                baseStrings.insert(n->fullName());
            else if (!related.m_path.isEmpty())
                baseStrings.insert(related.m_path.join(QLatin1String("::")));
        }
        if (!baseStrings.isEmpty()) {
            QStringList baseStringsAsList = baseStrings.values();
            baseStringsAsList.sort();
            writer.writeAttribute("bases", baseStringsAsList.join(QLatin1Char(',')));
        }
        if (!node->physicalModuleName().isEmpty())
            writer.writeAttribute("module", node->physicalModuleName());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
        if (auto category = node->comparisonCategory(); category != ComparisonCategory::None)
            writer.writeAttribute("comparison_category", comparisonCategoryAsString(category));
    } break;
    case NodeType::HeaderFile: {
        const auto *headerNode = static_cast<const HeaderNode *>(node);
        if (!headerNode->physicalModuleName().isEmpty())
            writer.writeAttribute("module", headerNode->physicalModuleName());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
        writer.writeAttribute("title", headerNode->title());
        writer.writeAttribute("fulltitle", headerNode->fullTitle());
        writer.writeAttribute("subtitle", headerNode->subtitle());
    } break;
    case NodeType::Namespace: {
        const auto *namespaceNode = static_cast<const NamespaceNode *>(node);
        if (!namespaceNode->physicalModuleName().isEmpty())
            writer.writeAttribute("module", namespaceNode->physicalModuleName());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case NodeType::QmlValueType:
    case NodeType::QmlType: {
        const auto *qmlTypeNode = static_cast<const QmlTypeNode *>(node);
        writer.writeAttribute("title", qmlTypeNode->title());
        writer.writeAttribute("fulltitle", qmlTypeNode->fullTitle());
        writer.writeAttribute("subtitle", qmlTypeNode->subtitle());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case NodeType::Page:
    case NodeType::Example:
    case NodeType::ExternalPage: {
        if (node->isExample())
            writer.writeAttribute("subtype", "example");
        else if (node->isExternalPage())
            writer.writeAttribute("subtype", "externalpage");
        else
            writer.writeAttribute("subtype", (static_cast<PageNode*>(node)->isAttribution() ? "attribution" : "page"));

        const auto *pageNode = static_cast<const PageNode *>(node);
        writer.writeAttribute("title", pageNode->title());
        writer.writeAttribute("fulltitle", pageNode->fullTitle());
        writer.writeAttribute("subtitle", pageNode->subtitle());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case NodeType::Group:
    case NodeType::Module:
    case NodeType::QmlModule: {
        const auto *collectionNode = static_cast<const CollectionNode *>(node);
        writer.writeAttribute("seen", collectionNode->wasSeen() ? "true" : "false");
        writer.writeAttribute("title", collectionNode->title());
        if (!collectionNode->subtitle().isEmpty())
            writer.writeAttribute("subtitle", collectionNode->subtitle());
        if (!collectionNode->physicalModuleName().isEmpty())
            writer.writeAttribute("module", collectionNode->physicalModuleName());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case NodeType::QmlProperty: {
        auto *qmlPropertyNode = static_cast<QmlPropertyNode *>(node);
        writer.writeAttribute("type", qmlPropertyNode->dataType());
        writer.writeAttribute("attached", qmlPropertyNode->isAttached() ? "true" : "false");
        writer.writeAttribute("writable", qmlPropertyNode->isReadOnly() ? "false" : "true");
        if (qmlPropertyNode->isRequired())
            writer.writeAttribute("required", "true");
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case NodeType::Property: {
        const auto *propertyNode = static_cast<const PropertyNode *>(node);

        if (propertyNode->propertyType() == PropertyNode::PropertyType::BindableProperty)
            writer.writeAttribute("bindable", "true");

        if (!propertyNode->isWritable())
            writer.writeAttribute("writable", "false");

        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
        // Property access function names
        for (qsizetype i{0}; i < (qsizetype)PropertyNode::FunctionRole::NumFunctionRoles; ++i) {
            auto role{(PropertyNode::FunctionRole)i};
            for (const auto *fnNode : propertyNode->functions(role)) {
                writer.writeStartElement(PropertyNode::roleName(role));
                writer.writeAttribute("name", fnNode->name());
                writer.writeEndElement();
            }
        }
    } break;
    case NodeType::Variable: {
        const auto *variableNode = static_cast<const VariableNode *>(node);
        writer.writeAttribute("type", variableNode->dataType());
        writer.writeAttribute("static", variableNode->isStatic() ? "true" : "false");
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case NodeType::QmlEnum:
    case NodeType::Enum: {
        const auto *enumNode = static_cast<const EnumNode *>(node);
        if (enumNode->isScoped())
            writer.writeAttribute("scoped", "true");
        if (enumNode->flagsType())
            writer.writeAttribute("typedef", enumNode->flagsType()->fullDocumentName());
        const auto &items = enumNode->items();
        for (const auto &item : items) {
            writer.writeStartElement("value");
            writer.writeAttribute("name", item.name());
            if (node->isEnumType(Genus::CPP))
                writer.writeAttribute("value", item.value());
            if (!item.since().isEmpty())
                writer.writeAttribute("since", item.since());
            writer.writeEndElement(); // value
        }
    } break;
    case NodeType::Typedef: {
        const auto *typedefNode = static_cast<const TypedefNode *>(node);
        if (typedefNode->associatedEnum())
            writer.writeAttribute("enum", typedefNode->associatedEnum()->fullDocumentName());
    } break;
    case NodeType::TypeAlias:
        writer.writeAttribute("aliasedtype", static_cast<const TypeAliasNode *>(node)->aliasedType());
        break;
    case NodeType::Function: // Now processed in generateFunctionSection()
    default:
        break;
    }

    writeTargets(writer, node);

    /*
      Some nodes have a table of contents. For these, we close
      the opening tag, create sub-elements for the items in the
      table of contents, and then add a closing tag for the
      element. Elements for all other nodes are closed in the
      opening tag.
    */
    if (node->isPageNode() || node->isCollectionNode()) {
        if (node->doc().hasTableOfContents()) {
            for (int i = 0; i < node->doc().tableOfContents().size(); ++i) {
                Atom *item = node->doc().tableOfContents()[i];
                int level = node->doc().tableOfContentsLevels()[i];
                QString title = Text::sectionHeading(item).toString();
                writer.writeStartElement("contents");
                writer.writeAttribute("name", Tree::refForAtom(item));
                writer.writeAttribute("title", title);
                writer.writeAttribute("level", QString::number(level));
                writer.writeEndElement(); // contents
            }
        }
    }
    // WebXMLGenerator - skip the nested <page> elements for example
    // files/images, as the generator produces them separately
    if (node->isExample() && m_gen->format() != QLatin1String("WebXML")) {
        const auto *exampleNode = static_cast<const ExampleNode *>(node);
        const auto &files = exampleNode->files();
        for (const QString &file : files) {
            writer.writeStartElement("page");
            writer.writeAttribute("name", file);
            QString href = m_gen->linkForExampleFile(file);
            writer.writeAttribute("href", href);
            writer.writeAttribute("status", "active");
            writer.writeAttribute("subtype", "file");
            writer.writeAttribute("title", "");
            writer.writeAttribute("fulltitle", Generator::exampleFileTitle(exampleNode, file));
            writer.writeAttribute("subtitle", file);
            writer.writeEndElement(); // page
        }
        const auto &images = exampleNode->images();
        for (const QString &file : images) {
            writer.writeStartElement("page");
            writer.writeAttribute("name", file);
            QString href = m_gen->linkForExampleFile(file);
            writer.writeAttribute("href", href);
            writer.writeAttribute("status", "active");
            writer.writeAttribute("subtype", "image");
            writer.writeAttribute("title", "");
            writer.writeAttribute("fulltitle", Generator::exampleFileTitle(exampleNode, file));
            writer.writeAttribute("subtitle", file);
            writer.writeEndElement(); // page
        }
    }
    // Append to the section if the callback object was set
    if (post)
        post->append(writer, node);

    post_ = post;
    return true;
}

/*!
  This function writes a <function> element for \a fn to the
  index file using \a writer.
 */
void QDocIndexFiles::generateFunctionSection(QXmlStreamWriter &writer, FunctionNode *fn)
{
    if (fn->isInternal() && !Config::instance().showInternal())
        return;

    const QString objName = fn->name();
    writer.writeStartElement("function");
    writer.writeAttribute("name", objName);

    const QString fullName = fn->fullDocumentName();
    if (fullName != objName)
        writer.writeAttribute("fullname", fullName);
    const QString href = m_gen->fullDocumentLocation(fn);
    if (!href.isEmpty())
        writer.writeAttribute("href", href);
    if (fn->threadSafeness() != Node::UnspecifiedSafeness)
        writer.writeAttribute("threadsafety", getThreadSafenessString(fn->threadSafeness()));
    writer.writeAttribute("status", getStatusString(fn->status()));
    writer.writeAttribute("access", getAccessString(fn->access()));

    const Location &declLocation = fn->declLocation();
    if (!declLocation.fileName().isEmpty())
        writer.writeAttribute("location", declLocation.fileName());
    if (m_storeLocationInfo && !declLocation.filePath().isEmpty()) {
        writer.writeAttribute("filepath", declLocation.filePath());
        writer.writeAttribute("lineno", QString("%1").arg(declLocation.lineNo()));
    }

    if (fn->hasDoc())
        writer.writeAttribute("documented", "true");
    if (fn->isRelatedNonmember())
        writer.writeAttribute("related", QString::number(indexForNode(fn)));
    if (!fn->since().isEmpty())
        writer.writeAttribute("since", fn->since());

    const QString brief = fn->doc().trimmedBriefText(fn->name()).toString();
    writer.writeAttribute("meta", fn->metanessString());
    if (fn->isCppNode()) {
        if (!fn->isNonvirtual())
            writer.writeAttribute("virtual", fn->virtualness());

        if (fn->isConst())
            writer.writeAttribute("const", "true");
        if (fn->isStatic())
            writer.writeAttribute("static", "true");
        if (fn->isFinal())
            writer.writeAttribute("final", "true");
        if (fn->isOverride())
            writer.writeAttribute("override", "true");
        if (fn->isExplicit())
            writer.writeAttribute("explicit", "true");
        if (fn->isConstexpr())
            writer.writeAttribute("constexpr", "true");

        if (auto noexcept_info = fn->getNoexcept()) {
            writer.writeAttribute("noexcept", "true");
            if (!(*noexcept_info).isEmpty()) writer.writeAttribute("noexcept_expression", *noexcept_info);
        }

        /*
          This ensures that for functions that have overloads,
          the first function written is the one that is not an
          overload, and the overloads follow it immediately in
          the index file numbered from 1 to n.
         */
        if (fn->isOverload() && (fn->overloadNumber() > 0)) {
            writer.writeAttribute("overload", "true");
            writer.writeAttribute("overload-number", QString::number(fn->overloadNumber()));
        }
        if (fn->isRef())
            writer.writeAttribute("refness", QString::number(1));
        else if (fn->isRefRef())
            writer.writeAttribute("refness", QString::number(2));
        if (fn->hasAssociatedProperties()) {
            QStringList associatedProperties;
            for (const auto *node : fn->associatedProperties()) {
                associatedProperties << node->name();
            }
            associatedProperties.sort();
            writer.writeAttribute("associated-property",
                                  associatedProperties.join(QLatin1Char(',')));
        }
    }

    const auto &return_type = fn->returnType();
    if (!return_type.isEmpty())
        writer.writeAttribute("type", std::move(return_type));

    const auto &declared_return_type = fn->declaredReturnType();
    if (declared_return_type.has_value())
        writer.writeAttribute("declaredtype", declared_return_type.value());

    if (fn->isCppNode()) {
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);

        /*
        Note: The "signature" attribute is written to the
        index file, but it is not read back in by qdoc. However,
        we need it for the webxml generator.
        */
        const QString signature = appendAttributesToSignature(fn);
        writer.writeAttribute("signature", signature);

        QStringList groups = m_qdb->groupNamesForNode(fn);
        if (!groups.isEmpty())
            writer.writeAttribute("groups", groups.join(QLatin1Char(',')));
    }

    for (int i = 0; i < fn->parameters().count(); ++i) {
        const Parameter &parameter = fn->parameters().at(i);
        writer.writeStartElement("parameter");
        writer.writeAttribute("type", parameter.type());
        writer.writeAttribute("name", parameter.name());
        writer.writeAttribute("default", parameter.defaultValue());
        writer.writeEndElement(); // parameter
    }

    writeTargets(writer, fn);

    // Append to the section if the callback object was set
    if (post_)
        post_->append(writer, fn);

    writer.writeEndElement(); // function
}

/*!
    \internal

    Constructs the signature to be written to an index file for the function
    represented by FunctionNode \a fn.

    'const' is already part of FunctionNode::signature(), which forms the basis
    for the signature returned by this method. The method adds, where
    applicable, the C++ keywords "final", "override", or "= 0", to the
    signature carried by the FunctionNode itself.
 */
QString QDocIndexFiles::appendAttributesToSignature(const FunctionNode *fn) const noexcept
{
    QString signature = fn->signature(Node::SignatureReturnType);

    if (fn->isFinal())
        signature += " final";
    if (fn->isOverride())
        signature += " override";
    if (fn->isPureVirtual())
        signature += " = 0";

    return signature;
}

/*!
  Outputs a <function> element to the index for each FunctionNode in
  an \a aggregate, using \a writer.
  The \a aggregate has a function map that contains all the
  function nodes (a vector of overloads) indexed by function
  name.

  If a function element represents an overload, it has an
  \c overload attribute set to \c true and an \c {overload-number}
  attribute set to the function's overload number.
 */
void QDocIndexFiles::generateFunctionSections(QXmlStreamWriter &writer, Aggregate *aggregate)
{
    for (auto functions : std::as_const(aggregate->functionMap())) {
        std::for_each(functions.begin(), functions.end(),
            [this,&writer](FunctionNode *fn) {
                generateFunctionSection(writer, fn);
            }
        );
    }
}

/*!
  Generate index sections for the child nodes of the given \a node
  using the \a writer specified.
*/
void QDocIndexFiles::generateIndexSections(QXmlStreamWriter &writer, Node *node,
                                           IndexSectionWriter *post)
{
    /*
      Note that groups, modules, QML modules, and proxies are written
      after all the other nodes.
     */
    if (node->isCollectionNode() || node->isGroup() || node->isModule() ||
        node->isQmlModule() || node->isProxyNode())
        return;

    if (node->isInternal() && !Config::instance().showInternal())
        return;

    if (generateIndexSection(writer, node, post)) {
        if (node->isAggregate()) {
            auto *aggregate = static_cast<Aggregate *>(node);
            // First write the function children, then write the nonfunction children.
            generateFunctionSections(writer, aggregate);
            const auto &nonFunctionList = aggregate->nonfunctionList();
            for (auto *node : nonFunctionList)
                generateIndexSections(writer, node, post);
        }

        if (node == root_) {
            /*
              We wait until the end of the index file to output the group, module,
              QML module, and proxy nodes. By outputting them at the end, when we read
              the index file back in, all the group/module/proxy member
              nodes will have already been created. It is then only necessary to
              create the collection node and add each member to its member list.
            */
            const CNMap &groups = m_qdb->groups();
            if (!groups.isEmpty()) {
                for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
                        writer.writeEndElement();
                }
            }

            const CNMap &modules = m_qdb->modules();
            if (!modules.isEmpty()) {
                for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
                        writer.writeEndElement();
                }
            }

            const CNMap &qmlModules = m_qdb->qmlModules();
            if (!qmlModules.isEmpty()) {
                for (auto it = qmlModules.constBegin(); it != qmlModules.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
                        writer.writeEndElement();
                }
            }

            for (auto *p : m_qdb->primaryTree()->proxies()) {
                if (generateIndexSection(writer, p, post)) {
                    auto aggregate = static_cast<Aggregate *>(p);
                    generateFunctionSections(writer, aggregate);
                    for (auto *n : aggregate->nonfunctionList())
                        generateIndexSections(writer, n, post);
                    writer.writeEndElement();
                }
            }
        }

        writer.writeEndElement();
    }
}

/*!
  Writes a qdoc module index in XML to a file named \a fileName.
  \a url is the \c url attribute of the <INDEX> element.
  \a title is the \c title attribute of the <INDEX> element.
  \a g is a pointer to the current Generator in use, stored for later use.
 */
void QDocIndexFiles::generateIndex(const QString &fileName, const QString &url,
                                   const QString &title)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    qCDebug(lcQdoc) << "Writing index file:" << fileName;

    m_gen = Generator::currentGenerator();
    m_relatedNodes.clear();
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeDTD("<!DOCTYPE QDOCINDEX>");

    writer.writeStartElement("INDEX");
    writer.writeAttribute("url", url);
    writer.writeAttribute("title", title);
    writer.writeAttribute("version", m_qdb->version());
    writer.writeAttribute("project", Config::instance().get(CONFIG_PROJECT).asString());

    root_ = m_qdb->primaryTreeRoot();
    if (!root_->tree()->indexTitle().isEmpty())
        writer.writeAttribute("indexTitle", root_->tree()->indexTitle());

    generateIndexSections(writer, root_, nullptr);

    writer.writeEndElement(); // INDEX
    writer.writeEndElement(); // QDOCINDEX
    writer.writeEndDocument();
    file.close();
}

QT_END_NAMESPACE
