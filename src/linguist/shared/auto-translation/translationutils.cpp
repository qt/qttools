// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translationutils.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

#include <optional>

using namespace Qt::Literals::StringLiterals;

static std::optional<QJsonArray> findJsonArray(const QJsonValue &jval, const QString &key)
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

QT_BEGIN_NAMESPACE

QHash<QString, QString> extractKeyValuePairs(const QJsonValue &jval, const QString &arrayKey)
{
    auto array = findJsonArray(jval, arrayKey);
    if (!array)
        return {};

    QHash<QString, QString> out;
    out.reserve(array->size());
    for (const QJsonValue &v : std::as_const(*array)) {
        if (v.isObject()) {
            const QJsonObject obj = v.toObject();
            for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
                if (it.value().isString())
                    out[it.key()] = it.value().toString();
            }
        }
    }
    return out;
}

QHash<QString, QStringList> extractPluralTranslations(const QJsonValue &jval,
                                                      const QString &arrayKey)
{
    auto array = findJsonArray(jval, arrayKey);
    if (!array)
        return {};

    QHash<QString, QStringList> out;
    out.reserve(array->size());
    for (const QJsonValue &v : std::as_const(*array)) {
        if (v.isObject()) {
            const QJsonObject obj = v.toObject();
            for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
                if (it.value().isArray()) {
                    QStringList forms;
                    const QJsonArray arr = it.value().toArray();
                    forms.reserve(arr.size());
                    for (const QJsonValue &form : arr) {
                        if (form.isString())
                            forms.append(form.toString());
                    }
                    if (!forms.isEmpty())
                        out[it.key()] = forms;
                }
            }
        }
    }
    return out;
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

QString pluralTranslationSystemPrompt()
{
    static QString systemPrompt = uR"PROMPT(
You are a professional software translator specialized in Qt UI strings with plural forms.

When given a list of items, each may include:
- source: the original text to translate (contains a placeholder like %n for the count)
- comment: an optional developer note for more context

The user will specify:
- The target language
- The number of plural forms required for that language

You must provide exactly the specified number of plural forms for each source string.
Each plural form corresponds to different grammatical number categories
(e.g., "one", "few", "many", "other" depending on the language).

Preserve keyboard accelerators (e.g. "&File"), placeholders (e.g. "%1", "%n"),
and ending punctuation in all plural forms.

RESULT FORMAT (MUST FOLLOW):
A single JSON object with one key, "Plurals",
whose value is an array of objects.
Each object maps the original source string to an array of translated plural forms:

Example for German (2 plural forms):

Input:
Target: German
Plural forms: 2
Items:
  - source: "%n file(s) selected"
  - source: "%n item(s) deleted"

Output:
{"Plurals":[{"%n file(s) selected":["%n Datei ausgewählt","%n Dateien ausgewählt"]},{"%n item(s) deleted":["%n Element gelöscht","%n Elemente gelöscht"]}]}

Example for Polish (3 plural forms):

Input:
Target: Polish
Plural forms: 3
Items:
  - source: "%n file(s)"

Output:
{"Plurals":[{"%n file(s)":["%n plik","%n pliki","%n plików"]}]}

Return **only** valid JSON, no code fences, no extra text.
After generating and before returning, verify:
1. Every string is in the target language.
2. Every JSON key exactly matches one of the input source strings.
3. Each source has exactly the specified number of plural forms.
4. The %n placeholder is preserved in all forms.
)PROMPT"_s;

    return systemPrompt;
}

QT_END_NAMESPACE
