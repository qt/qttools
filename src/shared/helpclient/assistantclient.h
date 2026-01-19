// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ASSISTANTCLIENT_H
#define ASSISTANTCLIENT_H

#include "helpclient.h"

#include <QtCore/qprocess.h>

QT_BEGIN_NAMESPACE

class QString;

class AssistantClient : public HelpClient
{
public:
    Q_DISABLE_COPY_MOVE(AssistantClient)

    AssistantClient();
    ~AssistantClient() override;

    bool showPage(const QString &path, QString *errorMessage) override;
    bool activateIdentifier(const QString &identifier, QString *errorMessage) override;
    QString documentUrl(const QString &module) const override;

    bool isRunning() const;

private:
    static QString binary();
    void readyReadStandardError();
    void processTerminated(int exitCode, QProcess::ExitStatus exitStatus);
    bool sendCommand(const QString &cmd, QString *errorMessage);
    bool ensureRunning(QString *errorMessage);

    QProcess *m_process = nullptr;
};

QT_END_NAMESPACE

#endif // ASSISTANTCLIENT_H
