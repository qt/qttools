// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCore/qobject.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qnetworkinterface.h>

QT_BEGIN_NAMESPACE

class QCoapReply;
class QCoapResourceDiscoveryReply;
class QCoapRequest;
class QCoapProtocol;
class QCoapConnection;
class QCoapSecurityConfiguration;
class QCoapMessage;
class QIODevice;

class QCoapClientPrivate;
class Q_COAP_EXPORT QCoapClient : public QObject
{
    Q_OBJECT
#if QT_CONFIG(networkinterface)
    Q_PROPERTY(QNetworkInterface bindInterface READ bindInterface WRITE setBindInterface
                                               NOTIFY bindInterfaceChanged)
#endif
public:
    explicit QCoapClient(QtCoap::SecurityMode securityMode = QtCoap::SecurityMode::NoSecurity,
                         QObject *parent = nullptr);
    ~QCoapClient();

    QCoapReply *get(const QCoapRequest &request);
    QCoapReply *get(const QUrl &url);
    QCoapReply *put(const QCoapRequest &request, const QByteArray &data = QByteArray());
    QCoapReply *put(const QCoapRequest &request, QIODevice *device);
    QCoapReply *put(const QUrl &url, const QByteArray &data = QByteArray());
    QCoapReply *post(const QCoapRequest &request, const QByteArray &data = QByteArray());
    QCoapReply *post(const QCoapRequest &request, QIODevice *device);
    QCoapReply *post(const QUrl &url, const QByteArray &data = QByteArray());
    QCoapReply *deleteResource(const QCoapRequest &request);
    QCoapReply *deleteResource(const QUrl &url);
    QCoapReply *observe(const QCoapRequest &request);
    QCoapReply *observe(const QUrl &request);
    void cancelObserve(QCoapReply *notifiedReply);
    void cancelObserve(const QUrl &url);
    void disconnect();

    QCoapResourceDiscoveryReply *discover(
            QtCoap::MulticastGroup group = QtCoap::MulticastGroup::AllCoapNodesIPv4,
            int port = QtCoap::DefaultPort,
            const QString &discoveryPath = QLatin1String("/.well-known/core"));
    QCoapResourceDiscoveryReply *discover(
            const QUrl &baseUrl,
            const QString &discoveryPath = QLatin1String("/.well-known/core"));

    void setSecurityConfiguration(const QCoapSecurityConfiguration &configuration);
    void setBlockSize(quint16 blockSize);
    void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value);
    void setMaximumServerResponseDelay(uint responseDelay);
    void setAckTimeout(uint ackTimeout);
    void setAckRandomFactor(double ackRandomFactor);
    void setMaximumRetransmitCount(uint maximumRetransmitCount);
    void setMinimumTokenSize(int tokenSize);

#if QT_CONFIG(networkinterface)
    void setBindInterface(const QNetworkInterface &iface);
    QNetworkInterface bindInterface() const;
#endif

Q_SIGNALS:
    void finished(QCoapReply *reply);
    void responseToMulticastReceived(QCoapReply *reply, const QCoapMessage &message,
                                     const QHostAddress &sender);
    void error(QCoapReply *reply, QtCoap::Error error);

#if QT_CONFIG(networkinterface)
    void bindInterfaceChanged(const QNetworkInterface &iface);
#endif

protected:
    Q_DECLARE_PRIVATE(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
