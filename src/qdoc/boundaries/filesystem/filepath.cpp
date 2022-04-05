/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#include "filepath.hpp"

/*!
 * \class FilePath
 *
 * \brief Represents a path to a file that was known to exist on the
 * filesystem.
 *
 * An instance of this type guarantees that, at the time of creation
 * of the instance, the contained path represented an existing,
 * readable file.
 *
 * The type is intended to be used whenever a user-provided path to a
 * file is encountered the first time, validating that it can be
 * used later on for the duration of a QDoc execution and
 * canonicalizing the original path.
 *
 * Such a usage example could be when resolving a file whose path is
 * provided by the user.
 *
 * Similarly, it is intended to be used at the API boundaries,
 * internally, to relieve the called element of the requirement to
 * check the validity of a path when a file is required and to
 * ensure that a single format of the path is encountered.
 *
 * Do note that the guarantees provided by this type do not
 * necessarily hold after the time of creation of an instance.
 * Indeed, the underlying filesystem may have changed.
 *
 * It is possible to renew the contract by obtaining a new instance:
 *
 * \code
 *     FilePath old...
 *
 *     ...
 *
 *     auto current{FilePath::refine(old.value())};
 * \endcode
 *
 * QDoc itself will not generally perform destructive operations on
 * its input files during an execution and, as such, it is never
 * required to renew a contract. Ensuring that the underlying input
 * files are indeed immutable is out-of-scope for QDoc and it is
 * allowed to consider a case where the contract was invalidated as
 * undefined behavior.
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {wrapped_type_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {has_equality_operator_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {has_less_than_operator_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {has_strictly_less_than_operator_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {has_greater_than_operator_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {has_strictly_greater_than_operator_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {refine_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {value_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {copy_constructor_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {copy_assignment_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {move_constructor_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {move_assignment_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {conversion_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {operator_equal_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {operator_unequal_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {operator_less_than_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {operator_less_than_or_equal_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {operator_greater_than_documentation} {FilePath}
 */

/*!
 * \include boundaries/refined_typedef_members.qdocinc {operator_greater_than_or_equal_documentation} {FilePath}
 */
