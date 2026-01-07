// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OPENAICOMPATIBLE_H
#define OPENAICOMPATIBLE_H

#include "translationprotocol.h"

QT_BEGIN_NAMESPACE

class QJsonObject;
class QJsonArray;

class OpenAICompatible : public TranslationProtocol
{
public:
    OpenAICompatible();
    ~OpenAICompatible() override;
    QList<Batch> makeBatches(const Messages &messages, const QString &userContext) const override;
    QByteArray payload(const Batch &b) const override;
    QHash<QString, QStringList> extractTranslations(const QByteArray &response,
                                                    bool plural) override;
    QStringList extractModels(const QByteArray &data) const override;

    std::optional<QByteArray> stageModel(const QString &modelName) override;
    void setUrl(const QString &url) override;
    QUrl translationEndpoint() const override;
    QUrl discoveryEndpoint() const override;
    void onRequestRejected() override;

private:
    // JSON format stages for fallback mechanism:
    // JsonObject: llama.cpp style {"type": "json_object"}
    // JsonSchema: LM Studio style {"type": "json_schema", "json_schema": {...}}
    // None: No response_format (for thinking models or unsupported servers)
    enum class JsonFormatStage { JsonObject, JsonSchema, None };

    QString makePrompt(const Batch &b) const;
    void decrementFormatCounter();

    std::unique_ptr<QJsonObject> m_payloadBase;
    QString m_url;
    JsonFormatStage m_formatStage = JsonFormatStage::JsonObject;
    std::atomic_int m_formatTryCounter;
    bool m_formatLocked = false;
};

QT_END_NAMESPACE

#endif // OPENAICOMPATIBLE_H
