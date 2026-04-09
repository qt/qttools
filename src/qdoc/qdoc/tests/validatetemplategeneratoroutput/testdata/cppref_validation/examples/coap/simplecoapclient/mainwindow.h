// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCoap/qcoapnamespace.h>
#include <QCoapOption>
#include <QCoapResource>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QCoapClient;
class QCoapResourceDiscoveryReply;
class QCoapMessage;
class QCoapReply;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void fillHostSelector();
    void addMessage(const QString &message, bool isError = false);

private slots:
    void onFinished(QCoapReply *reply);
    void onError(QCoapReply *reply, QtCoap::Error error);
    void onDiscovered(QCoapResourceDiscoveryReply *reply, QList<QCoapResource> resources);
    void onNotified(QCoapReply *reply, const QCoapMessage &message);

    void on_runButton_clicked();
    void on_discoverButton_clicked();
    void on_observeButton_clicked();
    void on_addOptionsButton_clicked();
    void on_contentButton_clicked();
    void on_resourceComboBox_editTextChanged(const QString &text);
    void on_methodComboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QCoapClient *m_client;
    QList<QCoapOption> m_options;
    QByteArray m_currentData;
};

#endif // MAINWINDOW_H
