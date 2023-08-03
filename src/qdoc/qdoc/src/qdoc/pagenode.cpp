// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "pagenode.h"

#include "aggregate.h"

QT_BEGIN_NAMESPACE

/*!
  \class PageNode
  \brief A PageNode is a Node that generates a documentation page.

  Not all subclasses of Node produce documentation pages. FunctionNode,
  PropertyNode, and EnumNode are all examples of subclasses of Node that
  don't produce documentation pages but add documentation to a page.
  They are always child nodes of an Aggregate, and Aggregate inherits
  PageNode.

  Not every subclass of PageNode inherits Aggregate. ExternalPageNode,
  ExampleNode, and CollectionNode are subclasses of PageNode that are
  not subclasses of Aggregate. Because they are not subclasses of
  Aggregate, they can't have children. But they still generate, or
  link to, a documentation page.
 */

/*! \fn QString PageNode::title() const
  Returns the node's title, which is used for the page title.
 */

/*! \fn QString PageNode::subtitle() const
  Returns the node's subtitle, which may be empty.
 */

/*!
  Returns the node's full title.
 */
QString PageNode::fullTitle() const
{
    return title();
}

/*!
  Sets the node's \a title, which is used for the page title.
  Returns true. Adds the node to the parent() nonfunction map
  using the \a title as the key.
 */
bool PageNode::setTitle(const QString &title)
{
    m_title = title;
    parent()->addChildByTitle(this, title);
    return true;
}

/*!
  \fn bool PageNode::setSubtitle(const QString &subtitle)
  Sets the node's \a subtitle. Returns true;
 */

/*! \fn PageNode::PageNode(Aggregate *parent, const QString &name)
  This constructor sets the PageNode's \a parent and the \a name is the
  argument of the \c {\\page} command. The node type is set to Node::Page.
 */

/*! \fn PageNode::PageNode(NodeType type, Aggregate *parent, const QString &name)
  This constructor is not called directly. It is called by the constructors of
  subclasses of PageNode, usually Aggregate. The node type is set to \a type,
  and the parent pointer is set to \a parent. \a name is the argument of the topic
  command that resulted in the PageNode being created. This could be \c {\\class}
  or \c {\\namespace}, for example.
 */

/*! \fn PageNode::~PageNode()
  The destructor is virtual, and it does nothing.
 */

/*! \fn bool PageNode::isPageNode() const
  Always returns \c true because this is a PageNode.
 */

/*! \fn bool PageNode::isTextPageNode() const
  Returns \c true if this instance of PageNode is not an Aggregate.
  The significance of a \c true return value is that this PageNode
  doesn't have children, because it is not an Aggregate.

  \sa Aggregate.
 */

/*! \fn QString PageNode::imageFileName() const
  If this PageNode is an ExampleNode, the image file name
  data member is returned. Otherwise an empty string is
  returned.
 */

/*! \fn void PageNode::setImageFileName(const QString &ifn)
  If this PageNode is an ExampleNode, the image file name
  data member is set to \a ifn. Otherwise the function does
  nothing.
 */

/*! \fn bool PageNode::noAutoList() const
  Returns the value of the no auto-list flag.
 */

/*! \fn void PageNode::setNoAutoList(bool b)
  Sets the no auto-list flag to \a b.
 */

/*! \fn const QStringList &PageNode::groupNames() const
  Returns a const reference to the string list containing all the group names.
 */

/*! \fn void PageNode::appendGroupName(const QString &t)
  Appends \a t to the list of group names.
 */

QT_END_NAMESPACE
