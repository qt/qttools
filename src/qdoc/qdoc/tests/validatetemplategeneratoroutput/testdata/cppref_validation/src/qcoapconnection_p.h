// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPCONNECTION_P_H
#define QCOAPCONNECTION_P_H

#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoapsecurityconfiguration.h>

#include <QtCore/qqueue.h>
#include <QtCore/qobject.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qnetworkinterface.h>
#include <private/qobject_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QCoapConnectionPrivate;
class Q_AUTOTEST_EXPORT QCoapConnection : public QObject
{
    Q_OBJECT
public:
    enum class ConnectionState : quint8 {
        Unconnected,
        Bound
    };

    explicit QCoapConnection(QtCoap::SecurityMode securityMode = QtCoap::SecurityMode::NoSecurity,
                             QObject *parent = nullptr);
    virtual ~QCoapConnection();

    bool isSecure() const;
    QtCoap::SecurityMode securityMode() const;
    ConnectionState state() const;
    QCoapSecurityConfiguration securityConfiguration() const;

    Q_INVOKABLE void setSecurityConfiguration(const QCoapSecurityConfiguration &configuration);
    Q_INVOKABLE void disconnect();

#if QT_CONFIG(networkinterface)
    void setUdpNetworkInterface(const QNetworkInterface &iface);
    QNetworkInterface udpNetworkInterface() const;
#endif

Q_SIGNALS:
    void error(QAbstractSocket::SocketError error);
    void readyRead(const QByteArray &data, const QHostAddress &sender);
    void bound();
    void securityConfigurationChanged();

private:
    void startToSendRequest();

protected:
    QCoapConnection(QObjectPrivate &dd, QObject *parent = nullptr);

    virtual void bind(const QString &host, quint16 port) = 0;
    virtual void writeData(const QByteArray &data, const QString &host, quint16 port) = 0;
    virtual void close() = 0;

private:
    friend class QCoapProtocolPrivate;

    Q_DECLARE_PRIVATE(QCoapConnection)
};

struct CoapFrame {
    QByteArray currentPdu;
    QString host;
    quint16 port = 0;

    CoapFrame(const QByteArray &pdu, const QString &hostName, quint16 portNumber)
    : currentPdu(pdu), host(hostName), port(portNumber) {}
};

class Q_AUTOTEST_EXPORT QCoapConnectionPrivate : public QObjectPrivate
{
public:
    QCoapConnectionPrivate(QtCoap::SecurityMode security = QtCoap::SecurityMode::NoSecurity);

    ~QCoapConnectionPrivate() override = default;

    void sendRequest(const QByteArray &request, const QString &host, quint16 port);

    QCoapSecurityConfiguration securityConfiguration;
    QtCoap::SecurityMode securityMode;
    QCoapConnection::ConnectionState state;
    QQueue<CoapFrame> framesToSend;
#if QT_CONFIG(networkinterface)
    QNetworkInterface udpNetworkInterface;
#endif

    Q_DECLARE_PUBLIC(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_P_H
