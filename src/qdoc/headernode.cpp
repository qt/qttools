/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#include "headernode.h"

QT_BEGIN_NAMESPACE

/*!
  \class Headernode
  \brief This class represents a C++ header file.
 */

HeaderNode::HeaderNode(Aggregate *parent, const QString &name) : Aggregate(HeaderFile, parent, name)
{
    // Add the include file with enclosing angle brackets removed
    if (name.startsWith(QChar('<')) && name.length() > 2)
        Aggregate::addIncludeFile(name.mid(1).chopped(1));
    else
        Aggregate::addIncludeFile(name);
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
