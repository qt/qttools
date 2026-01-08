// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

// Dummy type traits for testing (avoid standard library dependency)
namespace detail {
    template<typename T> struct is_integral { static constexpr bool value = false; };
    template<> struct is_integral<int> { static constexpr bool value = true; };
    template<> struct is_integral<long> { static constexpr bool value = true; };

    template<typename T> struct is_floating { static constexpr bool value = false; };
    template<> struct is_floating<float> { static constexpr bool value = true; };
    template<> struct is_floating<double> { static constexpr bool value = true; };
}

/*!
    \module Cpp20Concepts
    \title C++20 Concepts Test Module
    \brief A module for testing C++20 concepts documentation.
*/

/*!
    \class ConceptExample
    \inmodule Cpp20Concepts
    \brief A class demonstrating functions with requires clauses.
*/
class ConceptExample
{
public:
    /*!
        Adds two integral values \a a and \a b.

        This function template uses a requires clause to constrain
        the template parameter T to integral types only.
    */
    template<typename T> requires detail::is_integral<T>::value
    T addIntegrals(T a, T b) { return a + b; }

    /*!
        Multiplies two floating-point values \a a and \a b.

        Uses a requires clause with floating-point constraint.
    */
    template<typename T> requires detail::is_floating<T>::value
    T multiplyFloats(T a, T b) { return a * b; }

    /*!
        Converts \a value to double using compound constraint.

        Uses a requires clause with multiple constraints combined
        using logical operators.
    */
    template<typename T> requires (detail::is_integral<T>::value || detail::is_floating<T>::value)
    double toDouble(T value) { return static_cast<double>(value); }
};

/*!
    \class ConstrainedContainer
    \inmodule Cpp20Concepts
    \brief A container template with a requires clause on the class itself.
*/
template<typename T> requires detail::is_integral<T>::value
class ConstrainedContainer
{
public:
    /*!
        Constructs a container with the given \a size.
    */
    explicit ConstrainedContainer(int size) : m_size(size) {}

    /*!
        Returns the size of the container.
    */
    int size() const { return m_size; }

private:
    int m_size;
};

/*!
    \class TrailingRequiresExample
    \inmodule Cpp20Concepts
    \brief A class demonstrating functions with trailing requires clauses.

    Trailing requires clauses appear after the function declaration,
    rather than after the template parameter list.
*/
class TrailingRequiresExample
{
public:
    /*!
        Processes \a value with a trailing requires clause.

        The trailing requires clause appears after the function signature,
        which is an alternative syntax to placing it after the template
        parameter list.
    */
    template<typename T>
    T process(T value) requires detail::is_integral<T>::value
    { return value * 2; }

    /*!
        Transforms \a input using trailing constraint.

        This demonstrates that trailing requires clauses work
        with any constraint expression.
    */
    template<typename T>
    double transform(T input) requires detail::is_floating<T>::value
    { return static_cast<double>(input) + 1.0; }

    /*!
        Combines \a a and \a b with both template-head and trailing requires.

        This demonstrates a function with requires clauses in both positions,
        which is rare but valid C++20 syntax. Both constraints must be satisfied.
    */
    template<typename T> requires detail::is_integral<T>::value
    T combine(T a, T b) requires (sizeof(T) <= 4)
    { return a + b; }
};

/*!
    \class RequiresExpressionExample
    \inmodule Cpp20Concepts
    \brief A class demonstrating requires-expressions for ad-hoc constraints.

    Requires-expressions use the nested \c{requires requires(...) { ... }}
    syntax to define constraints inline without defining a named concept.
*/
class RequiresExpressionExample
{
public:
    /*!
        Iterates over \a container using a requires-expression.

        The requires-expression checks that the type has \c begin()
        and \c end() member functions, making it iterable.
    */
    template<typename T>
        requires requires(T t) { t.begin(); t.end(); }
    void iterate(T& container);

    /*!
        Gets the size of \a container using a simple requires-expression.

        Demonstrates a requires-expression that checks for a \c size()
        member function.
    */
    template<typename T>
        requires requires(T t) { t.size(); }
    int getSize(const T& container);
};

