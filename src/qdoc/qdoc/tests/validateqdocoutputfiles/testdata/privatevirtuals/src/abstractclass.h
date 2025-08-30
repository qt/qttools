// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module WassilyKandinsky
    \brief The father of abstract art.
*/

/*!
    \class AbstractClass
    \inmodule WassilyKandinsky
    \brief A test class with private pure virtual methods.

    This class demonstrates the corner case of private pure virtual methods.
    These methods are private but must still be implemented by derived classes.
*/
class AbstractClass
{
public:
    /*!
        \brief Public constructor.

        Creates an AbstractClass instance.
    */
    AbstractClass();

    /*!
        \brief Public pure virtual method.

        This pure virtual method is publicly accessible.
    */
    virtual void publicPureVirtual() = 0;

protected:
    /*!
        \brief Protected pure virtual method.

        This pure virtual method is accessible to derived classes.
    */
    virtual void protectedPureVirtual() = 0;

private:
    /*!
        \brief Private pure virtual method for internal implementation.

        This method is private but pure virtual, meaning derived classes
        must implement it even though it's not publicly accessible.
        This creates a special case for documentation.
    */
    virtual void privatePureVirtual() = 0;

    /*!
        \brief Another private pure virtual method.

        Testing multiple private pure virtual methods.
    */
    virtual int privateVirtualCalculation() = 0;

    /*!
        \brief Regular private method (not pure virtual).

        This should respect normal includeprivate settings.
    */
    void regularPrivateMethod();

    /*!
        \brief Private virtual method (not pure).

        This should also respect normal includeprivate settings.
    */
    virtual void privateVirtualMethod();
};

