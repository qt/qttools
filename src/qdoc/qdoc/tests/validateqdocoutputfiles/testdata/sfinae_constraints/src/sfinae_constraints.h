// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SFINAE_CONSTRAINTS_H
#define SFINAE_CONSTRAINTS_H

namespace std {
template <bool B, class T = void>
struct enable_if {};
template <class T>
struct enable_if<true, T> { using type = T; };
template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename T>
struct is_integral { static constexpr bool value = false; };
template <>
struct is_integral<int> { static constexpr bool value = true; };
template <typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;

template <typename T>
struct is_arithmetic { static constexpr bool value = false; };
template <>
struct is_arithmetic<int> { static constexpr bool value = true; };
template <>
struct is_arithmetic<double> { static constexpr bool value = true; };
template <typename T>
inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

template <typename T>
struct is_signed { static constexpr bool value = false; };
template <>
struct is_signed<int> { static constexpr bool value = true; };
template <typename T>
inline constexpr bool is_signed_v = is_signed<T>::value;
} // namespace std

template <typename T>
using if_integral = std::enable_if_t<std::is_integral_v<T>, bool>;

template <typename T>
using if_arithmetic = std::enable_if_t<std::is_arithmetic_v<T>, bool>;

template <typename T>
using if_signed = std::enable_if_t<std::is_signed_v<T>, bool>;

namespace QtPrivate {
template <typename T>
using if_scoped = std::enable_if_t<std::is_integral_v<T>, bool>;
} // namespace QtPrivate

// Multi-layer sugar: alias through an intermediate alias
template <typename T>
using if_numeric = if_arithmetic<T>;

namespace QDocTests {

class SfinaeContainer {
public:
    template <typename T, if_integral<T> = true>
    void process(const T &value);

    template <typename T, if_arithmetic<T> = true, if_signed<T> = true>
    void multiConstrained(const T &value);

    template <typename T, if_integral<T> = true>
        requires std::is_integral<T>::value
    void mixedConstraints(const T &value);

    template <typename T, QtPrivate::if_scoped<T> = true>
    void namespacedAlias(const T &value);

    template <typename T, if_numeric<T> = true>
    void multiLayerSugar(const T &value);
};

} // namespace QDocTests

#endif // SFINAE_CONSTRAINTS_H
