// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ASSISTANT_H
#define ASSISTANT_H

#include <QCoreApplication>
#include <QProcess>
#include <QScopedPointer>
#include <QString>

class Assistant
{
    Q_DECLARE_TR_FUNCTIONS(Assistant)

public:
    ~Assistant();
    void showDocumentation(const QString &file);

private:
    bool startAssistant();
    void showError(const QString &message);
    void finished(int exitCode, QProcess::ExitStatus status);

    QScopedPointer<QProcess> m_process;
};

#endif
