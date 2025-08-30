// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module TestModule
    \brief Jsut another test case.
*/

/*!
    \class TestClass
    \inmodule TestModule
    \brief A test class demonstrating private member documentation.

    This class contains documented members at all three access levels:
    public, protected, and private. It is used to test the includeprivate
    configuration option.
*/
class TestClass
{
public:
    /*!
        \brief A public enumeration.

        \value PublicValue1 First public value
        \value PublicValue2 Second public value
    */
    enum PublicEnum {
        PublicValue1,
        PublicValue2
    };

    /*!
        \brief Default constructor.

        Creates a TestClass instance.
    */
    TestClass();

    /*!
        \brief Public function.

        This function is publicly accessible.
    */
    void publicFunction();

    /*!
        \brief Public variable.

        This variable is publicly accessible.
    */
    int publicVariable = 0;

protected:
  /*!
      \brief A protected enumeration.

      \value ProtectedValue1 First protected value
      \value ProtectedValue2 Second protected value
  */
    enum ProtectedEnum {
        ProtectedValue1,
        ProtectedValue2
    };

    /*!
        \brief Protected function.

        This function is accessible to derived classes.
    */
    void protectedFunction();

    /*!
        \brief Protected variable.

        This variable is accessible to derived classes.
    */
    int protectedVariable = 0;

private:
  /*!
      \brief A private enumeration for internal use.

      This enumeration is used internally by the class implementation.
      It should only appear in documentation when includeprivate is enabled.

      \value PrivateValue1 First private value
      \value PrivateValue2 Second private value
  */
    enum PrivateEnum {
        PrivateValue1,
        PrivateValue2
    };

    /*!
        \brief Private function for internal operations.

        This function performs internal operations and should only be
        documented when includeprivate configuration is enabled.
    */
    void privateFunction();

    /*!
        \brief Private helper function.

        This is another private function to test multiple private
        function documentation.
    */
    void privateHelper();

    /*!
        \brief Private variable for internal state.

        This variable holds internal state and should only appear
        in documentation when includeprivate is enabled.
    */
    int privateVariable = 42;

    /*!
        \brief Another private variable.

        This tests multiple private variable documentation.
    */
    bool privateBoolVar = false;
};

/*!
    \relates TestClass
    \brief A related non-member function.

    This function is related to TestClass but not a member. It takes \a obj as
    argument for shits and giggles.
*/
void relatedFunction(const TestClass &obj);

