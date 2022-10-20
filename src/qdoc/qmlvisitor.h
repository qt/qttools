/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef QMLVISITOR_H
#define QMLVISITOR_H

#include "node.h"
#include "qmltypenode.h"

#include <QtCore/qstring.h>

#ifndef QT_NO_DECLARATIVE
#    include <private/qqmljsastvisitor_p.h>
#    include <private/qqmljsengine_p.h>
#endif

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

#ifndef QT_NO_DECLARATIVE
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
#endif

QT_END_NAMESPACE

#endif
