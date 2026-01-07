// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "ollama.h"
#include "translationsettings.h"
#include "translationutils.h"
#include "translatormessage.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

Ollama::Ollama()
    : m_payloadBase(std::make_unique<QJsonObject>()),
      m_useJsonFormat(TranslationSettings::maxJsonFormatTries())
{
    m_payloadBase->insert("stream"_L1, false);
    m_payloadBase->insert("think"_L1, false);

    QJsonObject opts;
    opts.insert("temperature"_L1, TranslationSettings::temperature());
    m_payloadBase->insert("options"_L1, opts);
}

Ollama::~Ollama() = default;

QList<Batch> Ollama::makeBatches(const Messages &messages, const QString &userContext) const
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

QHash<QString, QStringList> Ollama::extractTranslations(const QByteArray &response, bool plural)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(response, &err);
    if (err.error != QJsonParseError::NoError) {
        m_useJsonFormat--;
        return {};
    }

    QHash<QString, QStringList> translations;
    if (plural) {
        translations = extractPluralTranslations(doc.object(), "Plurals"_L1);
    } else {
        auto singleTranslations = extractKeyValuePairs(doc.object(), "Translations"_L1);
        for (auto it = singleTranslations.cbegin(); it != singleTranslations.cend(); ++it)
            translations[it.key()] << it.value();
    }

    if (translations.isEmpty()) {
        m_useJsonFormat--;
        return translations;
    }

    // If we get a successful response by using json format, the model
    // is a formatted model. So we want to prevent falling back to
    // non formatted model (harmony) if there are occasional empty
    // responses later.
    if (m_useJsonFormat > 0)
        m_useJsonFormat = std::numeric_limits<int>::max();

    return translations;
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

    if (m_useJsonFormat > 0)
        req.insert("format"_L1, "json"_L1);

    return QJsonDocument(req).toJson();
}

std::optional<QByteArray> Ollama::stageModel(const QString &modelName)
{
    if (auto m = m_payloadBase->constFind("model"_L1);
        m == m_payloadBase->constEnd() || *m != modelName) {
        m_useJsonFormat = TranslationSettings::maxJsonFormatTries();
        m_payloadBase->insert("model"_L1, modelName);
    }

    std::optional<QByteArray> res;
    if (!m_lastWakeupTimer.isValid()
        || m_lastWakeupTimer.hasExpired(TranslationSettings::ollamaWakeUpTimeoutMs())) {
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

void Ollama::onRequestRejected()
{
    m_useJsonFormat--;
}

QString Ollama::makePrompt(const Batch &b) const
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
