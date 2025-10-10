// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "machinetranslator.h"
#include "ollama.h"
#include "translatormessage.h"
#include "simtexth.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

MachineTranslator::MachineTranslator()
    : m_request(std::make_unique<QNetworkRequest>()),
      m_manager(std::make_unique<QNetworkAccessManager>()),
      m_translator(std::make_unique<Ollama>())
{
    m_request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json"_L1);
    m_request->setTransferTimeout(5 * 60 * 1000);
}

MachineTranslator::~MachineTranslator() = default;

void MachineTranslator::translate(const Messages &messages, const QString &userContext)
{
    QMutexLocker locker(&m_queueMutex);

    m_pendingBatches.clear();
    auto batches = m_translator->makeBatches(messages, userContext);

    for (auto &b : batches)
        m_pendingBatches.enqueue(std::move(b));

    processNextBatches();
}

void MachineTranslator::setUrl(const QString &url)
{
    m_translator->setUrl(url);
    m_request->setUrl(m_translator->translationEndpoint());
}

void MachineTranslator::activateTranslationModel(const QString &modelName)
{
    if (auto wakeupPayload = m_translator->stageModel(modelName)) {
        // after several minutes of being idle, Ollama offloads the model
        // and the rest API needs to be waken up. Trying to connect with
        // Ollama for the first time after several minutes wakes up the
        // server, but for some reason the server doesn't queue the
        // connection request that was sent to it while it was not awake. As a result,
        // the connection in QNetworkAccessManager is half broken and not working,
        // without us knowing about it.
        // Here we are using a new QNetworkAccessManager instance to send
        // the first connection request and wake up the model. Then we delete the
        // QNetworkAccessManager instance since it contains a broken connection
        // and will try to use it for the next requests otherwise.

        auto *tempManager = new QNetworkAccessManager(this);

        QNetworkRequest wakeupRequest(m_translator->translationEndpoint());
        wakeupRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"_L1);
        wakeupRequest.setTransferTimeout(30000);

        QNetworkReply *reply = tempManager->post(wakeupRequest, *wakeupPayload);

        connect(reply, &QNetworkReply::finished, this, [tempManager, reply]() {
            reply->deleteLater();
            tempManager->deleteLater();
        });
    }
}

void MachineTranslator::requestModels()
{
    QNetworkRequest req(m_translator->discoveryEndpoint());
    QNetworkReply *reply = m_manager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        QStringList models;
        if (reply->error() == QNetworkReply::NoError) {
            const QByteArray response = reply->readAll();
            models = m_translator->extractModels(response);
        }
        emit modelsReceived(std::move(models));
    });
}

void MachineTranslator::translateBatch(Batch b)
{
    Q_ASSERT_X(!m_queueMutex.tryLock(), Q_FUNC_INFO,
               "The function requires m_queueMutex to be held.");
    if (m_stopped)
        return;
    m_inFlightCount++;
    const QByteArray body = m_translator->payload(b);
    QNetworkReply *reply = m_manager->post(*m_request, body);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, batch = std::move(b), session = m_session.load()] {
                translationReceived(reply, std::move(batch), session);
            });
}

void MachineTranslator::processNextBatches()
{
    Q_ASSERT_X(!m_queueMutex.tryLock(), Q_FUNC_INFO,
               "The function requires m_queueMutex to be held.");
    if (m_stopped || m_pendingBatches.isEmpty())
        return;

    const int batchesToSchedule =
            qMin(s_maxConcurrentBatches - m_inFlightCount, m_pendingBatches.size());
    for (int i = 0; i < batchesToSchedule; ++i) {
        Batch batch = m_pendingBatches.dequeue();
        translateBatch(std::move(batch));
    }
}

void MachineTranslator::translationReceived(QNetworkReply *reply, Batch b, int session)
{
    reply->deleteLater();

    if (m_stopped || session != m_session) {
        QMutexLocker locker(&m_queueMutex);
        m_inFlightCount--;
        m_pendingBatches.clear();
        return;
    }

    bool shouldRetry = false;

    if (reply->error() != QNetworkReply::NoError) {
        const bool isRetriableError = reply->error() == QNetworkReply::OperationCanceledError
                || reply->error() == QNetworkReply::TimeoutError
                || reply->error() == QNetworkReply::UnknownNetworkError;
        shouldRetry = b.tries < s_maxTries && isRetriableError;
        if (!shouldRetry) {
            QList<const TranslatorMessage *> failed;
            for (const auto &i : std::as_const(b.items))
                failed.append(i.msg);
            emit translationFailed(std::move(failed));
        }
    } else {
        const QByteArray response = reply->readAll();
        QList<Item> items = std::move(b.items);
        QHash<const TranslatorMessage *, QString> out;
        QHash<QString, QString> translations = m_translator->extractTranslations(response);

        // First pass: exact matches
        QList<Item> nonMatched;
        for (Item &i : items) {
            if (i.msg->translation().isEmpty()) {
                if (auto translation = translations.find(i.msg->sourceText());
                    translation != translations.end()) {
                    out[i.msg] = *translation;
                    translations.erase(translation);
                } else {
                    nonMatched.append(std::move(i));
                }
            }
        }

        // Second pass: fuzzy matching for non-matched items with unused translations
        constexpr int similarityThreshold = 200;
        for (Item &i : nonMatched) {
            StringSimilarityMatcher matcher(i.msg->sourceText());
            QString bestMatch;
            int bestScore = 0;
            for (auto it = translations.cbegin(); it != translations.cend(); ++it) {
                const int score = matcher.getSimilarityScore(it.key());
                if (score >= similarityThreshold && score > bestScore) {
                    bestScore = score;
                    bestMatch = it.key();
                }
            }

            if (!bestMatch.isEmpty())
                out[i.msg] = translations.take(bestMatch);
            else
                b.items.append(std::move(i));
        }

        const bool nonTranslatedItems = !b.items.empty();
        shouldRetry = nonTranslatedItems && b.tries < s_maxTries;
        if (nonTranslatedItems && !shouldRetry) {
            QList<const TranslatorMessage *> failed;
            for (const auto &i : std::as_const(b.items))
                failed.append(i.msg);
            emit translationFailed(std::move(failed));
        }
        if (!out.empty())
            emit batchTranslated(std::move(out));
    }

    QMutexLocker locker(&m_queueMutex);
    m_inFlightCount--;
    if (shouldRetry) {
        b.tries++;
        m_pendingBatches.prepend(std::move(b)); // Front of queue for priority
    }
    processNextBatches();
}

QT_END_NAMESPACE
