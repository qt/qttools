// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "remotecontrol.h"

#include <QDir>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QProcess>

using namespace Qt::StringLiterals;

RemoteControl::RemoteControl()
{
    ui.setupUi(this);
    connect(ui.launchButton, &QPushButton::clicked, this, &RemoteControl::onLaunchClicked);

    connect(ui.indexButton, &QPushButton::clicked, this, &RemoteControl::onIndexClicked);
    connect(ui.indexLineEdit, &QLineEdit::returnPressed, this, &RemoteControl::onIndexClicked);

    connect(ui.idButton, &QPushButton::clicked, this, &RemoteControl::onIdClicked);
    connect(ui.idLineEdit, &QLineEdit::returnPressed, this, &RemoteControl::onIdClicked);

    connect(ui.urlButton, &QPushButton::clicked, this, &RemoteControl::onUrlClicked);
    connect(ui.urlLineEdit, &QLineEdit::returnPressed, this, &RemoteControl::onUrlClicked);

    connect(ui.syncContentsButton, &QPushButton::clicked, this,
            [this] { sendCommand("SyncContents"_L1); });

    connect(ui.contentsCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        sendCommand(checked ? "Show Contents"_L1 : "Hide Contents"_L1);
    });
    connect(ui.indexCheckBox, &QCheckBox::toggled, this,
            [this](bool checked) { sendCommand(checked ? "Show Index"_L1 : "Hide Index"_L1); });
    connect(ui.bookmarksCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        sendCommand(checked ? "Show Bookmarks"_L1 : "Hide Bookmarks"_L1);
    });

    connect(ui.actionQuit, &QAction::triggered, this, &QMainWindow::close);

    const QString versionString = QString::number(QT_VERSION_MAJOR)
            + QString::number(QT_VERSION_MINOR) + QString::number(QT_VERSION_PATCH);
    ui.startUrlLineEdit->setText("qthelp://org.qt-project.qtdoc."_L1 + versionString
                                 + "/qdoc/qdoc-index.html"_L1);

    process = new QProcess(this);
    connect(process, &QProcess::finished, this, [this] {
        ui.launchButton->setEnabled(true);
        ui.startUrlLineEdit->setEnabled(true);
        ui.actionGroupBox->setEnabled(false);
    });
}

RemoteControl::~RemoteControl()
{
    if (process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished(3000);
    }
}

void RemoteControl::onLaunchClicked()
{
    if (process->state() == QProcess::Running)
        return;

    QString app = QLibraryInfo::path(QLibraryInfo::BinariesPath);
#if !defined(Q_OS_MAC)
    app += "/assistant"_L1;
#else
    app += "/Assistant.app/Contents/MacOS/Assistant"_L1;
#endif

    process->start(app, {"-enableRemoteControl"_L1});
    if (!process->waitForStarted()) {
        QMessageBox::critical(this, tr("Remote Control"),
                tr("Could not start Qt Assistant from %1.").arg(QDir::toNativeSeparators(app)));
        return;
    }

    ui.contentsCheckBox->setChecked(true);
    ui.indexCheckBox->setChecked(true);
    ui.bookmarksCheckBox->setChecked(true);

    if (!ui.startUrlLineEdit->text().isEmpty())
        sendCommand("SetSource "_L1 + ui.startUrlLineEdit->text());

    ui.launchButton->setEnabled(false);
    ui.startUrlLineEdit->setEnabled(false);
    ui.actionGroupBox->setEnabled(true);
}

void RemoteControl::onIndexClicked()
{
    sendCommand("ActivateKeyword "_L1 + ui.indexLineEdit->text());
}

void RemoteControl::onIdClicked()
{
    sendCommand("ActivateIdentifier "_L1 + ui.idLineEdit->text());
}

void RemoteControl::onUrlClicked()
{
    sendCommand("SetSource "_L1 + ui.urlLineEdit->text());
}

void RemoteControl::sendCommand(const QString &cmd)
{
    if (process->state() != QProcess::Running)
        return;
    process->write(cmd.toLocal8Bit() + '\n');
}
