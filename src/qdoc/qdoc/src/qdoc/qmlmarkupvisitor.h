// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLMARKUPVISITOR_H
#define QMLMARKUPVISITOR_H

#include "node.h"
#include "tree.h"
#include "utilities.h"

#include <QtCore/qstring.h>

#include <private/qqmljsastvisitor_p.h>
#include <private/qqmljsengine_p.h>

QT_BEGIN_NAMESPACE

class QmlMarkupVisitor : public QQmlJS::AST::Visitor
{
public:
    enum ExtraType { Comment, Pragma };

    QmlMarkupVisitor(const QString &code, const QList<QQmlJS::SourceLocation> &pragmas,
                     QQmlJS::Engine *engine);
    ~QmlMarkupVisitor() override = default;

    QString markedUpCode();
    [[nodiscard]] bool hasError() const;

    bool visit(QQmlJS::AST::UiImport *) override;
    void endVisit(QQmlJS::AST::UiImport *) override;

    bool visit(QQmlJS::AST::UiPublicMember *) override;
    bool visit(QQmlJS::AST::UiObjectDefinition *) override;

    bool visit(QQmlJS::AST::UiObjectInitializer *) override;
    void endVisit(QQmlJS::AST::UiObjectInitializer *) override;

    bool visit(QQmlJS::AST::UiObjectBinding *) override;
    bool visit(QQmlJS::AST::UiScriptBinding *) override;
    bool visit(QQmlJS::AST::UiArrayBinding *) override;
    bool visit(QQmlJS::AST::UiArrayMemberList *) override;
    bool visit(QQmlJS::AST::UiQualifiedId *) override;

    bool visit(QQmlJS::AST::ThisExpression *) override;
    bool visit(QQmlJS::AST::IdentifierExpression *) override;
    bool visit(QQmlJS::AST::NullExpression *) override;
    bool visit(QQmlJS::AST::TrueLiteral *) override;
    bool visit(QQmlJS::AST::FalseLiteral *) override;
    bool visit(QQmlJS::AST::NumericLiteral *) override;
    bool visit(QQmlJS::AST::StringLiteral *) override;
    bool visit(QQmlJS::AST::RegExpLiteral *) override;
    bool visit(QQmlJS::AST::ArrayPattern *) override;

    bool visit(QQmlJS::AST::ObjectPattern *) override;
    void endVisit(QQmlJS::AST::ObjectPattern *) override;

    bool visit(QQmlJS::AST::PatternElementList *) override;
    bool visit(QQmlJS::AST::Elision *) override;
    bool visit(QQmlJS::AST::PatternProperty *) override;
    bool visit(QQmlJS::AST::ArrayMemberExpression *) override;
    bool visit(QQmlJS::AST::FieldMemberExpression *) override;
    bool visit(QQmlJS::AST::NewMemberExpression *) override;
    bool visit(QQmlJS::AST::NewExpression *) override;
    bool visit(QQmlJS::AST::ArgumentList *) override;
    bool visit(QQmlJS::AST::PostIncrementExpression *) override;
    bool visit(QQmlJS::AST::PostDecrementExpression *) override;
    bool visit(QQmlJS::AST::DeleteExpression *) override;
    bool visit(QQmlJS::AST::VoidExpression *) override;
    bool visit(QQmlJS::AST::TypeOfExpression *) override;
    bool visit(QQmlJS::AST::PreIncrementExpression *) override;
    bool visit(QQmlJS::AST::PreDecrementExpression *) override;
    bool visit(QQmlJS::AST::UnaryPlusExpression *) override;
    bool visit(QQmlJS::AST::UnaryMinusExpression *) override;
    bool visit(QQmlJS::AST::TildeExpression *) override;
    bool visit(QQmlJS::AST::NotExpression *) override;
    bool visit(QQmlJS::AST::BinaryExpression *) override;
    bool visit(QQmlJS::AST::ConditionalExpression *) override;
    bool visit(QQmlJS::AST::CommaExpression *) override;

    bool visit(QQmlJS::AST::Block *) override;
    void endVisit(QQmlJS::AST::Block *) override;

    bool visit(QQmlJS::AST::VariableStatement *) override;
    bool visit(QQmlJS::AST::VariableDeclarationList *) override;
    bool visit(QQmlJS::AST::EmptyStatement *) override;
    bool visit(QQmlJS::AST::ExpressionStatement *) override;
    bool visit(QQmlJS::AST::IfStatement *) override;
    bool visit(QQmlJS::AST::DoWhileStatement *) override;
    bool visit(QQmlJS::AST::WhileStatement *) override;
    bool visit(QQmlJS::AST::ForStatement *) override;
    bool visit(QQmlJS::AST::ForEachStatement *) override;
    bool visit(QQmlJS::AST::ContinueStatement *) override;
    bool visit(QQmlJS::AST::BreakStatement *) override;
    bool visit(QQmlJS::AST::ReturnStatement *) override;
    bool visit(QQmlJS::AST::WithStatement *) override;

    bool visit(QQmlJS::AST::CaseBlock *) override;
    void endVisit(QQmlJS::AST::CaseBlock *) override;

    bool visit(QQmlJS::AST::SwitchStatement *) override;
    bool visit(QQmlJS::AST::CaseClause *) override;
    bool visit(QQmlJS::AST::DefaultClause *) override;
    bool visit(QQmlJS::AST::LabelledStatement *) override;
    bool visit(QQmlJS::AST::ThrowStatement *) override;
    bool visit(QQmlJS::AST::TryStatement *) override;
    bool visit(QQmlJS::AST::Catch *) override;
    bool visit(QQmlJS::AST::Finally *) override;
    bool visit(QQmlJS::AST::FunctionDeclaration *) override;
    bool visit(QQmlJS::AST::FunctionExpression *) override;
    bool visit(QQmlJS::AST::FormalParameterList *) override;
    bool visit(QQmlJS::AST::DebuggerStatement *) override;

private:
    typedef QHash<QString, QString> StringHash;
    void addExtra(quint32 start, quint32 finish);
    void addMarkedUpToken(QQmlJS::SourceLocation &location, const QString &text,
                          const StringHash &attributes = StringHash());
    void addVerbatim(QQmlJS::SourceLocation first,
                     QQmlJS::SourceLocation last = QQmlJS::SourceLocation());
    QString sourceText(QQmlJS::SourceLocation &location);
    void throwRecursionDepthError() final;

    QQmlJS::Engine *m_engine { nullptr };
    QList<ExtraType> m_extraTypes {};
    QList<QQmlJS::SourceLocation> m_extraLocations {};
    QString m_source {};
    QString m_output {};
    quint32 m_cursor {};
    int m_extraIndex {};
    bool m_hasRecursionDepthError { false };
};
Q_DECLARE_TYPEINFO(QmlMarkupVisitor::ExtraType, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif
