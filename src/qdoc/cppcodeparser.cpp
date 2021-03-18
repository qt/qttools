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

/*
  cppcodeparser.cpp
*/

#include "cppcodeparser.h"

#include "config.h"
#include "generator.h"
#include "loggingcategory.h"
#include "qdocdatabase.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>

#include <algorithm>
#include <errno.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

/* qmake ignore Q_OBJECT */

QSet<QString> CppCodeParser::excludeDirs;
QSet<QString> CppCodeParser::excludeFiles;

static QSet<QString> topicCommands_;
static QSet<QString> metaCommands_;

/*!
  The constructor initializes some regular expressions
  and initializes the tokenizer variables.
 */
CppCodeParser::CppCodeParser()
{
    if (topicCommands_.isEmpty()) {
        topicCommands_ << COMMAND_CLASS << COMMAND_DITAMAP << COMMAND_DONTDOCUMENT << COMMAND_ENUM
                       << COMMAND_EXAMPLE << COMMAND_EXTERNALPAGE << COMMAND_FN << COMMAND_GROUP
                       << COMMAND_HEADERFILE << COMMAND_MACRO << COMMAND_MODULE << COMMAND_NAMESPACE
                       << COMMAND_PAGE << COMMAND_PROPERTY << COMMAND_TYPEALIAS << COMMAND_TYPEDEF
                       << COMMAND_VARIABLE << COMMAND_QMLTYPE << COMMAND_QMLPROPERTY
                       << COMMAND_QMLPROPERTYGROUP // mws 13/03/2019
                       << COMMAND_QMLATTACHEDPROPERTY << COMMAND_QMLSIGNAL
                       << COMMAND_QMLATTACHEDSIGNAL << COMMAND_QMLMETHOD
                       << COMMAND_QMLATTACHEDMETHOD << COMMAND_QMLBASICTYPE << COMMAND_QMLMODULE
                       << COMMAND_JSTYPE << COMMAND_JSPROPERTY
                       << COMMAND_JSPROPERTYGROUP // mws 13/03/2019
                       << COMMAND_JSATTACHEDPROPERTY << COMMAND_JSSIGNAL << COMMAND_JSATTACHEDSIGNAL
                       << COMMAND_JSMETHOD << COMMAND_JSATTACHEDMETHOD << COMMAND_JSBASICTYPE
                       << COMMAND_JSMODULE << COMMAND_STRUCT << COMMAND_UNION;
    }
    if (metaCommands_.isEmpty()) {
        metaCommands_ = commonMetaCommands();
        metaCommands_ << COMMAND_CONTENTSPAGE << COMMAND_INHEADERFILE << COMMAND_NEXTPAGE
                      << COMMAND_OVERLOAD << COMMAND_PREVIOUSPAGE << COMMAND_QMLINSTANTIATES
                      << COMMAND_REIMP << COMMAND_RELATES;
    }
}

/*!
  The constructor initializes a map of special node types
  for identifying important nodes. And it initializes
  some filters for identifying and excluding certain kinds of files.
 */
void CppCodeParser::initializeParser()
{
    CodeParser::initializeParser();

    /*
      All these can appear in a C++ namespace. Don't add
      anything that can't be in a C++ namespace.
     */
    nodeTypeMap_.insert(COMMAND_NAMESPACE, Node::Namespace);
    nodeTypeMap_.insert(COMMAND_CLASS, Node::Class);
    nodeTypeMap_.insert(COMMAND_STRUCT, Node::Struct);
    nodeTypeMap_.insert(COMMAND_UNION, Node::Union);
    nodeTypeMap_.insert(COMMAND_ENUM, Node::Enum);
    nodeTypeMap_.insert(COMMAND_TYPEALIAS, Node::TypeAlias);
    nodeTypeMap_.insert(COMMAND_TYPEDEF, Node::Typedef);
    nodeTypeMap_.insert(COMMAND_PROPERTY, Node::Property);
    nodeTypeMap_.insert(COMMAND_VARIABLE, Node::Variable);

    nodeTypeTestFuncMap_.insert(COMMAND_NAMESPACE, &Node::isNamespace);
    nodeTypeTestFuncMap_.insert(COMMAND_CLASS, &Node::isClassNode);
    nodeTypeTestFuncMap_.insert(COMMAND_STRUCT, &Node::isStruct);
    nodeTypeTestFuncMap_.insert(COMMAND_UNION, &Node::isUnion);
    nodeTypeTestFuncMap_.insert(COMMAND_ENUM, &Node::isEnumType);
    nodeTypeTestFuncMap_.insert(COMMAND_TYPEALIAS, &Node::isTypeAlias);
    nodeTypeTestFuncMap_.insert(COMMAND_TYPEDEF, &Node::isTypedef);
    nodeTypeTestFuncMap_.insert(COMMAND_PROPERTY, &Node::isProperty);
    nodeTypeTestFuncMap_.insert(COMMAND_VARIABLE, &Node::isVariable);

    Config &config = Config::instance();
    QStringList exampleFilePatterns =
            config.getStringList(CONFIG_EXAMPLES + Config::dot + CONFIG_FILEEXTENSIONS);

    // Used for excluding dirs and files from the list of example files
    const auto &excludeDirsList = config.getCanonicalPathList(CONFIG_EXCLUDEDIRS);
    excludeDirs = QSet<QString>(excludeDirsList.cbegin(), excludeDirsList.cend());
    const auto &excludeFilesList = config.getCanonicalPathList(CONFIG_EXCLUDEDIRS);
    excludeFiles = QSet<QString>(excludeFilesList.cbegin(), excludeFilesList.cend());

    if (!exampleFilePatterns.isEmpty())
        exampleNameFilter = exampleFilePatterns.join(' ');
    else
        exampleNameFilter = "*.cpp *.h *.js *.xq *.svg *.xml *.dita *.ui";

    QStringList exampleImagePatterns =
            config.getStringList(CONFIG_EXAMPLES + Config::dot + CONFIG_IMAGEEXTENSIONS);

    if (!exampleImagePatterns.isEmpty())
        exampleImageFilter = exampleImagePatterns.join(' ');
    else
        exampleImageFilter = "*.png";
}

/*!
  Clear the map of common node types and call
  the same function in the base class.
 */
void CppCodeParser::terminateParser()
{
    nodeTypeMap_.clear();
    nodeTypeTestFuncMap_.clear();
    excludeDirs.clear();
    excludeFiles.clear();
    CodeParser::terminateParser();
}

/*!
  Returns a list of extensions for header files.
 */
QStringList CppCodeParser::headerFileNameFilter()
{
    return QStringList();
}

/*!
  Returns a list of extensions for source files, i.e. not
  header files.
 */
QStringList CppCodeParser::sourceFileNameFilter()
{
    return QStringList();
}

/*!
  Returns the set of strings reopresenting the topic commands.
 */
const QSet<QString> &CppCodeParser::topicCommands()
{
    return topicCommands_;
}

/*!
  Process the topic \a command found in the \a doc with argument \a arg.
 */
Node *CppCodeParser::processTopicCommand(const Doc &doc, const QString &command,
                                         const ArgLocPair &arg)
{
    ExtraFuncData extra;
    if (command == COMMAND_FN) {
        Q_UNREACHABLE();
    } else if (nodeTypeMap_.contains(command)) {
        /*
          We should only get in here if the command refers to
          something that can appear in a C++ namespace,
          i.e. a class, another namespace, an enum, a typedef,
          a property or a variable. I think these are handled
          this way to allow the writer to refer to the entity
          without including the namespace qualifier.
         */
        Node::NodeType type = nodeTypeMap_[command];
        QStringList words = arg.first.split(QLatin1Char(' '));
        QStringList path;
        int idx = 0;
        Node *node = nullptr;

        if (type == Node::Variable && words.size() > 1)
            idx = words.size() - 1;
        path = words[idx].split("::");

        node = qdb_->findNodeInOpenNamespace(path, nodeTypeTestFuncMap_[command]);
        if (node == nullptr)
            node = qdb_->findNodeByNameAndType(path, nodeTypeTestFuncMap_[command]);
        if (node == nullptr) {
            if (isWorthWarningAbout(doc)) {
                doc.location().warning(
                        tr("Cannot find '%1' specified with '\\%2' in any header file")
                                .arg(arg.first)
                                .arg(command));
            }
        } else if (node->isAggregate()) {
            if (type == Node::Namespace) {
                NamespaceNode *ns = static_cast<NamespaceNode *>(node);
                ns->markSeen();
                ns->setWhereDocumented(ns->tree()->camelCaseModuleName());
            }
            /*
              This treats a class as a namespace.
             */
            if ((type == Node::Class) || (type == Node::Namespace) || (type == Node::Struct)
                || (type == Node::Union)) {
                if (path.size() > 1) {
                    path.pop_back();
                    QString ns = path.join(QLatin1String("::"));
                    qdb_->insertOpenNamespace(ns);
                }
            }
        }
        return node;
    } else if (command == COMMAND_EXAMPLE) {
        if (Config::generateExamples) {
            ExampleNode *en = new ExampleNode(qdb_->primaryTreeRoot(), arg.first);
            en->setLocation(doc.startLocation());
            setExampleFileLists(en);
            return en;
        }
    } else if (command == COMMAND_EXTERNALPAGE) {
        ExternalPageNode *epn = new ExternalPageNode(qdb_->primaryTreeRoot(), arg.first);
        epn->setLocation(doc.startLocation());
        return epn;
    } else if (command == COMMAND_HEADERFILE) {
        HeaderNode *hn = new HeaderNode(qdb_->primaryTreeRoot(), arg.first);
        hn->setLocation(doc.startLocation());
        return hn;
    } else if (command == COMMAND_GROUP) {
        CollectionNode *cn = qdb_->addGroup(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_MODULE) {
        CollectionNode *cn = qdb_->addModule(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_QMLMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode *cn = qdb_->addQmlModule(blankSplit[0]);
        cn->setLogicalModuleInfo(blankSplit);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_JSMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode *cn = qdb_->addJsModule(blankSplit[0]);
        cn->setLogicalModuleInfo(blankSplit);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_PAGE) {
        Node::PageType ptype = Node::ArticlePage;
        QStringList args = arg.first.split(QLatin1Char(' '));
        if (args.size() > 1) {
            QString t = args[1].toLower();
            if (t == "howto")
                ptype = Node::HowToPage;
            else if (t == "api")
                ptype = Node::ApiPage;
            else if (t == "example")
                ptype = Node::ExamplePage;
            else if (t == "overview")
                ptype = Node::OverviewPage;
            else if (t == "tutorial")
                ptype = Node::TutorialPage;
            else if (t == "faq")
                ptype = Node::FAQPage;
            else if (t == "attribution")
                ptype = Node::AttributionPage;
        }
        PageNode *pn = new PageNode(qdb_->primaryTreeRoot(), args[0], ptype);
        pn->setLocation(doc.startLocation());
        return pn;
    } else if (command == COMMAND_QMLTYPE) {
        QmlTypeNode *qcn = nullptr;
        Node *candidate = qdb_->primaryTreeRoot()->findChildNode(arg.first, Node::QML);
        if (candidate != nullptr && candidate->isQmlType())
            qcn = static_cast<QmlTypeNode *>(candidate);
        else
            qcn = new QmlTypeNode(qdb_->primaryTreeRoot(), arg.first);
        qcn->setLocation(doc.startLocation());
        return qcn;
    } else if (command == COMMAND_JSTYPE) {
        QmlTypeNode *qcn = nullptr;
        Node *candidate = qdb_->primaryTreeRoot()->findChildNode(arg.first, Node::JS);
        if (candidate != nullptr && candidate->isJsType())
            qcn = static_cast<QmlTypeNode *>(candidate);
        else
            qcn = new QmlTypeNode(qdb_->primaryTreeRoot(), arg.first, Node::JsType);
        qcn->setLocation(doc.startLocation());
        return qcn;
    } else if (command == COMMAND_QMLBASICTYPE) {
        QmlBasicTypeNode *n = new QmlBasicTypeNode(qdb_->primaryTreeRoot(), arg.first);
        n->setLocation(doc.startLocation());
        return n;
    } else if (command == COMMAND_JSBASICTYPE) {
        QmlBasicTypeNode *n =
                new QmlBasicTypeNode(qdb_->primaryTreeRoot(), arg.first, Node::JsBasicType);
        n->setLocation(doc.startLocation());
        return n;
    } else if ((command == COMMAND_QMLSIGNAL) || (command == COMMAND_QMLMETHOD)
               || (command == COMMAND_QMLATTACHEDSIGNAL) || (command == COMMAND_QMLATTACHEDMETHOD)
               || (command == COMMAND_JSSIGNAL) || (command == COMMAND_JSMETHOD)
               || (command == COMMAND_JSATTACHEDSIGNAL) || (command == COMMAND_JSATTACHEDMETHOD)) {
        Q_UNREACHABLE();
    }
    return nullptr;
}

/*!
  A QML property argument has the form...

  <type> <QML-type>::<name>
  <type> <QML-module>::<QML-type>::<name>

  This function splits the argument into one of those
  two forms. The three part form is the old form, which
  was used before the creation of Qt Quick 2 and Qt
  Components. A <QML-module> is the QML equivalent of a
  C++ namespace. So this function splits \a arg on "::"
  and stores the parts in \a type, \a module, \a qmlTypeName,
  and \a name, and returns \c true. If any part other than
  \a module is not found, a qdoc warning is emitted and
  false is returned.

  \note The two QML types \e{Component} and \e{QtObject}
  never have a module qualifier.
 */
bool CppCodeParser::splitQmlPropertyArg(const QString &arg, QString &type, QString &module,
                                        QString &qmlTypeName, QString &name,
                                        const Location &location)
{
    QStringList blankSplit = arg.split(QLatin1Char(' '));
    if (blankSplit.size() > 1) {
        type = blankSplit[0];
        QStringList colonSplit(blankSplit[1].split("::"));
        if (colonSplit.size() == 3) {
            module = colonSplit[0];
            qmlTypeName = colonSplit[1];
            name = colonSplit[2];
            return true;
        }
        if (colonSplit.size() == 2) {
            module.clear();
            qmlTypeName = colonSplit[0];
            name = colonSplit[1];
            return true;
        }
        QString msg = "Unrecognizable QML module/component qualifier for " + arg;
        location.warning(tr(msg.toLatin1().data()));
    } else {
        QString msg = "Missing property type for " + arg;
        location.warning(tr(msg.toLatin1().data()));
    }
    return false;
}

/*!
 */
void CppCodeParser::processQmlProperties(const Doc &doc, NodeList &nodes, DocList &docs)
{
    const TopicList &topics = doc.topicsUsed();
    if (topics.isEmpty())
        return;

    QString arg;
    QString type;
    QString group;
    QString module;
    QString property;
    QString qmlTypeName;

    Topic topic = topics.at(0);
    bool jsProps = isJSPropertyTopic(topic.topic);
    arg = topic.args;
    if (splitQmlPropertyArg(arg, type, module, qmlTypeName, property, doc.location())) {
        int i = property.indexOf('.');
        if (i != -1)
            group = property.left(i);
    }

    NodeList sharedNodes;
    QmlTypeNode *qmlType = qdb_->findQmlType(module, qmlTypeName);
    if (qmlType == nullptr)
        qmlType = new QmlTypeNode(qdb_->primaryTreeRoot(), qmlTypeName);

    for (int i = 0; i < topics.size(); ++i) {
        QString cmd = topics.at(i).topic;
        arg = topics.at(i).args;
        if ((cmd == COMMAND_QMLPROPERTY) || (cmd == COMMAND_QMLATTACHEDPROPERTY)
            || (cmd == COMMAND_JSPROPERTY) || (cmd == COMMAND_JSATTACHEDPROPERTY)) {
            bool attached = topics.at(i).topic.contains(QLatin1String("attached"));
            if (splitQmlPropertyArg(arg, type, module, qmlTypeName, property, doc.location())) {
                if (qmlType != qdb_->findQmlType(module, qmlTypeName)) {
                    QString msg = tr("All properties in a group must belong to the same type: '%1'")
                                          .arg(arg);
                    doc.startLocation().warning(msg);
                    continue;
                }
                if (qmlType->hasQmlProperty(property, attached) != nullptr) {
                    QString msg = tr("QML property documented multiple times: '%1'").arg(arg);
                    doc.startLocation().warning(msg);
                    continue;
                }
                QmlPropertyNode *qpn = new QmlPropertyNode(qmlType, property, type, attached);
                qpn->setLocation(doc.startLocation());
                qpn->setGenus(jsProps ? Node::JS : Node::QML);
                nodes.append(qpn);
                docs.append(doc);
                sharedNodes << qpn;
            }
        } else {
            doc.startLocation().warning(
                    tr("Command '\\%1'; not allowed with QML/JS property commands").arg(cmd));
        }
    }

    // Construct a SharedCommentNode (scn) if multiple topics generated
    // valid nodes. Note that it's important to do this *after* constructing
    // the topic nodes - which need to be written to index before the related
    // scn.
    if (sharedNodes.count() > 1) {
        SharedCommentNode *scn = new SharedCommentNode(qmlType, sharedNodes.count(), group);
        scn->setLocation(doc.startLocation());
        nodes.append(scn);
        docs.append(doc);
        for (const auto n : sharedNodes)
            scn->append(n);
        scn->sort();
    }
}

/*!
  Returns the set of strings representing the common metacommands
  plus some other metacommands.
 */
const QSet<QString> &CppCodeParser::metaCommands()
{
    return metaCommands_;
}

/*!
  Process the metacommand \a command in the context of the
  \a node associated with the topic command and the \a doc.
  \a arg is the argument to the metacommand.

  \a node is guaranteed to be non-null.
 */
void CppCodeParser::processMetaCommand(const Doc &doc, const QString &command,
                                       const ArgLocPair &argLocPair, Node *node)
{
    QString arg = argLocPair.first;
    if (command == COMMAND_INHEADERFILE) {
        if (node->isAggregate())
            static_cast<Aggregate *>(node)->addIncludeFile(arg);
        else
            doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_INHEADERFILE));
    } else if (command == COMMAND_OVERLOAD) {
        /*
          Note that this might set the overload flag of the
          primary function. This is ok because the overload
          flags and overload numbers will be resolved later
          in Aggregate::normalizeOverloads().
         */
        if (node->isFunction())
            static_cast<FunctionNode *>(node)->setOverloadFlag();
        else if (node->isSharedCommentNode())
            static_cast<SharedCommentNode *>(node)->setOverloadFlags();
        else
            doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_OVERLOAD));
    } else if (command == COMMAND_REIMP) {
        if (node->parent() && !node->parent()->isInternal()) {
            if (node->isFunction()) {
                FunctionNode *fn = static_cast<FunctionNode *>(node);
                // The clang visitor class will have set the
                // qualified name of the ovverridden function.
                // If the name of the overridden function isn't
                // set, issue a warning.
                if (fn->overridesThis().isEmpty() && isWorthWarningAbout(doc)) {
                    doc.location().warning(tr("Cannot find base function for '\\%1' in %2()")
                                                   .arg(COMMAND_REIMP)
                                                   .arg(node->name()),
                                           tr("The function either doesn't exist in any "
                                              "base class with the same signature or it "
                                              "exists but isn't virtual."));
                }
                fn->setReimpFlag();
            } else {
                doc.location().warning(
                        tr("Ignored '\\%1' in %2").arg(COMMAND_REIMP).arg(node->name()));
            }
        }
    } else if (command == COMMAND_RELATES) {
        QStringList path = arg.split("::");
        Aggregate *aggregate = qdb_->findRelatesNode(path);
        if (aggregate == nullptr)
            aggregate = new ProxyNode(node->root(), arg);

        if (node->parent() == aggregate) { // node is already a child of aggregate
            doc.location().warning(
                    tr("Invalid '\\%1' (already a member of '%2')").arg(COMMAND_RELATES, arg));
        } else {
            if (node->isAggregate()) {
                doc.location().warning(tr("Invalid '\\%1' not allowed in '\\%2'")
                                               .arg(COMMAND_RELATES, node->nodeTypeString()));
            } else if (!node->isRelatedNonmember() &&
                       !node->parent()->isNamespace() && !node->parent()->isHeader()) {
                if (!doc.isInternal()) {
                    doc.location().warning(tr("Invalid '\\%1' ('%2' must be global)")
                                                   .arg(COMMAND_RELATES, node->name()));
                }
            } else if (!node->isRelatedNonmember() && !node->parent()->isHeader()) {
                aggregate->adoptChild(node);
                node->setRelatedNonmember(true);
            } else {
                /*
                  There are multiple \relates commands. This
                  one is not the first, so clone the node as
                  a child of aggregate.
                 */
                Node *clone = node->clone(aggregate);
                if (clone == nullptr) {
                    doc.location().warning(tr("Invalid '\\%1' (multiple uses not allowed in '%2')")
                                                   .arg(COMMAND_RELATES, node->nodeTypeString()));
                } else {
                    clone->setRelatedNonmember(true);
                }
            }
        }
    } else if (command == COMMAND_CONTENTSPAGE) {
        qCWarning(lcQdoc, "(qdoc) The \\contentspage command is obsolete and should not be used.");
        setLink(node, Node::ContentsLink, arg);
    } else if (command == COMMAND_NEXTPAGE) {
        setLink(node, Node::NextLink, arg);
    } else if (command == COMMAND_PREVIOUSPAGE) {
        setLink(node, Node::PreviousLink, arg);
    } else if (command == COMMAND_STARTPAGE) {
        setLink(node, Node::StartLink, arg);
    } else if (command == COMMAND_QMLINHERITS) {
        if (node->name() == arg)
            doc.location().warning(tr("%1 tries to inherit itself").arg(arg));
        else if (node->isQmlType() || node->isJsType()) {
            QmlTypeNode *qmlType = static_cast<QmlTypeNode *>(node);
            qmlType->setQmlBaseName(arg);
        }
    } else if (command == COMMAND_QMLINSTANTIATES) {
        if (node->isQmlType() || node->isJsType()) {
            ClassNode *classNode = qdb_->findClassNode(arg.split("::"));
            if (classNode)
                node->setClassNode(classNode);
            else
                doc.location().warning(tr("C++ class %1 not found: \\instantiates %1").arg(arg));
        } else
            doc.location().warning(tr("\\instantiates is only allowed in \\qmltype"));
    } else if (command == COMMAND_QMLDEFAULT) {
        node->markDefault();
    } else if (command == COMMAND_QMLREADONLY) {
        node->markReadOnly(1);
    } else if ((command == COMMAND_QMLABSTRACT) || (command == COMMAND_ABSTRACT)) {
        if (node->isQmlType() || node->isJsType())
            node->setAbstract(true);
    } else if (command == COMMAND_DEPRECATED) {
        node->setStatus(Node::Obsolete);
    } else if (command == COMMAND_INGROUP || command == COMMAND_INPUBLICGROUP) {
        // Note: \ingroup and \inpublicgroup are the same (and now recognized as such).
        qdb_->addToGroup(arg, node);
    } else if (command == COMMAND_INMODULE) {
        qdb_->addToModule(arg, node);
    } else if (command == COMMAND_INQMLMODULE) {
        qdb_->addToQmlModule(arg, node);
    } else if (command == COMMAND_INJSMODULE) {
        qdb_->addToJsModule(arg, node);
    } else if (command == COMMAND_MAINCLASS) {
        node->doc().location().warning(
                tr("'\\mainclass' is deprecated. Consider '\\ingroup mainclasses'"));
    } else if (command == COMMAND_OBSOLETE) {
        node->setStatus(Node::Obsolete);
    } else if (command == COMMAND_NONREENTRANT) {
        node->setThreadSafeness(Node::NonReentrant);
    } else if (command == COMMAND_PRELIMINARY) {
        // \internal wins.
        if (!node->isInternal())
            node->setStatus(Node::Preliminary);
    } else if (command == COMMAND_INTERNAL) {
        if (!showInternal())
            node->markInternal();
    } else if (command == COMMAND_REENTRANT) {
        node->setThreadSafeness(Node::Reentrant);
    } else if (command == COMMAND_SINCE) {
        node->setSince(arg);
    } else if (command == COMMAND_WRAPPER) {
        node->setWrapper();
    } else if (command == COMMAND_THREADSAFE) {
        node->setThreadSafeness(Node::ThreadSafe);
    } else if (command == COMMAND_TITLE) {
        if (!node->setTitle(arg))
            doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_TITLE));
        else if (node->isExample())
            qdb_->addExampleNode(static_cast<ExampleNode *>(node));
    } else if (command == COMMAND_SUBTITLE) {
        if (!node->setSubtitle(arg))
            doc.location().warning(tr("Ignored '\\%1'").arg(COMMAND_SUBTITLE));
    } else if (command == COMMAND_QTVARIABLE) {
        node->setQtVariable(arg);
        if (!node->isModule() && !node->isQmlModule())
            doc.location().warning(
                    tr("Command '\\%1' is only meaningful in '\\module' and '\\qmlmodule'.")
                            .arg(COMMAND_QTVARIABLE));
    } else if (command == COMMAND_NOAUTOLIST) {
        node->setNoAutoList(true);
    }
}

/*!
  The topic command has been processed, and now \a doc and
  \a node are passed to this function to get the metacommands
  from \a doc and process them one at a time. \a node is the
  node where \a doc resides.
 */
void CppCodeParser::processMetaCommands(const Doc &doc, Node *node)
{
    const QStringList metaCommandsUsed = doc.metaCommandsUsed().values();
    for (const auto &command : metaCommandsUsed) {
        const ArgList args = doc.metaCommandArgs(command);
        for (const auto &arg : args)
            processMetaCommand(doc, command, arg, node);
    }
}

/*!
 Parse QML/JS signal/method topic commands.
 */
FunctionNode *CppCodeParser::parseOtherFuncArg(const QString &topic, const Location &location,
                                               const QString &funcArg)
{
    QString funcName;
    QString returnType;

    int leftParen = funcArg.indexOf(QChar('('));
    if (leftParen > 0)
        funcName = funcArg.left(leftParen);
    else
        funcName = funcArg;
    int firstBlank = funcName.indexOf(QChar(' '));
    if (firstBlank > 0) {
        returnType = funcName.left(firstBlank);
        funcName = funcName.right(funcName.length() - firstBlank - 1);
    }

    QStringList colonSplit(funcName.split("::"));
    if (colonSplit.size() < 2) {
        QString msg = "Unrecognizable QML module/component qualifier for " + funcArg;
        location.warning(tr(msg.toLatin1().data()));
        return nullptr;
    }
    QString moduleName;
    QString elementName;
    if (colonSplit.size() > 2) {
        moduleName = colonSplit[0];
        elementName = colonSplit[1];
    } else {
        elementName = colonSplit[0];
    }
    funcName = colonSplit.last();

    Aggregate *aggregate = qdb_->findQmlType(moduleName, elementName);
    if (aggregate == nullptr)
        aggregate = qdb_->findQmlBasicType(moduleName, elementName);
    if (aggregate == nullptr)
        return nullptr;

    QString params;
    QStringList leftParenSplit = funcArg.split('(');
    if (leftParenSplit.size() > 1) {
        QStringList rightParenSplit = leftParenSplit[1].split(')');
        if (rightParenSplit.size() > 0)
            params = rightParenSplit[0];
    }

    FunctionNode::Metaness metaness = FunctionNode::getMetanessFromTopic(topic);
    bool attached = topic.contains(QLatin1String("attached"));
    FunctionNode *fn = new FunctionNode(metaness, aggregate, funcName, attached);
    fn->setAccess(Node::Public);
    fn->setLocation(location);
    fn->setReturnType(returnType);
    fn->setParameters(params);
    return fn;
}

/*!
  Parse the macro arguments in \a macroArg ad hoc, without using
  any actual parser. If successful, return a pointer to the new
  FunctionNode for the macro. Otherwise return null. \a location
  is used for reporting errors.
 */
FunctionNode *CppCodeParser::parseMacroArg(const Location &location, const QString &macroArg)
{
    QStringList leftParenSplit = macroArg.split('(');
    if (leftParenSplit.isEmpty())
        return nullptr;
    QString macroName;
    FunctionNode *oldMacroNode = nullptr;
    QStringList blankSplit = leftParenSplit[0].split(' ');
    if (blankSplit.size() > 0) {
        macroName = blankSplit.last();
        oldMacroNode = qdb_->findMacroNode(macroName);
    }
    QString returnType;
    if (blankSplit.size() > 1) {
        blankSplit.removeLast();
        returnType = blankSplit.join(' ');
    }
    QString params;
    if (leftParenSplit.size() > 1) {
        const QString &afterParen = leftParenSplit.at(1);
        int rightParen = afterParen.indexOf(')');
        if (rightParen >= 0)
            params = afterParen.left(rightParen);
    }
    int i = 0;
    while (i < macroName.length() && !macroName.at(i).isLetter())
        i++;
    if (i > 0) {
        returnType += QChar(' ') + macroName.left(i);
        macroName = macroName.mid(i);
    }
    FunctionNode::Metaness metaness = FunctionNode::MacroWithParams;
    if (params.isEmpty())
        metaness = FunctionNode::MacroWithoutParams;
    FunctionNode *macro = new FunctionNode(metaness, qdb_->primaryTreeRoot(), macroName);
    macro->setAccess(Node::Public);
    macro->setLocation(location);
    macro->setReturnType(returnType);
    macro->setParameters(params);
    if (oldMacroNode && macro->compare(oldMacroNode)) {
        location.warning(tr("\\macro %1 documented more than once").arg(macroArg));
        oldMacroNode->doc().location().warning(tr("(The previous doc is here)"));
    }
    return macro;
}

void CppCodeParser::setExampleFileLists(ExampleNode *en)
{
    Config &config = Config::instance();
    QString fullPath = config.getExampleProjectFile(en->name());
    if (fullPath.isEmpty()) {
        QString details = QLatin1String("Example directories: ")
                + config.getCanonicalPathList(CONFIG_EXAMPLEDIRS).join(QLatin1Char(' '));
        en->location().warning(
                tr("Cannot find project file for example '%1'").arg(en->name()), details);
        return;
    }

    QDir exampleDir(QFileInfo(fullPath).dir());

    QStringList exampleFiles = Config::getFilesHere(exampleDir.path(), exampleNameFilter, Location(),
                                                    excludeDirs, excludeFiles);
    // Search for all image files under the example project, excluding doc/images directory.
    QSet<QString> excludeDocDirs(excludeDirs);
    excludeDocDirs.insert(exampleDir.path() + QLatin1String("/doc/images"));
    QStringList imageFiles = Config::getFilesHere(exampleDir.path(), exampleImageFilter, Location(),
                                                  excludeDocDirs, excludeFiles);
    if (!exampleFiles.isEmpty()) {
        // move main.cpp to the end, if it exists
        QString mainCpp;

        const auto isGeneratedOrMainCpp = [&mainCpp](const QString &fileName) {
            if (fileName.endsWith("/main.cpp")) {
                if (mainCpp.isEmpty())
                    mainCpp = fileName;
                return true;
            }
            return fileName.contains("/qrc_") || fileName.contains("/moc_")
                    || fileName.contains("/ui_");
        };

        exampleFiles.erase(
                std::remove_if(exampleFiles.begin(), exampleFiles.end(), isGeneratedOrMainCpp),
                exampleFiles.end());

        if (!mainCpp.isEmpty())
            exampleFiles.append(mainCpp);

        // Add any resource and project files
        exampleFiles += Config::getFilesHere(exampleDir.path(),
                QLatin1String("*.qrc *.pro *.qmlproject *.pyproject CMakeLists.txt qmldir"));
    }

    const int pathLen = exampleDir.path().size() - en->name().size();
    for (auto &file : exampleFiles)
        file = file.mid(pathLen);
    for (auto &file : imageFiles)
        file = file.mid(pathLen);

    en->setFiles(exampleFiles, fullPath.mid(pathLen));
    en->setImages(imageFiles);
}

/*!
  returns true if \a t is \e {jssignal}, \e {jsmethod},
  \e {jsattachedsignal}, or \e {jsattachedmethod}.
 */
bool CppCodeParser::isJSMethodTopic(const QString &t)
{
    return (t == COMMAND_JSSIGNAL || t == COMMAND_JSMETHOD || t == COMMAND_JSATTACHEDSIGNAL
            || t == COMMAND_JSATTACHEDMETHOD);
}

/*!
  returns true if \a t is \e {qmlsignal}, \e {qmlmethod},
  \e {qmlattachedsignal}, or \e {qmlattachedmethod}.
 */
bool CppCodeParser::isQMLMethodTopic(const QString &t)
{
    return (t == COMMAND_QMLSIGNAL || t == COMMAND_QMLMETHOD || t == COMMAND_QMLATTACHEDSIGNAL
            || t == COMMAND_QMLATTACHEDMETHOD);
}

/*!
  Returns true if \a t is \e {jsproperty}, \e {jspropertygroup},
  or \e {jsattachedproperty}.
 */
bool CppCodeParser::isJSPropertyTopic(const QString &t)
{
    return (t == COMMAND_JSPROPERTY || t == COMMAND_JSATTACHEDPROPERTY);
}

/*!
  Returns true if \a t is \e {qmlproperty}, \e {qmlpropertygroup},
  or \e {qmlattachedproperty}.
 */
bool CppCodeParser::isQMLPropertyTopic(const QString &t)
{
    return (t == COMMAND_QMLPROPERTY || t == COMMAND_QMLATTACHEDPROPERTY);
}

void CppCodeParser::processTopicArgs(const Doc &doc, const QString &topic, NodeList &nodes,
                                     DocList &docs)
{
    if (isQMLPropertyTopic(topic) || isJSPropertyTopic(topic)) {
        processQmlProperties(doc, nodes, docs);
    } else {
        ArgList args = doc.metaCommandArgs(topic);
        Node *node = nullptr;
        if (args.size() == 1) {
            if (topic == COMMAND_FN) {
                if (showInternal() || !doc.isInternal())
                    node = parserForLanguage("Clang")->parseFnArg(doc.location(), args[0].first);
            } else if (topic == COMMAND_MACRO) {
                node = parseMacroArg(doc.location(), args[0].first);
            } else if (isQMLMethodTopic(topic) || isJSMethodTopic(topic)) {
                node = parseOtherFuncArg(topic, doc.location(), args[0].first);
            } else if (topic == COMMAND_DONTDOCUMENT) {
                qdb_->primaryTree()->addToDontDocumentMap(args[0].first);
            } else {
                node = processTopicCommand(doc, topic, args[0]);
            }
            if (node != nullptr) {
                nodes.append(node);
                docs.append(doc);
            }
        } else if (args.size() > 1) {
            QVector<SharedCommentNode *> sharedCommentNodes;
            for (const auto &arg : qAsConst(args)) {
                node = nullptr;
                if (topic == COMMAND_FN) {
                    if (showInternal() || !doc.isInternal())
                        node = parserForLanguage("Clang")->parseFnArg(doc.location(), arg.first);
                } else if (topic == COMMAND_MACRO) {
                    node = parseMacroArg(doc.location(), arg.first);
                } else if (isQMLMethodTopic(topic) || isJSMethodTopic(topic)) {
                    node = parseOtherFuncArg(topic, doc.location(), arg.first);
                } else {
                    node = processTopicCommand(doc, topic, arg);
                }
                if (node != nullptr) {
                    bool found = false;
                    for (SharedCommentNode *scn : sharedCommentNodes) {
                        if (scn->parent() == node->parent()) {
                            scn->append(node);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        SharedCommentNode *scn = new SharedCommentNode(node);
                        sharedCommentNodes.append(scn);
                        nodes.append(scn);
                        docs.append(doc);
                    }
                }
            }
            for (auto *scn : sharedCommentNodes)
                scn->sort();
        }
    }
}

void CppCodeParser::processMetaCommands(NodeList &nodes, DocList &docs)
{
    QVector<Doc>::Iterator d = docs.begin();
    for (const auto &node : nodes) {
        if (node != nullptr) {
            processMetaCommands(*d, node);
            node->setDoc(*d);
            checkModuleInclusion(node);
            if (node->isAggregate()) {
                Aggregate *aggregate = static_cast<Aggregate *>(node);
                if (aggregate->includeFiles().isEmpty()) {
                    Aggregate *parent = aggregate;
                    while (parent->physicalModuleName().isEmpty() && (parent->parent() != nullptr))
                        parent = parent->parent();
                    if (parent == aggregate)
                        aggregate->addIncludeFile(aggregate->name());
                    else
                        aggregate->setIncludeFiles(parent->includeFiles());
                }
            }
        }
        ++d;
    }
}

bool CppCodeParser::hasTooManyTopics(const Doc &doc) const
{
    QSet<QString> topicCommandsUsed = topicCommands() & doc.metaCommandsUsed();
    if (topicCommandsUsed.count() > 1) {
        bool ok = true;
        for (const auto &t : topicCommandsUsed) {
            if (!t.startsWith(QLatin1String("qml")) && !t.startsWith(QLatin1String("js")))
                ok = false;
        }
        if (ok)
            return false;
        QString topicList;
        for (const auto &t : topicCommandsUsed)
            topicList += QLatin1String(" \\") + t + QLatin1Char(',');
        topicList[topicList.lastIndexOf(',')] = '.';
        int i = topicList.lastIndexOf(',');
        Q_ASSERT(i >= 0); // we had at least two commas
        topicList[i] = ' ';
        topicList.insert(i + 1, "and");
        doc.location().warning(tr("Multiple topic commands found in comment:%1").arg(topicList));
        return true;
    }
    return false;
}

QT_END_NAMESPACE
