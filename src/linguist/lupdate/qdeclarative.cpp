// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lupdate.h"

#include <translator.h>

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

static QString QmlMagicComment = u"TRANSLATOR"_s;

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

    void endVisit(AST::CallExpression *node) override
    {
        QString name;
        AST::ExpressionNode *base = node->base;

        while (base && base->kind == AST::Node::Kind_FieldMemberExpression) {
            auto memberExpr = static_cast<AST::FieldMemberExpression *>(base);
            name.prepend(memberExpr->name);
            name.prepend(QLatin1Char('.'));
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
                if (AST::cast<AST::TemplateLiteral *>(node->arguments->expression)) {
                    yyMsg(identLineNo)
                        << qPrintable(QStringLiteral("%1() cannot be used with template literals. "
                                                     "Ignoring\n").arg(name));
                    return;
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

                if (!sourcetext.isEmpty())
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("//% cannot be used with %1(). Ignoring\n").arg(name));

                TranslatorMessage msg(m_component, ParserTool::transcode(source),
                    comment, QString(), m_fileName,
                    node->firstSourceLocation().startLine, QStringList(),
                    TranslatorMessage::Unfinished, plural);
                msg.setExtraComment(ParserTool::transcode(extracomment.simplified()));
                msg.setId(msgid);
                msg.setExtras(extra);
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

                if (!sourcetext.isEmpty())
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("//% cannot be used with %1(). Ignoring\n").arg(name));

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
                msg.setExtraComment(ParserTool::transcode(extracomment.simplified()));
                msg.setId(msgid);
                msg.setExtras(extra);
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

                if (!msgid.isEmpty()) {
                    yyMsg(identLineNo) << qPrintable(QStringLiteral("//= cannot be used with %1(). Ignoring\n").arg(name));
                    return;
                }

                bool plural = node->arguments->next;

                TranslatorMessage msg(QString(), ParserTool::transcode(sourcetext),
                    QString(), QString(), m_fileName,
                    node->firstSourceLocation().startLine, QStringList(),
                    TranslatorMessage::Unfinished, plural);
                msg.setExtraComment(ParserTool::transcode(extracomment.simplified()));
                msg.setId(id);
                msg.setExtras(extra);
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
        }

        return false;
    }

    Engine *engine;
    Translator *m_translator;
    ConversionData &m_cd;
    QString m_fileName;
    QString m_component;

    // comments
    QString extracomment;
    QString msgid;
    TranslatorMessage::ExtraData extra;
    QString sourcetext;
    QString trcontext;
    QList<SourceLocation> m_todo;
};

QString createErrorString(const QString &filename, const QString &code, Parser &parser)
{
    // print out error
    QStringList lines = code.split(QLatin1Char('\n'));
    lines.append(QLatin1String("\n")); // sentinel.
    QString errorString;

    const auto messages = parser.diagnosticMessages();
    for (const DiagnosticMessage &m : messages) {

        if (m.isWarning())
            continue;

        const int line = m.loc.startLine;
        const int column = m.loc.startColumn;
        QString error = filename + QLatin1Char(':')
                        + QString::number(line) + QLatin1Char(':') + QString::number(column)
                        + QLatin1String(": error: ") + m.message + QLatin1Char('\n');

        const QString textLine = lines.at(line > 0 ? line - 1 : 0);
        error += textLine + QLatin1Char('\n');
        for (int i = 0, end = qMin(column > 0 ? column - 1 : 0, textLine.size()); i < end; ++i) {
            const QChar ch = textLine.at(i);
            if (ch.isSpace())
                error += ch;
            else
                error += QLatin1Char(' ');
        }
        error += QLatin1String("^\n");
        errorString += error;
    }
    return errorString;
}

void FindTrCalls::postVisit(AST::Node *node)
{
    if (node->statementCast() != 0 || node->uiObjectMemberCast()) {
        processComments(node->lastSourceLocation().end());

        if (!sourcetext.isEmpty() || !extracomment.isEmpty() || !msgid.isEmpty() || !extra.isEmpty()) {
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
    extracomment.clear();
    msgid.clear();
    extra.clear();
    sourcetext.clear();
}

void FindTrCalls::processComment(const SourceLocation &loc)
{
    if (!loc.length)
        return;

    const QStringView commentStr = engine->midRef(loc.begin(), loc.length);
    const QChar *chars = commentStr.constData();
    const int length = commentStr.size();

    // Try to match the logic of the C++ parser.
    if (*chars == QLatin1Char(':') && chars[1].isSpace()) {
        if (!extracomment.isEmpty())
            extracomment += QLatin1Char(' ');
        extracomment += QString(chars+2, length-2);
    } else if (*chars == QLatin1Char('=') && chars[1].isSpace()) {
        msgid = QString(chars+2, length-2).simplified();
    } else if (*chars == QLatin1Char('~') && chars[1].isSpace()) {
        QString text = QString(chars+2, length-2).trimmed();
        int k = text.indexOf(QLatin1Char(' '));
        if (k > -1) {
            QString commentvalue = text.mid(k + 1).trimmed();
            if (commentvalue.startsWith(QLatin1Char('"')) && commentvalue.endsWith(QLatin1Char('"'))
               && commentvalue.size() != 1) {
               commentvalue = commentvalue.sliced(1, commentvalue.size() - 2);
            }
            extra.insert(text.left(k), commentvalue);
        }
    } else if (*chars == QLatin1Char('%') && chars[1].isSpace()) {
        sourcetext.reserve(sourcetext.size() + length-2);
        ushort *ptr = (ushort *)sourcetext.data() + sourcetext.size();
        int p = 2, c;
        forever {
            if (p >= length)
                break;
            c = chars[p++].unicode();
            if (std::isspace(c))
                continue;
            if (c != '"') {
                yyMsg(loc.startLine) << "Unexpected character in meta string\n";
                break;
            }
            forever {
                if (p >= length) {
                  whoops:
                    yyMsg(loc.startLine) << "Unterminated meta string\n";
                    break;
                }
                c = chars[p++].unicode();
                if (c == '"')
                    break;
                if (c == '\\') {
                    if (p >= length)
                        goto whoops;
                    c = chars[p++].unicode();
                    if (c == '\r' || c == '\n')
                        goto whoops;
                    *ptr++ = '\\';
                }
                *ptr++ = c;
            }
        }
        sourcetext.resize(ptr - (ushort *)sourcetext.data());
    } else {
        int idx = 0;
        ushort c;
        while ((c = chars[idx].unicode()) == ' ' || c == '\t' || c == '\r' || c == '\n')
            ++idx;
        if (!memcmp(chars + idx, QmlMagicComment.unicode(), QmlMagicComment.size() * 2)) {
            idx += QmlMagicComment.size();
            QString comment = QString(chars + idx, length - idx).simplified();
            int k = comment.indexOf(QLatin1Char(' '));
            if (k == -1) {
                trcontext = comment;
            } else {
                trcontext = comment.left(k);
                comment.remove(0, k + 1);
                TranslatorMessage msg(
                        trcontext, QString(),
                        comment, QString(),
                        m_fileName, loc.startLine, QStringList(),
                        TranslatorMessage::Finished, /*plural=*/false);
                msg.setExtraComment(extracomment.simplified());
                extracomment.clear();
                m_translator->append(msg);
                m_translator->setExtras(extra);
                extra.clear();
            }

            m_component = trcontext;
        }
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

static bool load(Translator &translator, const QString &filename, ConversionData &cd, bool qmlMode)
{
    cd.m_sourceFileName = filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        cd.appendError(QStringLiteral("Cannot open %1: %2").arg(filename, file.errorString()));
        return false;
    }

    QString code;
    if (!qmlMode) {
        code = QTextStream(&file).readAll();
    } else {
        QTextStream ts(&file);
        code = ts.readAll();
    }

    Engine driver;
    Parser parser(&driver);

    Lexer lexer(&driver);
    lexer.setCode(code, /*line = */ 1, qmlMode);
    driver.setLexer(&lexer);

    if (qmlMode ? parser.parse() : parser.parseProgram()) {
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
    return load(translator, filename, cd, /*qmlMode=*/ true);
}

bool loadQScript(Translator &translator, const QString &filename, ConversionData &cd)
{
    return load(translator, filename, cd, /*qmlMode=*/ false);
}

QT_END_NAMESPACE
