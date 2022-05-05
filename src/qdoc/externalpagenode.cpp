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

#include "externalpagenode.h"

QT_BEGIN_NAMESPACE

/*!
  \class ExternalPageNode

  \brief The ExternalPageNode represents an external documentation page.

  Qdoc can generate links to pages that are not part of the documentation
  being generated. 3rd party software pages are often referenced by links
  from the QT documentation. Qdoc creates an ExternalPageNode when it sees
  an \c {\\externalpage} command. The HTML generator can then use the node
  when it needs to create links to the external page.

  ExternalPageNode inherits PageNode.
*/

/*! \fn ExternalPageNode::ExternalPageNode(Aggregate *parent, const QString &name)
  The constructor creates an ExternalPageNode as a child node of \a parent.
  It's \a name is the argument from the \c {\\externalpage} command. The node
  type is Node::ExternalPage, and the page type is Node::ArticlePage.
 */

QT_END_NAMESPACE
