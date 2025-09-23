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
    Contains multiple anonymous types to force Clang path-based naming.
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

    /*!
        \variable AxisAndAngle::rotationCache

        Cached rotation data with multiple anonymous types to trigger disambiguation.
    */
    mutable struct {
        bool isValid = false;
        union {
            float quaternion[4];
            struct {
                float w, x, y, z;
            } components;
        } rotationData;
        struct {
            AngleT lastAngle;
            bool isDirty = true;
        } metadata;
    } rotationCache;

    /*!
        \variable AxisAndAngle::transformationMatrix

        Complex nested anonymous types similar to Qt's real patterns.
    */
    union {
        float matrix[16];
        struct {
            struct { float m11, m12, m13, m14; } row1;
            struct { float m21, m22, m23, m24; } row2;
            struct { float m31, m32, m33, m34; } row3;
            struct { float m41, m42, m43, m44; } row4;
        } rows;
        struct {
            float elements[4][4];
        } array2d;
    } transformationMatrix;

    AngleT angle;
};

#endif // TEMPLATESTRUCT_H

