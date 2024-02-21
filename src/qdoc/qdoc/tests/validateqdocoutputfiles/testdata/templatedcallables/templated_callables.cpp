// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#pragma once

#include "templated_callables.h"

/*!
 * \headerfile <templated_callables.h>
 * \inmodule TestQDoc
 *
 * Containing headers for the tested free functions.
 */

/*!
 * \class TemplatedClass
 * \inmodule TestQDoc
 *
 * Containing record for the tested methods.
 */

/*!
 * \fn template<typename T> template <typename U> void TemplatedClass<T>::templated_method_with_type_template_parameter(U)
 *
 *  A templated method under a templated class with a non-defaulted type template parameter.
 */

/*!
 * \fn template<typename T> void templated_function_with_type_template_parameter(T)
 * \relates <templated_callables.h>
 *
 *  A templated function with a non-defaulted type template parameter.
 */

/*!
 * \fn template<typename T> template <typename U> void TemplatedClass<T>::templated_method_with_defaulted_type_template_parameter(U)
 *
 *  A templated method under a templated class with a defaulted type template parameter.
 */

/*!
 * \fn template<typename T> void templated_function_with_defaulted_type_template_parameter(T)
 * \relates <templated_callables.h>
 *
 *  A templated function with a defaulted type template parameter.
 */

/*!
 * \fn template<typename T> template <typename... Ts> void TemplatedClass<T>::templated_method_with_type_template_parameter_pack(Ts...)
 *
 *  A templated method under a templated class with a type template parameter pack.
 */

/*!
 * \fn template<typename... Ts> void templated_function_with_type_template_parameter_pack(Ts...)
 * \relates <templated_callables.h>
 *
 *  A templated function with a type template parameter pack.
 */

/*!
 * \fn template<typename T> template <int Size> void TemplatedClass<T>::templated_method_with_non_type_template_parameter()
 *
 *  A templated method under a templated class with a non-defaulted non type template parameter.
 */

/*!
 * \fn template<char Category> void templated_function_with_non_type_template_parameter()
 * \relates <templated_callables.h>
 *
 *  A templated function with a non-defaulted non type template parameter.
 */

/*!
 * \fn template<typename T> template <int Size = 10> void TemplatedClass<T>::templated_method_with_defaulted_non_type_template_parameter()
 *
 *  A templated method under a templated class with a defaulted non type template parameter.
 */

/*!
 * \fn template<char Category = 'A'> void templated_function_with_defaulted_non_type_template_parameter()
 * \relates <templated_callables.h>
 *
 *  A templated function with a defaulted non type template parameter.
 */

/*!
 * \fn template<typename T> template <int... Dimensions> void TemplatedClass<T>::templated_method_with_non_type_template_parameter_pack()
 *
 *  A templated method under a templated class with a non type template parameter pack.
 */

/*!
 * \fn template<unsigned... Weights> void templated_function_with_non_type_template_parameter_pack()
 * \relates <templated_callables.h>
 *
 *  A templated function with a non type template parameter pack.
 */

/*!
 * \fn template<typename T> template <auto Predicate> void TemplatedClass<T>::templated_method_with_placeholder_non_type_template_parameter()
 *
 *  A templated method under a templated class with a placeholder non type template parameter.
 */

/*!
 * \fn template<auto Iterator> void templated_function_with_placeholder_non_type_template_parameter()
 * \relates <templated_callables.h>
 *
 *  A templated function with a placeholder non type template parameter.
 */

/*!
 * \fn template<typename T> template <typename U, template <typename> typename X> void TemplatedClass<T>::templated_method_with_template_template_parameter(X<U>)
 *
 *  A templated method under a templated class with a non-defaulted template template parameter.
 */

/*!
 * \fn template <typename T, template <typename> typename K> void templated_function_with_template_template_parameter(K<T>)
 * \relates <templated_callables.h>
 *
 *  A templated function with a non-defaulted template template parameter.
 */

/*!
 * \fn template<typename T> template <typename U, int size, template <typename, int> typename Container = std::array> void TemplatedClass<T>::templated_method_with_defaulted_template_template_parameter(Container<U, size>)
 *
 *  A templated method under a templated class with a defaulted template template parameter.
 */

/*!
 * \fn template <typename T, int size, template <typename, int> typename Container = std::array> void templated_function_with_defaulted_template_template_parameter(Container<T, size>)
 * \relates <templated_callables.h>
 *
 *  A templated function with a defaulted template template parameter.
 */

/*!
 * \fn template<typename T> template <typename U, template <typename> typename... Container> void TemplatedClass<T>::templated_method_with_template_template_parameter_pack(Container<U>...)
 *
 *  A templated method under a templated class with a template template parameter pack.
 */

/*!
 * \fn template <typename T, template <typename> typename... Container> void templated_function_with_template_template_parameter_pack(Container<T>...)
 * \relates <templated_callables.h>
 *
 *  A templated function with a template template parameter pack.
 */
