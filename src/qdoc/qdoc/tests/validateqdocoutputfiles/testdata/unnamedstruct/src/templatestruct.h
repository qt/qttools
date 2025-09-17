// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEMPLATESTRUCT_H
#define TEMPLATESTRUCT_H

/*!
    \module UnnamedStructModule
    \title Unnamed Struct Module

    This module demonstrates the unnamed struct rendering issue.
*/

/*!
    \class AxisAndAngle
    \inmodule UnnamedStructModule

    Test struct similar to QQuaternion::AxisAndAngle to reproduce the bug.
*/
template <typename AngleT>
struct AxisAndAngle
{
    /*!
        \variable AxisAndAngle::axis

        The 3D axis that together with an angle corresponds to a quaternion.

        An unnamed struct containing members x, y, z.
    */
    struct {
        float x, y, z;
    } axis;

    AngleT angle;
};

#endif // TEMPLATESTRUCT_H

