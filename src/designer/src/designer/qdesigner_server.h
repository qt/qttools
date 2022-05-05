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

#ifndef QDESIGNER_SERVER_H
#define QDESIGNER_SERVER_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QTcpServer;
class QTcpSocket;

class QDesignerServer: public QObject
{
    Q_OBJECT
public:
    explicit QDesignerServer(QObject *parent = nullptr);
    ~QDesignerServer() override;

    quint16 serverPort() const;

    static void sendOpenRequest(int port, const QStringList &files);

private slots:
    void handleNewConnection();
    void readFromClient();
    void socketClosed();

private:
    QTcpServer *m_server;
    QTcpSocket *m_socket;
};

class QDesignerClient: public QObject
{
    Q_OBJECT
public:
    explicit QDesignerClient(quint16 port, QObject *parent = nullptr);
    ~QDesignerClient() override;

private slots:
    void readFromSocket();

private:
    QTcpSocket *m_socket;
};

QT_END_NAMESPACE

#endif // QDESIGNER_SERVER_H
