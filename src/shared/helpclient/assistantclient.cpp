// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "assistantclient.h"

#include <QtCore/qstring.h>
#include <QtCore/qprocess.h>
#include <QtCore/qdir.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qobject.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

constexpr bool debugAssistantClient = false;

AssistantClient::AssistantClient() = default;

AssistantClient::~AssistantClient()
{
    if (isRunning()) {
        QString errorMessage;
        if (!sendCommand("quit"_L1, &errorMessage) || !m_process->waitForFinished(1000)) {
            m_process->terminate();
            m_process->waitForFinished();
        }
    }
    delete m_process;
}

bool AssistantClient::showPage(const QString &path, QString *errorMessage)
{
    const QString cmd = "SetSource "_L1 + path;
    return sendCommand(cmd, errorMessage);
}

bool AssistantClient::activateIdentifier(const QString &identifier, QString *errorMessage)
{
    const QString cmd = "ActivateIdentifier "_L1 + identifier;
    return sendCommand(cmd, errorMessage);
}

bool AssistantClient::sendCommand(const QString &cmd, QString *errorMessage)
{
    if constexpr (debugAssistantClient)
        qDebug() << "sendCommand " << cmd;
    if (!ensureRunning(errorMessage))
        return false;
    if (!m_process->isWritable() || m_process->bytesToWrite() > 0) {
        *errorMessage = QCoreApplication::translate("AssistantClient", "Unable to send request: Assistant is not responding.");
        return false;
    }
    QTextStream str(m_process);
    str << cmd << "\n\n";
    return true;
}

bool AssistantClient::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

QString AssistantClient::binary()
{
    QString app = QLibraryInfo::path(QLibraryInfo::BinariesPath) + QDir::separator();
#if !defined(Q_OS_MACOS)
    app += "assistant"_L1;
#else
    app += "Assistant.app/Contents/MacOS/Assistant"_L1;
#endif

#if defined(Q_OS_WIN)
    app += ".exe"_L1;
#endif

    return app;
}

void AssistantClient::readyReadStandardError()
{
     qWarning("%s: %s",
              qPrintable(QDir::toNativeSeparators(m_process->program())),
              m_process->readAllStandardError().constData());
}

void AssistantClient::processTerminated(int exitCode, QProcess::ExitStatus exitStatus)
{
    const QString binary = QDir::toNativeSeparators(m_process->program());
    if (exitStatus != QProcess::NormalExit)
        qWarning("%s: crashed.", qPrintable(binary));
    else if (exitCode != 0)
        qWarning("%s: terminated with exit code %d.", qPrintable(binary), exitCode);
}

bool AssistantClient::ensureRunning(QString *errorMessage)
{
    if (isRunning())
        return true;

    if (!m_process) {
        m_process = new QProcess;
        auto exitHandler = [this](int exitCode, QProcess::ExitStatus exitStatus) { this->processTerminated(exitCode, exitStatus); };
        QObject::connect(m_process, &QProcess::finished, m_process, exitHandler);
        auto readHandler = [this]() { this->readyReadStandardError(); };
        QObject::connect(m_process, &QProcess::readyReadStandardError, m_process, readHandler);
    }

    const QString app = binary();
    if (!QFileInfo(app).isFile()) {
        *errorMessage = QCoreApplication::translate("AssistantClient", "The binary '%1' does not exist.").arg(app);
        return false;
    }
    if constexpr (debugAssistantClient)
        qDebug() << "Running " << app;
    // run
    QStringList args{u"-enableRemoteControl"_s};

    // AXIVION DISABLE Style Qt-Security-QProcessStart: False positive, path is absolute
    m_process->start(app, args);
    // AXIVION ENABLE Style Qt-Security-QProcessStart

    if (!m_process->waitForStarted()) {
        *errorMessage = QCoreApplication::translate("AssistantClient", "Unable to launch assistant (%1).").arg(app);
        return false;
    }
    return true;
}

QString AssistantClient::documentUrl(const QString &module) const
{
    const int qtVersion = QT_VERSION;
    QString rc;
    QTextStream(&rc) << "qthelp://org.qt-project." << module << '.'
                     << (qtVersion >> 16) << ((qtVersion >> 8) & 0xFF) << (qtVersion & 0xFF)
                     << '/' << module << '/';
    return rc;
}

QT_END_NAMESPACE
