// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

/*!
    \module MultilineTemplates
    \title Multiline Templates Test Module
    \brief A module for testing multi-line template signature rendering.
*/

/*!
    \class ComplexTemplateClass
    \inmodule MultilineTemplates
    \brief A class with functions that have complex template signatures.

    This class tests QDoc's ability to render template signatures with
    many parameters in a readable multi-line format.
*/
class ComplexTemplateClass
{
public:
    /*!
        Processes values \a t, \a u, and \a v.

        This should trigger multi-line rendering since it has more than
        two template parameters.
    */
    template<typename T, typename U, typename V>
    void threeParams(T t, U u, V v) {}

    /*!
        Processes values \a a, \a b, \a c, and \a d.

        Complex default values make single-line rendering unreadable.
    */
    template<typename First, typename Second, typename Third, typename Fourth = int>
    void fourParamsWithDefault(First a, Second b, Third c, Fourth d = Fourth()) {}

    /*!
        Configures \a setup and \a done handlers.

        This has template parameters with very long default type names.
    */
    template<typename SetupHandler = ComplexTemplateClass,
             typename DoneHandler = ComplexTemplateClass,
             typename ErrorHandler = ComplexTemplateClass>
    void complexDefaults(SetupHandler setup = SetupHandler(),
                        DoneHandler done = DoneHandler()) {}

    /*!
        Transforms \a input using \a pred to filter and returns Result.

        Tests multi-line template rendering combined with a requires clause.
        The requires clause should appear after the closing '>' on the same line.
    */
    template<typename Input, typename Pred, typename Result>
        requires (sizeof(Input) > 0)
    Result transformFiltered(Input input, Pred pred) { return Result{}; }
};

/*!
    \class SimpleTemplateClass
    \inmodule MultilineTemplates
    \brief A class with simple template signatures for comparison.
*/
class SimpleTemplateClass
{
public:
    /*!
        Processes a single \a value.

        This should remain on a single line.
    */
    template<typename T>
    void oneParam(T value) {}

    /*!
        Processes \a first and \a second values.

        Two parameters should still be single-line.
    */
    template<typename T, typename U>
    void twoParams(T first, U second) {}
};
