/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
