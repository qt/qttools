// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "openaicompatible.h"
#include "translationsettings.h"
#include "translationutils.h"
#include "translatormessage.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

OpenAICompatible::OpenAICompatible()
    : m_payloadBase(std::make_unique<QJsonObject>()),
      m_formatTryCounter(TranslationSettings::maxJsonFormatTries())
{
    m_payloadBase->insert("stream"_L1, false);
    m_payloadBase->insert("temperature"_L1, TranslationSettings::temperature());
}

OpenAICompatible::~OpenAICompatible() = default;

QList<Batch> OpenAICompatible::makeBatches(const Messages &messages,
                                           const QString &userContext) const
{
    QHash<QString, QList<const TranslatorMessage *>> nonPluralGroups;
    QHash<QString, QList<const TranslatorMessage *>> pluralGroups;

    for (const auto &item : messages.items) {
        const QString key = item->context() + item->label();
        if (item->isPlural())
            pluralGroups[key].append(item);
        else
            nonPluralGroups[key].append(item);
    }

    const int maxBatchSize = TranslationSettings::maxBatchSize();
    QList<Batch> out;
    out.reserve(nonPluralGroups.size() + pluralGroups.size());

    auto createBatches = [&](const QHash<QString, QList<const TranslatorMessage *>> &groups,
                             int pluralFormsCount) {
        for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
            auto msgIt = it.value().cbegin();
            while (msgIt != it.value().cend()) {
                Batch b;
                b.srcLang = messages.srcLang;
                b.tgtLang = messages.tgtLang;
                b.context = it.key();
                b.userContext = userContext;
                b.pluralFormsCount = pluralFormsCount;
                b.items.reserve(it.value().size());
                while (msgIt != it.value().cend() && b.items.size() < maxBatchSize) {
                    Item item;
                    item.msg = *msgIt;
                    item.translation = item.msg->translation();
                    b.items.append(std::move(item));
                    msgIt++;
                }
                out.append(std::move(b));
            }
        }
    };

    createBatches(nonPluralGroups, 1);
    createBatches(pluralGroups, messages.pluralFormsCount);

    return out;
}

QHash<QString, QStringList> OpenAICompatible::extractTranslations(const QByteArray &response,
                                                                  bool plural)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(response, &err);
    if (err.error != QJsonParseError::NoError) {
        decrementFormatCounter();
        return {};
    }

    // OpenAI format: { "choices": [{ "message": { "content": "..." } }] }
    const QJsonObject root = doc.object();
    const QJsonArray choices = root.value("choices"_L1).toArray();
    if (choices.isEmpty()) {
        decrementFormatCounter();
        return {};
    }

    const QJsonObject firstChoice = choices.first().toObject();
    const QJsonObject message = firstChoice.value("message"_L1).toObject();
    const QString content = message.value("content"_L1).toString();

    // Parse the content as JSON to extract translations
    QJsonDocument contentDoc = QJsonDocument::fromJson(content.toUtf8(), &err);
    QJsonValue contentValue;
    if (err.error == QJsonParseError::NoError) {
        contentValue = contentDoc.object();
    } else {
        // Try to extract JSON from the content string
        contentValue = content;
    }

    QHash<QString, QStringList> translations;
    if (plural) {
        translations = extractPluralTranslations(contentValue, "Plurals"_L1);
    } else {
        auto singleTranslations = extractKeyValuePairs(contentValue, "Translations"_L1);
        for (auto it = singleTranslations.cbegin(); it != singleTranslations.cend(); ++it)
            translations[it.key()] << it.value();
    }

    if (translations.isEmpty()) {
        decrementFormatCounter();
        return translations;
    }

    // Lock in the current format stage once we get a successful response.
    // This prevents unnecessary fallback attempts due to occasional empty responses.
    m_formatLocked = true;

    return translations;
}

QStringList OpenAICompatible::extractModels(const QByteArray &response) const
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(response, &err);
    if (err.error != QJsonParseError::NoError)
        return {};

    // OpenAI format: { "data": [{ "id": "model-name", ... }] }
    const QJsonObject obj = doc.object();
    const QJsonArray arr = obj.value("data"_L1).toArray();
    QStringList models;
    for (const QJsonValue &v : arr)
        models.append(v.toObject().value("id"_L1).toString());
    return models;
}

QByteArray OpenAICompatible::payload(const Batch &b) const
{
    QJsonObject systemMessage;
    systemMessage.insert("role"_L1, "system"_L1);
    const bool plural = b.pluralFormsCount > 1;
    systemMessage.insert("content"_L1,
                         plural ? pluralTranslationSystemPrompt() : translationSystemPrompt());

    QJsonObject userMessage;
    userMessage.insert("role"_L1, "user"_L1);
    userMessage.insert("content"_L1, makePrompt(b));

    QJsonArray messages;
    messages.append(systemMessage);
    messages.append(userMessage);

    QJsonObject req = *m_payloadBase;
    req.insert("messages"_L1, messages);

    switch (m_formatStage) {
    case JsonFormatStage::JsonObject: {
        // llama.cpp style: {"type": "json_object"}
        QJsonObject responseFormat;
        responseFormat.insert("type"_L1, "json_object"_L1);
        req.insert("response_format"_L1, responseFormat);
        break;
    }
    case JsonFormatStage::JsonSchema: {
        // LM Studio style: {"type": "json_schema", "json_schema": {...}}
        QJsonObject schema;
        schema.insert("type"_L1, "object"_L1);
        QJsonObject properties;
        QJsonObject translationsArray;
        translationsArray.insert("type"_L1, "array"_L1);
        properties.insert("Translations"_L1, translationsArray);
        schema.insert("properties"_L1, properties);
        QJsonArray required;
        required.append("Translations"_L1);
        schema.insert("required"_L1, required);

        QJsonObject jsonSchema;
        jsonSchema.insert("name"_L1, "translations"_L1);
        jsonSchema.insert("schema"_L1, schema);

        QJsonObject responseFormat;
        responseFormat.insert("type"_L1, "json_schema"_L1);
        responseFormat.insert("json_schema"_L1, jsonSchema);
        req.insert("response_format"_L1, responseFormat);
        break;
    }
    case JsonFormatStage::None:
        // No response_format - rely on prompt instructions
        break;
    }

    return QJsonDocument(req).toJson();
}

std::optional<QByteArray> OpenAICompatible::stageModel(const QString &modelName)
{
    if (auto m = m_payloadBase->constFind("model"_L1);
        m == m_payloadBase->constEnd() || *m != modelName) {
        // Reset format fallback state for new model
        m_formatStage = JsonFormatStage::JsonObject;
        m_formatTryCounter = TranslationSettings::maxJsonFormatTries();
        m_formatLocked = false;
        m_payloadBase->insert("model"_L1, modelName);
    }

    // OpenAI-compatible servers typically don't need wake-up requests
    // as they keep models loaded or handle loading transparently
    return std::nullopt;
}

void OpenAICompatible::setUrl(const QString &url)
{
    m_url = url;
}

QUrl OpenAICompatible::translationEndpoint() const
{
    QString base = m_url;
    if (!base.endsWith(u'/'))
        base += u'/';
    return QUrl(base + "v1/chat/completions"_L1);
}

QUrl OpenAICompatible::discoveryEndpoint() const
{
    QString base = m_url;
    if (!base.endsWith(u'/'))
        base += u'/';
    return QUrl(base + "v1/models"_L1);
}

void OpenAICompatible::onRequestRejected()
{
    decrementFormatCounter();
}

void OpenAICompatible::decrementFormatCounter()
{
    if (m_formatLocked)
        return;

    if (--m_formatTryCounter <= 0) {
        // Move to next format stage
        const int maxTries = TranslationSettings::maxJsonFormatTries();
        switch (m_formatStage) {
        case JsonFormatStage::JsonObject:
            m_formatStage = JsonFormatStage::JsonSchema;
            m_formatTryCounter = maxTries;
            break;
        case JsonFormatStage::JsonSchema:
            m_formatStage = JsonFormatStage::None;
            m_formatTryCounter = maxTries;
            break;
        case JsonFormatStage::None:
            // Already at the last stage, nothing more to try
            break;
        }
    }
}

QString OpenAICompatible::makePrompt(const Batch &b) const
{
    QStringList lines;
    lines.reserve(b.items.size() + 32);

    if (!b.userContext.isEmpty())
        lines << "Application Context: "_L1 + b.userContext;

    lines << "Context: "_L1 + b.context;
    lines << "Target: "_L1 + b.tgtLang;
    if (b.pluralFormsCount > 1)
        lines << "Plural forms: "_L1 + QString::number(b.pluralFormsCount);
    lines << "Items:"_L1;
    for (const Item &it : b.items) {
        QString line = "- source: '%1'"_L1.arg(it.msg->sourceText());
        if (const QString comment = it.msg->comment(); !comment.isEmpty())
            line += ", comment: '%1'"_L1.arg(comment);
        lines << line;
    }

    return lines.join(QLatin1Char('\n'));
}

QT_END_NAMESPACE
