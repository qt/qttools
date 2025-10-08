// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OLLAMA_H
#define OLLAMA_H

#include "translationprotocol.h"

#include <QElapsedTimer>

QT_BEGIN_NAMESPACE

class QJsonObject;
class QJsonArray;

class Ollama : public TranslationProtocol
{
public:
    Ollama();
    ~Ollama() override;
    QList<Batch> makeBatches(const Messages &messages, const QString &userContext) const override;
    QByteArray payload(const Batch &b) const override;
    QHash<QString, QString> extractTranslations(const QByteArray &response) override;
    QStringList extractModels(const QByteArray &data) const override;

    std::optional<QByteArray> stageModel(const QString &modelName) override;
    void setUrl(const QString &url) override;
    QUrl translationEndpoint() const override;
    QUrl discoveryEndpoint() const override;

private:
    QString makePrompt(const Batch &b) const;
    QString makeSystemPrompt() const;

    std::unique_ptr<QJsonObject> m_payloadBase;
    std::unique_ptr<QJsonObject> m_systemMessage;
    QString m_url;
    std::atomic_int m_useJsonFormat = s_maxJsonFormatTry;
    QElapsedTimer m_lastWakeupTimer;
    static constexpr int s_maxJsonFormatTry = 3;
    static constexpr int s_maxBatchSize = 20;
    static constexpr int s_wakeUpTimeOut = 4 * 60 * 1000;
};

QT_END_NAMESPACE

#endif // OLLAMA_H
