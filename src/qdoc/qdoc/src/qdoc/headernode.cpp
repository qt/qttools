// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "headernode.h"

QT_BEGIN_NAMESPACE

/*!
  \class Headernode
  \brief This class represents a C++ header file.
 */

    HeaderNode::HeaderNode(Aggregate *parent, const QString &name) : Aggregate(NodeType::HeaderFile, parent, name)
{
    // Set the include file with enclosing angle brackets removed
    if (name.startsWith(QChar('<')) && name.size() > 2)
        Aggregate::setIncludeFile(name.mid(1).chopped(1));
    else
        Aggregate::setIncludeFile(name);
}

/*!
    Returns \c true if QDoc must generate documentation for this
    header file node; that is, it matches the inclusion policy
    and is documented, OR has children that match that same criteria.
*/
bool HeaderNode::docMustBeGenerated() const
{
    if (isInAPI())
        return true;

    return std::any_of(m_children.cbegin(), m_children.cend(),
                       [](Node *child) { return child->isInAPI(); });
}

QT_END_NAMESPACE
