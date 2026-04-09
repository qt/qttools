// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapconnection_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcCoapConnection, "qt.coap.connection")

/*!
    \internal

    \class QCoapConnection
    \inmodule QtCoap

    \brief The QCoapConnection class defines an interface for
    handling transfers of frames to a server.

    It isolates CoAP clients from the transport in use, so that any
    client can be used with any supported transport.
*/

/*!
    \internal

    \enum QCoapConnection::ConnectionState

    This enum specifies the state of the underlying transport.

    \value Unconnected      The underlying transport is not yet ready for data transmission.

    \value Bound            The underlying transport is ready for data transmission. For example,
                            if QUdpSocket is used for the transport, this corresponds to
                            QAbstractSocket::BoundState.
    \sa state(), bound()
*/

/*!
    \internal

    \fn void QCoapConnection::error(QAbstractSocket::SocketError error)

    This signal is emitted when a connection error occurs. The \a error
    parameter describes the type of error that occurred.
*/

/*!
    \internal

    \fn void QCoapConnection::readyRead(const QByteArray &data, const QHostAddress &sender)

    This signal is emitted when a network reply is available. The \a data
    parameter supplies the received data, and the \a sender parameter supplies
    the sender address.
*/

/*!
    \internal

    \fn void QCoapConnection::bound()

    This signal is emitted when the underlying transport is ready for data transmission.
    Derived implementations must emit this signal whenever they are ready to start
    transferring data frames to and from the server.

    \sa bind()
*/

/*!
    \fn void QCoapConnection::securityConfigurationChanged()

    This signal is emitted when the security configuration is changed.
*/

/*!
    \internal

    \fn void QCoapConnection::bind(const QString &host, quint16 port)

    Prepares the underlying transport for data transmission to to the given \a host
    address on \a port. Emits the bound() signal when the transport is ready.

    This is a pure virtual method.

    \sa bound(), close()
*/

/*!
    \internal

    \fn void QCoapConnection::close()

    Closes the open sockets and connections to free the underlying transport.
    This is a pure virtual method.

    \sa bind()
*/

/*!
    \internal

    \fn void QCoapConnection::writeData(const QByteArray &data, const QString &host, quint16 port)

    Sends the given \a data frame to the host address \a host at port \a port.

    This is a pure virtual method.
*/

QCoapConnectionPrivate::QCoapConnectionPrivate(QtCoap::SecurityMode security)
    : securityMode(security)
    , state(QCoapConnection::ConnectionState::Unconnected)
{}

/*!
    Constructs a new CoAP connection for the given \a securityMode and
    sets \a parent as its parent.
*/
QCoapConnection::QCoapConnection(QtCoap::SecurityMode securityMode, QObject *parent)
    : QCoapConnection(*new QCoapConnectionPrivate(securityMode), parent)
{
}

/*!
    \internal

    Constructs a new new CoAP connection as a child of \a parent, with \a dd
    as its \c d_ptr. This constructor must be used when internally subclassing
    the QCoapConnection class.
*/
QCoapConnection::QCoapConnection(QObjectPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    connect(this, &QCoapConnection::bound, this,
            [this]() {
                Q_D(QCoapConnection);
                d->state = ConnectionState::Bound;
                startToSendRequest();
            });
}

/*!
    \internal

    Releases any resources held by QCoapConnection.
*/
QCoapConnection::~QCoapConnection()
{
}

/*!
    \internal

    Prepares the underlying transport for data transmission and sends the given
    \a request frame to the given \a host at the given \a port when the transport
    is ready.

    The preparation of the transport is done by calling the pure virtual bind() method,
    which needs to be implemented by derived classes.
*/
void
QCoapConnectionPrivate::sendRequest(const QByteArray &request, const QString &host, quint16 port)
{
    Q_Q(QCoapConnection);

    CoapFrame frame(request, host, port);
    framesToSend.enqueue(frame);

    if (state == QCoapConnection::ConnectionState::Unconnected)
        q->bind(host, port);
    else
        q->startToSendRequest();
}

/*!
    \internal

    Returns \c true if security is used, returns \c false otherwise.
*/
bool QCoapConnection::isSecure() const
{
    Q_D(const QCoapConnection);
    return d->securityMode != QtCoap::SecurityMode::NoSecurity;
}

/*!
    \internal

    Returns the security mode.
*/
QtCoap::SecurityMode QCoapConnection::securityMode() const
{
    Q_D(const QCoapConnection);
    return d->securityMode;
}

/*!
    \internal

    Returns the connection state.
*/
QCoapConnection::ConnectionState QCoapConnection::state() const
{
    Q_D(const QCoapConnection);
    return d->state;
}

/*!
    \internal

    Sends the last stored frame to the server by calling the pure virtual
    writeData() method.
*/
void QCoapConnection::startToSendRequest()
{
    Q_D(QCoapConnection);

    Q_ASSERT(!d->framesToSend.isEmpty());
    while (!d->framesToSend.isEmpty()) {
        const CoapFrame frame = d->framesToSend.dequeue();
        writeData(frame.currentPdu, frame.host, frame.port);
    }
}

/*!
    Sets the security configuration parameters from the \a configuration.
    The security configuration will be ignored if the QtCoap::NoSecurity mode is
    used for connection.

    \note This method must be called before the handshake starts.
*/
void QCoapConnection::setSecurityConfiguration(const QCoapSecurityConfiguration &configuration)
{
    Q_D(QCoapConnection);

    if (isSecure()) {
        d->securityConfiguration = configuration;
        emit securityConfigurationChanged();
    } else {
        qCWarning(lcCoapConnection, "Security is disabled, security configuration will be ignored.");
    }
}

/*!
    \internal

    Returns the security configuration.
*/
QCoapSecurityConfiguration QCoapConnection::securityConfiguration() const
{
    Q_D(const QCoapConnection);
    return d->securityConfiguration;
}

/*!
    \internal

    Closes the open sockets and connections to free the transport and clears
    the connection state.
*/
void QCoapConnection::disconnect()
{
    Q_D(QCoapConnection);

    close();

    d->framesToSend.clear();
    d->state = ConnectionState::Unconnected;
}

#if QT_CONFIG(networkinterface)
/*!
    \internal
*/
void QCoapConnection::setUdpNetworkInterface(const QNetworkInterface &iface)
{
    Q_D(QCoapConnection);
    d->udpNetworkInterface = iface;
}

/*!
    \internal
*/
QNetworkInterface QCoapConnection::udpNetworkInterface() const
{
    Q_D(const QCoapConnection);
    return d->udpNetworkInterface;
}
#endif

QT_END_NAMESPACE
