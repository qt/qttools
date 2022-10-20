/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "usingclause.h"

QT_BEGIN_NAMESPACE

/*!
  \struct UsingClause
  \brief This is supposed to describe a using clause, but I think it is not used.

  This struct is only used in ClassNode. It describes a \c using clause that
  was parsed. But now it looks like it is not actually used at all.

  Maybe we can get rid of it?
*/

/*! \fn UsingClause::UsingClause()
  The default constructor does nothing. It is only used for allocating empty
  instances of UsingClause in containers.
 */

/*! \fn UsingClause::UsingClause(const QString &signature)
  We assume the node that the using clause refers to has not yet been
  created, so this constructor provides its \a signature, which is the
  qualified path name of whatever it is.
 */

/*! \fn const QString &UsingClause::signature() const
  This function returns a const reference to the signature, which is the qualified
  path name of whatever the using clause refers to.
 */

/*! \fn const Node *UsingClause::node()
  This function returns a pointer to the node which has been resolved by looking
  up the signature in the qdoc database. If it wasn't resolved, \c nullptr is returned.
 */

/*! \fn void UsingClause::setNode(const Node *n)
  This function is called when the signature can be resolved. The node pointer
  is set to \a n.
 */

QT_END_NAMESPACE
