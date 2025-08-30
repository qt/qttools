// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module ShowInternalModule
    \brief Test module for showinternal functionality.
*/

/*!
    \class InternalClass
    \inmodule ShowInternalModule
    \brief A test class demonstrating showinternal functionality.

    This class contains a mix of regular and internal documentation.
    Regular members should always appear, while internal members
    should only appear when showinternal is enabled.
*/
class InternalClass
{
public:
    /*!
        \brief Regular public constructor.

        This should always appear in documentation.
    */
    InternalClass();

    /*!
        \brief Regular public function.

        This should always appear in documentation.
    */
    void publicFunction();

    /*!
        \internal
        \brief Internal public function.

        This should only appear when showinternal is enabled.
    */
    void internalPublicFunction();

    /*!
        \brief Regular public enum.

        This should always appear in documentation.

        \value RegularValue Regular enum value
        \value AnotherValue Another regular value
    */
    enum PublicEnum {
        RegularValue,
        AnotherValue
    };

protected:
    /*!
        \brief Regular protected function.

        This should always appear in documentation.
    */
    void protectedFunction();

    /*!
        \internal
        \brief Internal protected function.

        This should only appear when showinternal is enabled.
    */
    void internalProtectedFunction();

private:
    /*!
        \brief Regular private function.

        This should appear only when includeprivate is enabled,
        regardless of showinternal setting.
    */
    void regularPrivateFunction();

    /*!
        \internal
        \brief Internal private function.

        This should only appear when BOTH showinternal AND
        includeprivate are enabled.
    */
    void internalPrivateFunction();

    /*!
        \brief Regular private variable.

        Should follow normal includeprivate rules.
    */
    int regularPrivateVar = 0;

    /*!
        \internal
        \brief Internal private variable.

        Should require both showinternal and includeprivate.
    */
    int internalPrivateVar = 42;
};

/*!
    \internal
    \class InternalOnlyClass
    \inmodule ShowInternalModule
    \brief A completely internal class.

    This entire class is marked internal and should only appear
    when showinternal is enabled.
*/
class InternalOnlyClass
{
public:
    /*!
        \brief Constructor of internal class.

        Even though this is a regular comment, the class is internal.
    */
    InternalOnlyClass();

    /*!
        \internal
        \brief Internal function in internal class.

        This is internal within an already internal class.
    */
    void internalFunction();
};

/*!
    \internal
    \relates ShowInternalModule
    \brief Internal global function.

    This should only appear when showinternal is enabled.
*/
void internalGlobalFunction();

/*!
    \brief Regular global function.
    \relates ShowInternalModule

    This should always appear in documentation.
*/
void regularGlobalFunction();

