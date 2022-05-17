// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "remotecontrol.h"

#include <QDir>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>

RemoteControl::RemoteControl(QWidget *parent, Qt::WindowFlags flags)
        : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    connect(ui.indexLineEdit, SIGNAL(returnPressed()),
        this, SLOT(on_indexButton_clicked()));
    connect(ui.identifierLineEdit, SIGNAL(returnPressed()),
        this, SLOT(on_identifierButton_clicked()));
    connect(ui.urlLineEdit, SIGNAL(returnPressed()),
        this, SLOT(on_urlButton_clicked()));

    QString rc;
    QTextStream(&rc) << QLatin1String("qthelp://org.qt-project.qtdoc.")
                     << (QT_VERSION >> 16) << ((QT_VERSION >> 8) & 0xFF)
                     << (QT_VERSION & 0xFF)
                     << QLatin1String("/qtdoc/index.html");

    ui.startUrlLineEdit->setText(rc);

    process = new QProcess(this);
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(helpViewerClosed()));
}

RemoteControl::~RemoteControl()
{
    if (process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished(3000);
    }
}

void RemoteControl::on_actionQuit_triggered()
{
    close();
}

void RemoteControl::on_launchButton_clicked()
{
    if (process->state() == QProcess::Running)
        return;

    QString app = QLibraryInfo::path(QLibraryInfo::BinariesPath);
#if !defined(Q_OS_MAC)
    app += QLatin1String("/assistant");
#else
    app += QLatin1String("/Assistant.app/Contents/MacOS/Assistant");
#endif

    ui.contentsCheckBox->setChecked(true);
    ui.indexCheckBox->setChecked(true);
    ui.bookmarksCheckBox->setChecked(true);

    QStringList args;
    args << QLatin1String("-enableRemoteControl");
    process->start(app, args);
    if (!process->waitForStarted()) {
        QMessageBox::critical(
                this, tr("Remote Control"),
                tr("Could not start Qt Assistant from %1.").arg(QDir::toNativeSeparators(app)));
        return;
    }

    if (!ui.startUrlLineEdit->text().isEmpty())
        sendCommand(QLatin1String("SetSource ")
            + ui.startUrlLineEdit->text());

    ui.launchButton->setEnabled(false);
    ui.startUrlLineEdit->setEnabled(false);
    ui.actionGroupBox->setEnabled(true);
}

void RemoteControl::sendCommand(const QString &cmd)
{
    if (process->state() != QProcess::Running)
        return;
    process->write(cmd.toLocal8Bit() + '\n');
}

void RemoteControl::on_indexButton_clicked()
{
    sendCommand(QLatin1String("ActivateKeyword ")
        + ui.indexLineEdit->text());
}

void RemoteControl::on_identifierButton_clicked()
{
    sendCommand(QLatin1String("ActivateIdentifier ")
        + ui.identifierLineEdit->text());
}

void RemoteControl::on_urlButton_clicked()
{
    sendCommand(QLatin1String("SetSource ")
        + ui.urlLineEdit->text());
}

void RemoteControl::on_syncContentsButton_clicked()
{
    sendCommand(QLatin1String("SyncContents"));
}

void RemoteControl::on_contentsCheckBox_toggled(bool checked)
{
    sendCommand(checked ?
        QLatin1String("Show Contents") : QLatin1String("Hide Contents"));
}

void RemoteControl::on_indexCheckBox_toggled(bool checked)
{
    sendCommand(checked ?
        QLatin1String("Show Index") : QLatin1String("Hide Index"));
}

void RemoteControl::on_bookmarksCheckBox_toggled(bool checked)
{
    sendCommand(checked ?
        QLatin1String("Show Bookmarks") : QLatin1String("Hide Bookmarks"));
}

void RemoteControl::helpViewerClosed()
{
    ui.launchButton->setEnabled(true);
    ui.startUrlLineEdit->setEnabled(true);
    ui.actionGroupBox->setEnabled(false);
}
