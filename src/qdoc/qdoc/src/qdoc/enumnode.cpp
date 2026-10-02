// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "enumnode.h"

#include "aggregate.h"
#include "nativeenum.h"
#include "typedefnode.h"

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

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

/*!
  Returns the display name of the enumeration value \a enumValue for use in
  the constant column of a value table: the value as documented, qualified
  with the scope in which this enum was declared. A related nonmember is
  documented under the related class, but its values keep the C++ scope in
  which the enum was declared, so the scope walk starts from the
  declaration parent. This mirrors the naming of
  CppCodeMarker::markedUpEnumValue (HTML, WebXML) and
  DocBookGenerator::generateEnumValue (DocBook), minus the markup, so that
  the template generators show the same constant names.
*/
QString EnumNode::qualifiedValueName(const QString &enumValue) const
{
    const auto *node = declarationParent() ? declarationParent() : parent();

    const NativeEnum *nativeEnum{nullptr};
    if (auto *ne_if = dynamic_cast<const NativeEnumInterface *>(this))
        nativeEnum = ne_if->nativeEnum();

    if (nativeEnum && nativeEnum->enumNode()
            && !enumValue.startsWith("%1."_L1.arg(nativeEnum->prefix())))
        return "%1.%2"_L1.arg(nativeEnum->prefix(), enumValue);

    // Respect existing prefixes in \value arguments of \qmlenum topics.
    if (isEnumType(Genus::QML)
            && enumValue.section(' ', 0, 0).contains('.'_L1))
        return enumValue;

    QStringList parts;
    const auto *self = static_cast<const Node *>(this);
    while (!node->isHeader() && node->parent()) {
        parts.prepend(node->name());
        if (node->parent() == self || node->parent()->name().isEmpty())
            break;
        node = node->parent();
    }
    if (isScoped())
        parts.append(name());

    parts.append(enumValue);
    const auto &delim = (genus() == Genus::QML) ? "."_L1 : "::"_L1;
    return parts.join(delim);
}

QT_END_NAMESPACE
