// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translationutils.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

std::optional<QJsonArray> findJsonArray(const QJsonValue &jval, const QString &key)
{
    if (jval.isObject()) {
        const QJsonObject obj = jval.toObject();
        auto it = obj.find(key);
        if (it != obj.end() && it->isArray())
            return it->toArray();
        for (it = obj.constBegin(); it != obj.constEnd(); ++it) {
            if (it.key().trimmed() == key && it.value().isArray())
                return it.value().toArray();
            if (const auto r = findJsonArray(it.value(), key); r)
                return r;
        }
    } else if (jval.isArray()) {
        const QJsonArray arr = jval.toArray();
        for (const QJsonValue &element : arr)
            if (const auto r = findJsonArray(element, key); r)
                return r;
    } else if (jval.isString()) {
        QString str = jval.toString();
        const int startIdx = str.indexOf('{'_L1);
        const int endIdx = str.lastIndexOf('}'_L1);
        if (startIdx < 0 || endIdx < 0)
            return {};
        str.slice(startIdx, endIdx - startIdx + 1);
        QJsonParseError err;
        auto inner = QJsonDocument::fromJson(str.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError || !inner.isObject())
            return {};
        const auto obj = inner.object();
        if (auto it = obj.find(key); it != obj.end()) {
            if (it.value().isArray())
                return it.value().toArray();
        }
    }
    return {};
}

QString translationSystemPrompt()
{
    static QString systemPrompt = uR"(
You are a professional software translator specialized in Qt UI strings.

When given a list of items of the given 'Context', each may include:
- source: the original text to translate
- comment: an optional developer note for more context

If "Application Context" is provided, use it to understand the domain and terminology
appropriate for the application (e.g., medical, financial, gaming) to produce more
accurate and contextually appropriate translations.

Translate the items into the **target language** specified by the user,
preserving keyboard accelerators (e.g. "&File"), placeholders (e.g. "%1"),
and ending punctuation.

RESULT FORMAT (MUST FOLLOW):
A single JSON object with one key, "Translations",
whose value is an array of objects.
Each object maps the original source string to translated string:

Two examples:

Input:
Context: MainWindow
Target: German
Items:
  - source: "File"
  - source: "Exit"
  - source: "&Open", comment: "opens a document"

Output:
{"Translations":[{"File":"Datei"},{"Exit":"Beenden"},{"&Open":"&Öffnen"}]}

Input:
Context: MainWindow
Target: French
Items:
– source: "File"
– source: "Exit"
Output:
{"Translations":[{"File":"Fichier"},{"Exit":"Quitter"}]}

Return **only** valid JSON, no code fences, no extra text.
After generating and before returning, verify:
1. Every string is in the target language; if any aren't, correct them before returning.
2. Every JSON key exactly matches one of the input source strings.
3. No key equals its value.
4. Every string is translated
)"_s;

    return systemPrompt;
}

QT_END_NAMESPACE
