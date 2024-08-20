// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cppcodeparser.h"

#include "access.h"
#include "classnode.h"
#include "clangcodeparser.h"
#include "collectionnode.h"
#include "comparisoncategory.h"
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

#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

/*
  All these can appear in a C++ namespace. Don't add
  anything that can't be in a C++ namespace.
 */
static const QMap<QString, Node::NodeType> s_nodeTypeMap{
    { COMMAND_NAMESPACE, Node::Namespace }, { COMMAND_NAMESPACE, Node::Namespace },
    { COMMAND_CLASS, Node::Class },         { COMMAND_STRUCT, Node::Struct },
    { COMMAND_UNION, Node::Union },         { COMMAND_ENUM, Node::Enum },
    { COMMAND_TYPEALIAS, Node::TypeAlias }, { COMMAND_TYPEDEF, Node::Typedef },
    { COMMAND_PROPERTY, Node::Property },   { COMMAND_VARIABLE, Node::Variable }
};

typedef bool (Node::*NodeTypeTestFunc)() const;
static const QMap<QString, NodeTypeTestFunc> s_nodeTypeTestFuncMap{
    { COMMAND_NAMESPACE, &Node::isNamespace }, { COMMAND_CLASS, &Node::isClassNode },
    { COMMAND_STRUCT, &Node::isStruct },       { COMMAND_UNION, &Node::isUnion },
    { COMMAND_ENUM, &Node::isEnumType },       { COMMAND_TYPEALIAS, &Node::isTypeAlias },
    { COMMAND_TYPEDEF, &Node::isTypedef },     { COMMAND_PROPERTY, &Node::isProperty },
    { COMMAND_VARIABLE, &Node::isVariable },
};

CppCodeParser::CppCodeParser(FnCommandParser&& parser)
    : fn_parser{parser}
{
    Config &config = Config::instance();
    QStringList exampleFilePatterns{config.get(CONFIG_EXAMPLES
                                    + Config::dot
                                    + CONFIG_FILEEXTENSIONS).asStringList()};

    if (!exampleFilePatterns.isEmpty())
        m_exampleNameFilter = exampleFilePatterns.join(' ');
    else
        m_exampleNameFilter = "*.cpp *.h *.js *.xq *.svg *.xml *.ui";

    QStringList exampleImagePatterns{config.get(CONFIG_EXAMPLES
                                                + Config::dot
                                                + CONFIG_IMAGEEXTENSIONS).asStringList()};

    if (!exampleImagePatterns.isEmpty())
        m_exampleImageFilter = exampleImagePatterns.join(' ');
    else
        m_exampleImageFilter = "*.png";

    m_showLinkErrors = !config.get(CONFIG_NOLINKERRORS).asBool();
}

/*!
  Process the topic \a command found in the \a doc with argument \a arg.
 */
Node *CppCodeParser::processTopicCommand(const Doc &doc, const QString &command,
                                         const ArgPair &arg)
{
    QDocDatabase* database = QDocDatabase::qdocDB();

    if (command == COMMAND_FN) {
        Q_UNREACHABLE();
    } else if (s_nodeTypeMap.contains(command)) {
        /*
          We should only get in here if the command refers to
          something that can appear in a C++ namespace,
          i.e. a class, another namespace, an enum, a typedef,
          a property or a variable. I think these are handled
          this way to allow the writer to refer to the entity
          without including the namespace qualifier.
         */
        Node::NodeType type = s_nodeTypeMap[command];
        QStringList words = arg.first.split(QLatin1Char(' '));
        QStringList path;
        qsizetype idx = 0;
        Node *node = nullptr;

        if (type == Node::Variable && words.size() > 1)
            idx = words.size() - 1;
        path = words[idx].split("::");

        node = database->findNodeByNameAndType(path, s_nodeTypeTestFuncMap[command]);
        // Allow representing a type alias as a class
        if (node == nullptr && command == COMMAND_CLASS) {
            node = database->findNodeByNameAndType(path, &Node::isTypeAlias);
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
            if (CodeParser::isWorthWarningAbout(doc)) {
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
        }
        return node;
    } else if (command == COMMAND_EXAMPLE) {
        if (Config::generateExamples) {
            auto *en = new ExampleNode(database->primaryTreeRoot(), arg.first);
            en->setLocation(doc.startLocation());
            setExampleFileLists(en);
            return en;
        }
    } else if (command == COMMAND_EXTERNALPAGE) {
        auto *epn = new ExternalPageNode(database->primaryTreeRoot(), arg.first);
        epn->setLocation(doc.startLocation());
        return epn;
    } else if (command == COMMAND_HEADERFILE) {
        auto *hn = new HeaderNode(database->primaryTreeRoot(), arg.first);
        hn->setLocation(doc.startLocation());
        return hn;
    } else if (command == COMMAND_GROUP) {
        CollectionNode *cn = database->addGroup(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_MODULE) {
        CollectionNode *cn = database->addModule(arg.first);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_QMLMODULE) {
        QStringList blankSplit = arg.first.split(QLatin1Char(' '));
        CollectionNode *cn = database->addQmlModule(blankSplit[0]);
        cn->setLogicalModuleInfo(blankSplit);
        cn->setLocation(doc.startLocation());
        cn->markSeen();
        return cn;
    } else if (command == COMMAND_PAGE) {
        auto *pn = new PageNode(database->primaryTreeRoot(), arg.first.split(' ').front());
        pn->setLocation(doc.startLocation());
        return pn;
    } else if (command == COMMAND_QMLTYPE ||
               command == COMMAND_QMLVALUETYPE ||
               command == COMMAND_QMLBASICTYPE) {
        auto nodeType = (command == COMMAND_QMLTYPE) ? Node::QmlType : Node::QmlValueType;
        QString qmid;
        if (auto args = doc.metaCommandArgs(COMMAND_INQMLMODULE); !args.isEmpty())
            qmid = args.first().first;
        auto *qcn = database->findQmlTypeInPrimaryTree(qmid, arg.first);
        // A \qmlproperty may have already constructed a placeholder type
        // without providing a module identifier; allow such cases
        if (!qcn && !qmid.isEmpty())
            qcn = database->findQmlTypeInPrimaryTree(QString(), arg.first);
        if (!qcn || qcn->nodeType() != nodeType)
            qcn = new QmlTypeNode(database->primaryTreeRoot(), arg.first, nodeType);
        if (!qmid.isEmpty())
            database->addToQmlModule(qmid, qcn);
        qcn->setLocation(doc.startLocation());
        return qcn;
    } else if ((command == COMMAND_QMLSIGNAL) || (command == COMMAND_QMLMETHOD)
               || (command == COMMAND_QMLATTACHEDSIGNAL) || (command == COMMAND_QMLATTACHEDMETHOD)) {
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

std::vector<TiedDocumentation> CppCodeParser::processQmlProperties(const UntiedDocumentation &untied)
{
    const Doc &doc = untied.documentation;
    const TopicList &topics = doc.topicsUsed();
    if (topics.isEmpty())
        return {};

    QString arg;
    QString type;
    QString group;
    QString qmlModule;
    QString property;
    QString qmlTypeName;

    std::vector<TiedDocumentation> tied{};

    Topic topic = topics.at(0);
    arg = topic.m_args;
    if (splitQmlPropertyArg(arg, type, qmlModule, qmlTypeName, property, doc.location())) {
        qsizetype i = property.indexOf('.');
        if (i != -1)
            group = property.left(i);
    }

    QDocDatabase *database = QDocDatabase::qdocDB();

    NodeList sharedNodes;
    QmlTypeNode *qmlType = database->findQmlTypeInPrimaryTree(qmlModule, qmlTypeName);
    // Note: Constructing a QmlType node by default, as opposed to QmlValueType.
    // This may lead to unexpected behavior if documenting \qmlvaluetype's properties
    // before the type itself.
    if (qmlType == nullptr) {
        qmlType = new QmlTypeNode(database->primaryTreeRoot(), qmlTypeName, Node::QmlType);
        qmlType->setLocation(doc.startLocation());
        if (!qmlModule.isEmpty())
            database->addToQmlModule(qmlModule, qmlType);
    }

    for (const auto &topicCommand : topics) {
        QString cmd = topicCommand.m_topic;
        arg = topicCommand.m_args;
        if ((cmd == COMMAND_QMLPROPERTY) || (cmd == COMMAND_QMLATTACHEDPROPERTY)) {
            bool attached = cmd.contains(QLatin1String("attached"));
            if (splitQmlPropertyArg(arg, type, qmlModule, qmlTypeName, property, doc.location())) {
                if (qmlType != database->findQmlTypeInPrimaryTree(qmlModule, qmlTypeName)) {
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
                qpn->setGenus(Node::QML);

                tied.emplace_back(TiedDocumentation{doc, qpn});

                sharedNodes << qpn;
            }
        } else {
            doc.startLocation().warning(
                    QStringLiteral("Command '\\%1'; not allowed with QML property commands")
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

        tied.emplace_back(TiedDocumentation{doc, scn});

        for (const auto n : sharedNodes)
            scn->append(n);
        scn->sort();
    }

    return tied;
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
    QDocDatabase* database = QDocDatabase::qdocDB();

    QString arg = argPair.first;
    if (command == COMMAND_INHEADERFILE) {
        // TODO: [incorrect-constructs][header-arg]
        // The emptiness check for arg is required as,
        // currently, DocParser fancies passing (without any warning)
        // incorrect constructs doen the chain, such as an
        // "\inheaderfile" command with no argument.
        //
        // As it is the case here, we require further sanity checks to
        // preserve some of the semantic for the later phases.
        // This generally has a ripple effect on the whole codebase,
        // making it more complex and increasesing the surface of bugs.
        //
        // The following emptiness check should be removed as soon as
        // DocParser is enhanced with correct semantics.
        if (node->isAggregate() && !arg.isEmpty())
            static_cast<Aggregate *>(node)->setIncludeFile(arg);
        else
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_INHEADERFILE));
    } else if (command == COMMAND_COMPARES) {
        processComparesCommand(node, arg, doc.location());
    } else if (command == COMMAND_COMPARESWITH) {
        if (!node->isClassNode())
            doc.location().warning(
                    u"Found \\%1 command outside of \\%2 context."_s
                            .arg(COMMAND_COMPARESWITH, COMMAND_CLASS));
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
                if (fn->overridesThis().isEmpty() && CodeParser::isWorthWarningAbout(doc)) {
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
        // REMARK: Generates warnings only; Node instances are
        // adopted from the root namespace to other Aggregates
        // in a post-processing step, Aggregate::resolveRelates(),
        // after all topic commands are processed.
        if (node->isAggregate()) {
            doc.location().warning("Invalid '\\%1' not allowed in '\\%2'"_L1
                    .arg(COMMAND_RELATES, node->nodeTypeString()));
        } else if (!node->isRelatedNonmember() && node->parent()->isClassNode()) {
            if (!doc.isInternal()) {
                doc.location().warning("Invalid '\\%1' ('%2' must be global)"_L1
                        .arg(COMMAND_RELATES, node->name()));
            }
        }
    } else if (command == COMMAND_NEXTPAGE) {
        CodeParser::setLink(node, Node::NextLink, arg);
    } else if (command == COMMAND_PREVIOUSPAGE) {
        CodeParser::setLink(node, Node::PreviousLink, arg);
    } else if (command == COMMAND_STARTPAGE) {
        CodeParser::setLink(node, Node::StartLink, arg);
    } else if (command == COMMAND_QMLINHERITS) {
        if (node->name() == arg)
            doc.location().warning(QStringLiteral("%1 tries to inherit itself").arg(arg));
        else if (node->isQmlType()) {
            auto *qmlType = static_cast<QmlTypeNode *>(node);
            qmlType->setQmlBaseName(arg);
        }
    } else if (command == COMMAND_QMLNATIVETYPE || command == COMMAND_QMLINSTANTIATES) {
        if (command == COMMAND_QMLINSTANTIATES)
            doc.location().report(
                    u"\\nativetype is deprecated and will be removed in a future version. Use \\nativetype instead."_s);
        // TODO: COMMAND_QMLINSTANTIATES is deprecated since 6.8. Its remains should be removed no later than Qt 7.0.0.
        processQmlNativeTypeCommand(node, command, arg, doc.location());
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
    } else if (command == COMMAND_QMLENUMERATORSFROM) {
        if (!node->isQmlProperty()) {
            doc.location().warning("Ignored '\\%1', applies only to '\\%2'"_L1
                    .arg(command, COMMAND_QMLPROPERTY));
        } else if (!static_cast<QmlPropertyNode*>(node)->setEnumNode(argPair.first, argPair.second)) {
            doc.location().warning("Failed to find C++ enumeration '%2' passed to \\%1"_L1
                    .arg(command, arg), "Use \\value commands instead"_L1);
        }
    } else if (command == COMMAND_QMLREADONLY) {
        node->markReadOnly(true);
    }  else if (command == COMMAND_QMLREQUIRED) {
        if (!node->isQmlProperty())
            doc.location().warning(QStringLiteral("Ignored '\\%1'").arg(COMMAND_QMLREQUIRED));
        else
            static_cast<QmlPropertyNode *>(node)->setRequired();
    } else if ((command == COMMAND_QMLABSTRACT) || (command == COMMAND_ABSTRACT)) {
        if (node->isQmlType())
            node->setAbstract(true);
    } else if (command == COMMAND_DEPRECATED) {
        node->setDeprecated(argPair.second);
    } else if (command == COMMAND_INGROUP || command == COMMAND_INPUBLICGROUP) {
        // Note: \ingroup and \inpublicgroup are the same (and now recognized as such).
        database->addToGroup(arg, node);
    } else if (command == COMMAND_INMODULE) {
        database->addToModule(arg, node);
    } else if (command == COMMAND_INQMLMODULE) {
        // Handled when parsing topic commands
    } else if (command == COMMAND_OBSOLETE) {
        node->setStatus(Node::Deprecated);
    } else if (command == COMMAND_NONREENTRANT) {
        node->setThreadSafeness(Node::NonReentrant);
    } else if (command == COMMAND_PRELIMINARY) {
        // \internal wins.
        if (!node->isInternal())
            node->setStatus(Node::Preliminary);
    } else if (command == COMMAND_INTERNAL) {
        if (!Config::instance().showInternal())
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
            database->addExampleNode(static_cast<ExampleNode *>(node));
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
        if (node->isModule())
            node->setQtCMakeComponent(arg);
        else
            doc.location().warning(
                    QStringLiteral("Command '\\%1' is only meaningful in '\\module'.")
                            .arg(COMMAND_QTCMAKEPACKAGE));
    } else if (command == COMMAND_QTCMAKETARGETITEM) {
        if (node->isModule())
            node->setQtCMakeTargetItem(arg);
        else
            doc.location().warning(
                    QStringLiteral("Command '\\%1' is only meaningful in '\\module'.")
                            .arg(COMMAND_QTCMAKETARGETITEM));
    } else if (command == COMMAND_MODULESTATE ) {
        if (!node->isModule() && !node->isQmlModule()) {
            doc.location().warning(
                    QStringLiteral(
                            "Command '\\%1' is only meaningful in '\\module' and '\\qmlmodule'.")
                            .arg(COMMAND_MODULESTATE));
        } else {
            static_cast<CollectionNode*>(node)->setState(arg);
        }
    } else if (command == COMMAND_NOAUTOLIST) {
        if (!node->isCollectionNode() && !node->isExample()) {
            doc.location().warning(
                    QStringLiteral(
                            "Command '\\%1' is only meaningful in '\\module', '\\qmlmodule', `\\group` and `\\example`.")
                            .arg(COMMAND_NOAUTOLIST));
        } else {
            static_cast<PageNode*>(node)->setNoAutoList(true);
        }
    } else if (command == COMMAND_ATTRIBUTION) {
        // TODO: This condition is not currently exact enough, as it
        // will allow any non-aggregate `PageNode` to use the command,
        // For example, an `ExampleNode`.
        //
        // The command is intended only for internal usage by
        // "qattributionscanner" and should only work on `PageNode`s
        // that are generated from a "\page" command.
        //
        // It is already possible to provide a more restricted check,
        // albeit in a somewhat dirty way. It is not expected that
        // this warning will have any particular use.
        // If it so happens that a case where the too-broad scope of
        // the warning is a problem or hides a bug, modify the
        // condition to be restrictive enough.
        // Otherwise, wait until a more torough look at QDoc's
        // internal representations an way to enable "Attribution
        // Pages" is performed before looking at the issue again.
        if (!node->isTextPageNode()) {
            doc.location().warning(u"Command '\\%1' is only meaningful in '\\%2'"_s.arg(COMMAND_ATTRIBUTION, COMMAND_PAGE));
        } else { static_cast<PageNode*>(node)->markAttribution(); }
    }
}

/*!
    \internal
    Processes the argument \a arg that's passed to the \\compares command,
    and sets the comparison category of the \a node accordingly.

    If the argument is invalid, issue a warning at the location the command
    appears through \a loc.
*/
void CppCodeParser::processComparesCommand(Node *node, const QString &arg, const Location &loc)
{
    if (!node->isClassNode()) {
        loc.warning(u"Found \\%1 command outside of \\%2 context."_s.arg(COMMAND_COMPARES,
                                                                         COMMAND_CLASS));
        return;
    }

    if (auto category = comparisonCategoryFromString(arg.toStdString());
            category != ComparisonCategory::None) {
        node->setComparisonCategory(category);
    } else {
        loc.warning(u"Invalid argument to \\%1 command: `%2`"_s.arg(COMMAND_COMPARES, arg),
                    u"Valid arguments are `strong`, `weak`, `partial`, or `equality`."_s);
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
    std::vector<Node*> nodes_to_process{};
    if (node->isSharedCommentNode()) {
        auto scn = static_cast<SharedCommentNode*>(node);

        nodes_to_process.reserve(scn->count() + 1);
        std::copy(scn->collective().cbegin(), scn->collective().cend(), std::back_inserter(nodes_to_process));
    }

    // REMARK: Ordering is important here. If node is a
    // SharedCommentNode it MUST be processed after all its child
    // nodes.
    // Failure to do so can incur in incorrect warnings.
    // For example, if a shared documentation has a "\relates" command.
    // When the command is processed for the SharedCommentNode it will
    // apply to all its child nodes.
    // If a child node is processed after the SharedCommentNode that
    // contains it, that "\relates" command will be considered applied
    // already, resulting in a warning.
    nodes_to_process.push_back(node);

    const QStringList metaCommandsUsed = doc.metaCommandsUsed().values();
    for (const auto &command : metaCommandsUsed) {
        const ArgList args = doc.metaCommandArgs(command);
        for (const auto &arg : args) {
            std::for_each(nodes_to_process.cbegin(), nodes_to_process.cend(), [this, doc, command, arg](auto node){
                processMetaCommand(doc, command, arg, node);
            });
        }
    }
}

/*!
 Parse QML signal/method topic commands.
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

    QDocDatabase* database = QDocDatabase::qdocDB();

    auto *aggregate = database->findQmlTypeInPrimaryTree(moduleName, elementName);
    // Note: Constructing a QmlType node by default, as opposed to QmlValueType.
    // This may lead to unexpected behavior if documenting \qmlvaluetype's methods
    // before the type itself.
    if (!aggregate) {
        aggregate = new QmlTypeNode(database->primaryTreeRoot(), elementName, Node::QmlType);
        aggregate->setLocation(location);
        if (!moduleName.isEmpty())
            database->addToQmlModule(moduleName, aggregate);
    }

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
    QDocDatabase* database = QDocDatabase::qdocDB();

    QStringList leftParenSplit = macroArg.split('(');
    if (leftParenSplit.isEmpty())
        return nullptr;
    QString macroName;
    FunctionNode *oldMacroNode = nullptr;
    QStringList blankSplit = leftParenSplit[0].split(' ');
    if (!blankSplit.empty()) {
        macroName = blankSplit.last();
        oldMacroNode = database->findMacroNode(macroName);
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
    auto *macro = new FunctionNode(metaness, database->primaryTreeRoot(), macroName);
    macro->setAccess(Access::Public);
    macro->setLocation(location);
    macro->setReturnType(returnType);
    macro->setParameters(params);
    if (oldMacroNode && macro->parent() == oldMacroNode->parent()
                && compare(macro, oldMacroNode) == 0) {
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

    const auto& [excludeDirs, excludeFiles] = config.getExcludedPaths();

    QStringList exampleFiles = Config::getFilesHere(exampleDir.path(), m_exampleNameFilter,
                                                    Location(), excludeDirs, excludeFiles);
    // Search for all image files under the example project, excluding doc/images directory.
    QSet<QString> excludeDocDirs(excludeDirs);
    excludeDocDirs.insert(exampleDir.path() + QLatin1String("/doc/images"));
    QStringList imageFiles = Config::getFilesHere(exampleDir.path(), m_exampleImageFilter,
                                                  Location(), excludeDocDirs, excludeFiles);
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
                QLatin1String("*.qrc *.pro *.qmlproject *.pyproject CMakeLists.txt qmldir"),
                Location(), excludeDirs, excludeFiles);
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
  returns true if \a t is \e {qmlsignal}, \e {qmlmethod},
  \e {qmlattachedsignal}, or \e {qmlattachedmethod}.
 */
bool CppCodeParser::isQMLMethodTopic(const QString &t)
{
    return (t == COMMAND_QMLSIGNAL || t == COMMAND_QMLMETHOD || t == COMMAND_QMLATTACHEDSIGNAL
            || t == COMMAND_QMLATTACHEDMETHOD);
}

/*!
  Returns true if \a t is \e {qmlproperty}, \e {qmlpropertygroup},
  or \e {qmlattachedproperty}.
 */
bool CppCodeParser::isQMLPropertyTopic(const QString &t)
{
    return (t == COMMAND_QMLPROPERTY || t == COMMAND_QMLATTACHEDPROPERTY);
}

std::pair<std::vector<TiedDocumentation>, std::vector<FnMatchError>>
CppCodeParser::processTopicArgs(const UntiedDocumentation &untied)
{
    const Doc &doc = untied.documentation;

    if (doc.topicsUsed().isEmpty())
        return {};

    QDocDatabase *database = QDocDatabase::qdocDB();

    const QString topic = doc.topicsUsed().first().m_topic;

    std::vector<TiedDocumentation> tied{};
    std::vector<FnMatchError> errors{};

    if (isQMLPropertyTopic(topic)) {
        auto tied_qml = processQmlProperties(untied);
        tied.insert(tied.end(), tied_qml.begin(), tied_qml.end());
    } else {
        ArgList args = doc.metaCommandArgs(topic);
        Node *node = nullptr;
        if (args.size() == 1) {
            if (topic == COMMAND_FN) {
                if (Config::instance().showInternal() || !doc.isInternal()) {
                    auto result = fn_parser(doc.location(), args[0].first, args[0].second, untied.context);
                    if (auto *error = std::get_if<FnMatchError>(&result))
                        errors.emplace_back(*error);
                    else
                        node = std::get<Node*>(result);
                }
            } else if (topic == COMMAND_MACRO) {
                node = parseMacroArg(doc.location(), args[0].first);
            } else if (isQMLMethodTopic(topic)) {
                node = parseOtherFuncArg(topic, doc.location(), args[0].first);
            } else if (topic == COMMAND_DONTDOCUMENT) {
                database->primaryTree()->addToDontDocumentMap(args[0].first);
            } else {
                node = processTopicCommand(doc, topic, args[0]);
            }
            if (node != nullptr) {
                tied.emplace_back(TiedDocumentation{doc, node});
            }
        } else if (args.size() > 1) {
            QList<SharedCommentNode *> sharedCommentNodes;
            for (const auto &arg : std::as_const(args)) {
                node = nullptr;
                if (topic == COMMAND_FN) {
                    if (Config::instance().showInternal() || !doc.isInternal()) {
                        auto result = fn_parser(doc.location(), arg.first, arg.second, untied.context);
                        if (auto *error = std::get_if<FnMatchError>(&result))
                            errors.emplace_back(*error);
                        else
                            node = std::get<Node*>(result);
                    }
                } else if (topic == COMMAND_MACRO) {
                    node = parseMacroArg(doc.location(), arg.first);
                } else if (isQMLMethodTopic(topic)) {
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
                        tied.emplace_back(TiedDocumentation{doc, scn});
                    }
                }
            }
            for (auto *scn : sharedCommentNodes)
                scn->sort();
        }
    }
    return std::make_pair(tied, errors);
}

/*!
  For each node that is part of C++ API and produces a documentation
  page, this function ensures that the node belongs to a module.
 */
static void checkModuleInclusion(Node *n)
{
    if (n->physicalModuleName().isEmpty()) {
        if (n->isInAPI() && !n->name().isEmpty()) {
            switch (n->nodeType()) {
            case Node::Class:
            case Node::Struct:
            case Node::Union:
            case Node::Namespace:
            case Node::HeaderFile:
                break;
            default:
                return;
            }
            n->setPhysicalModuleName(Generator::defaultModuleName());
            QDocDatabase::qdocDB()->addToModule(Generator::defaultModuleName(), n);
            n->doc().location().warning(
                    QStringLiteral("Documentation for %1 '%2' has no \\inmodule command; "
                                   "using project name by default: %3")
                            .arg(Node::nodeTypeString(n->nodeType()), n->name(),
                                    n->physicalModuleName()));
        }
    }
}

void CppCodeParser::processMetaCommands(const std::vector<TiedDocumentation> &tied)
{
    for (auto [doc, node] : tied) {
        processMetaCommands(doc, node);
        node->setDoc(doc);
        checkModuleInclusion(node);
        if (node->isAggregate()) {
            auto *aggregate = static_cast<Aggregate *>(node);

            if (!aggregate->includeFile()) {
                Aggregate *parent = aggregate;
                while (parent->physicalModuleName().isEmpty() && (parent->parent() != nullptr))
                    parent = parent->parent();

                if (parent == aggregate)
                    // TODO: Understand if the name can be empty.
                    // In theory it should not be possible as
                    // there would be no aggregate to refer to
                    // such that this code is never reached.
                    //
                    // If the name can be empty, this would
                    // endanger users of the include file down the
                    // line, forcing them to ensure that, further
                    // to there being an actual include file, that
                    // include file is not an empty string, such
                    // that we would require a different way to
                    // generate the include file here.
                    aggregate->setIncludeFile(aggregate->name());
                else if (aggregate->includeFile())
                    aggregate->setIncludeFile(*parent->includeFile());
            }
        }
    }
}

void CppCodeParser::processQmlNativeTypeCommand(Node *node, const QString &cmd, const QString &arg, const Location &location)
{
    Q_ASSERT(node);
    if (!node->isQmlNode()) {
        location.warning(
                QStringLiteral("Command '\\%1' is only meaningful in '\\%2'")
                        .arg(cmd, COMMAND_QMLTYPE));
        return;
    }

    auto qmlNode = static_cast<QmlTypeNode *>(node);

    QDocDatabase *database = QDocDatabase::qdocDB();
    auto classNode = database->findClassNode(arg.split(u"::"_s));

    if (!classNode) {
        if (m_showLinkErrors) {
            location.warning(
                    QStringLiteral("C++ class %2 not found: \\%1 %2")
                            .arg(cmd, arg));
        }
        return;
    }

    if (qmlNode->classNode()) {
        location.warning(
                QStringLiteral("QML type %1 documented with %2 as its native type. Replacing %2 with %3")
                        .arg(qmlNode->name(), qmlNode->classNode()->name(), arg));
    }

    qmlNode->setClassNode(classNode);
    classNode->insertQmlNativeType(qmlNode);
}

QT_END_NAMESPACE
