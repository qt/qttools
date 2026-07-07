// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner.h"
#include "qdesigner_server.h"

#include <QtGui/qevent.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

#include <QtCore/qfileinfo.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

// ### review

static void readFiles(QIODevice *device)
{
    while (device->canReadLine()) {
        QString file = QString::fromUtf8(device->readLine());
        if (!file.isNull()) {
            file.remove(u'\n');
            file.remove(u'\r');
            if (QFile::exists(file))
                QCoreApplication::postEvent(qDesigner, new QFileOpenEvent(file));
        }
    }
}

QDesignerTcpServer::QDesignerTcpServer(QObject *parent) : QObject(parent), m_server(new QTcpServer(this))
{
    if (m_server->listen(QHostAddress::LocalHost, 0))
        connect(m_server, &QTcpServer::newConnection, this, &QDesignerTcpServer::handleNewConnection);
}

QDesignerTcpServer::~QDesignerTcpServer() = default;

quint16 QDesignerTcpServer::serverPort() const
{
    return m_server ? m_server->serverPort() : 0;
}

void QDesignerTcpServer::sendOpenRequest(int port, const QStringList &files)
{
    auto *sSocket = new QTcpSocket();
    sSocket->connectToHost(QHostAddress::LocalHost, port);
    if (sSocket->waitForConnected(3000)) {
        for (const QString &file : files) {
            QFileInfo fi(file);
            sSocket->write(fi.absoluteFilePath().toUtf8() + '\n');
        }
        sSocket->waitForBytesWritten(3000);
        sSocket->close();
    }
    delete sSocket;
}

void QDesignerTcpServer::readFromClient()
{
    readFiles(m_socket);
}

void QDesignerTcpServer::socketClosed()
{
    m_socket = nullptr;
}

void QDesignerTcpServer::handleNewConnection()
{
    // no need for more than one connection
    if (m_socket == nullptr) {
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, &QTcpSocket::readyRead, this, &QDesignerTcpServer::readFromClient);
        connect(m_socket, &QTcpSocket::disconnected, this, &QDesignerTcpServer::socketClosed);
    }
}

QDesignerTcpClient::QDesignerTcpClient(quint16 port, QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    m_socket->connectToHost(QHostAddress::LocalHost, port);
    connect(m_socket, &QTcpSocket::readyRead, this, &QDesignerTcpClient::readFromSocket);
}

QDesignerTcpClient::~QDesignerTcpClient()
{
    m_socket->close();
    m_socket->flush();
}

void QDesignerTcpClient::readFromSocket()
{
    readFiles(m_socket);
}

QT_END_NAMESPACE
