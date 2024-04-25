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
    template <typename U>
    void templated_method_with_type_template_parameter(U);

    template <typename U = bool>
    void templated_method_with_defaulted_type_template_parameter(U);

    template <typename... Ts>
    void templated_method_with_type_template_parameter_pack(Ts...);

    template <int Size>
    void templated_method_with_non_type_template_parameter();

    template <int Size = 10>
    void templated_method_with_defaulted_non_type_template_parameter();

    template <int... Dimensions>
    void templated_method_with_non_type_template_parameter_pack();

    template <auto Predicate>
    void templated_method_with_placeholder_non_type_template_parameter();

    template <typename U, template <typename> typename X>
    void templated_method_with_template_template_parameter(X<U>);

    template <typename U, int size, template <typename, int> typename Container = std::array>
    void templated_method_with_defaulted_template_template_parameter(Container<U, size>);

    template <typename U, template <typename> typename... Container>
    void templated_method_with_template_template_parameter_pack(Container<U>...);
};

template <typename T>
void templated_function_with_type_template_parameter(T);

template <typename T = char>
void templated_function_with_defaulted_type_template_parameter(T);

template <typename... Ts>
void templated_function_with_type_template_parameter_pack(Ts...);

template <char Category>
void templated_function_with_non_type_template_parameter();

template <char Category = 'A'>
void templated_function_with_defaulted_non_type_template_parameter();

template <unsigned... Weights>
void templated_function_with_non_type_template_parameter_pack();

template <auto Iterator>
void templated_function_with_placeholder_non_type_template_parameter();

template <typename T, template <typename> typename K>
void templated_function_with_template_template_parameter(K<T>);

template <typename T, int size, template <typename, int> typename Container = std::array>
void templated_function_with_defaulted_template_template_parameter(Container<T, size>);

template <typename T, template <typename> typename... Container>
void templated_function_with_template_template_parameter_pack(Container<T>...);
