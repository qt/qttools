// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "template_sugar.h"

/*!
    \module TemplateSugarModule
    \brief Module for template sugar tests.
*/

/*!
    \class MyContainer
    \inmodule TemplateSugarModule
    \brief A container class for testing template parameter type matching.

    \a T is the element type stored in the container.

    MyContainer exercises the code path in findNodeForCursor where
    template parameter types must match between header parsing and
    \\fn re-parsing.
*/

/*!
    \fn template <typename T> void MyContainer<T>::append(const T &value)

    Appends \a value to the container.
*/

/*!
    \fn template <typename T> void MyContainer<T>::prepend(T value)

    Prepends \a value to the container.
*/

/*!
    \fn template <typename T> MyContainer<T> MyContainer<T>::operator+(const MyContainer<T> &other) const

    Returns a container that is the concatenation of this container
    and \a other.
*/

/*!
    \fn template <typename T> void MyContainer<T>::replace(int index, const T &value)

    Replaces the item at \a index with \a value.
*/

/*!
    \class AliasUser
    \inmodule TemplateSugarModule
    \brief A class that uses aliased template types as parameters.

    AliasUser exercises canonical fallback for type aliases around
    template types. The alias introduces an extra layer of type sugar
    that the original fallback gating would have handled, verifying
    that broadening the fallback doesn't regress alias resolution.
*/

/*!
    \fn void AliasUser::process(ContainerAlias<int> items)

    Processes the given \a items.
*/

/*!
    \fn void AliasUser::transform(const ContainerAlias<double> &source, ContainerAlias<double> &dest)

    Transforms elements from \a source into \a dest.
*/

/*!
    \class Connector
    \inmodule TemplateSugarModule
    \brief A class for testing Q_QDOC-style template parameter overloads.

    Connector exercises the code path in findNodeForCursor where
    two overloads differ only in template parameter names, such as
    \c Functor vs \c PointerToMemberFunction. Without the
    TemplateTypeParmType guard, canonical comparison collapses both
    names to \c type-parameter-0-0, causing one overload to shadow
    the other.
*/

/*!
    \fn template <typename Functor> void Connector::invoke(Functor callback)

    Invokes \a callback asynchronously.
*/

/*!
    \fn template <typename PointerToMemberFunction> void Connector::invoke(PointerToMemberFunction callback, int priority)

    Invokes \a callback with the given \a priority.
*/

/*!
    \class Dispatcher
    \inmodule TemplateSugarModule
    \brief A class for testing wrapped template parameter types.

    \a T is the dispatched element type.

    Dispatcher exercises canonical fallback for wrapped template
    parameter types such as \c {T *}, \c {const T &}, and
    \c {MyContainer<T>}. These verify that canonical comparison
    is symmetric: both sides canonicalize the same way, so the
    fallback produces correct matches even though the user-facing
    template parameter name is lost in the canonical form.
*/

/*!
    \fn template <typename T> void Dispatcher<T>::byPointer(T *value)

    Dispatches via a pointer to \a value.
*/

/*!
    \fn template <typename T> void Dispatcher<T>::byConstRef(const T &value)

    Dispatches via a const reference to \a value.
*/

/*!
    \fn template <typename T> void Dispatcher<T>::nested(MyContainer<T> items)

    Dispatches all elements in \a items.
*/
