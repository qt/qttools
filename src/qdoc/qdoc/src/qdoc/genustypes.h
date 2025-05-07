// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef GENUSTYPES_H
#define GENUSTYPES_H

#include <QtGlobal>

#include <type_traits>

QT_BEGIN_NAMESPACE

/*!
    \headerfile genustypes.h

    \title Genus and NodeType enumerations

    This header file defines two main enumerations:
    - \l {Genus}: An unsigned char type that specifies whether a node in the
      documentation represents a C++ element, a QML element, a text document,
      or a combination used to indicate API types.
    - \l {NodeType}: An unsigned char type that identifies the precise subclass
      of a node, such as Namespace, Class, Function, etc.

    In addition, the file provides two inline utility functions:
    - \l hasCommonGenusType(), which performs a bitwise comparison between two
      \c Genus values to determine if they share any common bits.
    - \l isApiGenus(), which checks if a given \c Genus value represents an API
      type by verifying if it contains the C++ or QML flags.

    These enumerations and functions are used in QDoc to facilitate mapping of
    source elements to node categories, as well as to enable fine-grained search
    and filtering based on node types.
*/

/*!
    \enum Genus
    \relates genustypes.h

    An unsigned char value that specifies whether the Node represents a C++
    element, a QML element, or a text document. The Genus values are also passed
    to search functions to specify the Genus of Tree Node that can satisfy the
    search.

    \value DontCare The Genus is not specified. Used when calling Tree search
                    functions to indicate the search can accept any Genus of
                    Node.
    \value CPP The Node represents a C++ element.
    \value QML The Node represents a QML element.
    \value DOC The Node represents a text document.
    \value API The Node represents an API element.
*/
enum class Genus : unsigned char {
    DontCare = 0x0,
    CPP = 0x1,
    QML = 0x4,
    DOC = 0x8,
    API = CPP | QML
};

/*!
    \brief Determines if two genuses share any common bits.
    \relates genustypes.h

    Performs a bitwise AND operation between the underlying integer values of
    two Genus enums to determine if they share any common bits. Returns \c true
    if at least one bit is common between \a searchMask and \a candidate,
    \c false otherwise.

    \sa isApiGenus()
*/
inline constexpr bool hasCommonGenusType(Genus searchMask, Genus candidate)
{
    using Underlying = std::underlying_type_t<Genus>;
    return (static_cast<Underlying>(searchMask) & static_cast<Underlying>(candidate)) != 0;
}

/*!
    \brief Determines if a genus represents an API type.
    \relates genustypes.h

    Checks if the given genus, \a g,  has bits in common with either Genus::CPP
    or Genus::QML, which are considered API types. The function returns \c true
    if the genus represents an API type (CPP or QML), \c false otherwise.

    \sa hasCommonGenusType()
*/
inline constexpr bool isApiGenus(Genus g) {
    using Underlying = std::underlying_type_t<Genus>;
    // The API flag is defined as the combination of CPP and QML.
    constexpr Underlying apiMask = static_cast<Underlying>(Genus::API);
    return (static_cast<Underlying>(g) & apiMask) != 0;
}

/*!
    \enum GenusType
    \relates genustypes.h

    An unsigned char value that identifies an object as a
    particular subclass of Node.

    \value NoType The node type has not been set yet.
    \value Namespace The Node subclass is NamespaceNode, which represents a C++
           namespace.
    \value Class The Node subclass is ClassNode, which represents a C++ class.
    \value Struct The Node subclass is ClassNode, which represents a C struct.
    \value Union The Node subclass is ClassNode, which represents a C union
           (currently no known uses).
    \value HeaderFile The Node subclass is HeaderNode, which represents a header
           file.
    \value Page The Node subclass is PageNode, which represents a text page from
           a .qdoc file.
    \value Enum The Node subclass is EnumNode, which represents an enum type or
           enum class.
    \value Example The Node subclass is ExampleNode, which represents an example
           subdirectory.
    \value ExternalPage The Node subclass is ExternalPageNode, which is for
           linking to an external page.
    \value Function The Node subclass is FunctionNode, which can represent C++,
           and QML functions.
    \value Typedef The Node subclass is TypedefNode, which represents a C++
           typedef.
    \value Property The Node subclass is PropertyNode, which represents a use of
           Q_Property.
    \value Variable The Node subclass is VariableNode, which represents a global
           variable or class data member.
    \value Group The Node subclass is CollectionNode, which represents a group
           of documents.
    \value Module The Node subclass is CollectionNode, which represents a C++
           module.
    \value QmlEnum The Node subclass is QmlEnumNode, which represents a QML
           enumeration.
    \value QmlType The Node subclass is QmlTypeNode, which represents a QML
           type.
    \value QmlModule The Node subclass is CollectionNode, which represents a QML
           module.
    \value QmlProperty The Node subclass is QmlPropertyNode, which represents a
           property in a QML type.
    \value QmlBasicType The Node subclass is QmlTypeNode, which represents a
           value type like int, etc.
    \value SharedComment The Node subclass is SharedCommentNode, which
           represents a collection of nodes that share the same documentation
           comment.
    \omitvalue Collection
    \value Proxy The Node subclass is ProxyNode, which represents one or more
           entities that are documented in the current module but which actually
           reside in a different module.
    \omitvalue LastType
*/
enum class NodeType : unsigned char {
    NoType,
    Namespace,
    Class,
    Struct,
    Union,
    HeaderFile,
    Page,
    Enum,
    Example,
    ExternalPage,
    Function,
    Typedef,
    TypeAlias,
    Property,
    Variable,
    Group,
    Module,
    QmlEnum,
    QmlType,
    QmlModule,
    QmlProperty,
    QmlValueType,
    SharedComment,
    Collection,
    Proxy
};

QT_END_NAMESPACE

#endif // GENUSTYPES_H

