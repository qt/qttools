// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "ollama.h"
#include "translatormessage.h"

#include <QJsonObject>
#include <QJsonArray>

using namespace Qt::Literals::StringLiterals;

namespace {
static std::optional<QJsonArray> recursiveFind(const QJsonValue &jval, const QString &key)
{
    if (jval.isObject()) {
        const QJsonObject obj = jval.toObject();
        auto it = obj.find(key);
        if (it != obj.end() && it->isArray())
            return it->toArray();
        for (it = obj.constBegin(); it != obj.constEnd(); ++it) {
            if (it.key().trimmed() == key && it.value().isArray())
                return it.value().toArray();
            if (const auto r = recursiveFind(it.value(), key); r)
                return r;
        }
    } else if (jval.isArray()) {
        const QJsonArray arr = jval.toArray();
        for (const QJsonValue &element : arr)
            if (const auto r = recursiveFind(element, key); r)
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
} // namespace

QT_BEGIN_NAMESPACE

Ollama::Ollama()
    : m_payloadBase(std::make_unique<QJsonObject>()),
      m_systemMessage(std::make_unique<QJsonObject>())
{
    m_payloadBase->insert("stream"_L1, false);
    m_payloadBase->insert("think"_L1, false);

    QJsonObject opts;
    opts.insert("temperature"_L1, 0.05);
    m_payloadBase->insert("options"_L1, opts);

    m_systemMessage->insert("role"_L1, "system"_L1);
    m_systemMessage->insert("content"_L1, makeSystemPrompt());
}

Ollama::~Ollama() = default;

QList<Batch> Ollama::makeBatches(const Messages &messages, const QString &userContext) const
{
    QHash<QString, QList<const TranslatorMessage *>> groups;

    for (const auto &item : messages.items)
        groups[item->context() + item->label()].append(item);

    QList<Batch> out;
    out.reserve(groups.size());
    for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
        auto msgIt = it.value().cbegin();
        while (msgIt != it.value().cend()) {
            Batch b;
            b.srcLang = messages.srcLang;
            b.tgtLang = messages.tgtLang;
            b.context = it.key();
            b.userContext = userContext;
            b.items.reserve(it.value().size());
            while (msgIt != it.value().cend() && b.items.size() < s_maxBatchSize) {
                Item item;
                item.msg = *msgIt;
                item.translation = item.msg->translation();
                b.items.append(std::move(item));
                msgIt++;
            }
            out.append(std::move(b));
        }
    }
    return out;
}

QHash<QString, QString> Ollama::extractTranslations(const QByteArray &response)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(response, &err);
    if (err.error != QJsonParseError::NoError) {
        m_useJsonFormat--;
        return {};
    }

    auto translations = recursiveFind(doc.object(), "Translations"_L1);
    QHash<QString, QString> out;
    if (!translations) {
        m_useJsonFormat--;
        return out;
    }

    // If we get a successful response by using json format, the model
    // is a formatted model. So we want to prevent falling back to
    // non formatted model (harmony) if there are occasional empty
    // responses later.
    if (m_useJsonFormat > 0)
        m_useJsonFormat = std::numeric_limits<int>::max();

    out.reserve(translations->size());
    for (const QJsonValue &v : std::as_const(*translations)) {
        if (v.isObject()) {
            const QJsonObject obj = v.toObject();
            const QString key = obj.keys().first();
            if (QJsonValue val = obj.value(key); val.isString())
                out[key] = val.toString();
        }
    }
    return out;
}

QStringList Ollama::extractModels(const QByteArray &response) const
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(response, &err);
    if (err.error != QJsonParseError::NoError)
        return {};
    const QJsonObject obj = doc.object();
    const QJsonArray arr = obj.value("models"_L1).toArray();
    QStringList models;
    for (const QJsonValue &v : arr)
        models.append(v.toObject().value("name"_L1).toString());
    return models;
}

QByteArray Ollama::payload(const Batch &b) const
{
    QJsonObject userMessage;
    userMessage.insert("role"_L1, "user"_L1);
    userMessage.insert("content"_L1, makePrompt(b));

    QJsonArray messages;
    messages.append(*m_systemMessage);
    messages.append(userMessage);

    QJsonObject req = *m_payloadBase;
    req.insert("messages"_L1, messages);

    if (m_useJsonFormat > 0)
        req.insert("format"_L1, "json"_L1);

    return QJsonDocument(req).toJson();
}

std::optional<QByteArray> Ollama::stageModel(const QString &modelName)
{
    if (auto m = m_payloadBase->constFind("model"_L1);
        m == m_payloadBase->constEnd() || *m != modelName) {
        m_useJsonFormat = s_maxJsonFormatTry;
        m_payloadBase->insert("model"_L1, modelName);
    }

    std::optional<QByteArray> res;
    if (!m_lastWakeupTimer.isValid() || m_lastWakeupTimer.hasExpired(s_wakeUpTimeOut)) {
        m_lastWakeupTimer.start();
        QJsonObject wakeup;
        wakeup.insert("model"_L1, modelName);
        res.emplace(QJsonDocument(wakeup).toJson());
    }

    return res;
}

void Ollama::setUrl(const QString &url)
{
    m_url = url;
}

QUrl Ollama::translationEndpoint() const
{
    return QUrl(m_url).resolved(QUrl("/api/chat"_L1));
}

QUrl Ollama::discoveryEndpoint() const
{
    return QUrl(m_url).resolved(QUrl("/api/tags"_L1));
}

QString Ollama::makePrompt(const Batch &b) const
{
    QStringList lines;
    lines.reserve(b.items.size() + 32);

    if (!b.userContext.isEmpty())
        lines << "Application Context: "_L1 + b.userContext;

    lines << "Context: "_L1 + b.context;
    lines << "Target: "_L1 + b.tgtLang;
    lines << "Items:"_L1;
    for (const Item &it : b.items) {
        QString line = "- source: '%1'"_L1.arg(it.msg->sourceText());
        if (const QString comment = it.msg->comment(); !comment.isEmpty())
            line += ", comment: '%1'"_L1.arg(comment);
        lines << line;
    }

    return lines.join(QLatin1Char('\n'));
}

QString Ollama::makeSystemPrompt() const
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
