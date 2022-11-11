// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cppcodeparser.h"

#include "access.h"
#include "classnode.h"
#include "collectionnode.h"
#include "config.h"
#include "examplenode.h"
#include "externalpagenode.h"
#include "functionnode.h"
#include "generator.h"
#include "headernode.h"
#include "namespacenode.h"
#include "qdocdatabase.h"
#include "qmltypenode.h"
#include "qmlpropertynode.h"
#include "sharedcommentnode.h"
#include "utilities.h"

#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

/* qmake ignore Q_OBJECT */

QSet<QString> CppCodeParser::m_excludeDirs;
QSet<QString> CppCodeParser::m_excludeFiles;

static QSet<QString> topicCommands_;
static QSet<QString> metaCommands_;

/*!
  The constructor initializes some regular expressions
  and initializes the tokenizer variables.
 */
CppCodeParser::CppCodeParser()
{
    if (topicCommands_.isEmpty()) {
        topicCommands_ << COMMAND_CLASS << COMMAND_DONTDOCUMENT << COMMAND_ENUM << COMMAND_EXAMPLE
                       << COMMAND_EXTERNALPAGE << COMMAND_FN << COMMAND_GROUP << COMMAND_HEADERFILE
                       << COMMAND_MACRO << COMMAND_MODULE << COMMAND_NAMESPACE << COMMAND_PAGE
                       << COMMAND_PROPERTY << COMMAND_TYPEALIAS << COMMAND_TYPEDEF
                       << COMMAND_VARIABLE << COMMAND_QMLTYPE << COMMAND_QMLPROPERTY
                       << COMMAND_QMLPROPERTYGROUP // mws 13/03/2019
                       << COMMAND_QMLATTACHEDPROPERTY << COMMAND_QMLSIGNAL
                       << COMMAND_QMLATTACHEDSIGNAL << COMMAND_QMLMETHOD
                       << COMMAND_QMLATTACHEDMETHOD << COMMAND_QMLVALUETYPE << COMMAND_QMLBASICTYPE
                       << COMMAND_QMLMODULE
                       << COMMAND_JSTYPE << COMMAND_JSPROPERTY
                       << COMMAND_JSPROPERTYGROUP // mws 13/03/2019
                       << COMMAND_JSATTACHEDPROPERTY << COMMAND_JSSIGNAL << COMMAND_JSATTACHEDSIGNAL
                       << COMMAND_JSMETHOD << COMMAND_JSATTACHEDMETHOD << COMMAND_JSBASICTYPE
                       << COMMAND_JSMODULE << COMMAND_STRUCT << COMMAND_UNION;
    }
    if (metaCommands_.isEmpty()) {
        metaCommands_ = commonMetaCommands();
        metaCommands_ << COMMAND_INHEADERFILE << COMMAND_NEXTPAGE
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
    m_nodeTypeMap.insert(COMMAND_NAMESPACE, Node::Namespace);
    m_nodeTypeMap.insert(COMMAND_CLASS, Node::Class);
    m_nodeTypeMap.insert(COMMAND_STRUCT, Node::Struct);
    m_nodeTypeMap.insert(COMMAND_UNION, Node::Union);
    m_nodeTypeMap.insert(COMMAND_ENUM, Node::Enum);
    m_nodeTypeMap.insert(COMMAND_TYPEALIAS, Node::TypeAlias);
    m_nodeTypeMap.insert(COMMAND_TYPEDEF, Node::Typedef);
    m_nodeTypeMap.insert(COMMAND_PROPERTY, Node::Property);
    m_nodeTypeMap.insert(COMMAND_VARIABLE, Node::Variable);

    m_nodeTypeTestFuncMap.insert(COMMAND_NAMESPACE, &Node::isNamespace);
    m_nodeTypeTestFuncMap.insert(COMMAND_CLASS, &Node::isClassNode);
    m_nodeTypeTestFuncMap.insert(COMMAND_STRUCT, &Node::isStruct);
    m_nodeTypeTestFuncMap.insert(COMMAND_UNION, &Node::isUnion);
    m_nodeTypeTestFuncMap.insert(COMMAND_ENUM, &Node::isEnumType);
    m_nodeTypeTestFuncMap.insert(COMMAND_TYPEALIAS, &Node::isTypeAlias);
    m_nodeTypeTestFuncMap.insert(COMMAND_TYPEDEF, &Node::isTypedef);
    m_nodeTypeTestFuncMap.insert(COMMAND_PROPERTY, &Node::isProperty);
    m_nodeTypeTestFuncMap.insert(COMMAND_VARIABLE, &Node::isVariable);

    Config &config = Config::instance();
    QStringList exampleFilePatterns =
            config.getStringList(CONFIG_EXAMPLES + Config::dot + CONFIG_FILEEXTENSIONS);

    // Used for excluding dirs and files from the list of example files
    const auto &excludeDirsList = config.getCanonicalPathList(CONFIG_EXCLUDEDIRS);
    m_excludeDirs = QSet<QString>(excludeDirsList.cbegin(), excludeDirsList.cend());
    const auto &excludeFilesList = config.getCanonicalPathList(CONFIG_EXCLUDEDIRS);
    m_excludeFiles = QSet<QString>(excludeFilesList.cbegin(), excludeFilesList.cend());

    if (!exampleFilePatterns.isEmpty())
        m_exampleNameFilter = exampleFilePatterns.join(' ');
    else
        m_exampleNameFilter = "*.cpp *.h *.js *.xq *.svg *.xml *.ui";

    QStringList exampleImagePatterns =
            config.getStringList(CONFIG_EXAMPLES + Config::dot + CONFIG_IMAGEEXTENSIONS);

    if (!exampleImagePatterns.isEmpty())
        m_exampleImageFilter = exampleImagePatterns.join(' ');
    else
        m_exampleImageFilter = "*.png";

    m_showLinkErrors = !config.getBool(CONFIG_NOLINKERRORS);
}

/*!
  Clear the map of common node types and call
  the same function in the base class.
 */
void CppCodeParser::terminateParser()
{
    m_nodeTypeMap.clear();
    m_nodeTypeTestFuncMap.clear();
    m_excludeDirs.clear();
    m_excludeFiles.clear();
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
  Returns the set of strings representing the topic commands.
 */
const QSet<QString> &CppCodeParser::topicCommands()
{
    return topicCommands_;
}

/*!
  Process the topic \a command found in the \a doc with argument \a arg.
 */
Node *CppCodeParser::processTopicCommand(const Doc &doc, const QString &command,
                                         const ArgPair &arg)
{
    ExtraFuncData extra;
    if (command == COMMAND_FN) {
        Q_UNREACHABLE();
    } else if (m_nodeTypeMap.contains(command)) {
        /*
          We should only get in here if the command refers to
          something that can appear in a C++ namespace,
          i.e. a class, another namespace, an enum, a typedef,
          a property or a variable. I think these are handled
          this way to allow the writer to refer to the entity
          without including the namespace qualifier.
         */
        Node::NodeType type = m_nodeTypeMap[command];
        QStringList words = arg.first.split(QLatin1Char(' '));
        QStringList path;
        qsizetype idx = 0;
        Node *node = nullptr;

        if (type == Node::Variable && words.size() > 1)
            idx = words.size() - 1;
        path = words[idx].split("::");

        node = m_qdb->findNodeInOpenNamespace(path, m_nodeTypeTestFuncMap[command]);
        if (node == nullptr)
            node = m_qdb->findNodeByNameAndType(path, m_nodeTypeTestFuncMap[command]);
        // Allow representing a type alias as a class
        if (node == nullptr && command == COMMAND_CLASS) {
            node = m_qdb->findNodeByNameAndType(path, &Node::isTypeAlias);
            if (node) {
                auto access = node->access();
                auto loc = node->location();
                auto templateDecl = node->templateDecl();
                node = new ClassNode(Node::Class, node->parent(), node->name());
                node->setAccess(access);
                node->setLocation(loc);
                node->setTemplateDecl(templateDecl);
            }
        }
        if (node == nullptr) {
            if (isWorthWarningAbout(doc)) {
                doc.location().warning(
                        QStringLiteral("Cannot find '%1' specified with '\\%2' in any header file")
                                .arg(arg.first, command));
            }
        } else if (node->isAggregate()) {
            if (type == Node::Namespace) {
                auto *ns = static_cast<NamespaceNode *>(node);
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
                    m_qdb->insertOpenNamespace(ns);
                }
            }
        }
        return node;
    } else if (command == COMMAND_EXAMPLE) {
        if (Config::generateExamples) {
            auto *en = new ExampleNode(m_qdb->primaryTreeRoot(), arg.first);
            en->setLocation(doc.startLocation());
            setExampleFileLists(en);
            return en;
        }
    } else if (command == COMMAND_EXTERNALPAGE) {
        auto *epn = new ExternalPageNode(m_qdb->primaryTreeRoot(), arg.first);
        epn->setLocation(doc.startLocation());
        return epn;
    } else if (command == COMMAND_HEADERFILE) {
        auto *hn = new HeaderNode(m_qdb->primaryTreeRoot(), arg.first);
        hn->setLocation(doc.startLocation());
        return hn;
    } else if (command == COMMAND_GROUP) {
        CollectionNode *cn = m_qdb->addGroup(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_MODULE) {
        CollectionNode *cn = m_qdb->addModule(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_QMLMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode *cn = m_qdb->addQmlModule(blankSplit[0]);
        cn->setLogicalModuleInfo(blankSplit);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_JSMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode *cn = m_qdb->addJsModule(blankSplit[0]);
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
        auto *pn = new PageNode(m_qdb->primaryTreeRoot(), args[0], ptype);
        pn->setLocation(doc.startLocation());
        return pn;
    } else if (command == COMMAND_QMLTYPE) {
        QmlTypeNode *qcn = nullptr;
        Node *candidate = m_qdb->primaryTreeRoot()->findChildNode(arg.first, Node::QML);
        if (candidate != nullptr && candidate->isQmlType())
            qcn = static_cast<QmlTypeNode *>(candidate);
        else
            qcn = new QmlTypeNode(m_qdb->primaryTreeRoot(), arg.first);
        qcn->setLocation(doc.startLocation());
        return qcn;
    } else if (command == COMMAND_JSTYPE) {
        QmlTypeNode *qcn = nullptr;
        Node *candidate = m_qdb->primaryTreeRoot()->findChildNode(arg.first, Node::JS);
        if (candidate != nullptr && candidate->isJsType())
            qcn = static_cast<QmlTypeNode *>(candidate);
        else
            qcn = new QmlTypeNode(m_qdb->primaryTreeRoot(), arg.first, Node::JsType);
        qcn->setLocation(doc.startLocation());
        return qcn;
    } else if (command == COMMAND_QMLVALUETYPE || command == COMMAND_QMLBASICTYPE) {
        auto *node = new QmlValueTypeNode(m_qdb->primaryTreeRoot(), arg.first);
        node->setLocation(doc.startLocation());
        return node;
    } else if (command == COMMAND_JSBASICTYPE) {
        auto *node = new QmlValueTypeNode(m_qdb->primaryTreeRoot(), arg.first, Node::JsBasicType);
        node->setLocation(doc.startLocation());
        return node;
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
        location.warning(
                QStringLiteral("Unrecognizable QML module/component qualifier for %1").arg(arg));
    } else {
        location.warning(QStringLiteral("Missing property type for %1").arg(arg));
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
    bool jsProps = isJSPropertyTopic(topic.m_topic);
    arg = topic.m_args;
    if (splitQmlPropertyArg(arg, type, module, qmlTypeName, property, doc.location())) {
        qsizetype i = property.indexOf('.');
        if (i != -1)
            group = property.left(i);
    }

    NodeList sharedNodes;
    QmlTypeNode *qmlType = m_qdb->findQmlType(module, qmlTypeName);
    if (qmlType == nullptr)
        qmlType = new QmlTypeNode(m_qdb->primaryTreeRoot(), qmlTypeName);

    for (const auto &topicCommand : topics) {
        QString cmd = topicCommand.m_topic;
        arg = topicCommand.m_args;
        if ((cmd == COMMAND_QMLPROPERTY) || (cmd == COMMAND_QMLATTACHEDPROPERTY)
            || (cmd == COMMAND_JSPROPERTY) || (cmd == COMMAND_JSATTACHEDPROPERTY)) {
            bool attached = cmd.contains(QLatin1String("attached"));
            if (splitQmlPropertyArg(arg, type, module, qmlTypeName, property, doc.location())) {
                if (qmlType != m_qdb->findQmlType(module, qmlTypeName)) {
                    doc.startLocation().warning(
                            QStringLiteral(
                                    "All properties in a group must belong to the same type: '%1'")
                                    .arg(arg));
                    continue;
                }
                QmlPropertyNode *existingProperty = qmlType->hasQmlProperty(property, attached);
                if (existingProperty) {
                    processMetaCommands(doc, existingProperty);
                    if (!doc.body().isEmpty()) {
                        doc.startLocation().warning(
                                QStringLiteral("QML property documented multiple times: '%1'")
                                        .arg(arg), QStringLiteral("also seen here: %1")
                                                .arg(existingProperty->location().toString()));
                    }
                    continue;
                }
                auto *qpn = new QmlPropertyNode(qmlType, property, type, attached);
                qpn->setLocation(doc.startLocation());
                qpn->setGenus(jsProps ? Node::JS : Node::QML);
                nodes.append(qpn);
                docs.append(doc);
                sharedNodes << qpn;
            }
        } else {
            doc.startLocation().warning(
                    QStringLiteral("Command '\\%1'; not allowed with QML/JS property commands")
                            .arg(cmd));
        }
    }

    // Construct a SharedCommentNode (scn) if multiple topics generated
    // valid nodes. Note that it's important to do this *after* constructing
    // the topic nodes - which need to be written to index before the related
    // scn.
    if (sharedNodes.size() > 1) {
        auto *scn = new SharedCommentNode(qmlType, sharedNodes.size(), group);
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
                                       const ArgPair &argPair, Node *node)
{
    QString arg = argPair.first;
    if (command == COMMAND_INHEADERFILE) {
        if (node->isAggregate())
            static_cast<Aggregate *>(node)->addIncludeFile(arg);
        else
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_INHEADERFILE));
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
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_OVERLOAD));
    } else if (command == COMMAND_REIMP) {
        if (node->parent() && !node->parent()->isInternal()) {
            if (node->isFunction()) {
                auto *fn = static_cast<FunctionNode *>(node);
                // The clang visitor class will have set the
                // qualified name of the overridden function.
                // If the name of the overridden function isn't
                // set, issue a warning.
                if (fn->overridesThis().isEmpty() && isWorthWarningAbout(doc)) {
                    doc.location().warning(
                            QStringLiteral("Cannot find base function for '\\%1' in %2()")
                                    .arg(COMMAND_REIMP, node->name()),
                            QStringLiteral("The function either doesn't exist in any "
                                           "base class with the same signature or it "
                                           "exists but isn't virtual."));
                }
                fn->setReimpFlag();
            } else {
                doc.location().warning(
                        QStringLiteral("Ignored '\\%1' in %2").arg(COMMAND_REIMP, node->name()));
            }
        }
    } else if (command == COMMAND_RELATES) {
        QStringList path = arg.split("::");
        Aggregate *aggregate = m_qdb->findRelatesNode(path);
        if (aggregate == nullptr)
            aggregate = new ProxyNode(node->root(), arg);

        if (node->parent() == aggregate) { // node is already a child of aggregate
            doc.location().warning(QStringLiteral("Invalid '\\%1' (already a member of '%2')")
                                           .arg(COMMAND_RELATES, arg));
        } else {
            if (node->isAggregate()) {
                doc.location().warning(QStringLiteral("Invalid '\\%1' not allowed in '\\%2'")
                                               .arg(COMMAND_RELATES, node->nodeTypeString()));
            } else if (!node->isRelatedNonmember() &&
                       !node->parent()->isNamespace() && !node->parent()->isHeader()) {
                if (!doc.isInternal()) {
                    doc.location().warning(QStringLiteral("Invalid '\\%1' ('%2' must be global)")
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
                    doc.location().warning(
                            QStringLiteral("Invalid '\\%1' (multiple uses not allowed in '%2')")
                                    .arg(COMMAND_RELATES, node->nodeTypeString()));
                } else {
                    clone->setRelatedNonmember(true);
                }
            }
        }
    } else if (command == COMMAND_NEXTPAGE) {
        setLink(node, Node::NextLink, arg);
    } else if (command == COMMAND_PREVIOUSPAGE) {
        setLink(node, Node::PreviousLink, arg);
    } else if (command == COMMAND_STARTPAGE) {
        setLink(node, Node::StartLink, arg);
    } else if (command == COMMAND_QMLINHERITS) {
        if (node->name() == arg)
            doc.location().warning(QStringLiteral("%1 tries to inherit itself").arg(arg));
        else if (node->isQmlType() || node->isJsType()) {
            auto *qmlType = static_cast<QmlTypeNode *>(node);
            qmlType->setQmlBaseName(arg);
        }
    } else if (command == COMMAND_QMLINSTANTIATES) {
        if (node->isQmlType() || node->isJsType()) {
            ClassNode *classNode = m_qdb->findClassNode(arg.split("::"));
            if (classNode)
                node->setClassNode(classNode);
            else if (m_showLinkErrors) {
                doc.location().warning(
                        QStringLiteral("C++ class %2 not found: \\%1 %2")
                                .arg(command, arg));
            }
        } else {
            doc.location().warning(
                    QStringLiteral("\\%1 is only allowed in \\%2")
                            .arg(command, COMMAND_QMLTYPE));
        }
    } else if (command == COMMAND_DEFAULT) {
        if (!node->isQmlProperty()) {
            doc.location().warning(QStringLiteral("Ignored '\\%1', applies only to '\\%2'")
                    .arg(command, COMMAND_QMLPROPERTY));
        } else if (arg.isEmpty()) {
            doc.location().warning(QStringLiteral("Expected an argument for '\\%1' (maybe you meant '\\%2'?)")
                    .arg(command, COMMAND_QMLDEFAULT));
        } else {
            static_cast<QmlPropertyNode *>(node)->setDefaultValue(arg);
        }
    } else if (command == COMMAND_QMLDEFAULT) {
        node->markDefault();
    } else if (command == COMMAND_QMLREADONLY) {
        node->markReadOnly(true);
    }  else if (command == COMMAND_QMLREQUIRED) {
        if (!node->isQmlProperty())
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_QMLREQUIRED));
        else
            static_cast<QmlPropertyNode *>(node)->setRequired();
    } else if ((command == COMMAND_QMLABSTRACT) || (command == COMMAND_ABSTRACT)) {
        if (node->isQmlType() || node->isJsType())
            node->setAbstract(true);
    } else if (command == COMMAND_DEPRECATED) {
        node->setStatus(Node::Deprecated);
        if (!argPair.second.isEmpty())
            node->setDeprecatedSince(argPair.second);
    } else if (command == COMMAND_INGROUP || command == COMMAND_INPUBLICGROUP) {
        // Note: \ingroup and \inpublicgroup are the same (and now recognized as such).
        m_qdb->addToGroup(arg, node);
    } else if (command == COMMAND_INMODULE) {
        m_qdb->addToModule(arg, node);
    } else if (command == COMMAND_INQMLMODULE) {
        m_qdb->addToQmlModule(arg, node);
    } else if (command == COMMAND_INJSMODULE) {
        m_qdb->addToJsModule(arg, node);
    } else if (command == COMMAND_OBSOLETE) {
        node->setStatus(Node::Deprecated);
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
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_TITLE));
        else if (node->isExample())
            m_qdb->addExampleNode(static_cast<ExampleNode *>(node));
    } else if (command == COMMAND_SUBTITLE) {
        if (!node->setSubtitle(arg))
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_SUBTITLE));
    } else if (command == COMMAND_QTVARIABLE) {
        node->setQtVariable(arg);
        if (!node->isModule() && !node->isQmlModule())
            doc.location().warning(
                    QStringLiteral(
                            "Command '\\%1' is only meaningful in '\\module' and '\\qmlmodule'.")
                            .arg(COMMAND_QTVARIABLE));
    } else if (command == COMMAND_QTCMAKEPACKAGE) {
        node->setQtCMakeComponent(arg);
        if (!node->isModule())
            doc.location().warning(
                    QStringLiteral("Command '\\%1' is only meaningful in '\\module'.")
                            .arg(COMMAND_QTCMAKEPACKAGE));
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

    qsizetype leftParen = funcArg.indexOf(QChar('('));
    if (leftParen > 0)
        funcName = funcArg.left(leftParen);
    else
        funcName = funcArg;
    qsizetype firstBlank = funcName.indexOf(QChar(' '));
    if (firstBlank > 0) {
        returnType = funcName.left(firstBlank);
        funcName = funcName.right(funcName.size() - firstBlank - 1);
    }

    QStringList colonSplit(funcName.split("::"));
    if (colonSplit.size() < 2) {
        QString msg = "Unrecognizable QML module/component qualifier for " + funcArg;
        location.warning(msg.toLatin1().data());
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

    Aggregate *aggregate = m_qdb->findQmlType(moduleName, elementName);
    if (aggregate == nullptr)
        aggregate = m_qdb->findQmlBasicType(moduleName, elementName);
    if (aggregate == nullptr)
        return nullptr;

    QString params;
    QStringList leftParenSplit = funcArg.split('(');
    if (leftParenSplit.size() > 1) {
        QStringList rightParenSplit = leftParenSplit[1].split(')');
        if (!rightParenSplit.empty())
            params = rightParenSplit[0];
    }

    FunctionNode::Metaness metaness = FunctionNode::getMetanessFromTopic(topic);
    bool attached = topic.contains(QLatin1String("attached"));
    auto *fn = new FunctionNode(metaness, aggregate, funcName, attached);
    fn->setAccess(Access::Public);
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
    if (!blankSplit.empty()) {
        macroName = blankSplit.last();
        oldMacroNode = m_qdb->findMacroNode(macroName);
    }
    QString returnType;
    if (blankSplit.size() > 1) {
        blankSplit.removeLast();
        returnType = blankSplit.join(' ');
    }
    QString params;
    if (leftParenSplit.size() > 1) {
        const QString &afterParen = leftParenSplit.at(1);
        qsizetype rightParen = afterParen.indexOf(')');
        if (rightParen >= 0)
            params = afterParen.left(rightParen);
    }
    int i = 0;
    while (i < macroName.size() && !macroName.at(i).isLetter())
        i++;
    if (i > 0) {
        returnType += QChar(' ') + macroName.left(i);
        macroName = macroName.mid(i);
    }
    FunctionNode::Metaness metaness = FunctionNode::MacroWithParams;
    if (params.isEmpty())
        metaness = FunctionNode::MacroWithoutParams;
    auto *macro = new FunctionNode(metaness, m_qdb->primaryTreeRoot(), macroName);
    macro->setAccess(Access::Public);
    macro->setLocation(location);
    macro->setReturnType(returnType);
    macro->setParameters(params);
    if (macro->compare(oldMacroNode)) {
        location.warning(QStringLiteral("\\macro %1 documented more than once")
                .arg(macroArg), QStringLiteral("also seen here: %1")
                        .arg(oldMacroNode->doc().location().toString()));
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
                QStringLiteral("Cannot find project file for example '%1'").arg(en->name()),
                details);
        return;
    }

    QDir exampleDir(QFileInfo(fullPath).dir());

    QStringList exampleFiles = Config::getFilesHere(exampleDir.path(), m_exampleNameFilter,
                                                    Location(), m_excludeDirs, m_excludeFiles);
    // Search for all image files under the example project, excluding doc/images directory.
    QSet<QString> excludeDocDirs(m_excludeDirs);
    excludeDocDirs.insert(exampleDir.path() + QLatin1String("/doc/images"));
    QStringList imageFiles = Config::getFilesHere(exampleDir.path(), m_exampleImageFilter,
                                                  Location(), excludeDocDirs, m_excludeFiles);
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

    const qsizetype pathLen = exampleDir.path().size() - en->name().size();
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
                    node = parserForLanguage("Clang")->parseFnArg(doc.location(), args[0].first, args[0].second);
            } else if (topic == COMMAND_MACRO) {
                node = parseMacroArg(doc.location(), args[0].first);
            } else if (isQMLMethodTopic(topic) || isJSMethodTopic(topic)) {
                node = parseOtherFuncArg(topic, doc.location(), args[0].first);
            } else if (topic == COMMAND_DONTDOCUMENT) {
                m_qdb->primaryTree()->addToDontDocumentMap(args[0].first);
            } else {
                node = processTopicCommand(doc, topic, args[0]);
            }
            if (node != nullptr) {
                nodes.append(node);
                docs.append(doc);
            }
        } else if (args.size() > 1) {
            QList<SharedCommentNode *> sharedCommentNodes;
            for (const auto &arg : std::as_const(args)) {
                node = nullptr;
                if (topic == COMMAND_FN) {
                    if (showInternal() || !doc.isInternal())
                        node = parserForLanguage("Clang")->parseFnArg(doc.location(), arg.first, arg.second);
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
                        auto *scn = new SharedCommentNode(node);
                        sharedCommentNodes.append(scn);
                        nodes.append(scn);
                        docs.append(doc);
                    }
                    processMetaCommands(doc, node);
                }
            }
            for (auto *scn : sharedCommentNodes)
                scn->sort();
        }
    }
}

void CppCodeParser::processMetaCommands(NodeList &nodes, DocList &docs)
{
    QList<Doc>::Iterator d = docs.begin();
    for (const auto &node : nodes) {
        if (node != nullptr) {
            processMetaCommands(*d, node);
            node->setDoc(*d);
            checkModuleInclusion(node);
            if (node->isAggregate()) {
                auto *aggregate = static_cast<Aggregate *>(node);
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
    const QSet<QString> topicCommandsUsed = topicCommands() & doc.metaCommandsUsed();
    if (topicCommandsUsed.size() > 1) {
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
        qsizetype i = topicList.lastIndexOf(',');
        Q_ASSERT(i >= 0); // we had at least two commas
        topicList[i] = ' ';
        topicList.insert(i + 1, "and");
        doc.location().warning(
                QStringLiteral("Multiple topic commands found in comment:%1").arg(topicList));
        return true;
    }
    return false;
}

QT_END_NAMESPACE
