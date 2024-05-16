// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "cppcar.h"

/*!
    \qmlmodule QmlNativeTypeSynopsis
 */

/*!
    \qmltype Car
    \nativetype CppCar
    \inqmlmodule QmlNativeTypeSynopsis
    \ingroup qml_nativetype_synopsis
    \since 6.8
    \brief A car.

    \section1 Properties

    \list
    \li \c{color} The color of the car.
    \endlist

    \section1 Signals

    \list
    \li \c{engineStarted()} Emitted when the engine is started.
    \endlist

    \section1 Methods

    \list
    \li \c{startEngine()} Starts the engine.
    \endlist
 */

/*!
    \class CppCar
    \inmodule QmlNativeTypeSynopsis
    \brief A test class with an inventive name.
 */

/*!
    \qmltype Truck
    \nativetype CppCar
    \inqmlmodule QmlNativeTypeSynopsis
    \ingroup qml_nativetype_synopsis
    \since 6.8
    \brief A big car.

    \section1 Properties

    \list
    \li \c{color} The color of the big car.
    \endlist

    \section1 Signals

    \list
    \li \c{engineStarted()} Emitted when the big engine is started, along with more emissions than from a \l Car.
    \endlist

    \section1 Methods

    \list
    \li \c{startEngine()} Starts the big engine.
    \endlist
 */
