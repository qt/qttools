// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qfileinfo.h>
#include <QtCore/qstringlist.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

#include "qdesigner.h"
#include "qdesigner_server.h"

#include <qevent.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

// ### review

QDesignerServer::QDesignerServer(QObject *parent)
    : QObject(parent)
{
    m_server = new QTcpServer(this);
    if (m_server->listen(QHostAddress::LocalHost, 0)) {
        connect(m_server, &QTcpServer::newConnection,
                this, &QDesignerServer::handleNewConnection);
    }
}

QDesignerServer::~QDesignerServer() = default;

quint16 QDesignerServer::serverPort() const
{
    return m_server ? m_server->serverPort() : 0;
}

void QDesignerServer::sendOpenRequest(int port, const QStringList &files)
{
    QTcpSocket *sSocket = new QTcpSocket();
    sSocket->connectToHost(QHostAddress::LocalHost, port);
    if(sSocket->waitForConnected(3000))
    {
        for (const QString &file : files) {
            QFileInfo fi(file);
            sSocket->write(fi.absoluteFilePath().toUtf8() + '\n');
        }
        sSocket->waitForBytesWritten(3000);
        sSocket->close();
    }
    delete sSocket;
}

void QDesignerServer::readFromClient()
{
    while (m_socket->canReadLine()) {
        QString file = QString::fromUtf8(m_socket->readLine());
        if (!file.isNull()) {
            file.remove(u'\n');
            file.remove(u'\r');
            qDesigner->postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

void QDesignerServer::socketClosed()
{
    m_socket = nullptr;
}

void QDesignerServer::handleNewConnection()
{
    // no need for more than one connection
    if (m_socket == nullptr) {
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, &QTcpSocket::readyRead,
                this, &QDesignerServer::readFromClient);
        connect(m_socket, &QTcpSocket::disconnected,
                this, &QDesignerServer::socketClosed);
    }
}


QDesignerClient::QDesignerClient(quint16 port, QObject *parent)
: QObject(parent)
{
    m_socket = new QTcpSocket(this);
    m_socket->connectToHost(QHostAddress::LocalHost, port);
    connect(m_socket, &QTcpSocket::readyRead,
                this, &QDesignerClient::readFromSocket);

}

QDesignerClient::~QDesignerClient()
{
    m_socket->close();
    m_socket->flush();
}

void QDesignerClient::readFromSocket()
{
    while (m_socket->canReadLine()) {
        QString file = QString::fromUtf8(m_socket->readLine());
        if (!file.isNull()) {
            file.remove(u'\n');
            file.remove(u'\r');
            if (QFile::exists(file))
                qDesigner->postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

QT_END_NAMESPACE
