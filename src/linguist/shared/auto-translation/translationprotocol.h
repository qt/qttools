// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TRANSLATIONPROTOCOL_H
#define TRANSLATIONPROTOCOL_H

#include <QList>

QT_BEGIN_NAMESPACE

class TranslatorMessage;
class QUrl;

struct Messages
{
    QList<const TranslatorMessage *> items;
    QString srcLang;
    QString tgtLang;
};

struct Item
{
    QString translation;
    const TranslatorMessage *msg;
};

class Batch
{
public:
    QString srcLang;
    QString tgtLang;
    QString context;
    QString userContext;
    QList<Item> items;
    std::shared_ptr<std::atomic_int> counter;
};

class TranslationProtocol
{
public:
    virtual QList<Batch> makeBatches(const Messages &messages,
                                     const QString &userContext = {}) const = 0;
    virtual QByteArray payload(const Batch &b) const = 0;
    virtual QHash<QString, QString> extractTranslations(const QByteArray &data) = 0;
    virtual QStringList extractModels(const QByteArray &data) const = 0;

    // Stages the model for translation, optionally returning a wake-up payload
    virtual std::optional<QByteArray> stageModel(const QString &modelName) = 0;

    virtual void setUrl(const QString &url) = 0;
    virtual QUrl translationEndpoint() const = 0;
    virtual QUrl discoveryEndpoint() const = 0;
    virtual ~TranslationProtocol() = default;
};

QT_END_NAMESPACE

#endif // TRANSLATIONPROTOCOL_H
