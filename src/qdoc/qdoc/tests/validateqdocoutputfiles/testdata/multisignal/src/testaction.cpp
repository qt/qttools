// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testaction.h"

/*!
    \module TestModule
    \brief A test module.
*/

/*!
    \class TestAction
    \inmodule TestModule
    \brief A test class to demonstrate improved multi-property signal documentation.

    This class has properties with different notification patterns.
*/

/*!
    \fn TestAction::TestAction(QObject *parent)

    Constructs a TestAction with the given \a parent.
*/
TestAction::TestAction(QObject *parent)
    : QObject(parent)
{
}

/*!
    \property TestAction::enabled
    \brief whether the action is enabled

    This property determines if the action can be triggered.
*/
bool TestAction::isEnabled() const
{
    return m_enabled;
}

void TestAction::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit changed();
    }
}

/*!
    \property TestAction::visible
    \brief whether the action is visible

    This property determines if the action is shown in UI.
*/
bool TestAction::isVisible() const
{
    return m_visible;
}

void TestAction::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        emit changed();
    }
}

/*!
    \property TestAction::text
    \brief the action's display text

    This property holds the text that is displayed for the action.
*/
QString TestAction::text() const
{
    return m_text;
}

void TestAction::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        emit changed();
    }
}

/*!
    \property TestAction::checkable
    \brief whether the action is checkable

    This property determines if the action can be toggled.
*/
bool TestAction::isCheckable() const
{
    return m_checkable;
}

void TestAction::setCheckable(bool checkable)
{
    if (m_checkable != checkable) {
        m_checkable = checkable;
        emit singlePropertySignal();
    }
}

/*!
    \fn void TestAction::changed()

    This signal is emitted when certain properties of the action change.
    The properties that trigger this signal are enabled, visible, and text.
*/

/*!
    \fn void TestAction::singlePropertySignal()

    This signal is emitted when the checkable property changes.
*/

