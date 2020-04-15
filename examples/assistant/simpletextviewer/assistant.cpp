/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "assistant.h"

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QStandardPaths>

Assistant::Assistant() = default;

//! [0]
Assistant::~Assistant()
{
    if (!m_process.isNull() && m_process->state() == QProcess::Running) {
        QObject::disconnect(m_process.data(),
                            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                            nullptr, nullptr);
        m_process->terminate();
        m_process->waitForFinished(3000);
    }
}
//! [0]

//! [1]
void Assistant::showDocumentation(const QString &page)
{
    if (!startAssistant())
        return;

    QByteArray ba("SetSource ");
    ba.append("qthelp://org.qt-project.examples.simpletextviewer/doc/");

    m_process->write(ba + page.toLocal8Bit() + '\n');
}
//! [1]

QString documentationDirectory()
{
    QStringList paths;
#ifdef SRCDIR
    paths.append(QLatin1String(SRCDIR));
#endif
    paths.append(QLibraryInfo::location(QLibraryInfo::ExamplesPath));
    paths.append(QCoreApplication::applicationDirPath());
    paths.append(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation));
    for (const auto &dir : qAsConst(paths)) {
        const QString path = dir + QLatin1String("/documentation");
        if (QFileInfo::exists(path))
            return path;
    }
    return QString();
}

//! [2]
bool Assistant::startAssistant()
{
    if (m_process.isNull()) {
        m_process.reset(new QProcess());
        QObject::connect(m_process.data(),
                         QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         m_process.data(), [this](int exitCode, QProcess::ExitStatus status) {
                             this->finished(exitCode, status);
                         });
    }

    if (m_process->state() != QProcess::Running) {
        QString app = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator();
#ifndef Q_OS_DARWIN
        app += QLatin1String("assistant");
#else
        app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif

        const QString collectionDirectory = documentationDirectory();
        if (collectionDirectory.isEmpty()) {
            showError(tr("The documentation directory cannot be found"));
            return false;
        }

        QStringList args{QLatin1String("-collectionFile"),
                         collectionDirectory + QLatin1String("/simpletextviewer.qhc"),
                         QLatin1String("-enableRemoteControl")};

        m_process->start(app, args);

        if (!m_process->waitForStarted()) {
            showError(tr("Unable to launch Qt Assistant (%1): %2").arg(app, m_process->errorString()));
            return false;
        }
    }
    return true;
}
//! [2]

void Assistant::showError(const QString &message)
{
    QMessageBox::critical(QApplication::activeWindow(),
                          tr("Simple Text Viewer"), message);
}

void Assistant::finished(int exitCode, QProcess::ExitStatus status)
{
    const QString stdErr = QString::fromLocal8Bit(m_process->readAllStandardError());
    if (status != QProcess::NormalExit) {
        showError(tr("Assistant crashed: ").arg(stdErr));
    } else if (exitCode != 0) {
        showError(tr("Assistant exited with %1: %2").arg(exitCode).arg(stdErr));
    }
}
