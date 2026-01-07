// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OLLAMA_H
#define OLLAMA_H

#include "translationprotocol.h"

#include <QtCore/qelapsedtimer.h>

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
    QHash<QString, QStringList> extractTranslations(const QByteArray &response,
                                                    bool plural) override;
    QStringList extractModels(const QByteArray &data) const override;

    std::optional<QByteArray> stageModel(const QString &modelName) override;
    void setUrl(const QString &url) override;
    QUrl translationEndpoint() const override;
    QUrl discoveryEndpoint() const override;
    void onRequestRejected() override;

private:
    QString makePrompt(const Batch &b) const;

    std::unique_ptr<QJsonObject> m_payloadBase;
    QString m_url;
    std::atomic_int m_useJsonFormat;
    QElapsedTimer m_lastWakeupTimer;
};

QT_END_NAMESPACE

#endif // OLLAMA_H
