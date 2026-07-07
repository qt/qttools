// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDESIGNER_SERVER_H
#define QDESIGNER_SERVER_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QLocalSocket;
class QTcpServer;
class QTcpSocket;

// ### FIXME Qt 7: Remove
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

class QDesignerLocalSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit QDesignerLocalSocketClient(const QString &serverName, QObject *parent = nullptr);
    ~QDesignerLocalSocketClient() override;

private slots:
    void readFromSocket();

private:
    QLocalSocket *m_socket;
};

// ### FIXME Qt 7: Remove
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
