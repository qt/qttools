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

#include "proxynode.h"

#include "tree.h"

QT_BEGIN_NAMESPACE

/*!
  \class ProxyNode
  \brief A class for representing an Aggregate that is documented in a different module.

  This class is used to represent an Aggregate (usually a class)
  that is located and documented in a different module. In the
  current module, a ProxyNode holds child nodes that are related
  to the class in the other module.

  For example, class QHash is located and documented in QtCore.
  There are many global functions named qHash() in QtCore that
  are all related to class QHash using the \c relates command.
  There are also a few qHash() function in QtNetwork that are
  related to QHash. These functions must be documented when the
  documentation for QtNetwork is generated, but the reference
  page for QHash must link to that documentation in its related
  nonmembers list.

  The ProxyNode allows qdoc to construct links to the related
  functions (or other things?) in QtNetwork from the reference
  page in QtCore.
 */

/*!
  Constructs the ProxyNode, which at this point looks like any
  other Aggregate, and then finds the Tree this node is in and
  appends this node to that Tree's proxy list so it will be
  easy to find later.
 */
ProxyNode::ProxyNode(Aggregate *parent, const QString &name) : Aggregate(Node::Proxy, parent, name)
{
    tree()->appendProxy(this);
}

/*! \fn bool ProxyNode::docMustBeGenerated() const
  Returns true because a ProxyNode always means some documentation
  must be generated.
*/

/*! \fn bool ProxyNode::isRelatableType() const
  Returns true because the ProxyNode exists so that elements
  can be related to it with the \c {\\relates} command.
*/

QT_END_NAMESPACE
