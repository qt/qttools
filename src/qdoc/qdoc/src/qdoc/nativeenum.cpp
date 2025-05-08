// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "nativeenum.h"

#include "enumnode.h"
#include "node.h"
#include "qdocdatabase.h"

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
    \class NativeEnum
    \brief Encapsulates information about native (C++) enum values.

    The NativeEnum resolves a qualified path to a C++ enum into a
    EnumNode pointer. This information is used for replicating the
    enumerator documentation in QML API reference.
*/

/*!
    \class NativeEnumInterface
    \brief Interface implemented by Node subclasses that can refer
           to a C++ enum.

    Nodes implementing this interface provide a const and a non-const
    getter returning a NativeEnum pointer.
*/

/*!
    Locates the node specified by \a path and sets it as the C++ enumeration
    associated with this property.

    \a registeredQmlName is a prefix to use in the generated enum value
    documentation; each enumerator is prefixed with \c {<registeredQmlName>.}

    \note The target EnumNode is searched under the primary tree only.

    Returns \c true on success.
*/
bool NativeEnum::resolve(const QString &path, const QString &registeredQmlName)
{
    m_enumNode = static_cast<EnumNode*>(
        QDocDatabase::qdocDB()->primaryTree()->findNodeByNameAndType(path.split(u"::"_s), &Node::isEnumType)
    );
    if (m_enumNode && !registeredQmlName.isEmpty())
        m_prefix = registeredQmlName;
    return m_enumNode != nullptr;
}

QT_END_NAMESPACE
