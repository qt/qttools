// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    for (const auto &item : std::as_const(m_items)) {
        if (item.name() == name)
            return item.value();
    }
    return QString();
}

/*!
  Sets \a since information to a named enum \a value, if it
  exists in this enum.
*/
void EnumNode::setSince(const QString &value, const QString &since)
{
    auto it = std::find_if(m_items.begin(), m_items.end(), [value](EnumItem ev) {
            return ev.name() == value;
    });
    if (it != m_items.end())
        it->setSince(since);
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
