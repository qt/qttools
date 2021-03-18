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

#include "qdocindexfiles.h"

#include "atom.h"
#include "config.h"
#include "generator.h"
#include "location.h"
#include "loggingcategory.h"
#include "qdocdatabase.h"
#include "qdoctagfiles.h"

#include <QtCore/qdebug.h>
#include <QtCore/qxmlstream.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

enum QDocAttr {
    QDocAttrNone,
    QDocAttrAttribution,
    QDocAttrExample,
    QDocAttrFile,
    QDocAttrImage,
    QDocAttrDocument,
    QDocAttrExternalPage
};

static Node *root_ = nullptr;
static IndexSectionWriter *post_ = nullptr;

/*!
  \class QDocIndexFiles

  This class handles qdoc index files.
 */

QDocIndexFiles *QDocIndexFiles::qdocIndexFiles_ = nullptr;

/*!
  Constructs the singleton QDocIndexFiles.
 */
QDocIndexFiles::QDocIndexFiles() : gen_(nullptr)
{
    qdb_ = QDocDatabase::qdocDB();
    storeLocationInfo_ = Config::instance().getBool(CONFIG_LOCATIONINFO);
}

/*!
  Destroys the singleton QDocIndexFiles.
 */
QDocIndexFiles::~QDocIndexFiles()
{
    qdb_ = nullptr;
    gen_ = nullptr;
}

/*!
  Creates the singleton. Allows only one instance of the class
  to be created. Returns a pointer to the singleton.
 */
QDocIndexFiles *QDocIndexFiles::qdocIndexFiles()
{
    if (qdocIndexFiles_ == nullptr)
        qdocIndexFiles_ = new QDocIndexFiles;
    return qdocIndexFiles_;
}

/*!
  Destroys the singleton.
 */
void QDocIndexFiles::destroyQDocIndexFiles()
{
    if (qdocIndexFiles_ != nullptr) {
        delete qdocIndexFiles_;
        qdocIndexFiles_ = nullptr;
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

static bool readingRoot = true;

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

    // Generate a relative URL between the install dir and the index file
    // when the -installdir command line option is set.
    QString indexUrl;
    if (Config::installDir.isEmpty()) {
        indexUrl = attrs.value(QLatin1String("url")).toString();
    } else {
        // Use a fake directory, since we will copy the output to a sub directory of
        // installDir when using "make install". This is just for a proper relative path.
        // QDir installDir(path.section('/', 0, -3) + "/outputdir");
        QDir installDir(path.section('/', 0, -3) + '/' + Generator::outputSubdir());
        indexUrl = installDir.relativeFilePath(path).section('/', 0, -2);
    }
    project_ = attrs.value(QLatin1String("project")).toString();
    QString indexTitle = attrs.value(QLatin1String("indexTitle")).toString();
    basesList_.clear();

    NamespaceNode *root = qdb_->newIndexTree(project_);
    if (!root) {
        qWarning() << "Issue parsing index tree" << path;
        return;
    }

    root->tree()->setIndexTitle(indexTitle);

    // Scan all elements in the XML file, constructing a map that contains
    // base classes for each class found.
    while (reader.readNextStartElement()) {
        readingRoot = true;
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
    QStringRef elementName = reader.name();

    QString name = attributes.value(QLatin1String("name")).toString();
    QString href = attributes.value(QLatin1String("href")).toString();
    Node *node;
    Location location;
    Aggregate *parent = nullptr;

    bool hasReadChildren = false;

    if (current->isAggregate())
        parent = static_cast<Aggregate *>(current);

    QString filePath;
    int lineNo = 0;
    if (attributes.hasAttribute(QLatin1String("filepath"))) {
        filePath = attributes.value(QLatin1String("filepath")).toString();
        lineNo = attributes.value("lineno").toInt();
    }
    if (elementName == QLatin1String("namespace")) {
        NamespaceNode *ns = new NamespaceNode(parent, name);
        node = ns;
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name.toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(name.toLower() + ".html");
    } else if (elementName == QLatin1String("class") || elementName == QLatin1String("struct")
               || elementName == QLatin1String("union")) {
        Node::NodeType type = Node::Class;
        if (elementName == QLatin1String("class"))
            type = Node::Class;
        else if (elementName == QLatin1String("struct"))
            type = Node::Struct;
        else if (elementName == QLatin1String("union"))
            type = Node::Union;
        node = new ClassNode(type, parent, name);
        if (attributes.hasAttribute(QLatin1String("bases"))) {
            QString bases = attributes.value(QLatin1String("bases")).toString();
            if (!bases.isEmpty())
                basesList_.append(
                        QPair<ClassNode *, QString>(static_cast<ClassNode *>(node), bases));
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
    } else if (elementName == QLatin1String("qmlclass")) {
        QmlTypeNode *qcn = new QmlTypeNode(parent, name);
        qcn->setTitle(attributes.value(QLatin1String("title")).toString());
        QString logicalModuleName = attributes.value(QLatin1String("qml-module-name")).toString();
        if (!logicalModuleName.isEmpty())
            qdb_->addToQmlModule(logicalModuleName, qcn);
        bool abstract = false;
        if (attributes.value(QLatin1String("abstract")) == QLatin1String("true"))
            abstract = true;
        qcn->setAbstract(abstract);
        QString qmlFullBaseName = attributes.value(QLatin1String("qml-base-type")).toString();
        if (!qmlFullBaseName.isEmpty()) {
            qcn->setQmlBaseName(qmlFullBaseName);
        }
        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value("location").toString();
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);
        node = qcn;
    } else if (elementName == QLatin1String("jstype")) {
        QmlTypeNode *qcn = new QmlTypeNode(parent, name);
        qcn->setGenus(Node::JS);
        qcn->setTitle(attributes.value(QLatin1String("title")).toString());
        QString logicalModuleName = attributes.value(QLatin1String("js-module-name")).toString();
        if (!logicalModuleName.isEmpty())
            qdb_->addToQmlModule(logicalModuleName, qcn);
        bool abstract = false;
        if (attributes.value(QLatin1String("abstract")) == QLatin1String("true"))
            abstract = true;
        qcn->setAbstract(abstract);
        QString qmlFullBaseName = attributes.value(QLatin1String("js-base-type")).toString();
        if (!qmlFullBaseName.isEmpty()) {
            qcn->setQmlBaseName(qmlFullBaseName);
        }
        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value("location").toString();
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);
        node = qcn;
    } else if (elementName == QLatin1String("qmlbasictype")) {
        QmlBasicTypeNode *qbtn = new QmlBasicTypeNode(parent, name);
        qbtn->setTitle(attributes.value(QLatin1String("title")).toString());
        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value("location").toString();
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);
        node = qbtn;
    } else if (elementName == QLatin1String("jsbasictype")) {
        QmlBasicTypeNode *qbtn = new QmlBasicTypeNode(parent, name);
        qbtn->setGenus(Node::JS);
        qbtn->setTitle(attributes.value(QLatin1String("title")).toString());
        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value("location").toString();
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);
        node = qbtn;
    } else if (elementName == QLatin1String("qmlproperty")) {
        QString type = attributes.value(QLatin1String("type")).toString();
        bool attached = false;
        if (attributes.value(QLatin1String("attached")) == QLatin1String("true"))
            attached = true;
        bool readonly = false;
        if (attributes.value(QLatin1String("writable")) == QLatin1String("false"))
            readonly = true;
        QmlPropertyNode *qpn = new QmlPropertyNode(parent, name, type, attached);
        qpn->markReadOnly(readonly);
        node = qpn;
    } else if (elementName == QLatin1String("jsproperty")) {
        QString type = attributes.value(QLatin1String("type")).toString();
        bool attached = false;
        if (attributes.value(QLatin1String("attached")) == QLatin1String("true"))
            attached = true;
        bool readonly = false;
        if (attributes.value(QLatin1String("writable")) == QLatin1String("false"))
            readonly = true;
        QmlPropertyNode *qpn = new QmlPropertyNode(parent, name, type, attached);
        qpn->setGenus(Node::JS);
        qpn->markReadOnly(readonly);
        node = qpn;
    } else if (elementName == QLatin1String("group")) {
        CollectionNode *cn = qdb_->addGroup(name);
        cn->setTitle(attributes.value(QLatin1String("title")).toString());
        cn->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            cn->markSeen();
        node = cn;
    } else if (elementName == QLatin1String("module")) {
        CollectionNode *cn = qdb_->addModule(name);
        cn->setTitle(attributes.value(QLatin1String("title")).toString());
        cn->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            cn->markSeen();
        node = cn;
    } else if (elementName == QLatin1String("qmlmodule")) {
        QString t = attributes.value(QLatin1String("qml-module-name")).toString();
        CollectionNode *cn = qdb_->addQmlModule(t);
        QStringList info;
        info << t << attributes.value(QLatin1String("qml-module-version")).toString();
        cn->setLogicalModuleInfo(info);
        cn->setTitle(attributes.value(QLatin1String("title")).toString());
        cn->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            cn->markSeen();
        node = cn;
    } else if (elementName == QLatin1String("jsmodule")) {
        QString t = attributes.value(QLatin1String("js-module-name")).toString();
        CollectionNode *cn = qdb_->addJsModule(t);
        QStringList info;
        info << t << attributes.value(QLatin1String("js-module-version")).toString();
        cn->setLogicalModuleInfo(info);
        cn->setTitle(attributes.value(QLatin1String("title")).toString());
        cn->setSubtitle(attributes.value(QLatin1String("subtitle")).toString());
        if (attributes.value(QLatin1String("seen")) == QLatin1String("true"))
            cn->markSeen();
        node = cn;
    } else if (elementName == QLatin1String("page")) {
        QDocAttr subtype = QDocAttrNone;
        Node::PageType ptype = Node::NoPageType;
        QString attr = attributes.value(QLatin1String("subtype")).toString();
        if (attr == QLatin1String("attribution")) {
            subtype = QDocAttrDocument;
            ptype = Node::AttributionPage;
        } else if (attr == QLatin1String("example")) {
            subtype = QDocAttrExample;
            ptype = Node::ExamplePage;
        } else if (attr == QLatin1String("file")) {
            subtype = QDocAttrFile;
            ptype = Node::NoPageType;
        } else if (attr == QLatin1String("image")) {
            subtype = QDocAttrImage;
            ptype = Node::NoPageType;
        } else if (attr == QLatin1String("page")) {
            subtype = QDocAttrDocument;
            ptype = Node::ArticlePage;
        } else if (attr == QLatin1String("externalpage")) {
            subtype = QDocAttrExternalPage;
            ptype = Node::ArticlePage;
        } else
            goto done;

        if (current->isExample()) {
            ExampleNode *en = static_cast<ExampleNode *>(current);
            if (subtype == QDocAttrFile) {
                en->appendFile(name);
                goto done;
            } else if (subtype == QDocAttrImage) {
                en->appendImage(name);
                goto done;
            }
        }
        PageNode *pn = nullptr;
        if (subtype == QDocAttrExample)
            pn = new ExampleNode(parent, name);
        else if (subtype == QDocAttrExternalPage)
            pn = new ExternalPageNode(parent, name);
        else
            pn = new PageNode(parent, name, ptype);
        pn->setTitle(attributes.value(QLatin1String("title")).toString());

        if (attributes.hasAttribute(QLatin1String("location")))
            name = attributes.value(QLatin1String("location")).toString();

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + name);
        else if (!indexUrl.isNull())
            location = Location(name);

        node = pn;

    } else if (elementName == QLatin1String("enum")) {
        EnumNode *enumNode = new EnumNode(parent, name, attributes.hasAttribute("scoped"));

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

        while (reader.readNextStartElement()) {
            QXmlStreamAttributes childAttributes = reader.attributes();
            if (reader.name() == QLatin1String("value")) {

                EnumItem item(childAttributes.value(QLatin1String("name")).toString(),
                              childAttributes.value(QLatin1String("value")).toString());
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
        node = new TypedefNode(parent, name);

        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (elementName == QLatin1String("alias")) {
        node = new TypeAliasNode(parent, name, attributes.value(QLatin1String("aliasedtype")).toString());
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

    } else if (elementName == QLatin1String("property")) {
        node = new PropertyNode(parent, name);

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
        FunctionNode *fn = new FunctionNode(metaness, parent, name, attached);
        if (fn->isCppNode()) {
            fn->setReturnType(attributes.value(QLatin1String("type")).toString());
            fn->setVirtualness(attributes.value(QLatin1String("virtual")).toString());
            fn->setConst(attributes.value(QLatin1String("const")) == QLatin1String("true"));
            fn->setStatic(attributes.value(QLatin1String("static")) == QLatin1String("true"));
            fn->setFinal(attributes.value(QLatin1String("final")) == QLatin1String("true"));
            fn->setOverride(attributes.value(QLatin1String("override")) == QLatin1String("true"));
            int refness = attributes.value(QLatin1String("refness")).toUInt();
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
        }

        node = fn;
        if (!indexUrl.isEmpty())
            location = Location(indexUrl + QLatin1Char('/') + parent->name().toLower() + ".html");
        else if (!indexUrl.isNull())
            location = Location(parent->name().toLower() + ".html");

        hasReadChildren = true;
    } else if (elementName == QLatin1String("variable")) {
        node = new VariableNode(parent, name);
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
        QString access = attributes.value(QLatin1String("access")).toString();
        if (access == "public")
            node->setAccess(Node::Public);
        else if (access == "protected")
            node->setAccess(Node::Protected);
        else if ((access == "private") || (access == "internal"))
            node->setAccess(Node::Private);
        else
            node->setAccess(Node::Public);
        if (attributes.hasAttribute(QLatin1String("related")))
            node->setRelatedNonmember(attributes.value(QLatin1String("related"))
                                      == QLatin1String("true"));

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

        QString status = attributes.value(QLatin1String("status")).toString();
        if (status == QLatin1String("obsolete"))
            node->setStatus(Node::Obsolete);
        else if (status == QLatin1String("deprecated"))
            node->setStatus(Node::Obsolete);
        else if (status == QLatin1String("preliminary"))
            node->setStatus(Node::Preliminary);
        else if (status == QLatin1String("active"))
            node->setStatus(Node::Active);
        else if (status == QLatin1String("internal"))
            node->setStatus(Node::Internal);
        else if (status == QLatin1String("ignored"))
            node->setStatus(Node::DontDocument);
        else
            node->setStatus(Node::Active);

        QString physicalModuleName = attributes.value(QLatin1String("module")).toString();
        if (!physicalModuleName.isEmpty())
            qdb_->addToModule(physicalModuleName, node);
        if (!href.isEmpty()) {
            node->setUrl(href);
            // Include the index URL if it exists
            if (!node->isExternalPage() && !indexUrl.isEmpty())
                node->setUrl(indexUrl + QLatin1Char('/') + href);
        }

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
            for (const auto &name : groupNames) {
                qdb_->addToGroup(name, node);
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
        node->setOutputSubdirectory(project_.toLower());
        QString briefAttr = attributes.value(QLatin1String("brief")).toString();
        if (!briefAttr.isEmpty()) {
            node->setReconstitutedBrief(briefAttr);
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
    qdb_->insertTarget(name, title, type, node, priority);
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
    QPair<ClassNode *, QString> pair;
    for (const auto &pair : qAsConst(basesList_)) {
        const QStringList bases = pair.second.split(QLatin1Char(','));
        for (const auto &base : bases) {
            QStringList basePath = base.split(QString("::"));
            Node *n = qdb_->findClassNode(basePath);
            if (n)
                pair.first->addResolvedBaseClass(Node::Public, static_cast<ClassNode *>(n));
            else
                pair.first->addUnresolvedBaseClass(Node::Public, basePath, QString());
        }
    }
    // No longer needed.
    basesList_.clear();
}

static const QString getAccessString(Node::Access t)
{

    switch (t) {
    case Node::Public:
        return QLatin1String("public");
    case Node::Protected:
        return QLatin1String("protected");
    case Node::Private:
        return QLatin1String("private");
    default:
        break;
    }
    return QLatin1String("public");
}

static const QString getStatusString(Node::Status t)
{
    switch (t) {
    case Node::Obsolete:
    case Node::Deprecated:
        return QLatin1String("obsolete");
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

static const QString getThreadSafenessString(Node::ThreadSafeness t)
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
  Generate the index section with the given \a writer for the \a node
  specified, returning true if an element was written, and returning
  false if an element is not written.

  \note Function nodes are processed in generateFunctionSection()
 */
bool QDocIndexFiles::generateIndexSection(QXmlStreamWriter &writer, Node *node,
                                          IndexSectionWriter *post)
{
    if (gen_ == nullptr)
        gen_ = Generator::currentGenerator();

    Q_ASSERT(gen_);

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
    case Node::Namespace:
        nodeName = "namespace";
        break;
    case Node::Class:
        nodeName = "class";
        break;
    case Node::Struct:
        nodeName = "struct";
        break;
    case Node::Union:
        nodeName = "union";
        break;
    case Node::HeaderFile:
        nodeName = "header";
        break;
    case Node::QmlType:
        nodeName = "qmlclass";
        if (node->logicalModule() != nullptr)
            logicalModuleName = node->logicalModule()->logicalModuleName();
        baseNameAttr = "qml-base-type";
        moduleNameAttr = "qml-module-name";
        moduleVerAttr = "qml-module-version";
        qmlFullBaseName = node->qmlFullBaseName();
        break;
    case Node::JsType:
        nodeName = "jstype";
        baseNameAttr = "js-base-type";
        moduleNameAttr = "js-module-name";
        moduleVerAttr = "js-module-version";
        if (node->logicalModule() != nullptr)
            logicalModuleName = node->logicalModule()->logicalModuleName();
        qmlFullBaseName = node->qmlFullBaseName();
        break;
    case Node::QmlBasicType:
        nodeName = "qmlbasictype";
        break;
    case Node::JsBasicType:
        nodeName = "jsbasictype";
        break;
    case Node::Page:
    case Node::Example:
    case Node::ExternalPage:
        nodeName = "page";
        break;
    case Node::Group:
        nodeName = "group";
        break;
    case Node::Module:
        nodeName = "module";
        break;
    case Node::QmlModule:
        nodeName = "qmlmodule";
        moduleNameAttr = "qml-module-name";
        moduleVerAttr = "qml-module-version";
        logicalModuleName = node->logicalModuleName();
        logicalModuleVersion = node->logicalModuleVersion();
        break;
    case Node::JsModule:
        nodeName = "jsmodule";
        moduleNameAttr = "js-module-name";
        moduleVerAttr = "js-module-version";
        logicalModuleName = node->logicalModuleName();
        logicalModuleVersion = node->logicalModuleVersion();
        break;
    case Node::Enum:
        nodeName = "enum";
        break;
    case Node::Typedef:
        nodeName = "typedef";
        break;
    case Node::TypeAlias:
        nodeName = "alias";
        break;
    case Node::Property:
        nodeName = "property";
        break;
    case Node::Variable:
        nodeName = "variable";
        break;
    case Node::QmlProperty:
        nodeName = "qmlproperty";
        break;
    case Node::JsProperty:
        nodeName = "jsProperty";
        break;
    case Node::Proxy:
        nodeName = "proxy";
        break;
    case Node::Function: // Now processed in generateFunctionSection()
    default:
        return false;
    }

    QString objName = node->name();
    // Special case: only the root node should have an empty name.
    if (objName.isEmpty() && node != qdb_->primaryTreeRoot())
        return false;

    writer.writeStartElement(nodeName);

    if (!node->isTextPageNode() && !node->isCollectionNode() && !node->isHeader()) {
        if (node->threadSafeness() != Node::UnspecifiedSafeness)
            writer.writeAttribute("threadsafety", getThreadSafenessString(node->threadSafeness()));
    }

    writer.writeAttribute("name", objName);

    // Write module and base type info for QML/JS types
    if (!moduleNameAttr.isEmpty()) {
        if (!logicalModuleName.isEmpty())
            writer.writeAttribute(moduleNameAttr, logicalModuleName);
        else
            writer.writeAttribute(moduleNameAttr, node->name());
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
        href = gen_->fullDocumentLocation(node);
    } else
        href = node->name();
    if (node->isQmlNode() || node->isJsNode()) {
        Aggregate *p = node->parent();
        if (p && (p->isQmlType() || p->isJsType()) && p->isAbstract())
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
    if (storeLocationInfo_ && !declLocation.filePath().isEmpty()) {
        writer.writeAttribute("filepath", declLocation.filePath());
        writer.writeAttribute("lineno", QString("%1").arg(declLocation.lineNo()));
    }

    if (node->isRelatedNonmember())
        writer.writeAttribute("related", "true");

    if (!node->since().isEmpty())
        writer.writeAttribute("since", node->since());

    if (node->hasDoc())
        writer.writeAttribute("documented", "true");

    QString brief = node->doc().trimmedBriefText(node->name()).toString();
    switch (node->nodeType()) {
    case Node::Class:
    case Node::Struct:
    case Node::Union: {
        // Classes contain information about their base classes.
        const ClassNode *classNode = static_cast<const ClassNode *>(node);
        const QVector<RelatedClass> bases = classNode->baseClasses();
        QSet<QString> baseStrings;
        for (const auto &related : bases) {
            ClassNode *n = related.node_;
            if (n)
                baseStrings.insert(n->fullName());
            else if (!related.path_.isEmpty())
                baseStrings.insert(related.path_.join(QLatin1String("::")));
        }
        if (!baseStrings.isEmpty()) {
            QStringList baseStringsAsList = baseStrings.values();
            baseStringsAsList.sort();
            writer.writeAttribute("bases", baseStringsAsList.join(QLatin1Char(',')));
        }
        if (!node->physicalModuleName().isEmpty())
            writer.writeAttribute("module", node->physicalModuleName());
        if (!classNode->groupNames().isEmpty())
            writer.writeAttribute("groups", classNode->groupNames().join(QLatin1Char(',')));
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::HeaderFile: {
        const HeaderNode *hn = static_cast<const HeaderNode *>(node);
        if (!hn->physicalModuleName().isEmpty())
            writer.writeAttribute("module", hn->physicalModuleName());
        if (!hn->groupNames().isEmpty())
            writer.writeAttribute("groups", hn->groupNames().join(QLatin1Char(',')));
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
        writer.writeAttribute("title", hn->title());
        writer.writeAttribute("fulltitle", hn->fullTitle());
        writer.writeAttribute("subtitle", hn->subtitle());
    } break;
    case Node::Namespace: {
        const NamespaceNode *ns = static_cast<const NamespaceNode *>(node);
        if (!ns->physicalModuleName().isEmpty())
            writer.writeAttribute("module", ns->physicalModuleName());
        if (!ns->groupNames().isEmpty())
            writer.writeAttribute("groups", ns->groupNames().join(QLatin1Char(',')));
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::JsType:
    case Node::QmlType: {
        const QmlTypeNode *qcn = static_cast<const QmlTypeNode *>(node);
        writer.writeAttribute("title", qcn->title());
        writer.writeAttribute("fulltitle", qcn->fullTitle());
        writer.writeAttribute("subtitle", qcn->subtitle());
        if (!qcn->groupNames().isEmpty())
            writer.writeAttribute("groups", qcn->groupNames().join(QLatin1Char(',')));
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::Page:
    case Node::Example:
    case Node::ExternalPage: {
        /*
          Page nodes (anything that generates a doc page)
          no longer have a subtype. Some of the subtypes
          (Example, External, and Header) have been promoted
          to be node types. They have become subclasses of
          PageNode or, in the case of Header, a subclass of
          Aggregate. The processing for other subtypes that
          have not (yet) been promoted to be node types is
          determined by the PageType enum.
        */
        bool writeModuleName = false;
        if (node->isExample()) {
            writer.writeAttribute("subtype", "example");
            writeModuleName = true;
        } else if (node->isExternalPage()) {
            writer.writeAttribute("subtype", "externalpage");
        } else {
            if (node->pageType() == Node::AttributionPage)
                writer.writeAttribute("subtype", "attribution");
            else
                writer.writeAttribute("subtype", "page");
            writeModuleName = true;
        }
        const PageNode *pn = static_cast<const PageNode *>(node);
        writer.writeAttribute("title", pn->title());
        writer.writeAttribute("fulltitle", pn->fullTitle());
        writer.writeAttribute("subtitle", pn->subtitle());
        if (!node->physicalModuleName().isEmpty() && writeModuleName)
            writer.writeAttribute("module", node->physicalModuleName());
        if (!pn->groupNames().isEmpty())
            writer.writeAttribute("groups", pn->groupNames().join(QLatin1Char(',')));
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::Group: {
        const CollectionNode *cn = static_cast<const CollectionNode *>(node);
        writer.writeAttribute("seen", cn->wasSeen() ? "true" : "false");
        writer.writeAttribute("title", cn->title());
        if (!cn->subtitle().isEmpty())
            writer.writeAttribute("subtitle", cn->subtitle());
        if (!cn->physicalModuleName().isEmpty())
            writer.writeAttribute("module", cn->physicalModuleName());
        if (!cn->groupNames().isEmpty())
            writer.writeAttribute("groups", cn->groupNames().join(QLatin1Char(',')));
        /*
          This is not read back in, so it probably
          shouldn't be written out in the first place.
        */
        if (!cn->members().isEmpty()) {
            QStringList names;
            const auto &members = cn->members();
            for (const Node *member : members)
                names.append(member->name());
            writer.writeAttribute("members", names.join(QLatin1Char(',')));
        }
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::Module: {
        const CollectionNode *cn = static_cast<const CollectionNode *>(node);
        writer.writeAttribute("seen", cn->wasSeen() ? "true" : "false");
        writer.writeAttribute("title", cn->title());
        if (!cn->subtitle().isEmpty())
            writer.writeAttribute("subtitle", cn->subtitle());
        if (!cn->physicalModuleName().isEmpty())
            writer.writeAttribute("module", cn->physicalModuleName());
        if (!cn->groupNames().isEmpty())
            writer.writeAttribute("groups", cn->groupNames().join(QLatin1Char(',')));
        /*
          This is not read back in, so it probably
          shouldn't be written out in the first place.
        */
        if (!cn->members().isEmpty()) {
            QStringList names;
            const auto &members = cn->members();
            for (const Node *member : members)
                names.append(member->name());
            writer.writeAttribute("members", names.join(QLatin1Char(',')));
        }
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::JsModule:
    case Node::QmlModule: {
        const CollectionNode *cn = static_cast<const CollectionNode *>(node);
        writer.writeAttribute("seen", cn->wasSeen() ? "true" : "false");
        writer.writeAttribute("title", cn->title());
        if (!cn->subtitle().isEmpty())
            writer.writeAttribute("subtitle", cn->subtitle());
        if (!cn->physicalModuleName().isEmpty())
            writer.writeAttribute("module", cn->physicalModuleName());
        if (!cn->groupNames().isEmpty())
            writer.writeAttribute("groups", cn->groupNames().join(QLatin1Char(',')));
        /*
          This is not read back in, so it probably
          shouldn't be written out in the first place.
        */
        if (!cn->members().isEmpty()) {
            QStringList names;
            const auto &members = cn->members();
            for (const Node *member : members)
                names.append(member->name());
            writer.writeAttribute("members", names.join(QLatin1Char(',')));
        }
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::JsProperty:
    case Node::QmlProperty: {
        QmlPropertyNode *qpn = static_cast<QmlPropertyNode *>(node);
        writer.writeAttribute("type", qpn->dataType());
        writer.writeAttribute("attached", qpn->isAttached() ? "true" : "false");
        writer.writeAttribute("writable", qpn->isWritable() ? "true" : "false");
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::Property: {
        const PropertyNode *propertyNode = static_cast<const PropertyNode *>(node);
        writer.writeAttribute("type", propertyNode->dataType());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
        const auto &getters = propertyNode->getters();
        for (const auto *fnNode : getters) {
            if (fnNode) {
                const FunctionNode *functionNode = static_cast<const FunctionNode *>(fnNode);
                writer.writeStartElement("getter");
                writer.writeAttribute("name", functionNode->name());
                writer.writeEndElement(); // getter
            }
        }
        const auto &setters = propertyNode->setters();
        for (const auto *fnNode : setters) {
            if (fnNode) {
                const FunctionNode *functionNode = static_cast<const FunctionNode *>(fnNode);
                writer.writeStartElement("setter");
                writer.writeAttribute("name", functionNode->name());
                writer.writeEndElement(); // setter
            }
        }
        const auto &resetters = propertyNode->resetters();
        for (const auto *fnNode : resetters) {
            if (fnNode) {
                const FunctionNode *functionNode = static_cast<const FunctionNode *>(fnNode);
                writer.writeStartElement("resetter");
                writer.writeAttribute("name", functionNode->name());
                writer.writeEndElement(); // resetter
            }
        }
        const auto &notifiers = propertyNode->notifiers();
        for (const auto *fnNode : notifiers) {
            if (fnNode) {
                const FunctionNode *functionNode = static_cast<const FunctionNode *>(fnNode);
                writer.writeStartElement("notifier");
                writer.writeAttribute("name", functionNode->name());
                writer.writeEndElement(); // notifier
            }
        }
    } break;
    case Node::Variable: {
        const VariableNode *variableNode = static_cast<const VariableNode *>(node);
        writer.writeAttribute("type", variableNode->dataType());
        writer.writeAttribute("static", variableNode->isStatic() ? "true" : "false");
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
    } break;
    case Node::Enum: {
        const EnumNode *enumNode = static_cast<const EnumNode *>(node);
        if (enumNode->isScoped())
            writer.writeAttribute("scoped", "true");
        if (enumNode->flagsType())
            writer.writeAttribute("typedef", enumNode->flagsType()->fullDocumentName());
        const auto &items = enumNode->items();
        for (const auto &item : items) {
            writer.writeStartElement("value");
            writer.writeAttribute("name", item.name());
            writer.writeAttribute("value", item.value());
            writer.writeEndElement(); // value
        }
    } break;
    case Node::Typedef: {
        const TypedefNode *typedefNode = static_cast<const TypedefNode *>(node);
        if (typedefNode->associatedEnum())
            writer.writeAttribute("enum", typedefNode->associatedEnum()->fullDocumentName());
    } break;
    case Node::TypeAlias:
        writer.writeAttribute("aliasedtype", static_cast<const TypeAliasNode *>(node)->aliasedType());
        break;
    case Node::Function: // Now processed in generateFunctionSection()
    default:
        break;
    }

    /*
      For our pages, we canonicalize the target, keyword and content
      item names so that they can be used by qdoc for other sets of
      documentation.

      The reason we do this here is that we don't want to ruin
      externally composed indexes, containing non-qdoc-style target names
      when reading in indexes.

      targets and keywords are now allowed in any node, not just inner nodes.
    */

    if (node->doc().hasTargets()) {
        bool external = false;
        if (node->isExternalPage())
            external = true;
        const auto &targets = node->doc().targets();
        for (const Atom *target : targets) {
            QString title = target->string();
            QString name = Doc::canonicalTitle(title);
            writer.writeStartElement("target");
            if (!external)
                writer.writeAttribute("name", name);
            else
                writer.writeAttribute("name", title);
            if (name != title)
                writer.writeAttribute("title", title);
            writer.writeEndElement(); // target
        }
    }
    if (node->doc().hasKeywords()) {
        const auto &keywords = node->doc().keywords();
        for (const Atom *keyword : keywords) {
            QString title = keyword->string();
            QString name = Doc::canonicalTitle(title);
            writer.writeStartElement("keyword");
            writer.writeAttribute("name", name);
            if (name != title)
                writer.writeAttribute("title", title);
            writer.writeEndElement(); // keyword
        }
    }

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
                writer.writeAttribute("name", Doc::canonicalTitle(title));
                writer.writeAttribute("title", title);
                writer.writeAttribute("level", QString::number(level));
                writer.writeEndElement(); // contents
            }
        }
    }
    // WebXMLGenerator - skip the nested <page> elements for example
    // files/images, as the generator produces them separately
    if (node->isExample() && gen_->format() != QLatin1String("WebXML")) {
        const ExampleNode *en = static_cast<const ExampleNode *>(node);
        const auto &files = en->files();
        for (const QString &file : files) {
            writer.writeStartElement("page");
            writer.writeAttribute("name", file);
            QString href = gen_->linkForExampleFile(file, en);
            writer.writeAttribute("href", href);
            writer.writeAttribute("status", "active");
            writer.writeAttribute("subtype", "file");
            writer.writeAttribute("title", "");
            writer.writeAttribute("fulltitle", Generator::exampleFileTitle(en, file));
            writer.writeAttribute("subtitle", file);
            writer.writeEndElement(); // page
        }
        const auto &images = en->images();
        for (const QString &file : images) {
            writer.writeStartElement("page");
            writer.writeAttribute("name", file);
            QString href = gen_->linkForExampleFile(file, en);
            writer.writeAttribute("href", href);
            writer.writeAttribute("status", "active");
            writer.writeAttribute("subtype", "image");
            writer.writeAttribute("title", "");
            writer.writeAttribute("fulltitle", Generator::exampleFileTitle(en, file));
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
    QString objName = fn->name();
    writer.writeStartElement("function");
    writer.writeAttribute("name", objName);

    QString fullName = fn->fullDocumentName();
    if (fullName != objName)
        writer.writeAttribute("fullname", fullName);
    QString href = gen_->fullDocumentLocation(fn);
    if (!href.isEmpty())
        writer.writeAttribute("href", href);
    if (fn->threadSafeness() != Node::UnspecifiedSafeness)
        writer.writeAttribute("threadsafety", getThreadSafenessString(fn->threadSafeness()));
    writer.writeAttribute("status", getStatusString(fn->status()));
    writer.writeAttribute("access", getAccessString(fn->access()));

    const Location &declLocation = fn->declLocation();
    if (!declLocation.fileName().isEmpty())
        writer.writeAttribute("location", declLocation.fileName());
    if (storeLocationInfo_ && !declLocation.filePath().isEmpty()) {
        writer.writeAttribute("filepath", declLocation.filePath());
        writer.writeAttribute("lineno", QString("%1").arg(declLocation.lineNo()));
    }

    if (fn->hasDoc())
        writer.writeAttribute("documented", "true");
    if (fn->isRelatedNonmember())
        writer.writeAttribute("related", "true");
    if (!fn->since().isEmpty())
        writer.writeAttribute("since", fn->since());

    QString brief = fn->doc().trimmedBriefText(fn->name()).toString();
    writer.writeAttribute("meta", fn->metanessString());
    if (fn->isCppNode()) {
        writer.writeAttribute("virtual", fn->virtualness());
        writer.writeAttribute("const", fn->isConst() ? "true" : "false");
        writer.writeAttribute("static", fn->isStatic() ? "true" : "false");
        writer.writeAttribute("final", fn->isFinal() ? "true" : "false");
        writer.writeAttribute("override", fn->isOverride() ? "true" : "false");
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
        writer.writeAttribute("type", fn->returnType());
        if (!brief.isEmpty())
            writer.writeAttribute("brief", brief);
        /*
          Note: The "signature" attribute is written to the
          index file, but it is not read back in by qdoc. However,
          we need it for the webxml generator.
        */
        QString signature = fn->signature(false, false);
        // 'const' is already part of FunctionNode::signature()
        if (fn->isFinal())
            signature += " final";
        if (fn->isOverride())
            signature += " override";
        if (fn->isPureVirtual())
            signature += " = 0";
        writer.writeAttribute("signature", signature);

        for (int i = 0; i < fn->parameters().count(); ++i) {
            const Parameter &parameter = fn->parameters().at(i);
            writer.writeStartElement("parameter");
            writer.writeAttribute("type", parameter.type());
            writer.writeAttribute("name", parameter.name());
            writer.writeAttribute("default", parameter.defaultValue());
            writer.writeEndElement(); // parameter
        }
    }

    // Append to the section if the callback object was set
    if (post_)
        post_->append(writer, fn);

    writer.writeEndElement(); // function
}

/*!
  This function outputs a <function> element to the index file
  for each FunctionNode in \a aggregate using the \a writer.
  The \a aggregate has a function map that contains all the
  function nodes indexed by function name. But the map is not
  used as a multimap, so if the \a aggregate contains multiple
  functions with the same name, only one of those functions is
  in the function map index. The others are linked to that
  function using the next overload pointer.

  So this function generates a <function> element for a function
  followed by a function element for each of its overloads. If a
  <function> element represents an overload, it has an \c overload
  attribute set to \c true and an \c {overload-number} attribute
  set to the function's overload number. If the <function>
  element does not represent an overload, the <function> element
  has neither of these attributes.
 */
void QDocIndexFiles::generateFunctionSections(QXmlStreamWriter &writer, Aggregate *aggregate)
{
    FunctionMap &functionMap = aggregate->functionMap();
    if (!functionMap.isEmpty()) {
        for (auto it = functionMap.begin(); it != functionMap.end(); ++it) {
            FunctionNode *fn = it.value();
            while (fn != nullptr) {
                generateFunctionSection(writer, fn);
                fn = fn->nextOverload();
            }
        }
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
      Note that groups, modules, and QML modules are written
      after all the other nodes.
     */
    if (node->isCollectionNode() || node->isGroup() || node->isModule() || node->isQmlModule()
        || node->isJsModule())
        return;

    if (generateIndexSection(writer, node, post)) {
        if (node->isAggregate()) {
            Aggregate *aggregate = static_cast<Aggregate *>(node);
            // First write the function children, then write the nonfunction children.
            generateFunctionSections(writer, aggregate);
            const auto &nonFunctionList = aggregate->nonfunctionList();
            for (auto *node : nonFunctionList)
                generateIndexSections(writer, node, post);
        }

        if (node == root_) {
            /*
              We wait until the end of the index file to output the group, module,
              and QML module elements. By outputting them at the end, when we read
              the index file back in, all the group, module, and QML module member
              elements will have already been created. It is then only necessary to
              create the group, module, or QML module element and add each member to
              its member list.
            */
            const CNMap &groups = qdb_->groups();
            if (!groups.isEmpty()) {
                for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
                        writer.writeEndElement();
                }
            }

            const CNMap &modules = qdb_->modules();
            if (!modules.isEmpty()) {
                for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
                        writer.writeEndElement();
                }
            }

            const CNMap &qmlModules = qdb_->qmlModules();
            if (!qmlModules.isEmpty()) {
                for (auto it = qmlModules.constBegin(); it != qmlModules.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
                        writer.writeEndElement();
                }
            }

            const CNMap &jsModules = qdb_->jsModules();
            if (!jsModules.isEmpty()) {
                for (auto it = jsModules.constBegin(); it != jsModules.constEnd(); ++it) {
                    if (generateIndexSection(writer, it.value(), post))
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
                                   const QString &title, Generator *g)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    qCDebug(lcQdoc) << "Writing index file:" << fileName;

    gen_ = g;
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeDTD("<!DOCTYPE QDOCINDEX>");

    writer.writeStartElement("INDEX");
    writer.writeAttribute("url", url);
    writer.writeAttribute("title", title);
    writer.writeAttribute("version", qdb_->version());
    writer.writeAttribute("project", Config::instance().getString(CONFIG_PROJECT));

    root_ = qdb_->primaryTreeRoot();
    if (!root_->tree()->indexTitle().isEmpty())
        writer.writeAttribute("indexTitle", root_->tree()->indexTitle());

    generateIndexSections(writer, root_, nullptr);

    writer.writeEndElement(); // INDEX
    writer.writeEndElement(); // QDOCINDEX
    writer.writeEndDocument();
    file.close();
}

QT_END_NAMESPACE
