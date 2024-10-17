// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#pragma once

// dummy declaration
namespace std {
    template <class T, int N> struct array;
}

template <typename T>
class TemplatedClass
{
public:
    /*!
    A templated method under a templated class with a non-defaulted type template parameter.
    */
    template <typename U>
    void templated_method_with_type_template_parameter(U);

    /*!
    A templated method under a templated class with a defaulted type template parameter.
    */
    template <typename U = bool>
    void templated_method_with_defaulted_type_template_parameter(U);

    /*!
    A templated method under a templated class with a type template parameter pack.
    */
    template <typename... Ts>
    void templated_method_with_type_template_parameter_pack(Ts...);

    /*!
    A templated method under a templated class with a non-defaulted non type template parameter.
    */
    template <int Size>
    void templated_method_with_non_type_template_parameter();

    /*!
    A templated method under a templated class with a defaulted non type template parameter.
    */
    template <int Size = 10>
    void templated_method_with_defaulted_non_type_template_parameter();

    /*!
    A templated method under a templated class with a non type template parameter pack.
    */
    template <int... Dimensions>
    void templated_method_with_non_type_template_parameter_pack();

    /*!
    A templated method under a templated class with a placeholder non type template parameter.
    */
    template <auto Predicate>
    void templated_method_with_placeholder_non_type_template_parameter();

    /*!
    A templated method under a templated class with a non-defaulted template template parameter.
    */
    template <typename U, template <typename> typename X>
    void templated_method_with_template_template_parameter(X<U>);

    /*!
    A templated method under a templated class with a defaulted template template parameter.
    */
    template <typename U, int size, template <typename, int> typename Container = std::array>
    void templated_method_with_defaulted_template_template_parameter(Container<U, size>);

    /*!
    A templated method under a templated class with a template template parameter pack.
    */
    template <typename U, template <typename> typename... Container>
    void templated_method_with_template_template_parameter_pack(Container<U>...);
};

/*!
\relates <templated_callables.h>
A templated function with a non-defaulted type template parameter.
*/
template <typename T>
void templated_function_with_type_template_parameter(T);

/*!
\relates <templated_callables.h>
A templated function with a defaulted type template parameter.
*/
template <typename T = char>
void templated_function_with_defaulted_type_template_parameter(T);

/*!
\relates <templated_callables.h>
A templated function with a type template parameter pack.
*/
template <typename... Ts>
void templated_function_with_type_template_parameter_pack(Ts...);

/*!
\relates <templated_callables.h>
A templated function with a non-defaulted non type template parameter.
*/
template <char Category>
void templated_function_with_non_type_template_parameter();

/*!
\relates <templated_callables.h>
A templated function with a defaulted non type template parameter.
*/
template <char Category = 'A'>
void templated_function_with_defaulted_non_type_template_parameter();

/*!
\relates <templated_callables.h>
A templated function with a non type template parameter pack.
*/
template <unsigned... Weights>
void templated_function_with_non_type_template_parameter_pack();

/*!
\relates <templated_callables.h>
A templated function with a placeholder non type template parameter.
*/
template <auto Iterator>
void templated_function_with_placeholder_non_type_template_parameter();

/*!
\relates <templated_callables.h>
A templated function with a non-defaulted template template parameter.
*/
template <typename T, template <typename> typename K>
void templated_function_with_template_template_parameter(K<T>);

/*!
\relates <templated_callables.h>
A templated function with a defaulted template template parameter.
*/
template <typename T, int size, template <typename, int> typename Container = std::array>
void templated_function_with_defaulted_template_template_parameter(Container<T, size>);

/*!
\relates <templated_callables.h>
A templated function with a template template parameter pack.
*/
template <typename T, template <typename> typename... Container>
void templated_function_with_template_template_parameter_pack(Container<T>...);
