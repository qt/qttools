// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TRANSLATIONUTILS_H
#define TRANSLATIONUTILS_H

#include <QJsonArray>
#include <QString>

#include <optional>

QT_BEGIN_NAMESPACE

// Recursively searches for a JSON array with the given key in the JSON value.
std::optional<QJsonArray> findJsonArray(const QJsonValue &jval, const QString &key);

// Returns the system prompt used for translation requests.
QString translationSystemPrompt();

QT_END_NAMESPACE

#endif // TRANSLATIONUTILS_H
