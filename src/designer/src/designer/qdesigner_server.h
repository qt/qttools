// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDESIGNER_SERVER_H
#define QDESIGNER_SERVER_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QTcpServer;
class QTcpSocket;

class QDesignerTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit QDesignerTcpServer(QObject *parent = nullptr);
    ~QDesignerTcpServer() override;

    quint16 serverPort() const;

    static void sendOpenRequest(int port, const QStringList &files);

private slots:
    void handleNewConnection();
    void readFromClient();
    void socketClosed();

private:
    QTcpServer *m_server;
    QTcpSocket *m_socket = nullptr;
};

class QDesignerTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit QDesignerTcpClient(quint16 port, QObject *parent = nullptr);
    ~QDesignerTcpClient() override;

private slots:
    void readFromSocket();

private:
    QTcpSocket *m_socket;
};

QT_END_NAMESPACE

#endif // QDESIGNER_SERVER_H
