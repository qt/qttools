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

  Returns a human-readable description of \a node's type, using the plural
  form when \a plural is true.
*/
QString typeString(const Node *node, bool plural)
{
    switch (node->nodeType()) {
    case NodeType::Namespace:
        return plural ? "namespaces"_L1 : "namespace"_L1;
    case NodeType::Class:
        return plural ? "classes"_L1 : "class"_L1;
    case NodeType::Struct:
        return plural ? "structs"_L1 : "struct"_L1;
    case NodeType::Union:
        return plural ? "unions"_L1 : "union"_L1;
    case NodeType::QmlType:
    case NodeType::QmlValueType:
        return plural ? "types"_L1 : "type"_L1;
    case NodeType::Page:
        return "documentation"_L1;
    case NodeType::Enum:
        return plural ? "enums"_L1 : "enum"_L1;
    case NodeType::Typedef:
    case NodeType::TypeAlias:
        return plural ? "typedefs"_L1 : "typedef"_L1;
    case NodeType::Function: {
        const auto fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case Metaness::QmlSignal:
            return plural ? "signals"_L1 : "signal"_L1;
        case Metaness::QmlSignalHandler:
            return plural ? "signal handlers"_L1 : "signal handler"_L1;
        case Metaness::QmlMethod:
            return plural ? "methods"_L1 : "method"_L1;
        case Metaness::MacroWithParams:
        case Metaness::MacroWithoutParams:
            return plural ? "macros"_L1 : "macro"_L1;
        default:
            break;
        }
        return plural ? "functions"_L1 : "function"_L1;
    }
    case NodeType::QmlEnum:
        return plural ? "enumerations"_L1 : "enumeration"_L1;
    case NodeType::Property:
    case NodeType::QmlProperty:
        return plural ? "properties"_L1 : "property"_L1;
    case NodeType::Module:
    case NodeType::QmlModule:
        return plural ? "modules"_L1 : "module"_L1;
    case NodeType::Variable:
        return plural ? "variables"_L1 : "variable"_L1;
    case NodeType::Concept:
        return plural ? "concepts"_L1 : "concept"_L1;
    case NodeType::SharedComment: {
        const auto *shared = static_cast<const SharedCommentNode *>(node);
        if (shared->isPropertyGroup())
            return plural ? "property groups"_L1 : "property group"_L1;
        const auto &collective = shared->collective();
        return collective.first()->nodeTypeString();
    }
    default:
        return "documentation"_L1;
    }
}

} // namespace NodeText

QT_END_NAMESPACE

