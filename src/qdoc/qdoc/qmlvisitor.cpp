// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmlvisitor.h"

#include "aggregate.h"
#include "codechunk.h"
#include "codeparser.h"
#include "functionnode.h"
#include "node.h"
#include "qdocdatabase.h"
#include "qmlpropertynode.h"
#include "tokenizer.h"
#include "utilities.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qglobal.h>

#include <private/qqmljsast_p.h>
#include <private/qqmljsengine_p.h>

QT_BEGIN_NAMESPACE

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
  represented by the \a node and processes the qdoc commands
  in that comment. The processed documentation is stored in
  the \a node.

  If a qdoc comment is found for \a location, true is returned.
  If a comment is not found there, false is returned.
 */
bool QmlDocVisitor::applyDocumentation(QQmlJS::SourceLocation location, Node *node)
{
    QQmlJS::SourceLocation loc = precedingComment(location.begin());

    if (loc.isValid()) {
        QString source = m_document.mid(loc.offset, loc.length);
        Location start(m_filePath);
        start.setLineNo(loc.startLine);
        start.setColumnNo(loc.startColumn);
        Location finish(m_filePath);
        finish.setLineNo(loc.startLine);
        finish.setColumnNo(loc.startColumn);

        Doc doc(start, finish, source.mid(1), m_commands, m_topics);
        const TopicList &topicsUsed = doc.topicsUsed();
        NodeList nodes;
        auto *parent{node->parent()};
        node->setDoc(doc);
        nodes.append(node);
        if (!topicsUsed.empty()) {
            for (int i = 0; i < topicsUsed.size(); ++i) {
                QString topic = topicsUsed.at(i).m_topic;
                if (!topic.startsWith(QLatin1String("qml")))
                    continue; // maybe a qdoc warning here? mws 18/07/18
                QString args = topicsUsed.at(i).m_args;
                if (topic.endsWith(QLatin1String("property"))) {
                    auto *qmlProperty = static_cast<QmlPropertyNode *>(node);
                    QmlPropArgs qpa;
                    if (splitQmlPropertyArg(doc, args, qpa)) {
                        if (qpa.m_name == node->name()) {
                            if (qmlProperty->isAlias())
                                qmlProperty->setDataType(qpa.m_type);
                        } else {
                            bool isAttached = topic.contains(QLatin1String("attached"));
                            QmlPropertyNode *n = parent->hasQmlProperty(qpa.m_name, isAttached);
                            if (n == nullptr)
                                n = new QmlPropertyNode(parent, qpa.m_name, qpa.m_type, isAttached);
                            n->setLocation(doc.location());
                            n->setDoc(doc);
                            // Use the const-overload of QmlPropertyNode::isReadOnly() as there's
                            // no associated C++ property to resolve the read-only status from
                            n->markReadOnly(const_cast<const QmlPropertyNode *>(qmlProperty)->isReadOnly()
                                            && !isAttached);
                            if (qmlProperty->isDefault())
                                n->markDefault();
                            nodes.append(n);
                        }
                    } else
                        qCDebug(lcQdoc) << "Failed to parse QML property:" << topic << args;
                } else if (topic.endsWith(QLatin1String("method")) || topic == COMMAND_QMLSIGNAL) {
                    if (node->isFunction()) {
                        auto *fn = static_cast<FunctionNode *>(node);
                        QmlSignatureParser qsp(fn, args, doc.location());
                    }
                }
            }
        }
        for (const auto &node : nodes)
            applyMetacommands(loc, node, doc);
        m_usedComments.insert(loc.offset);
        if (doc.isEmpty()) {
            return false;
        }
        return true;
    }
    Location codeLoc(m_filePath);
    codeLoc.setLineNo(location.startLine);
    node->setLocation(codeLoc);
    return false;
}

QmlSignatureParser::QmlSignatureParser(FunctionNode *func, const QString &signature,
                                       const Location &loc)
    : signature_(signature), func_(func), location_(loc)
{
    QByteArray latin1 = signature.toLatin1();
    Tokenizer stringTokenizer(location_, latin1);
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

        if (tok_ != Tok_Ident) {
            while (match(Tok_signed) || match(Tok_unsigned) || match(Tok_short) || match(Tok_long)
                   || match(Tok_int64)) {
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

        if (match(Tok_Gulbrandsen))
            type->append(previousLexeme());
        else
            break;
    }

    while (match(Tok_Ampersand) || match(Tok_Aster) || match(Tok_const) || match(Tok_Caret))
        type->append(previousLexeme());

    /*
      The usual case: Look for an optional identifier, then for
      some array brackets.
     */
    type->appendHotspot();

    if ((var != nullptr) && match(Tok_Ident))
        *var = previousLexeme();

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
  A QML property argument has the form...

  <type> <component>::<name>
  <type> <QML-module>::<component>::<name>

  This function splits the argument into one of those
  two forms. The three part form is the old form, which
  was used before the creation of QtQuick 2 and Qt
  Components. A <QML-module> is the QML equivalent of a
  C++ namespace. So this function splits \a arg on "::"
  and stores the parts in the \e {type}, \e {module},
  \e {component}, and \a {name}, fields of \a qpa. If it
  is successful, it returns \c true. If not enough parts
  are found, a qdoc warning is emitted and false is
  returned.
 */
bool QmlDocVisitor::splitQmlPropertyArg(const Doc &doc, const QString &arg, QmlPropArgs &qpa)
{
    qpa.clear();
    QStringList blankSplit = arg.split(QLatin1Char(' '));
    if (blankSplit.size() > 1) {
        qpa.m_type = blankSplit[0];
        QStringList colonSplit(blankSplit[1].split("::"));
        if (colonSplit.size() == 3) {
            qpa.m_module = colonSplit[0];
            qpa.m_component = colonSplit[1];
            qpa.m_name = colonSplit[2];
            return true;
        } else if (colonSplit.size() == 2) {
            qpa.m_component = colonSplit[0];
            qpa.m_name = colonSplit[1];
            return true;
        } else if (colonSplit.size() == 1) {
            qpa.m_name = colonSplit[0];
            return true;
        }
        doc.location().warning(
                QStringLiteral("Unrecognizable QML module/component qualifier for %1.").arg(arg));
    } else {
        doc.location().warning(QStringLiteral("Missing property type for %1.").arg(arg));
    }
    return false;
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
                node->setStatus(Node::Deprecated);
                if (!args[0].second.isEmpty())
                    node->setDeprecatedSince(args[0].second);
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

  Note that this visit() function creates the qdoc object node as a
  QmlType.
 */
bool QmlDocVisitor::visit(QQmlJS::AST::UiObjectDefinition *definition)
{
    QString type = getFullyQualifiedId(definition->qualifiedTypeNameId);
    m_nestingLevel++;

    if (m_current->isNamespace()) {
        QmlTypeNode *component = nullptr;
        Node *candidate = m_current->findChildNode(m_name, Node::QML);
        if (candidate != nullptr)
            component = static_cast<QmlTypeNode *>(candidate);
        else
            component = new QmlTypeNode(m_current, m_name, Node::QmlType);
        component->setTitle(m_name);
        component->setImportList(m_importList);
        m_importList.clear();
        if (applyDocumentation(definition->firstSourceLocation(), component))
            component->setQmlBaseName(type);
        m_current = component;
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
    m_importList.append(ImportRec(name, version, importUri));

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
                    qmlPropNode = new QmlPropertyNode(qmlType, name, type, false);
                qmlPropNode->markReadOnly(member->isReadonly());
                if (member->isDefaultMember())
                    qmlPropNode->markDefault();
                if (member->requiredToken().isValid())
                    qmlPropNode->setRequired();
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
