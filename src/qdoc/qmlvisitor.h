// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QMLVISITOR_H
#define QMLVISITOR_H

#include "node.h"
#include "qmltypenode.h"

#include <QtCore/qstring.h>

#include <private/qqmljsastvisitor_p.h>
#include <private/qqmljsengine_p.h>

QT_BEGIN_NAMESPACE

class Aggregate;

struct QmlPropArgs
{
    QString m_type {};
    QString m_module {};
    QString m_component {};
    QString m_name;

    void clear()
    {
        m_type.clear();
        m_module.clear();
        m_component.clear();
        m_name.clear();
    }
};

class QmlDocVisitor : public QQmlJS::AST::Visitor
{
public:
    QmlDocVisitor(const QString &filePath, const QString &code, QQmlJS::Engine *engine,
                  const QSet<QString> &commands, const QSet<QString> &topics);
    ~QmlDocVisitor() override = default;

    bool visit(QQmlJS::AST::UiImport *import) override;
    void endVisit(QQmlJS::AST::UiImport *definition) override;

    bool visit(QQmlJS::AST::UiObjectDefinition *definition) override;
    void endVisit(QQmlJS::AST::UiObjectDefinition *definition) override;

    bool visit(QQmlJS::AST::UiPublicMember *member) override;
    void endVisit(QQmlJS::AST::UiPublicMember *definition) override;

    bool visit(QQmlJS::AST::UiObjectBinding *) override;
    void endVisit(QQmlJS::AST::UiObjectBinding *) override;
    void endVisit(QQmlJS::AST::UiArrayBinding *) override;
    bool visit(QQmlJS::AST::UiArrayBinding *) override;

    bool visit(QQmlJS::AST::IdentifierPropertyName *idproperty) override;

    bool visit(QQmlJS::AST::FunctionDeclaration *) override;
    void endVisit(QQmlJS::AST::FunctionDeclaration *) override;

    bool visit(QQmlJS::AST::UiScriptBinding *) override;
    void endVisit(QQmlJS::AST::UiScriptBinding *) override;

    bool visit(QQmlJS::AST::UiQualifiedId *) override;
    void endVisit(QQmlJS::AST::UiQualifiedId *) override;

    void throwRecursionDepthError() final;
    [[nodiscard]] bool hasError() const;

private:
    QString getFullyQualifiedId(QQmlJS::AST::UiQualifiedId *id);
    [[nodiscard]] QQmlJS::SourceLocation precedingComment(quint32 offset) const;
    bool applyDocumentation(QQmlJS::SourceLocation location, Node *node);
    void applyMetacommands(QQmlJS::SourceLocation location, Node *node, Doc &doc);
    bool splitQmlPropertyArg(const Doc &doc, const QString &arg, QmlPropArgs &qpa);

    QQmlJS::Engine *m_engine { nullptr };
    quint32 m_lastEndOffset {};
    quint32 m_nestingLevel {};
    QString m_filePath {};
    QString m_name {};
    QString m_document {};
    ImportList m_importList {};
    QSet<QString> m_commands {};
    QSet<QString> m_topics {};
    QSet<quint32> m_usedComments {};
    Aggregate *m_current { nullptr };
    bool hasRecursionDepthError { false };
};

QT_END_NAMESPACE

#endif
