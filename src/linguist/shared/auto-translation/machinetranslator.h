#ifndef MACHINETRANSLATOR_H
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#define MACHINETRANSLATOR_H

#include "translationprotocol.h"

#include <QtCore/qmutex.h>
#include <QtCore/qobject.h>
#include <QtCore/qqueue.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QNetworkRequest;
class QNetworkAccessManager;
class QNetworkReply;

enum class TranslationApiType { Ollama, OpenAICompatible };

class MachineTranslator : public QObject
{
    Q_OBJECT
public:
    MachineTranslator();
    ~MachineTranslator();

    void setApiType(TranslationApiType type);

    void translate(const Messages &messages, const QString &userContext = QString());
    void stop() noexcept { m_stopped = true; }
    void start() noexcept;
    void setUrl(const QString &url);
    void setApiKey(const QString &apiKey);
    void activateTranslationModel(const QString &modelName);
    void requestModels();

signals:
    void batchTranslated(QHash<const TranslatorMessage *, QStringList> translations);
    void translationFailed(QList<const TranslatorMessage *>);
    void modelsReceived(QStringList models);
    void debugLog(const QByteArray &message, bool fromLlm);

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
};

QT_END_NAMESPACE

#endif // MACHINETRANSLATOR_H
