// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
