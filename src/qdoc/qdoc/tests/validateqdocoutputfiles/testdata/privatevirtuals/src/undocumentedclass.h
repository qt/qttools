// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \class UndocumentedVirtualsClass
    \inmodule WassilyKandinsky
    \brief A test class with undocumented and internal pure virtual methods.

    This class tests edge cases of undocumented and internal private pure virtuals.
*/
class UndocumentedVirtualsClass
{
public:
    /*!
        \brief Public constructor.
    */
    UndocumentedVirtualsClass();

private:
    /*!
        \brief Documented private pure virtual.

        This one should appear in documentation.
    */
    virtual void documentedPrivatePureVirtual() = 0;

    /*!
        \internal
        \brief Internal private pure virtual method.

        This method is marked internal so should not appear in documentation
        even though it's pure virtual and private.
    */
    virtual void internalPrivatePureVirtual() = 0;

    // Undocumented private pure virtual - no QDoc comment at all
    virtual void undocumentedPrivatePureVirtual() = 0;

    // Undocumented regular private method for comparison
    void undocumentedPrivateMethod();
};

