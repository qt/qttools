// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

/*!
    \module TestModule
    \title Test Module
*/

/*!
    \class Widget
    \inmodule TestModule
    \brief Base widget class.
*/

/*!
    \variable Widget::color
    \brief The color of the widget.
*/

/*!
    \fn int Widget::getColor() const
    \brief Returns the widget color.
*/

/*!
    \fn void Widget::setColor(int c)
    \brief Sets the widget color to \a c.
*/

/*!
    \class Button
    \inmodule TestModule
    \brief A button widget.
    \inherits Widget

    The Button class inherits the \l color variable from Widget.
    This link should resolve to Widget::color, not to the "color"
    section in the style reference page.
*/

/*!
    \fn void Button::click()
    \brief Simulates a button click.
*/

