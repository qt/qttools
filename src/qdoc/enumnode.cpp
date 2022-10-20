/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "enumnode.h"

#include "aggregate.h"
#include "typedefnode.h"

QT_BEGIN_NAMESPACE

/*!
  \class EnumNode
 */

/*!
  Add \a item to the enum type's item list.
 */
void EnumNode::addItem(const EnumItem &item)
{
    m_items.append(item);
    m_names.insert(item.name());
}

/*!
  Returns the access level of the enumeration item named \a name.
  Apparently it is private if it has been omitted by qdoc's
  omitvalue command. Otherwise it is public.
 */
Access EnumNode::itemAccess(const QString &name) const
{
    if (doc().omitEnumItemNames().contains(name))
        return Access::Private;
    return Access::Public;
}

/*!
  Returns the enum value associated with the enum \a name.
 */
QString EnumNode::itemValue(const QString &name) const
{
    for (const auto &item : qAsConst(m_items)) {
        if (item.name() == name)
            return item.value();
    }
    return QString();
}

/*!
  Clone this node on the heap and make the clone a child of
  \a parent.

  Returns a pointer to the clone.
 */
Node *EnumNode::clone(Aggregate *parent)
{
    auto *en = new EnumNode(*this); // shallow copy
    en->setParent(nullptr);
    parent->addChild(en);

    return en;
}

void EnumNode::setFlagsType(TypedefNode *typedefNode)
{
    m_flagsType = typedefNode;
    typedefNode->setAssociatedEnum(this);
}

QT_END_NAMESPACE
