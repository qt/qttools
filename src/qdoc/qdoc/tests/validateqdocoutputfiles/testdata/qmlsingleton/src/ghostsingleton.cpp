// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "ghostsingleton.h"

/*!
    \class GhostSingleton
    \brief A C++ class with QML_SINGLETON but no \qmltype documentation.

    This class should NOT generate any QML documentation pages or index entries.
    It exists purely to test that QML_SINGLETON detection doesn't create "ghost"
    QML documentation when only C++ documentation is present.
*/

GhostSingleton::GhostSingleton(QObject *parent)
    : QObject(parent), m_message("Ghost message from singleton")
{
}

QString GhostSingleton::getGhostMessage() const
{
    return m_message;
}
