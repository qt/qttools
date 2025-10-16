#ifndef MACHINETRANSLATOR_H
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#define MACHINETRANSLATOR_H

#include "translationprotocol.h"

#include <QString>
#include <QObject>
#include <QQueue>
#include <QMutex>

QT_BEGIN_NAMESPACE

class QNetworkRequest;
class QNetworkAccessManager;
class QNetworkReply;

class MachineTranslator : public QObject
{
    Q_OBJECT
public:
    MachineTranslator();
    ~MachineTranslator();

    void translate(const Messages &messages, const QString &userContext = QString());
    void stop() noexcept { m_stopped = true; }
    void start() noexcept;
    void setUrl(const QString &url);
    void activateTranslationModel(const QString &modelName);
    void requestModels();

signals:
    void batchTranslated(QHash<const TranslatorMessage *, QString> translations);
    void translationFailed(QList<const TranslatorMessage *>);
    void modelsReceived(QStringList models);

private:
    std::atomic_bool m_stopped = false;
    std::atomic_int m_session = 0;
    std::unique_ptr<QNetworkRequest> m_request;
    std::unique_ptr<QNetworkAccessManager> m_manager;
    std::unique_ptr<TranslationProtocol> m_translator;
    QQueue<Batch> m_pendingBatches;
    int m_inFlightCount = 0;
    QMutex m_queueMutex;

    void translateBatch(Batch b);
    void translationReceived(QNetworkReply *reply, Batch b, int session);
    void processNextBatches();

    // Allow up to 6 retries to accommodate JSON format fallback.
    // Gives 2-3 attempts with JSON format, then some attempts without.
    static constexpr int s_maxTries = 6;
    static constexpr int s_maxConcurrentBatches = 6;
};

QT_END_NAMESPACE

#endif // MACHINETRANSLATOR_H
