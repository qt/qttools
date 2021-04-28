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

#include "relatedclass.h"

#include "node.h"

/*!
  \struct RelatedClass
  \brief A struct for indicating that a ClassNode is related in some way to another ClassNode.

  This struct has nothing to do with the \c {\\relates} command. This struct
  is used for indicating that a ClassNode is a base class of another ClassNode,
  is a derived class of another ClassNode, or is an ignored base class of
  another ClassNode. This struct is only used in ClassNode.
*/

/*! \fn RelatedClass::RelatedClass()
  The default constructor does nothing. It is only used for allocating empty
  instances of RelatedClass in containers.
 */

/*! \fn RelatedClass::RelatedClass(Access access, ClassNode *node)
  This is the constructor used when the related class has been resolved.
  In other words, when the ClassNode has been created so that \a node is
  not \c nullptr.
*/

/*! \fn RelatedClass::RelatedClass(Access access, const QStringList &path, const QString &signature)
  This is the constructor used when the related class has not bee resolved,
  because it hasn't been created yet. In that case, we store the qualified
  \a path name of the class and the \a signature of the class, which I think
  is just the name of the class.

  \note We might be able to simplify the whole RelatedClass concept. Maybe we
  can get rid of it completely.
*/

/*! \fn bool RelatedClass::isPrivate() const
  Returns \c true if this RelatedClass is marked as Access::Private.
*/
bool RelatedClass::isPrivate() const
{
    return (m_access == Access::Private);
}

