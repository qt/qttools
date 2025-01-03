// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "headernode.h"

QT_BEGIN_NAMESPACE

/*!
  \class Headernode
  \brief This class represents a C++ header file.
 */

HeaderNode::HeaderNode(Aggregate *parent, const QString &name) : Aggregate(HeaderFile, parent, name)
{
    // Set the include file with enclosing angle brackets removed
    if (name.startsWith(QChar('<')) && name.size() > 2)
        Aggregate::setIncludeFile(name.mid(1).chopped(1));
    else
        Aggregate::setIncludeFile(name);
}

/*!
  Returns true if this header file node is not private and
  contains at least one public child node with documentation.
 */
bool HeaderNode::docMustBeGenerated() const
{
    if (isInAPI())
        return true;
    return hasDocumentedChildren();
}

/*!
  Returns true if this header file node contains at least one
  child that has documentation and is not private or internal.
 */
bool HeaderNode::hasDocumentedChildren() const
{
    return std::any_of(m_children.cbegin(), m_children.cend(),
                       [](Node *child) { return child->isInAPI(); });
}

QT_END_NAMESPACE
