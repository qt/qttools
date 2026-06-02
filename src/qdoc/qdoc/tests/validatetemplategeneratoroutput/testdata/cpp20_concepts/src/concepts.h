// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

namespace detail {
    template<typename T> struct is_integral { static constexpr bool value = false; };
    template<> struct is_integral<int> { static constexpr bool value = true; };
    template<> struct is_integral<long> { static constexpr bool value = true; };
}

/*!
    \module Cpp20Concepts
    \title C++20 Concepts Test Module
    \brief A module for testing C++20 concept autolinking in synopses.
*/

// Real C++20 concept declaration — the source that ConceptSpecializationExpr
// sub-expressions in the constrained items below resolve to.
template <typename T>
concept Integral = detail::is_integral<T>::value;

/*!
    \concept Integral
    \inmodule Cpp20Concepts
    \brief A type that satisfies integral arithmetic.

    The Integral concept is satisfied by any type modelling the built-in
    integer types: \c {char}, \c {short}, \c {int}, \c {long}, and their
    \c {unsigned} counterparts.
*/

/*!
    \class IntegralCalculator
    \inmodule Cpp20Concepts
    \brief A calculator demonstrating concept autolinking in requires clauses.

    IntegralCalculator exercises the template-head and trailing requires
    forms against the documented \l Integral concept. The rendered
    synopses should hyperlink \c {Integral} to its concept reference page.
*/
class IntegralCalculator
{
public:
    /*!
        Sums \a a and \a b, constrained by the Integral concept.

        Exercises the template-head requires form, which appears before
        the function declaration.
    */
    template <typename T> requires Integral<T>
    T sum(T a, T b);

    /*!
        Doubles \a value using a trailing requires clause that names the
        Integral concept directly.
    */
    template <typename T>
    T processConcept(T value) requires Integral<T>;

    /*!
        Doubles \a value using the constrained-auto syntax. Used as a
        regression check that the existing constrained-auto autolink path
        is preserved alongside the new requires-clause work.
    */
    void doubleValue(Integral auto value);
};

/*!
    \class IntegralBox
    \inmodule Cpp20Concepts
    \brief A box holding any Integral type.

    IntegralBox demonstrates the direct concept-on-template-parameter
    form: the template parameter \a T is constrained by the Integral
    concept without an explicit \c {requires} clause.
*/
template <Integral T>
class IntegralBox
{
public:
    IntegralBox(T value) : m_value(value) {}
    T value() const { return m_value; }
private:
    T m_value;
};
