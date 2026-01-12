// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LINKIR_H
#define LINKIR_H

#include <QJsonObject>
#include <QString>

QT_BEGIN_NAMESPACE

struct LinkIR
{
    QString target;         //! Pre-resolved URL or anchor
    QString text;           //! Display text for the link
    QString title;          //! Optional title/tooltip attribute

    enum class State : unsigned char {
        Resolved,
        External,
        Unresolved,
        Broken
    };

    State state{State::Resolved};

    QString originalTarget;

    [[nodiscard]] QJsonObject toJson() const;

    [[nodiscard]] bool isValid() const { return !target.isEmpty() || state == State::Unresolved; }
    [[nodiscard]] bool isResolved() const { return state == State::Resolved; }
    [[nodiscard]] bool isExternal() const { return state == State::External; }
};

QT_END_NAMESPACE

#endif // LINKIR_H

