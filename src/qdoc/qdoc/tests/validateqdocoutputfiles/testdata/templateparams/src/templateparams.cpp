// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "templateparams.h"

/*!
    \module TestModule
    \title Test Module
    \brief A test module for template parameter documentation.
*/

/*!
    \class FullyDocumented
    \inmodule TestModule
    \brief A template class with all parameters documented.

    \a T is the element type stored in the container.
    \a Allocator is the allocator type used for memory management.
*/

/*!
    \fn template <typename T, typename Allocator> void FullyDocumented<T, Allocator>::add(T value)

    Adds \a value to the container. The type \a T determines how the
    value is stored.
*/

/*!
    \class PartiallyDocumented
    \inmodule TestModule
    \brief A template class missing documentation for U.

    \a T is the first type parameter.
*/

/*!
    \fn template <typename T> void templateFunc(T value)
    \relates FullyDocumented

    A template function that takes \a value of type \a T.
*/

/*!
    \class Container
    \inmodule TestModule
    \brief A container with key-value pairs.

    \a Key is the type used for keys.
    \a Value is the type used for values.
*/

/*!
    \fn template <typename Key, typename Value> void Container<Key, Value>::insert(Key k, Value v)

    Inserts key \a k with value \a v into the container.
    The types \a Key and \a Value are inherited from the class template.
*/

/*!
    \fn template <typename Key, typename Value> template <typename Other> void Container<Key, Value>::merge(Other source)

    Merges \a source into this container. The \a Other type must be
    compatible with this container's \a Key and \a Value types.
*/

/*!
    \class WrongParamDocumented
    \inmodule TestModule
    \brief A template class with a typo in the documented parameter name.

    \a U is documented but the actual parameter is T.
*/
