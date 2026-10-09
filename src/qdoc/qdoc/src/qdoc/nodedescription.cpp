// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nodedescription.h"

#include "functionnode.h"
#include "genustypes.h"
#include "node.h"
#include "sharedcommentnode.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \namespace NodeText
    \internal
    \brief Provides human-readable descriptions of QDoc nodes.
*/
namespace NodeText {

/*!
  \enum NodeText::GrammaticalNumber
  \brief Specifies the singular or plural form of a node type description.

  \value Singular Use the singular form.
  \value Plural Use the plural form.
*/

/*!
  \internal

  Returns a human-readable description of \a node's type in the specified
  grammatical \a number. The default is Singular because most callers
  describe one node.

  \sa NodeText::GrammaticalNumber
*/
QString nodeTypeDescription(const Node *node, GrammaticalNumber number)
{
    switch (node->nodeType()) {
    case NodeType::Namespace:
        return number == GrammaticalNumber::Singular ? "namespace"_L1 : "namespaces"_L1;
    case NodeType::Class:
        return number == GrammaticalNumber::Singular ? "class"_L1 : "classes"_L1;
    case NodeType::Struct:
        return number == GrammaticalNumber::Singular ? "struct"_L1 : "structs"_L1;
    case NodeType::Union:
        return number == GrammaticalNumber::Singular ? "union"_L1 : "unions"_L1;
    case NodeType::QmlType:
    case NodeType::QmlValueType:
        return number == GrammaticalNumber::Singular ? "type"_L1 : "types"_L1;
    case NodeType::Page:
        return "documentation"_L1;
    case NodeType::Enum:
        return number == GrammaticalNumber::Singular ? "enum"_L1 : "enums"_L1;
    case NodeType::Typedef:
    case NodeType::TypeAlias:
        return number == GrammaticalNumber::Singular ? "typedef"_L1 : "typedefs"_L1;
    case NodeType::Function: {
        const auto fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case Metaness::QmlSignal:
            return number == GrammaticalNumber::Singular ? "signal"_L1 : "signals"_L1;
        case Metaness::QmlSignalHandler:
            return number == GrammaticalNumber::Singular ? "signal handler"_L1 : "signal handlers"_L1;
        case Metaness::QmlMethod:
            return number == GrammaticalNumber::Singular ? "method"_L1 : "methods"_L1;
        case Metaness::MacroWithParams:
        case Metaness::MacroWithoutParams:
            return number == GrammaticalNumber::Singular ? "macro"_L1 : "macros"_L1;
        default:
            break;
        }
        return number == GrammaticalNumber::Singular ? "function"_L1 : "functions"_L1;
    }
    case NodeType::QmlEnum:
        return number == GrammaticalNumber::Singular ? "enumeration"_L1 : "enumerations"_L1;
    case NodeType::Property:
    case NodeType::QmlProperty:
        return number == GrammaticalNumber::Singular ? "property"_L1 : "properties"_L1;
    case NodeType::Module:
    case NodeType::QmlModule:
        return number == GrammaticalNumber::Singular ? "module"_L1 : "modules"_L1;
    case NodeType::Variable:
        return number == GrammaticalNumber::Singular ? "variable"_L1 : "variables"_L1;
    case NodeType::Concept:
        return number == GrammaticalNumber::Singular ? "concept"_L1 : "concepts"_L1;
    case NodeType::SharedComment: {
        const auto *shared = static_cast<const SharedCommentNode *>(node);
        if (shared->isPropertyGroup())
            return number == GrammaticalNumber::Singular ? "property group"_L1 : "property groups"_L1;
        const auto &collective = shared->collective();
        return collective.first()->nodeTypeString();
    }
    default:
        return "documentation"_L1;
    }
}

} // namespace NodeText

QT_END_NAMESPACE
