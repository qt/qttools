// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OLLAMA_H
#define OLLAMA_H

#include "translationprotocol.h"

QT_BEGIN_NAMESPACE

class QJsonObject;
class QJsonArray;

class Ollama : public TranslationProtocol
{
public:
    Ollama();
    ~Ollama() override;
    QList<Batch> makeBatches(const Messages &messages) const override;
    QByteArray payload(const Batch &b) const override;
    QHash<QString, QString> extractTranslations(const QByteArray &response) const override;
    QStringList extractModels(const QByteArray &data) const override;

    void setTranslationModel(const QString &modelName) override;
    void setUrl(const QString &url) override;
    QUrl translationEndpoint() const override;
    QUrl discoveryEndpoint() const override;

private:
    QString makePrompt(const Batch &b) const;
    QString makeSystemPrompt() const;

    std::unique_ptr<QJsonObject> m_payloadBase;
    std::unique_ptr<QJsonObject> m_systemMessage;
    QString m_url;
    static constexpr int s_maxBatchSize = 20;
};

QT_END_NAMESPACE

#endif // OLLAMA_H
