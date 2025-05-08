// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlvisitor.h"

#include "aggregate.h"
#include "codechunk.h"
#include "codeparser.h"
#include "functionnode.h"
#include "genustypes.h"
#include "nativeenum.h"
#include "node.h"
#include "qdocdatabase.h"
#include "qmlpropertyarguments.h"
#include "qmlpropertynode.h"
#include "sharedcommentnode.h"
#include "tokenizer.h"
#include "utilities.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qglobal.h>

#include <private/qqmljsast_p.h>
#include <private/qqmljsengine_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
  The constructor stores all the parameters in local data members.
 */
QmlDocVisitor::QmlDocVisitor(const QString &filePath, const QString &code, QQmlJS::Engine *engine,
                             const QSet<QString> &commands, const QSet<QString> &topics)
    : m_nestingLevel(0)
{
    m_lastEndOffset = 0;
    this->m_filePath = filePath;
    this->m_name = QFileInfo(filePath).baseName();
    m_document = code;
    this->m_engine = engine;
    this->m_commands = commands;
    this->m_topics = topics;
    m_current = QDocDatabase::qdocDB()->primaryTreeRoot();
}

/*!
  Returns the location of the nearest comment above the \a offset.
 */
QQmlJS::SourceLocation QmlDocVisitor::precedingComment(quint32 offset) const
{
    const auto comments = m_engine->comments();
    for (auto it = comments.rbegin(); it != comments.rend(); ++it) {
        QQmlJS::SourceLocation loc = *it;

        if (loc.begin() <= m_lastEndOffset) {
            // Return if we reach the end of the preceding structure.
            break;
        } else if (m_usedComments.contains(loc.begin())) {
            // Return if we encounter a previously used comment.
            break;
        } else if (loc.begin() > m_lastEndOffset && loc.end() < offset) {
            // Only examine multiline comments in order to avoid snippet markers.
            if (m_document.at(loc.offset - 1) == QLatin1Char('*')) {
                QString comment = m_document.mid(loc.offset, loc.length);
                if (comment.startsWith(QLatin1Char('!')) || comment.startsWith(QLatin1Char('*'))) {
                    return loc;
                }
            }
        }
    }

    return QQmlJS::SourceLocation();
}

class QmlSignatureParser
{
public:
    QmlSignatureParser(FunctionNode *func, const QString &signature, const Location &loc);
    void readToken() { tok_ = tokenizer_->getToken(); }
    QString lexeme() { return tokenizer_->lexeme(); }
    QString previousLexeme() { return tokenizer_->previousLexeme(); }

    bool match(int target);
    bool matchTypeAndName(CodeChunk *type, QString *var);
    bool matchParameter();
    bool matchFunctionDecl();

private:
    QString signature_;
    QStringList names_;
    Tokenizer *tokenizer_;
    int tok_;
    FunctionNode *func_;
    const Location &location_;
};

/*!
  Finds the nearest unused qdoc comment above the QML entity
  represented by a \a node and processes the qdoc commands
  in that comment. The processed documentation is stored in
  \a node.

  If \a node is a \c nullptr and there is a valid comment block,
  the QML module identifier (\inqmlmodule argument) is used
  for searching an existing QML type node. If an existing node
  is not found, constructs a new QmlTypeNode instance.

  Returns a pointer to the QmlTypeNode instance if one was
  found or constructed. Otherwise, returns a pointer to the \a
  node that was passed as an argument.
 */
Node *QmlDocVisitor::applyDocumentation(QQmlJS::SourceLocation location, Node *node)
{
    QQmlJS::SourceLocation loc = precedingComment(location.begin());
    Location comment_loc(m_filePath);

    // No preceding comment; construct a new QML type if
    // needed.
    if (!loc.isValid()) {
        if (!node)
            node = new QmlTypeNode(m_current, m_name, NodeType::QmlType);
        comment_loc.setLineNo(location.startLine);
        node->setLocation(comment_loc);
        return node;
    }

    QString source = m_document.mid(loc.offset + 1, loc.length - 1);
    comment_loc.setLineNo(loc.startLine);
    comment_loc.setColumnNo(loc.startColumn);

    Doc doc(comment_loc, comment_loc, source, m_commands, m_topics);
    const TopicList &topicsUsed = doc.topicsUsed();
    NodeList nodes;
    if (!node) {
        QString qmid;
        if (auto args = doc.metaCommandArgs(COMMAND_INQMLMODULE); !args.isEmpty())
            qmid = args.first().first;
        node = QDocDatabase::qdocDB()->findQmlTypeInPrimaryTree(qmid, m_name);
        if (!node) {
            node = new QmlTypeNode(m_current, m_name, NodeType::QmlType);
            node->setLocation(comment_loc);
        }
    }

    auto *parent{node->parent()};
    nodes << node;
    if (!topicsUsed.empty()) {
        for (int i = 0; i < topicsUsed.size(); ++i) {
            QString topic = topicsUsed.at(i).m_topic;
            QString args = topicsUsed.at(i).m_args;
            if (topic.endsWith(QLatin1String("property"))) {
                auto *qmlProperty = static_cast<QmlPropertyNode *>(node);
                if (auto qpa = QmlPropertyArguments::parse(args, doc.location())) {
                    if (qpa->m_name == node->name()) {
                        // Allow overriding data type from the arguments
                        qmlProperty->setDataType(qpa->m_type);
                    } else {
                        bool isAttached = topic.contains(QLatin1String("attached"));
                        QmlPropertyNode *n = parent->hasQmlProperty(qpa->m_name, isAttached);
                        if (n == nullptr)
                            n = new QmlPropertyNode(parent, qpa->m_name, qpa->m_type, isAttached);
                        n->setIsList(qpa->m_isList);
                        // Use the const-overload of QmlPropertyNode::isReadOnly() as there's
                        // no associated C++ property to resolve the read-only status from
                        n->markReadOnly(const_cast<const QmlPropertyNode *>(qmlProperty)->isReadOnly()
                                        && !isAttached);
                        if (qmlProperty->isDefault())
                            n->markDefault();
                        nodes << n;
                    }
                } else
                    qCDebug(lcQdoc) << "Failed to parse QML property:" << topic << args;
            } else if (topic == COMMAND_QMLSIGNAL || topic == COMMAND_QMLMETHOD ||
                       topic == COMMAND_QMLATTACHEDSIGNAL || topic == COMMAND_QMLATTACHEDMETHOD) {
                if (node->isFunction()) {
                    QmlSignatureParser qsp(static_cast<FunctionNode *>(node), args, doc.location());
                    if (nodes.size() > 1) {
                        doc.location().warning("\\%1 cannot be mixed with other topic commands"_L1.arg(topic));
                        nodes = {node};
                    }
                    break;
                }
            } else if (topic == COMMAND_QMLTYPE || topic == COMMAND_QMLVALUETYPE ||
                       topic == COMMAND_QMLBASICTYPE) {
                if (node->isQmlType()) {
                    if (nodes.size() > 1) {
                        doc.location().warning("\\%1 cannot be mixed with other topic commands"_L1.arg(topic));
                        nodes = {node};
                    }
                    break;
                }
            }
        }
    }

    for (auto *n : nodes) {
        applyMetacommands(loc, n, doc);
    }

    // Test if we're documenting a property group; We need three nodes at minimum:
    // The property that acts as the `root` in the group, and two more to make a
    // group. Each property must be prefixed with the root name + '.'.
    bool isPropertyGroup{false};
    if (nodes.size() > 2 && parent->isQmlType()) {
        isPropertyGroup =
            std::all_of(std::next(nodes.cbegin()), nodes.cend(), [node](const Node *p) {
                return p->name().startsWith("%1."_L1.arg(node->name()));
            });
    }
    // Construct a SharedCommentNode (SCN) representing a QML property group.
    //
    // Note that it's important to do this *after* constructing
    // the topic nodes - which need to be written to index before the related
    // SCN.
    if (isPropertyGroup) {
        // The first node in `nodes` is the group property;
        // create SCN with the same name
        QString group{nodes.takeFirst()->name()};
        auto *scn = new SharedCommentNode(static_cast<QmlTypeNode*>(parent),
                                          nodes.size(), group);
        scn->setDoc(doc);
        scn->setLocation(doc.startLocation());
        applyMetacommands(loc, scn, doc);
        for (const auto n : std::as_const(nodes))
            scn->append(n);
        scn->sort();
    } else {
        for (auto *n : nodes) {
            n->setDoc(doc);
            n->setLocation(doc.location());
        }
    }

    m_usedComments.insert(loc.offset);
    return node;
}

QmlSignatureParser::QmlSignatureParser(FunctionNode *func, const QString &signature,
                                       const Location &loc)
    : signature_(signature), func_(func), location_(loc)
{
    QByteArray latin1 = signature.toLatin1();
    Tokenizer stringTokenizer(location_, std::move(latin1));
    stringTokenizer.setParsingFnOrMacro(true);
    tokenizer_ = &stringTokenizer;
    readToken();
    matchFunctionDecl();
}

/*!
  If the current token matches \a target, read the next
  token and return true. Otherwise, don't read the next
  token, and return false.
 */
bool QmlSignatureParser::match(int target)
{
    if (tok_ == target) {
        readToken();
        return true;
    }
    return false;
}

/*!
  Parse a QML data type into \a type and an optional
  variable name into \a var.
 */
bool QmlSignatureParser::matchTypeAndName(CodeChunk *type, QString *var)
{
    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
     */
    for (;;) {
        bool virgin = true;

        // If not an identifier, try to match a sequence of qualifiers.
        if (tok_ != Tok_Ident) {
            while (match(Tok_signed) || match(Tok_unsigned) || match(Tok_short) || match(Tok_long)
                   || match(Tok_int64)) {
                // Append the matched qualifier token.
                type->append(previousLexeme());
                virgin = false;
            }
        }

        if (virgin) {
            if (match(Tok_Ident)) {
                type->append(previousLexeme());
            } else if (match(Tok_void) || match(Tok_int) || match(Tok_char) || match(Tok_double)
                       || match(Tok_Ellipsis))
                type->append(previousLexeme());
            else
                return false;
        } else if (match(Tok_int) || match(Tok_char) || match(Tok_double)) {
            type->append(previousLexeme());
        }

        // Match and append a namespace separator or break.
        if (match(Tok_Gulbrandsen))
            type->append(previousLexeme());
        else
            break;
    }

    // Matches a sequence of & * const ^ tokens.
    while (match(Tok_Ampersand) || match(Tok_Aster) || match(Tok_const) || match(Tok_Caret))
        type->append(previousLexeme());

    /*
      The usual case: Look for an optional identifier, then for
      some array brackets.
     */
    type->appendHotspot();

    // Set the variable name if it is unset and an identifier is matched.
    if ((var != nullptr) && match(Tok_Ident))
        *var = previousLexeme();

    // Skip pairs of braces and their contents.
    if (tok_ == Tok_LeftBracket) {
        int bracketDepth0 = tokenizer_->bracketDepth();
        while ((tokenizer_->bracketDepth() >= bracketDepth0 && tok_ != Tok_Eoi)
               || tok_ == Tok_RightBracket) {
            type->append(lexeme());
            readToken();
        }
    }
    return true;
}

bool QmlSignatureParser::matchParameter()
{
    QString name;
    CodeChunk type;
    CodeChunk defaultValue;

    bool result = matchTypeAndName(&type, &name);
    if (name.isEmpty()) {
        name = type.toString();
        type.clear();
    }

    if (!result)
        return false;
    if (match(Tok_Equal)) {
        int parenDepth0 = tokenizer_->parenDepth();
        while (tokenizer_->parenDepth() >= parenDepth0
               && (tok_ != Tok_Comma || tokenizer_->parenDepth() > parenDepth0)
               && tok_ != Tok_Eoi) {
            defaultValue.append(lexeme());
            readToken();
        }
    }
    func_->parameters().append(type.toString(), name, defaultValue.toString());
    return true;
}

bool QmlSignatureParser::matchFunctionDecl()
{
    CodeChunk returnType;

    qsizetype firstBlank = signature_.indexOf(QChar(' '));
    qsizetype leftParen = signature_.indexOf(QChar('('));
    if ((firstBlank > 0) && (leftParen - firstBlank) > 1) {
        if (!matchTypeAndName(&returnType, nullptr))
            return false;
    }

    while (match(Tok_Ident)) {
        names_.append(previousLexeme());
        if (!match(Tok_Gulbrandsen)) {
            previousLexeme();
            names_.pop_back();
            break;
        }
    }

    if (tok_ != Tok_LeftParen)
        return false;
    /*
      Parsing the parameters should be moved into class Parameters,
      but it can wait. mws 14/12/2018
     */
    readToken();

    func_->setLocation(location_);
    func_->setReturnType(returnType.toString());

    if (tok_ != Tok_RightParen) {
        func_->parameters().clear();
        do {
            if (!matchParameter())
                return false;
        } while (match(Tok_Comma));
    }
    if (!match(Tok_RightParen))
        return false;
    return true;
}

/*!
  Applies the metacommands found in the comment.
 */
void QmlDocVisitor::applyMetacommands(QQmlJS::SourceLocation, Node *node, Doc &doc)
{
    QDocDatabase *qdb = QDocDatabase::qdocDB();
    QSet<QString> metacommands = doc.metaCommandsUsed();
    if (metacommands.size() > 0) {
        metacommands.subtract(m_topics);
        for (const auto &command : std::as_const(metacommands)) {
            const ArgList args = doc.metaCommandArgs(command);
            if ((command == COMMAND_QMLABSTRACT) || (command == COMMAND_ABSTRACT)) {
                if (node->isQmlType()) {
                    node->setAbstract(true);
                }
            } else if (command == COMMAND_DEPRECATED) {
                node->setDeprecated(args[0].second);
            } else if (command == COMMAND_INQMLMODULE) {
                qdb->addToQmlModule(args[0].first, node);
            } else if (command == COMMAND_QMLINHERITS) {
                if (node->name() == args[0].first)
                    doc.location().warning(
                            QStringLiteral("%1 tries to inherit itself").arg(args[0].first));
                else if (node->isQmlType()) {
                    auto *qmlType = static_cast<QmlTypeNode *>(node);
                    qmlType->setQmlBaseName(args[0].first);
                }
            } else if (command == COMMAND_DEFAULT) {
                if (!node->isQmlProperty()) {
                    doc.location().warning(QStringLiteral("Ignored '\\%1', applies only to '\\%2'")
                            .arg(command, COMMAND_QMLPROPERTY));
                } else if (args.isEmpty() || args[0].first.isEmpty()) {
                    doc.location().warning(QStringLiteral("Expected an argument for '\\%1' (maybe you meant '\\%2'?)")
                            .arg(command, COMMAND_QMLDEFAULT));
                } else {
                    static_cast<QmlPropertyNode *>(node)->setDefaultValue(args[0].first);
                }
            } else if (command == COMMAND_QMLDEFAULT) {
                node->markDefault();
            } else if (command == COMMAND_QMLENUMERATORSFROM) {
                if (!node->isQmlProperty()) {
                    doc.location().warning("Ignored '\\%1', applies only to '\\%2'"_L1
                            .arg(command, COMMAND_QMLPROPERTY));
                } else if (!static_cast<QmlPropertyNode*>(node)->nativeEnum()->resolve(args[0].first, args[0].second)) {
                    doc.location().warning("Failed to find C++ enumeration '%2' passed to \\%1"_L1
                            .arg(command, args[0].first), "Use \\value commands instead"_L1);
                }
            } else if (command == COMMAND_QMLREADONLY) {
                node->markReadOnly(1);
            } else if (command == COMMAND_QMLREQUIRED) {
                if (node->isQmlProperty())
                    static_cast<QmlPropertyNode *>(node)->setRequired();
            } else if ((command == COMMAND_INGROUP) && !args.isEmpty()) {
                for (const auto &argument : args)
                    QDocDatabase::qdocDB()->addToGroup(argument.first, node);
            } else if (command == COMMAND_INTERNAL) {
                node->setStatus(Node::Internal);
            } else if (command == COMMAND_OBSOLETE) {
                node->setStatus(Node::Deprecated);
            } else if (command == COMMAND_PRELIMINARY) {
                node->setStatus(Node::Preliminary);
            } else if (command == COMMAND_SINCE) {
                QString arg = args[0].first; //.join(' ');
                node->setSince(arg);
            } else if (command == COMMAND_WRAPPER) {
                node->setWrapper();
            } else {
                doc.location().warning(
                        QStringLiteral("The \\%1 command is ignored in QML files").arg(command));
            }
        }
    }
}

/*!
  Reconstruct the qualified \a id using dot notation
  and return the fully qualified string.
 */
QString QmlDocVisitor::getFullyQualifiedId(QQmlJS::AST::UiQualifiedId *id)
{
    QString result;
    if (id) {
        result = id->name.toString();
        id = id->next;
        while (id != nullptr) {
            result += QChar('.') + id->name.toString();
            id = id->next;
        }
    }
    return result;
}

/*!
  Begin the visit of the object \a definition, recording it in the
  qdoc database. Increment the object nesting level, which is used
  to test whether we are at the public API level. The public level
  is level 1.

  Defers the construction of a QmlTypeNode instance to
  applyDocumentation(), by passing \c nullptr as the second
  argument.
 */
bool QmlDocVisitor::visit(QQmlJS::AST::UiObjectDefinition *definition)
{
    QString type = getFullyQualifiedId(definition->qualifiedTypeNameId);
    m_nestingLevel++;
    if (m_current->isNamespace()) {
        auto component = applyDocumentation(definition->firstSourceLocation(), nullptr);
        Q_ASSERT(component);
        auto *qmlTypeNode = static_cast<QmlTypeNode *>(component);
        if (!component->doc().isEmpty())
            qmlTypeNode->setQmlBaseName(type);
        qmlTypeNode->setTitle(m_name);
        qmlTypeNode->setImportList(m_importList);
        m_importList.clear();
        m_current = qmlTypeNode;
    }

    return true;
}

/*!
  End the visit of the object \a definition. In particular,
  decrement the object nesting level, which is used to test
  whether we are at the public API level. The public API
  level is level 1. It won't decrement below 0.
 */
void QmlDocVisitor::endVisit(QQmlJS::AST::UiObjectDefinition *definition)
{
    if (m_nestingLevel > 0) {
        --m_nestingLevel;
    }
    m_lastEndOffset = definition->lastSourceLocation().end();
}

bool QmlDocVisitor::visit(QQmlJS::AST::UiImport *import)
{
    QString name = m_document.mid(import->fileNameToken.offset, import->fileNameToken.length);
    if (name[0] == '\"')
        name = name.mid(1, name.size() - 2);
    QString version;
    if (import->version) {
        const auto start = import->version->firstSourceLocation().begin();
        const auto end = import->version->lastSourceLocation().end();
        version = m_document.mid(start, end - start);
    }
    QString importUri = getFullyQualifiedId(import->importUri);
    m_importList.append(ImportRec(std::move(name), std::move(version), std::move(importUri), import->importId));

    return true;
}

void QmlDocVisitor::endVisit(QQmlJS::AST::UiImport *definition)
{
    m_lastEndOffset = definition->lastSourceLocation().end();
}

bool QmlDocVisitor::visit(QQmlJS::AST::UiObjectBinding *)
{
    ++m_nestingLevel;
    return true;
}

void QmlDocVisitor::endVisit(QQmlJS::AST::UiObjectBinding *)
{
    --m_nestingLevel;
}

bool QmlDocVisitor::visit(QQmlJS::AST::UiArrayBinding *)
{
    return true;
}

void QmlDocVisitor::endVisit(QQmlJS::AST::UiArrayBinding *) {}

static QString qualifiedIdToString(QQmlJS::AST::UiQualifiedId *node)
{
    QString s;

    for (QQmlJS::AST::UiQualifiedId *it = node; it; it = it->next) {
        s.append(it->name);

        if (it->next)
            s.append(QLatin1Char('.'));
    }

    return s;
}

/*!
    Visits the public \a member declaration, which can be a
    signal or a property. It is a custom signal or property.
    Only visit the \a member if the nestingLevel is 1.
 */
bool QmlDocVisitor::visit(QQmlJS::AST::UiPublicMember *member)
{
    if (m_nestingLevel > 1) {
        return true;
    }
    switch (member->type) {
    case QQmlJS::AST::UiPublicMember::Signal: {
        if (m_current->isQmlType()) {
            auto *qmlType = static_cast<QmlTypeNode *>(m_current);
            if (qmlType) {
                FunctionNode::Metaness metaness = FunctionNode::QmlSignal;
                QString name = member->name.toString();
                auto *newSignal = new FunctionNode(metaness, m_current, name);
                Parameters &parameters = newSignal->parameters();
                for (QQmlJS::AST::UiParameterList *it = member->parameters; it; it = it->next) {
                    const QString type = it->type ? it->type->toString() : QString();
                    if (!type.isEmpty() && !it->name.isEmpty())
                        parameters.append(type, it->name.toString());
                }
                applyDocumentation(member->firstSourceLocation(), newSignal);
            }
        }
        break;
    }
    case QQmlJS::AST::UiPublicMember::Property: {
        QString type = qualifiedIdToString(member->memberType);
        if (m_current->isQmlType()) {
            auto *qmlType = static_cast<QmlTypeNode *>(m_current);
            if (qmlType) {
                QString name = member->name.toString();
                QmlPropertyNode *qmlPropNode = qmlType->hasQmlProperty(name);
                if (qmlPropNode == nullptr)
                    qmlPropNode = new QmlPropertyNode(qmlType, std::move(name), std::move(type), false);
                qmlPropNode->markReadOnly(member->isReadonly());
                if (member->isDefaultMember())
                    qmlPropNode->markDefault();
                if (member->requiredToken().isValid())
                    qmlPropNode->setRequired();
                qmlPropNode->setIsList(member->typeModifier == "list"_L1);
                applyDocumentation(member->firstSourceLocation(), qmlPropNode);
            }
        }
        break;
    }
    default:
        return false;
    }

    return true;
}

/*!
  End the visit of the \a member.
 */
void QmlDocVisitor::endVisit(QQmlJS::AST::UiPublicMember *member)
{
    m_lastEndOffset = member->lastSourceLocation().end();
}

bool QmlDocVisitor::visit(QQmlJS::AST::IdentifierPropertyName *)
{
    return true;
}

/*!
  Begin the visit of the function declaration \a fd, but only
  if the nesting level is 1.
 */
bool QmlDocVisitor::visit(QQmlJS::AST::FunctionDeclaration *fd)
{
    if (m_nestingLevel <= 1) {
        FunctionNode::Metaness metaness = FunctionNode::QmlMethod;
        if (!m_current->isQmlType())
            return true;
        QString name = fd->name.toString();
        auto *method = new FunctionNode(metaness, m_current, name);
        Parameters &parameters = method->parameters();
        QQmlJS::AST::FormalParameterList *formals = fd->formals;
        if (formals) {
            QQmlJS::AST::FormalParameterList *fp = formals;
            do {
                QString defaultValue;
                auto initializer = fp->element->initializer;
                if (initializer) {
                    auto loc = initializer->firstSourceLocation();
                    defaultValue = m_document.mid(loc.begin(), loc.length);
                }
                parameters.append(QString(), fp->element->bindingIdentifier.toString(),
                        defaultValue);
                fp = fp->next;
            } while (fp && fp != formals);
        }
        applyDocumentation(fd->firstSourceLocation(), method);
    }
    return true;
}

/*!
  End the visit of the function declaration, \a fd.
 */
void QmlDocVisitor::endVisit(QQmlJS::AST::FunctionDeclaration *fd)
{
    m_lastEndOffset = fd->lastSourceLocation().end();
}

/*!
  Begin the visit of the signal handler declaration \a sb, but only
  if the nesting level is 1.

  This visit is now deprecated. It has been decided to document
  public signals. If a signal handler must be discussed in the
  documentation, that discussion must take place in the comment
  for the signal.
 */
bool QmlDocVisitor::visit(QQmlJS::AST::UiScriptBinding *)
{
    return true;
}

void QmlDocVisitor::endVisit(QQmlJS::AST::UiScriptBinding *sb)
{
    m_lastEndOffset = sb->lastSourceLocation().end();
}

bool QmlDocVisitor::visit(QQmlJS::AST::UiQualifiedId *)
{
    return true;
}

void QmlDocVisitor::endVisit(QQmlJS::AST::UiQualifiedId *)
{
    // nothing.
}

void QmlDocVisitor::throwRecursionDepthError()
{
    hasRecursionDepthError = true;
}

bool QmlDocVisitor::hasError() const
{
    return hasRecursionDepthError;
}

QT_END_NAMESPACE
