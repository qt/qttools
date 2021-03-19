/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include "testcpp.h"

/*!
    \qmlmodule QDoc.Test \QDocTestVer
    \title QDoc.Test QML Module
    \brief QML Types for the Test module.
    \since 1.1
    \preliminary

    \testnoautolist
*/

/*!
    \qmlmodule Test.Empty 1.0
    \title No QML Types Here
    \brief A QML module with no member types.
*/

/*!
    \qmlmodule Test.NoVer
    \title Versionless QML Module
    \brief QML Types for the Test module without version.
    \since 1.1
*/

/*!
    \qmltype Type
    \instantiates TestQDoc::Test
    \inqmlmodule QDoc.Test
    \brief A QML type documented in a .cpp file.
*/

/*!
    \qmltype TypeNoVersion
    \instantiates TestQDoc::TestDerived
    \inqmlmodule Test.NoVer
    \brief Another QML type documented in a .cpp file.
*/

/*!
    \qmlproperty int Type::id
    \readonly
    \brief A read-only property.
*/

/*!
    \qmlproperty string QDoc.Test::Type::name
    \required
    \brief Name of the Test.
*/

/*!
    \qmlattachedproperty enumeration Type::type
    \default Type.NoType

    \value Type.NoType
           Nothing
    \value Type.SomeType
           Something
*/

/*!
    \qmlproperty int Type::group.first
    \qmlproperty int Type::group.second
    \qmlproperty int Type::group.third

    \brief A property group.
*/

/*!
    \qmlsignal Type::group.created

    This signal is prefixed with \e group.
*/

/*!
    \qmlproperty int Type::fourth
    \qmlproperty int Type::fifth

    \brief A group of properties sharing a documentation comment.
*/

/*!
    \qmlmethod Type Type::copy(a)

    Returns another Type based on \a a.
*/

/*!
    \qmlmethod Type::enable()
    \qmlmethod Type::disable()

    Enables or disables this type.
*/

/*!
    \qmlsignal Type::completed(int status)

    This signal is emitted when the operation completed with \a status.
*/

/*!
    \qmlattachedsignal Type::configured()

    This attached signal is emitted when the type was configured.
*/

/*!
    \qmlmethod Type::deprecatedMethod()

    \deprecated [6.2] This method has no replacement //! No full stop for test.

    This is a method that should include information about being deprecated
    and that it has been so since 6.2 in its docs.
*/
