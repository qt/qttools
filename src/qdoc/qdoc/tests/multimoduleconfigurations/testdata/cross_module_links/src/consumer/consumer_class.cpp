// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "consumer_class.h"

/*!
    \namespace ConsumerSpace
    \inmodule ConsumerModule
    \brief Namespace for consumer-defined types in the cross-module link fixture.
*/

/*!
    \class ConsumerSpace::ConsumerClass
    \inmodule UpstreamWidgets
    \ingroup upstream-controls
    \brief A consumer-defined class that joins an upstream-defined group.

    The consumer module parses \\ingroup upstream-controls, which creates
    a placeholder CollectionNode in the consumer's primary tree.
    QDocDatabase::mergeCollections then mirrors title and url from the
    authoritative version that lives in the upstream module's tree. The
    cross-module href for the "is part of" listing must point at the
    upstream module's published group page, not at a bare filename in
    the consumer's output directory.
*/

ConsumerSpace::ConsumerClass::ConsumerClass() = default;

void ConsumerSpace::ConsumerClass::operate()
{
}

/*!
    \fn template <typename T> void ConsumerSpace::ConsumerClass::doStuff(T value)

    Performs cross-module work on \a value, which must satisfy the
    UpstreamThing concept declared in the UpstreamWidgets dependency
    module. The rendered method synopsis autolinks UpstreamThing to
    its documentation in the upstream module via the dependency-index
    path, exercising cross-module concept autolinking.
*/
