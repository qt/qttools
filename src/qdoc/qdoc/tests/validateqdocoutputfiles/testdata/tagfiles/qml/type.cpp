// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    \modulestate Tech Preview
*/

/*!
    \qmltype Type
    \instantiates TestQDoc::Test
    \inqmlmodule QDoc.Test
    \brief A QML type documented in a .cpp file.
    \meta status { <Work In Progress> }
*/

/*!
    \qmltype TypeNoVersion
    \instantiates TestQDoc::TestDerived
    \inqmlmodule Test.NoVer
    \brief Another QML type documented in a .cpp file.
*/

/*!
    \qmltype OldType
    \inqmlmodule QDoc.Test
    \brief Deprecated old type.
    \deprecated [1.0]
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

    \deprecated [6.2] This method has no replacement.

    This is a method that should include information about being deprecated
    and that it has been so since 6.2 in its docs.
*/
