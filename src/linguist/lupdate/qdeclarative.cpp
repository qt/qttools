// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lupdate.h"

#include <translator.h>
#include <metastrings.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QTextStream>

#include <private/qqmljsengine_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsastvisitor_p.h>
#include <private/qqmljsast_p.h>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QtDebug>
#include <QStringList>

#include <iostream>
#include <cstdlib>
#include <cctype>

QT_BEGIN_NAMESPACE

using namespace QQmlJS;

using namespace Qt::StringLiterals;

class FindTrCalls: protected AST::Visitor
{
public:
    FindTrCalls(Engine *engine, ConversionData &cd)
        : engine(engine)
        , m_cd(cd)
    {
    }

    void operator()(Translator *translator, const QString &fileName, AST::Node *node)
    {
        m_todo = engine->comments();
        m_translator = translator;
        m_fileName = fileName;
        m_component = QFileInfo(fileName).completeBaseName();
        accept(node);

        // process the trailing comments
        processComments(0, /*flush*/ true);
    }

protected:
    using AST::Visitor::visit;
    using AST::Visitor::endVisit;

    void accept(AST::Node *node)
    { AST::Node::accept(node, this); }

    bool visit(AST::UiPragma *node) override
    {
        if (!node->name.isNull()) {
            if (node->name == "Translator"_L1) {
                m_component = node->values->value.toString();
            }
        }
        return false;
    }

    void endVisit(AST::TemplateLiteral *node) override
    {
        do {
            if (auto *expr = AST::cast<AST::CallExpression *>(node->expression))
                endVisit(expr);
            node = node->next;
        } while (node);
    }

    void endVisit(AST::CallExpression *node) override
    {
        QString name;
        AST::ExpressionNode *base = node->base;

        while (base && base->kind == AST::Node::Kind_FieldMemberExpression) {
            auto memberExpr = static_cast<AST::FieldMemberExpression *>(base);
            name.prepend(memberExpr->name);
            name.prepend(u'.');
            base = memberExpr->base;
        }

        if (AST::IdentifierExpression *idExpr = AST::cast<AST::IdentifierExpression *>(base)) {
            processComments(idExpr->identifierToken.begin());

            name = idExpr->name.toString() + name;
            const int identLineNo = idExpr->identifierToken.startLine;
            switch (trFunctionAliasManager.trFunctionByName(name)) {
            case TrFunctionAliasManager::Function_qsTr:
            case TrFunctionAliasManager::Function_QT_TR_NOOP: {
                if (!node->arguments) {
                    yyMsg(identLineNo)
                        << qPrintable(QStringLiteral("%1() requires at least one argument.\n")
                                      .arg(name));
                    return;
                }
                if (auto expr = AST::cast<AST::TemplateLiteral *>(node->arguments->expression)) {
                    if (expr->next) {
                        yyMsg(identLineNo)
                        << qPrintable(QStringLiteral("%1() template strings with "
                                    "arguments are not supported for translation.\n").arg(name));
                        return;
                    }
                }

                QString source;
                if (!createString(node->arguments->expression, &source))
                    return;

                QString comment;
                bool plural = false;
                if (AST::ArgumentList *commentNode = node->arguments->next) {
                    if (!createString(commentNode->expression, &comment)) {
                        comment.clear(); // clear possible invalid comments
                    }
                    if (commentNode->next)
                        plural = true;
                }

                if (!m_metaStrings.sourcetext().isEmpty())
                    yyMsg(identLineNo)
                            << qPrintable("//% cannot be used with %1(). Ignoring\n"_L1.arg(name));

                if (!m_metaStrings.label().isEmpty() && m_metaStrings.msgid().isEmpty())
                    yyMsg(identLineNo) << qPrintable(
                            "labels cannot be used with text-based translation. Ignoring\n"_L1);

                TranslatorMessage msg(m_component, ParserTool::transcode(source),
                    comment, QString(), m_fileName,
                    node->firstSourceLocation().startLine, QStringList(),
                    TranslatorMessage::Unfinished, plural);
                msg.setExtraComment(
                        ParserTool::transcode(m_metaStrings.extracomment().simplified()));
                msg.setId(m_metaStrings.msgid());
                if (!m_metaStrings.msgid().isEmpty())
                    msg.setLabel(m_metaStrings.label());
                msg.setExtras(m_metaStrings.extra());
                m_translator->extend(msg, m_cd);
                consumeComment();
                break; }
            case TrFunctionAliasManager::Function_qsTranslate:
            case TrFunctionAliasManager::Function_QT_TRANSLATE_NOOP: {
                if (! (node->arguments && node->arguments->next)) {
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("%1() requires at least two arguments.\n").arg(name));
                    return;
                }

                QString context;
                if (!createString(node->arguments->expression, &context))
                    return;

                AST::ArgumentList *sourceNode = node->arguments->next; // we know that it is a valid pointer.

                QString source;
                if (!createString(sourceNode->expression, &source))
                    return;

                if (!m_metaStrings.sourcetext().isEmpty())
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("//% cannot be used with %1(). Ignoring\n").arg(name));

                if (!m_metaStrings.label().isEmpty() && m_metaStrings.msgid().isEmpty())
                    yyMsg(identLineNo) << qPrintable(
                            "labels cannot be used with text-based translation. Ignoring\n"_L1);

                QString comment;
                bool plural = false;
                if (AST::ArgumentList *commentNode = sourceNode->next) {
                    if (!createString(commentNode->expression, &comment)) {
                        comment.clear(); // clear possible invalid comments
                    }

                    if (commentNode->next)
                        plural = true;
                }

                TranslatorMessage msg(context, ParserTool::transcode(source),
                    comment, QString(), m_fileName,
                    node->firstSourceLocation().startLine, QStringList(),
                    TranslatorMessage::Unfinished, plural);
                msg.setExtraComment(
                        ParserTool::transcode(m_metaStrings.extracomment().simplified()));
                msg.setId(m_metaStrings.msgid());
                if (!m_metaStrings.msgid().isEmpty())
                    msg.setLabel(m_metaStrings.label());
                msg.setExtras(m_metaStrings.extra());
                m_translator->extend(msg, m_cd);
                consumeComment();
                break; }
            case TrFunctionAliasManager::Function_qsTrId:
            case TrFunctionAliasManager::Function_QT_TRID_NOOP: {
                if (!node->arguments) {
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("%1() requires at least one argument.\n").arg(name));
                    return;
                }

                QString id;
                if (!createString(node->arguments->expression, &id))
                    return;

                if (!m_metaStrings.msgid().isEmpty()) {
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("//= cannot be used with %1(). Ignoring\n").arg(name));
                    return;
                }

                bool plural = node->arguments->next;

                TranslatorMessage msg(QString(), ParserTool::transcode(m_metaStrings.sourcetext()),
                                      QString(), QString(), m_fileName,
                                      node->firstSourceLocation().startLine, QStringList(),
                                      TranslatorMessage::Unfinished, plural);
                msg.setExtraComment(
                        ParserTool::transcode(m_metaStrings.extracomment().simplified()));
                msg.setId(id);
                msg.setLabel(m_metaStrings.label());
                msg.setExtras(m_metaStrings.extra());
                m_translator->extend(msg, m_cd);
                consumeComment();
                break; }
            }
        }
    }

    void postVisit(AST::Node *node) override;

private:
    std::ostream &yyMsg(int line)
    {
        return std::cerr << qPrintable(m_fileName) << ':' << line << ": ";
    }

    void throwRecursionDepthError() final
    {
        std::cerr << qPrintable(m_fileName) << ": "
                  << "Maximum statement or expression depth exceeded";
    }


    void processComments(quint32 offset, bool flush = false);
    void processComment(const SourceLocation &loc);
    void consumeComment();

    bool createString(AST::ExpressionNode *ast, QString *out)
    {
        if (AST::StringLiteral *literal = AST::cast<AST::StringLiteral *>(ast)) {
            out->append(literal->value);
            return true;
        } else if (AST::BinaryExpression *binop = AST::cast<AST::BinaryExpression *>(ast)) {
            if (binop->op == QSOperator::Add && createString(binop->left, out)) {
                if (createString(binop->right, out))
                    return true;
            }
        } else if (AST::TemplateLiteral *templit = AST::cast<AST::TemplateLiteral *>(ast)) {
            out->append(templit->value);
            return true;
        }

        return false;
    }

    Engine *engine;
    Translator *m_translator;
    ConversionData &m_cd;
    QString m_fileName;
    QString m_component;

    // comments
    MetaStrings m_metaStrings;
    QString trcontext;
    QList<SourceLocation> m_todo;
};

QString createErrorString(const QString &filename, const QString &code, Parser &parser)
{
    // print out error
    QStringList lines = code.split(u'\n');
    lines.append("\n"_L1); // sentinel.
    QString errorString;

    const auto messages = parser.diagnosticMessages();
    for (const DiagnosticMessage &m : messages) {

        if (m.isWarning())
            continue;

        const int line = m.loc.startLine;
        const int column = m.loc.startColumn;
        QString error = filename + u':' + QString::number(line) + u':' + QString::number(column)
                + ": error: "_L1 + m.message + u'\n';

        const QString textLine = lines.at(line > 0 ? line - 1 : 0);
        error += textLine + u'\n';
        for (int i = 0, end = qMin(column > 0 ? column - 1 : 0, textLine.size()); i < end; ++i) {
            const QChar ch = textLine.at(i);
            if (ch.isSpace())
                error += ch;
            else
                error += u' ';
        }
        error += "^\n"_L1;
        errorString += error;
    }
    return errorString;
}

void FindTrCalls::postVisit(AST::Node *node)
{
    if (node->statementCast() != 0 || node->uiObjectMemberCast()) {
        processComments(node->lastSourceLocation().end());

        if (m_metaStrings.hasData()) {
            yyMsg(node->lastSourceLocation().startLine) << "Discarding unconsumed meta data\n";
            consumeComment();
        }
    }
}

void FindTrCalls::processComments(quint32 offset, bool flush)
{
    for (; !m_todo.isEmpty(); m_todo.removeFirst()) {
        SourceLocation loc = m_todo.first();
        if (! flush && (loc.begin() >= offset))
            break;

        processComment(loc);
    }
}

void FindTrCalls::consumeComment()
{
    // keep the current `trcontext'
    m_metaStrings.clear();
}

void FindTrCalls::processComment(const SourceLocation &loc)
{
    if (!loc.length)
        return;

    auto commentStr = QString(engine->midRef(loc.begin(), loc.length));

    if (!m_metaStrings.parse(commentStr)) {
        yyMsg(loc.startLine) << m_metaStrings.popError().toStdString();
        return;
    }

    if (m_metaStrings.magicComment()) {
        auto [context, comment] = *m_metaStrings.magicComment();
        TranslatorMessage msg(ParserTool::transcode(context), QString(),
                              ParserTool::transcode(comment), QString(), m_fileName, loc.startLine,
                              QStringList(), TranslatorMessage::Finished, false);
        msg.setExtraComment(ParserTool::transcode(m_metaStrings.extracomment().simplified()));
        m_translator->append(msg);
        m_translator->setExtras(m_metaStrings.extra());
        m_metaStrings.clear();
    }
}

class HasDirectives: public Directives
{
public:
    HasDirectives(Lexer *lexer)
        : lexer(lexer)
        , directives(0)
    {
    }

    bool operator()() const { return directives != 0; }
    int end() const { return lastOffset; }

    void pragmaLibrary() override { consumeDirective(); }
    void importFile(const QString &, const QString &, int, int) override { consumeDirective(); }
    void importModule(const QString &, const QString &, const QString &, int, int) override { consumeDirective(); }

private:
    void consumeDirective()
    {
        ++directives;
        lastOffset = lexer->tokenOffset() + lexer->tokenLength();
    }

private:
    Lexer *lexer;
    int directives;
    int lastOffset;
};

enum CodeType {
    QMLCode,
    JSCode,
    MJSCode,
};

static bool load(Translator &translator, const QString &filename, ConversionData &cd, CodeType mode)
{
    cd.m_sourceFileName = filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        cd.appendError(QStringLiteral("Cannot open %1: %2").arg(filename, file.errorString()));
        return false;
    }

    QString code;
    if (mode != QMLCode) {
        code = QTextStream(&file).readAll();
    } else {
        QTextStream ts(&file);
        code = ts.readAll();
    }

    Engine driver;
    Parser parser(&driver);

    Lexer lexer(&driver);
    lexer.setCode(code, /*line = */ 1, mode == QMLCode);
    driver.setLexer(&lexer);

    bool rc;
    if (mode == QMLCode)
        rc = parser.parse();
    else if (mode == JSCode)
        rc = parser.parseProgram();
    else
        rc = parser.parseModule();

    if (rc) {
        FindTrCalls trCalls(&driver, cd);

        //find all tr calls in the code
        trCalls(&translator, filename, parser.rootNode());
    } else {
        QString error = createErrorString(filename, code, parser);
        cd.appendError(error);
        return false;
    }
    return true;
}

bool loadQml(Translator &translator, const QString &filename, ConversionData &cd)
{
    return load(translator, filename, cd, /*qmlMode=*/ QMLCode);
}

bool loadQScript(Translator &translator, const QString &filename, ConversionData &cd)
{
    return load(translator, filename, cd, /*qmlMode=*/ JSCode);
}

bool loadJSModule(Translator &translator, const QString &filename, ConversionData &cd)
{
    return load(translator, filename, cd, /*qmlMode=*/ MJSCode);
}

QT_END_NAMESPACE
