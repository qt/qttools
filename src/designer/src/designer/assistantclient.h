// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ASSISTANTCLIENT_H
#define ASSISTANTCLIENT_H

#include <QtCore/qprocess.h>

QT_BEGIN_NAMESPACE

class QString;

class AssistantClient : public QObject
{
    Q_OBJECT

public:
    AssistantClient();
    ~AssistantClient();

    bool showPage(const QString &path, QString *errorMessage);
    bool activateIdentifier(const QString &identifier, QString *errorMessage);
    bool activateKeyword(const QString &keyword, QString *errorMessage);

    bool isRunning() const;

    static QString documentUrl(const QString &prefix, int qtVersion = 0);
    // Root of the Qt Designer documentation
    static QString designerManualUrl(int qtVersion = 0);
    // Root of the Qt Reference documentation
    static QString qtReferenceManualUrl(int qtVersion = 0);

private slots:
    void readyReadStandardError();
    void processTerminated(int exitCode, QProcess::ExitStatus exitStatus);

private:
    static QString binary();
    bool sendCommand(const QString &cmd, QString *errorMessage);
    bool ensureRunning(QString *errorMessage);

    QProcess *m_process = nullptr;
};

QT_END_NAMESPACE

#endif // ASSISTANTCLIENT_H
