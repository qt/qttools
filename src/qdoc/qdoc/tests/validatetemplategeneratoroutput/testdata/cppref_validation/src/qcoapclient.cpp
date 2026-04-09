// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapclient_p.h"
#include "qcoapprotocol_p.h"
#include "qcoapreply.h"
#include "qcoapresourcediscoveryreply.h"
#include "qcoapnamespace.h"
#include "qcoapsecurityconfiguration.h"
#include "qcoapqudpconnection_p.h"
#include "qcoaprequest_p.h"
#include "qcoapreply_p.h"
#include <QtCore/qiodevice.h>
#include <QtCore/qurl.h>
#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qudpsocket.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcCoapClient, "qt.coap.client")

QCoapClientPrivate::QCoapClientPrivate(QCoapProtocol *protocol, QCoapConnection *connection)
    : protocol(protocol)
    , connection(connection)
    , workerThread(new QThread)
{
    protocol->moveToThread(workerThread);
    connection->moveToThread(workerThread);
    workerThread->start();
}

QCoapClientPrivate::~QCoapClientPrivate()
{
    workerThread->quit();
    workerThread->wait();
    delete workerThread;
    delete protocol;
    delete connection;
}

/*!
    \class QCoapClient
    \inmodule QtCoap

    \brief The QCoapClient class allows the application to
    send CoAP requests and receive replies.

    \reentrant

    The QCoapClient class contains signals that get triggered when the
    reply of a sent request has arrived.

    The application can use a QCoapClient to send requests over a CoAP
    network. It provides functions for standard requests: each returns a QCoapReply object,
    to which the response data shall be delivered; this can be read when the finished()
    signal arrives.

    A simple request can be sent with:
    \code
        QCoapClient *client = new QCoapClient(this);
        connect(client, &QCoapClient::finished, this, &TestClass::slotFinished);
        client->get(QCoapRequest(Qurl("coap://coap.me/test")));
    \endcode

    \note After processing of the request has finished, it is the responsibility
    of the user to delete the QCoapReply object at an appropriate time. Do not
    directly delete it inside the slot connected to finished(). You can use the
    deleteLater() function.

    You can also use an \e observe request. This can be used as above, or more
    conveniently with the QCoapReply::notified() signal:
    \code
        QCoapRequest request = QCoapRequest(Qurl("coap://coap.me/obs"));
        QCoapReply *reply = client->observe(request);
        connect(reply, &QCoapReply::notified, this, &TestClass::slotNotified);
    \endcode

    And the observation can be cancelled with:
    \code
        client->cancelObserve(reply);
    \endcode

    When a reply arrives, the QCoapClient emits a finished() signal.

    \note For a discovery request, the returned object is a QCoapResourceDiscoveryReply.
    It can be used the same way as a QCoapReply but contains also a list of
    resources.

    \sa QCoapRequest, QCoapReply, QCoapResourceDiscoveryReply
*/

/*!
    \fn void QCoapClient::finished(QCoapReply *reply)

    This signal is emitted along with the \l QCoapReply::finished() signal
    whenever a CoAP reply is received, after either a success or an error.
    The \a reply parameter will contain a pointer to the reply that has just
    been received.

    \sa error(), QCoapReply::finished(), QCoapReply::error()
*/

/*!
    \fn void QCoapClient::responseToMulticastReceived(QCoapReply *reply,
                                                      const QCoapMessage &message,
                                                      const QHostAddress &sender)

    This signal is emitted when a unicast response to a multicast request
    arrives. The \a reply parameter contains a pointer to the reply that has just
    been received, \a message contains the payload and the message details,
    and \a sender contains the sender address.

    \sa error(), QCoapReply::finished(), QCoapReply::error()
*/

/*!
    \fn void QCoapClient::error(QCoapReply *reply, QtCoap::Error error)

    This signal is emitted whenever an error occurs. The \a reply parameter
    can be \nullptr if the error is not related to a specific QCoapReply. The
    \a error parameter contains the error code.

    \sa finished(), QCoapReply::error(), QCoapReply::finished()
*/

/*!
    Constructs a QCoapClient object for the given \a securityMode and
    sets \a parent as the parent object.

    The default for \a securityMode is QtCoap::NoSecurity, which
    disables security.
*/
QCoapClient::QCoapClient(QtCoap::SecurityMode securityMode, QObject *parent) :
    QObject(*new QCoapClientPrivate(new QCoapProtocol, new QCoapQUdpConnection(securityMode)),
            parent)
{
    Q_D(QCoapClient);

    qRegisterMetaType<QCoapReply *>();
    qRegisterMetaType<QCoapMessage>();
    qRegisterMetaType<QPointer<QCoapReply>>();
    qRegisterMetaType<QPointer<QCoapResourceDiscoveryReply>>();
    qRegisterMetaType<QCoapConnection *>();
    qRegisterMetaType<QtCoap::Error>();
    qRegisterMetaType<QtCoap::ResponseCode>();
    qRegisterMetaType<QtCoap::Method>();
    qRegisterMetaType<QtCoap::SecurityMode>();
    qRegisterMetaType<QtCoap::MulticastGroup>();
    // Requires a name, as this is a typedef
    qRegisterMetaType<QCoapToken>("QCoapToken");
    qRegisterMetaType<QCoapMessageId>("QCoapMessageId");
    qRegisterMetaType<QAbstractSocket::SocketOption>();

    connect(d->connection, &QCoapConnection::readyRead, d->protocol,
            [this](const QByteArray &data, const QHostAddress &sender) {
                    Q_D(QCoapClient);
                    d->protocol->d_func()->onFrameReceived(data, sender);
            });
    connect(d->connection, &QCoapConnection::error, d->protocol,
            [this](QAbstractSocket::SocketError socketError) {
                    Q_D(QCoapClient);
                    d->protocol->d_func()->onConnectionError(socketError);
            });

    connect(d->protocol, &QCoapProtocol::finished,
            this, &QCoapClient::finished);
    connect(d->protocol, &QCoapProtocol::responseToMulticastReceived,
            this, &QCoapClient::responseToMulticastReceived);
    connect(d->protocol, &QCoapProtocol::error,
            this, &QCoapClient::error);
}

/*!
    \internal

    Sets the client's connection to \a customConnection.
*/
void QCoapClientPrivate::setConnection(QCoapConnection *customConnection)
{
    Q_Q(QCoapClient);

    delete connection;
    connection = customConnection;

    q->connect(connection, &QCoapConnection::readyRead, protocol,
            [this](const QByteArray &data, const QHostAddress &sender) {
                    protocol->d_func()->onFrameReceived(data, sender);
            });
    q->connect(connection, &QCoapConnection::error, protocol,
            [this](QAbstractSocket::SocketError socketError) {
                    protocol->d_func()->onConnectionError(socketError);
            });
}

/*!
    Destroys the QCoapClient object and frees up any
    resources. Note that QCoapReply objects that are returned from
    this class have the QCoapClient set as their parents, which means that
    they will be deleted along with it.
*/
QCoapClient::~QCoapClient()
{
    qDeleteAll(findChildren<QCoapReply *>(QString(), Qt::FindDirectChildrenOnly));
}

/*!
    Sends the \a request using the GET method and returns a new QCoapReply object.

    \sa post(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::get(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest = QCoapRequestPrivate::createRequest(request, QtCoap::Method::Get,
                                                                  d->connection->isSecure());
    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends a GET request to \a url and returns a new QCoapReply object.

    \sa post(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::get(const QUrl &url)
{
    QCoapRequest request(url);
    return get(request);
}

/*!
    Sends the \a request using the PUT method and returns a new QCoapReply
    object. Uses \a data as the payload for this request. If \a data is empty,
    the payload of the \a request will be used.

    \sa get(), post(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::put(const QCoapRequest &request, const QByteArray &data)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest = QCoapRequestPrivate::createRequest(request, QtCoap::Method::Put,
                                                                  d->connection->isSecure());
    if (!data.isEmpty())
        copyRequest.setPayload(data);
    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends the \a request using the PUT method and returns a new QCoapReply
    object. Uses \a device content as the payload for this request.
    A null device is treated as empty content, in which case the payload of the
    \a request will be used.

    \note The device has to be open and readable before calling this function.

    \sa get(), post(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::put(const QCoapRequest &request, QIODevice *device)
{
    return put(request, device ? device->readAll() : QByteArray());
}

/*!
    \overload

    Sends a PUT request to \a url and returns a new QCoapReply object.
    Uses \a data as the payload for this request.

    \sa get(), post(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::put(const QUrl &url, const QByteArray &data)
{
    return put(QCoapRequest(url), data);
}

/*!
    Sends the \a request using the POST method and returns a new QCoapReply
    object. Uses \a data as the payload for this request. If \a data is empty,
    the payload of the \a request will be used.

    \sa get(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::post(const QCoapRequest &request, const QByteArray &data)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest = QCoapRequestPrivate::createRequest(request, QtCoap::Method::Post,
                                                                  d->connection->isSecure());
    if (!data.isEmpty())
        copyRequest.setPayload(data);
    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends the \a request using the POST method and returns a new QCoapReply
    object. Uses \a device content as the payload for this request.
    A null device is treated as empty content, in which case the payload of the
    \a request will be used.

    \note The device has to be open and readable before calling this function.

    \sa get(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::post(const QCoapRequest &request, QIODevice *device)
{
    return post(request, device ? device->readAll() : QByteArray());
}

/*!
    \overload

    Sends a POST request to \a url and returns a new QCoapReply object.
    Uses \a data as the payload for this request.

    \sa get(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::post(const QUrl &url, const QByteArray &data)
{
    return post(QCoapRequest(url), data);
}

/*!
    Sends the \a request using the DELETE method and returns a new QCoapReply
    object.

    \sa get(), put(), post(), observe(), discover()
 */
QCoapReply *QCoapClient::deleteResource(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest = QCoapRequestPrivate::createRequest(request, QtCoap::Method::Delete,
                                                                  d->connection->isSecure());
    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends a DELETE request to the target \a url.

    \sa get(), put(), post(), observe(), discover()
 */
QCoapReply *QCoapClient::deleteResource(const QUrl &url)
{
    return deleteResource(QCoapRequest(url));
}

/*!
    \overload

    Discovers the resources available at the endpoints which have joined
    the \a group at the given \a port. Returns a new QCoapResourceDiscoveryReply
    object which emits the \l QCoapResourceDiscoveryReply::discovered() signal whenever
    a response arrives. The \a group is one of the CoAP multicast group addresses
    and defaults to QtCoap::AllCoapNodesIPv4.

    Discovery path defaults to "/.well-known/core", but can be changed
    by passing a different path to \a discoveryPath. Discovery is described in
    \l{https://tools.ietf.org/html/rfc6690#section-1.2.1}{RFC 6690}.

    \sa get(), post(), put(), deleteResource(), observe()
*/
QCoapResourceDiscoveryReply *QCoapClient::discover(QtCoap::MulticastGroup group, int port,
                                           const QString &discoveryPath)
{
    Q_D(QCoapClient);

    QString base;
    switch (group) {
    case QtCoap::MulticastGroup::AllCoapNodesIPv4:
        base = QStringLiteral("224.0.1.187");
        break;
    case QtCoap::MulticastGroup::AllCoapNodesIPv6LinkLocal:
        base = QStringLiteral("ff02::fd");
        break;
    case QtCoap::MulticastGroup::AllCoapNodesIPv6SiteLocal:
        base = QStringLiteral("ff05::fd");
        break;
    }

    QUrl discoveryUrl;
    discoveryUrl.setHost(base);
    discoveryUrl.setPath(discoveryPath);
    discoveryUrl.setPort(port);

    QCoapRequest request = QCoapRequestPrivate::createRequest(QCoapRequest(discoveryUrl),
                                                              QtCoap::Method::Get,
                                                              d->connection->isSecure());

    return d->sendDiscovery(request);
}

/*!
    Discovers the resources available at the given \a url and returns
    a new QCoapResourceDiscoveryReply object which emits the
    \l QCoapResourceDiscoveryReply::discovered() signal whenever the response
    arrives.

    Discovery path defaults to "/.well-known/core", but can be changed
    by passing a different path to \a discoveryPath. Discovery is described in
    \l{https://tools.ietf.org/html/rfc6690#section-1.2.1}{RFC 6690}.

    \sa get(), post(), put(), deleteResource(), observe()
*/
QCoapResourceDiscoveryReply *QCoapClient::discover(const QUrl &url, const QString &discoveryPath)
{
    Q_D(QCoapClient);

    QUrl discoveryUrl(url);
    discoveryUrl.setPath(url.path() + discoveryPath);

    QCoapRequest request = QCoapRequestPrivate::createRequest(QCoapRequest(discoveryUrl),
                                                              QtCoap::Method::Get,
                                                              d->connection->isSecure());
    return d->sendDiscovery(request);
}

/*!
    Sends a request to observe the target \a request and returns
    a new QCoapReply object which emits the \l QCoapReply::notified()
    signal whenever a new notification arrives.

    \sa cancelObserve(), get(), post(), put(), deleteResource(), discover()
*/
QCoapReply *QCoapClient::observe(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest = QCoapRequestPrivate::createRequest(request, QtCoap::Method::Get,
                                                                  d->connection->isSecure());
    copyRequest.enableObserve();

    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends a request to observe the target \a url and returns
    a new QCoapReply object which emits the \l QCoapReply::notified()
    signal whenever a new notification arrives.

    \sa cancelObserve(), get(), post(), put(), deleteResource(), discover()
*/
QCoapReply *QCoapClient::observe(const QUrl &url)
{
    return observe(QCoapRequest(url));
}

/*!
    \overload

    Cancels the observation of a resource using the reply \a notifiedReply returned by
    the observe() method.

    \sa observe()
*/
void QCoapClient::cancelObserve(QCoapReply *notifiedReply)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "cancelObserve",
                              Q_ARG(QPointer<QCoapReply>, QPointer<QCoapReply>(notifiedReply)));
}

/*!
    \overload

    Cancels the observation of a resource identified by the \a url.

    \sa observe()
*/
void QCoapClient::cancelObserve(const QUrl &url)
{
    Q_D(QCoapClient);
    const auto adjustedUrl = QCoapRequestPrivate::adjustedUrl(url, d->connection->isSecure());
    QMetaObject::invokeMethod(d->protocol, "cancelObserve", Q_ARG(QUrl, adjustedUrl));
}

/*!
    Closes the open sockets and connections to free the transport.

    \note In the secure mode this needs to be called before changing
    the security configuration or connecting to another server.

    \sa setSecurityConfiguration()
*/
void QCoapClient::disconnect()
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->connection, "disconnect", Qt::QueuedConnection);
}

/*!
    \internal

    Sends the CoAP \a request to its own URL and returns a new QCoapReply
    object.
*/
QCoapReply *QCoapClientPrivate::sendRequest(const QCoapRequest &request)
{
    Q_Q(QCoapClient);

    // Prepare the reply
    QCoapReply *reply = QCoapReplyPrivate::createCoapReply(request, q);

    if (!send(reply)) {
        delete reply;
        return nullptr;
    }

    return reply;
}

/*!
    \internal

    Sends the CoAP \a request to its own URL and returns a
    new QCoapResourceDiscoveryReply object.
*/
QCoapResourceDiscoveryReply *QCoapClientPrivate::sendDiscovery(const QCoapRequest &request)
{
    Q_Q(QCoapClient);

    // Prepare the reply
    QCoapResourceDiscoveryReply *reply = new QCoapResourceDiscoveryReply(request, q);

    if (!send(reply)) {
        delete reply;
        return nullptr;
    }

    return reply;
}

/*!
    \internal

    Connect to the reply and use the protocol to send it.
*/
bool QCoapClientPrivate::send(QCoapReply *reply)
{
    const auto scheme = connection->isSecure() ? QLatin1String("coaps") : QLatin1String("coap");
    if (reply->request().url().scheme() != scheme) {
        qCWarning(lcCoapClient, "Failed to send request, URL has an incorrect scheme.");
        return false;
    }

    if (!QCoapRequestPrivate::isUrlValid(reply->request().url())) {
        qCWarning(lcCoapClient, "Failed to send request for an invalid URL.");
        return false;
    }

    // According to https://tools.ietf.org/html/rfc7252#section-8.1,
    // multicast requests MUST be Non-confirmable.
    if (QHostAddress(reply->url().host()).isMulticast()
            && reply->request().type() == QCoapMessage::Type::Confirmable) {
        qCWarning(lcCoapClient, "Failed to send request, "
                                "multicast requests must be non-confirmable.");
        return false;
    }

    QMetaObject::invokeMethod(protocol, "sendRequest", Qt::QueuedConnection,
                              Q_ARG(QPointer<QCoapReply>, QPointer<QCoapReply>(reply)),
                              Q_ARG(QCoapConnection *, connection));

    return true;
}

/*!
    Sets the security configuration parameters from \a configuration.
    Configuration will be ignored if the QtCoap::NoSecurity mode is used.

    \note This method must be called before the handshake starts. If you need
    to change the security configuration after establishing a secure connection
    with the server, the client needs to be disconnected first.

    \sa disconnect()
*/
void QCoapClient::setSecurityConfiguration(const QCoapSecurityConfiguration &configuration)
{
    Q_D(QCoapClient);

    QMetaObject::invokeMethod(d->connection, "setSecurityConfiguration", Qt::QueuedConnection,
                              Q_ARG(QCoapSecurityConfiguration, configuration));
}

/*!
    Sets the maximum block size used by the protocol to \a blockSize
    when sending requests and receiving replies. The block size must be
    a power of two.
*/
void QCoapClient::setBlockSize(quint16 blockSize)
{
    Q_D(QCoapClient);

    QMetaObject::invokeMethod(d->protocol, "setBlockSize", Qt::QueuedConnection,
                              Q_ARG(quint16, blockSize));
}

/*!
    Sets the QUdpSocket socket \a option to \a value.
*/
void QCoapClient::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
    Q_D(QCoapClient);

    QMetaObject::invokeMethod(d->connection, "setSocketOption", Qt::QueuedConnection,
                              Q_ARG(QAbstractSocket::SocketOption, option),
                              Q_ARG(QVariant, value));
}

/*!
    Sets the \c MAX_SERVER_RESPONSE_DELAY value to \a responseDelay in milliseconds.
    The default is 250 seconds.

    As defined in \l {RFC 7390 - Section 2.5}, \c MAX_SERVER_RESPONSE_DELAY is the expected
    maximum response delay over all servers that the client can send a multicast request to.
*/
void QCoapClient::setMaximumServerResponseDelay(uint responseDelay)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "setMaximumServerResponseDelay", Qt::QueuedConnection,
                              Q_ARG(uint, responseDelay));
}

/*!
    Sets the \c ACK_TIMEOUT value defined in \l {RFC 7252 - Section 4.2} to
    \a ackTimeout in milliseconds. The default is 2000 ms.

    This timeout only applies to confirmable messages. The actual timeout for
    reliable transmissions is a random value between \c ACK_TIMEOUT and
    \c {ACK_TIMEOUT * ACK_RANDOM_FACTOR}.

    \sa setAckRandomFactor()
*/
void QCoapClient::setAckTimeout(uint ackTimeout)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "setAckTimeout", Qt::QueuedConnection,
                              Q_ARG(uint, ackTimeout));
}

/*!
    Sets the \c ACK_RANDOM_FACTOR value defined in \l {RFC 7252 - Section 4.2},
    to \a ackRandomFactor. This value should be greater than or equal to 1.
    The default is 1.5.

    \sa setAckTimeout()
*/
void QCoapClient::setAckRandomFactor(double ackRandomFactor)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "setAckRandomFactor", Qt::QueuedConnection,
                              Q_ARG(double, ackRandomFactor));
}

/*!
    Sets the \c MAX_RETRANSMIT value defined in \l {RFC 7252 - Section 4.2}
    to \a maximumRetransmitCount. This value should be less than or equal to 25.
    The default is 4.
*/
void QCoapClient::setMaximumRetransmitCount(uint maximumRetransmitCount)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "setMaximumRetransmitCount", Qt::QueuedConnection,
                              Q_ARG(uint, maximumRetransmitCount));
}

/*!
    Sets the minimum token size to \a tokenSize in bytes. For security reasons it is
    recommended to use tokens with a length of at least 4 bytes. The default value for
    this parameter is 4 bytes.
*/
void QCoapClient::setMinimumTokenSize(int tokenSize)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "setMinimumTokenSize", Qt::QueuedConnection,
                              Q_ARG(int, tokenSize));
}

#if QT_CONFIG(networkinterface)
/*!
    \property QCoapClient::bindInterface
    \brief the network interface to be used by the socket
    \since 6.11

    The default value is an \l {QNetworkInterface::isValid()}{invalid}
    QNetworkInterface object, meaning that incoming packets will be accepted
    from all network interfaces. Similarly, all network interfaces can be used
    to send outgoing packets.

    When a valid network interface is specified, incoming packets will only be
    accepted from that interface. Similarly, outgoing packets will only be sent
    using that interface.

    Changing the property only has an effect the next time the client binds to
    the socket, so make sure to call \l disconnect() if there was any prior
    communication.
*/
void QCoapClient::setBindInterface(const QNetworkInterface &iface)
{
    Q_D(QCoapClient);
    // QNI does not have operator==(). We use index() to distinguish the
    // interfaces, because this is the value provided by the OS, and it is
    // unlikely to change.
    // We also need to check isValid(), because an invalid interface has
    // index() == 0, which might clash with an existing interface index.
    if (iface.isValid() == d->interface.isValid()
        && iface.index() == d->interface.index()) {
        return;
    }
    d->interface = iface;
    QMetaObject::invokeMethod(d->connection, &QCoapConnection::setUdpNetworkInterface,
                              Qt::QueuedConnection, iface);
    Q_EMIT bindInterfaceChanged(iface);
}

QNetworkInterface QCoapClient::bindInterface() const
{
    Q_D(const QCoapClient);
    return d->interface;
}
#endif

QT_END_NAMESPACE
