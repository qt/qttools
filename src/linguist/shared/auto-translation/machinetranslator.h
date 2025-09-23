#ifndef MACHINETRANSLATOR_H
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#define MACHINETRANSLATOR_H

#include "translationprotocol.h"

#include <QString>
#include <QObject>

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
    void start() noexcept
    {
        m_session++;
        m_stopped = false;
    }
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
    void translateBatch(Batch b, int tries);
    void translationReceived(QNetworkReply *reply, Batch b, int tries, int session);

    static constexpr int s_maxTries = 3;
};

QT_END_NAMESPACE

#endif // MACHINETRANSLATOR_H
