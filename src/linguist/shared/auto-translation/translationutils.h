// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TRANSLATIONUTILS_H
#define TRANSLATIONUTILS_H

#include <QtCore/qhash.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

// Extracts key-value pairs from a JSON structure by finding an array with the given key.
// Recursively searches the JSON value for the array, then extracts all string key-value
// pairs from objects in that array. Handles both formats:
// 1. Array of single-key objects: [{"k1":"v1"},{"k2":"v2"}]
// 2. Array with multi-key object: [{"k1":"v1","k2":"v2"}]
QHash<QString, QString> extractKeyValuePairs(const QJsonValue &jval, const QString &arrayKey);

// Extracts key to string-list pairs from a JSON structure for plural translations.
// Expects format: [{"source1":["form1","form2"]},{"source2":["form1","form2"]}]
QHash<QString, QStringList> extractPluralTranslations(const QJsonValue &jval,
                                                      const QString &arrayKey);

// Returns the system prompt used for translation requests.
QString translationSystemPrompt();

// Returns the system prompt used for plural translation requests.
QString pluralTranslationSystemPrompt();

QT_END_NAMESPACE

#endif // TRANSLATIONUTILS_H
