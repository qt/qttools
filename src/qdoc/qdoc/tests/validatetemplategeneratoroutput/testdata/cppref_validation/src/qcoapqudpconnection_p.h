// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPQUDPCONNECTION_P_H
#define QCOAPQUDPCONNECTION_P_H

#include <QtCoap/qcoapsecurityconfiguration.h>
#include <private/qcoapconnection_p.h>

#include <QtNetwork/qudpsocket.h>

#include <QtCore/qpointer.h>
#include <QtCore/qqueue.h>

#include <optional>

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

class QDtls;
class QSslPreSharedKeyAuthenticator;
class QCoapQUdpConnectionPrivate;
class Q_AUTOTEST_EXPORT QCoapQUdpConnection : public QCoapConnection
{
    Q_OBJECT

public:
    explicit QCoapQUdpConnection(QtCoap::SecurityMode security = QtCoap::SecurityMode::NoSecurity,
                                 QObject *parent = nullptr);

    ~QCoapQUdpConnection() override = default;

    QUdpSocket *socket() const;

public Q_SLOTS:
    void setSocketOption(QAbstractSocket::SocketOption, const QVariant &value);

#if QT_CONFIG(dtls)
private Q_SLOTS:
    void pskRequired(QSslPreSharedKeyAuthenticator *authenticator);
    void handshakeTimeout();
#endif

protected:
    explicit QCoapQUdpConnection(QCoapQUdpConnectionPrivate &dd, QObject *parent = nullptr);

    void bind(const QString &host, quint16 port) override;
    void writeData(const QByteArray &data, const QString &host, quint16 port) override;
    void close() override;

    void createSocket();

    Q_DECLARE_PRIVATE(QCoapQUdpConnection)
};

class Q_AUTOTEST_EXPORT QCoapQUdpConnectionPrivate : public QCoapConnectionPrivate
{
public:
    QCoapQUdpConnectionPrivate(QtCoap::SecurityMode security = QtCoap::SecurityMode::NoSecurity);
    ~QCoapQUdpConnectionPrivate() override;

    virtual bool bind();

    void bindSocket();
    void writeToSocket(const QByteArray &data, const QString &host, quint16 port);
    QUdpSocket* socket() const { return udpSocket; }
    void socketReadyRead();

    void setSecurityConfiguration(const QCoapSecurityConfiguration &configuration);

    bool datagramMatchesNetworkInterface(const QNetworkDatagram &datagram) const;

#if QT_CONFIG(dtls)
    std::optional<QNetworkDatagram> receiveDatagramDecrypted() const;
    void handleEncryptedDatagram();

    QPointer<QDtls> dtls;
#endif
    QPointer<QUdpSocket> udpSocket;

    Q_DECLARE_PUBLIC(QCoapQUdpConnection)
};

QT_END_NAMESPACE

#endif // QCOAPQUDPCONNECTION_P_H
