// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "optiondialog.h"
#include "ui_mainwindow.h"

#include <QCoapClient>
#include <QCoapResourceDiscoveryReply>
#include <QCoapReply>
#include <QDateTime>
#include <QFileDialog>
#include <QHostInfo>
#include <QMessageBox>
#include <QMetaEnum>
#include <QNetworkInterface>

using namespace Qt::StringLiterals;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    m_client = new QCoapClient(QtCoap::SecurityMode::NoSecurity, this);
    connect(m_client, &QCoapClient::finished, this, &MainWindow::onFinished);
    connect(m_client, &QCoapClient::error, this, &MainWindow::onError);

    ui->setupUi(this);

    ui->methodComboBox->addItem(tr("Get"), QVariant::fromValue(QtCoap::Method::Get));
    ui->methodComboBox->addItem(tr("Put"), QVariant::fromValue(QtCoap::Method::Put));
    ui->methodComboBox->addItem(tr("Post"), QVariant::fromValue(QtCoap::Method::Post));
    ui->methodComboBox->addItem(tr("Delete"), QVariant::fromValue(QtCoap::Method::Delete));

    fillHostSelector();
    ui->hostComboBox->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fillHostSelector()
{
    const auto networkInterfaces = QNetworkInterface::allInterfaces();
    for (const auto &interface : networkInterfaces)
        for (const auto &address : interface.addressEntries())
            ui->hostComboBox->addItem(address.ip().toString());
}

void MainWindow::addMessage(const QString &message, bool isError)
{
    const QString content = "--------------- %1 ---------------\n%2\n\n"_L1
                                .arg(QDateTime::currentDateTime().toString(), message);
    ui->textEdit->setTextColor(isError ? Qt::red : Qt::black);
    ui->textEdit->insertPlainText(content);
    ui->textEdit->ensureCursorVisible();
}

void MainWindow::onFinished(QCoapReply *reply)
{
    if (reply->errorReceived() == QtCoap::Error::Ok)
        addMessage(reply->message().payload());
}

static QString errorMessage(QtCoap::Error errorCode)
{
    const auto error = QMetaEnum::fromType<QtCoap::Error>().valueToKey(static_cast<int>(errorCode));
    return MainWindow::tr("Request failed with error: %1\n").arg(error);
}

void MainWindow::onError(QCoapReply *reply, QtCoap::Error error)
{
    const auto errorCode = reply ? reply->errorReceived() : error;
    addMessage(errorMessage(errorCode), true);
}

void MainWindow::onDiscovered(QCoapResourceDiscoveryReply *reply, QList<QCoapResource> resources)
{
    if (reply->errorReceived() != QtCoap::Error::Ok)
        return;

    QString message;
    for (const auto &resource : std::as_const(resources)) {
        ui->resourceComboBox->addItem(resource.path());
        message += tr("Discovered resource: \"%1\" on path %2\n")
                        .arg(resource.title(), resource.path());
    }
    addMessage(message);
}

void MainWindow::onNotified(QCoapReply *reply, const QCoapMessage &message)
{
    if (reply->errorReceived() == QtCoap::Error::Ok) {
        addMessage(tr("Received observe notification with payload: %1")
                        .arg(QString::fromUtf8(message.payload())));
    }
}


static QString tryToResolveHostName(const QString hostName)
{
    const auto hostInfo = QHostInfo::fromName(hostName);
    if (!hostInfo.addresses().empty())
        return hostInfo.addresses().first().toString();

    return hostName;
}

void MainWindow::on_runButton_clicked()
{
    const auto msgType = ui->msgTypeCheckBox->isChecked() ? QCoapMessage::Type::Confirmable
                                                          : QCoapMessage::Type::NonConfirmable;
    QUrl url;
    url.setHost(tryToResolveHostName(ui->hostComboBox->currentText()));
    url.setPort(ui->portSpinBox->value());
    url.setPath(ui->resourceComboBox->currentText());

    QCoapRequest request(url, msgType);
    for (const auto &option : std::as_const(m_options))
        request.addOption(option);
    m_options.clear();

    const auto method = ui->methodComboBox->currentData(Qt::UserRole).value<QtCoap::Method>();
    switch (method) {
    case QtCoap::Method::Get:
        m_client->get(request);
        break;
    case QtCoap::Method::Put:
        m_client->put(request, m_currentData);
        break;
    case QtCoap::Method::Post:
        m_client->post(request, m_currentData);
        break;
    case QtCoap::Method::Delete:
        m_client->deleteResource(request);
        break;
    default:
        break;
    }
    m_currentData.clear();
}

void MainWindow::on_discoverButton_clicked()
{
    QUrl url;
    url.setHost(tryToResolveHostName(ui->hostComboBox->currentText()));
    url.setPort(ui->portSpinBox->value());

    QCoapResourceDiscoveryReply *discoverReply =
            m_client->discover(url, ui->discoveryPathEdit->text());
    if (discoverReply) {
        connect(discoverReply, &QCoapResourceDiscoveryReply::discovered,
                this, &MainWindow::onDiscovered);
    } else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Something went wrong, discovery request failed."));
    }
}

void MainWindow::on_observeButton_clicked()
{
    QUrl url;
    url.setHost(tryToResolveHostName(ui->hostComboBox->currentText()));
    url.setPort(ui->portSpinBox->value());
    url.setPath(ui->resourceComboBox->currentText());

    QCoapReply *observeReply = m_client->observe(url);
    if (!observeReply) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Something went wrong, observe request failed."));
        return;
    }

    connect(observeReply, &QCoapReply::notified, this, &MainWindow::onNotified);

    ui->cancelObserveButton->setEnabled(true);
    connect(ui->cancelObserveButton, &QPushButton::clicked, this, [this, url]() {
        m_client->cancelObserve(url);
        ui->cancelObserveButton->setEnabled(false);
    });
}

void MainWindow::on_addOptionsButton_clicked()
{
    OptionDialog dialog(m_options);
    if (dialog.exec() == QDialog::Accepted)
        m_options = dialog.options();
}

void MainWindow::on_contentButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (!dialog.exec())
        return;

    const auto fileName = dialog.selectedFiles().back();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to read from file %1").arg(fileName));
        return;
    }

    m_currentData = file.readAll();
}

void MainWindow::on_resourceComboBox_editTextChanged(const QString &text)
{
    ui->observeButton->setEnabled(!text.isEmpty());
}

void MainWindow::on_methodComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);

    const auto method = ui->methodComboBox->currentData(Qt::UserRole).value<QtCoap::Method>();
    ui->contentButton->setEnabled(method == QtCoap::Method::Put || method == QtCoap::Method::Post);
}
