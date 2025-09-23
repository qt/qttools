// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "machinetranslator.h"
#include "ollama.h"
#include "translatormessage.h"

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
    auto batches = m_translator->makeBatches(messages, userContext);
    for (auto &b : std::as_const(batches))
        translateBatch(b, 0);
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

void MachineTranslator::translateBatch(Batch b, int tries)
{
    if (m_stopped)
        return;
    const QByteArray body = m_translator->payload(b);
    QNetworkReply *reply = m_manager->post(*m_request, body);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, batch = std::move(b), session = m_session.load(), tries] {
                translationReceived(reply, std::move(batch), tries, session);
            });
}

void MachineTranslator::translationReceived(QNetworkReply *reply, Batch b, int tries, int session)
{
    reply->deleteLater();
    if (m_stopped || session != m_session.load())
        return;

    if (reply->error() != QNetworkReply::NoError) {
        if (tries < s_maxTries
            && (reply->error() == QNetworkReply::OperationCanceledError
                || reply->error() == QNetworkReply::TimeoutError
                || reply->error() == QNetworkReply::UnknownNetworkError)) {
            translateBatch(std::move(b), tries + 1);
            return;
        }

        QList<const TranslatorMessage *> failed;
        for (const auto &i : std::as_const(b.items))
            failed.append(i.msg);
        emit translationFailed(std::move(failed));
        return;
    }

    const QByteArray response = reply->readAll();
    QList<Item> items = std::move(b.items);
    QHash<const TranslatorMessage *, QString> out;
    const QHash<QString, QString> translations = m_translator->extractTranslations(response);
    for (Item &i : items) {
        if (i.msg->translation().isEmpty()) {
            if (QString translation = translations[i.msg->sourceText()]; !translation.isEmpty())
                out[i.msg] = std::move(translation);
            else
                b.items.append(std::move(i));
        }
    }

    if (out.empty() && tries == s_maxTries) {
        QList<const TranslatorMessage *> failed;
        for (const auto &i : std::as_const(b.items))
            failed.append(i.msg);
        emit translationFailed(std::move(failed));
    } else if (out.empty() && tries < s_maxTries) {
        translateBatch(std::move(b), tries + 1);
    } else {
        emit batchTranslated(std::move(out));
        if (!b.items.empty())
            translateBatch(std::move(b), tries);
    }
}

QT_END_NAMESPACE
