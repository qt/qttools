// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapreply_p.h"
#include "qcoapinternalreply_p.h"
#include "qcoapnamespace_p.h"

#include <QtCore/qmath.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
    Constructor.
*/
QCoapReplyPrivate::QCoapReplyPrivate(const QCoapRequest &req) :
    request(req)
{
}

/*!
    \internal
    Marks the reply as running, and sets the \a token and \a messageId of this
    exchange.

    \sa isRunning()
*/
void QCoapReplyPrivate::_q_setRunning(const QCoapToken &token, QCoapMessageId messageId)
{
    request.setToken(token);
    request.setMessageId(messageId);
    isRunning = true;
}

/*!
    \internal

    Sets the reply as finished.
*/
void QCoapReplyPrivate::_q_setObserveCancelled()
{
    Q_Q(QCoapReply);

    bool alreadyFinished = q->isFinished();

    isFinished = true;
    isRunning = false;

    if (!alreadyFinished)
        emit q->finished(q);
}

/*!
    \internal

    Sets the message and response code of this reply, unless reply is
    already finished.
*/
void QCoapReplyPrivate::_q_setContent(const QHostAddress &, const QCoapMessage &msg,
                                      QtCoap::ResponseCode code)
{
    Q_Q(QCoapReply);

    if (q->isFinished())
        return;

    message = msg;
    responseCode = code;
    seekBuffer(0);

    if (QtCoap::isError(responseCode))
        _q_setError(responseCode);
}

/*!
    \internal

    For an Observe request, notifies that a new message was received
    by emitting the notified() signal. If the reply is finished, no
    signal will be emitted.
*/
void QCoapReplyPrivate::_q_setNotified()
{
    Q_Q(QCoapReply);

    if (!q->isFinished())
        emit q->notified(q, message);
}

/*!
    \internal

    Sets the reply as finished, sending the finished() signal if it wasn't
    already.
*/
void QCoapReplyPrivate::_q_setFinished(QtCoap::Error newError)
{
    Q_Q(QCoapReply);

    if (q->isFinished())
        return;

    isFinished = true;
    isRunning = false;

    if (newError != QtCoap::Error::Ok)
        _q_setError(newError);

    emit q->finished(q);
}

/*!
    \internal

    Sets the error of the reply.

    \sa errorReceived()
*/
void QCoapReplyPrivate::_q_setError(QtCoap::Error newError)
{
    Q_Q(QCoapReply);
    if (error == newError)
        return;

    error = newError;
    emit q->error(q, error);
}

/*!
    \internal

    Sets the error of the reply.
*/
void QCoapReplyPrivate::_q_setError(QtCoap::ResponseCode code)
{
    _q_setError(QtCoap::errorForResponseCode(code));
}

/*!
    \class QCoapReply
    \inmodule QtCoap

    \brief The QCoapReply class holds the data of a CoAP reply.

    \reentrant

    The QCoapReply contains data related to a request sent with the
    QCoapClient.

    The finished() signal is emitted when the response is fully
    received or when the request fails.

    For \e Observe requests specifically, the notified() signal is emitted
    whenever a notification is received.

    \sa QCoapClient, QCoapRequest, QCoapResourceDiscoveryReply
*/

/*!
    \fn void QCoapReply::finished(QCoapReply* reply)

    This signal is emitted whenever the corresponding request finished,
    whether successfully or not. When a resource is observed, this signal
    will only be emitted once, when the observation ends.

    The \a reply parameter is the QCoapReply itself for convenience.

    \note If the QCoapReply is deleted while not finished, both aborted() and
    finished() signal will be emitted immediately before the QCoapReply is
    destroyed. Given the QCoapReply may have been deleted when receiving the
    signal, you should not rely on the \a reply to be still valid.

    \sa QCoapClient::finished(), isFinished(), notified(), aborted()
*/

/*!
    \fn void QCoapReply::notified(QCoapReply* reply, const QCoapMessage &message)

    This signal is emitted whenever a notification is received from an observed
    resource.

    Its \a message parameter is a QCoapMessage containing the payload and the
    message details. The \a reply parameter is the QCoapReply itself for
    convenience.

    \sa QCoapClient::finished(), isFinished(), finished(), notified()
*/

/*!
    \fn void QCoapReply::error(QCoapReply* reply, QtCoap::Error error)

    This signal is emitted whenever an error occurs and is followed by the
    finished() signal.

    Its \a reply parameters is the QCoapReply itself for convenience, and
    the \a error parameter is the error received.

    \sa finished(), aborted()
*/

/*!
    \fn void QCoapReply::aborted(const QCoapToken &token);

    This signal is emitted when the request is aborted or the reply is deleted.
    Its \a token parameter is the token of the exchange that has been aborted.

    \note If the QCoapReply is deleted while not finished, both aborted() and
    finished() signal will be emitted immediately before the QCoapReply is
    destroyed. Given the QCoapReply may have been deleted when receiving the
    signal, you should not rely on the sender() object to be still valid.

    \sa finished(), error()
*/

/*!
    \internal
    Constructs a new CoAP reply with \a dd as the d_ptr.
    This constructor must be used when subclassing internally
    the QCoapReply class.
*/
QCoapReply::QCoapReply(QCoapReplyPrivate &dd, QObject *parent) :
    QIODevice(dd, parent)
{
    open(QIODevice::ReadOnly);
}

/*!
    Destroys the QCoapReply and aborts the request if its response has
    not yet been received.
*/
QCoapReply::~QCoapReply()
{
    abortRequest();
}

/*!
  \internal

  \overload
*/
qint64 QCoapReply::readData(char *data, qint64 maxSize)
{
    Q_D(QCoapReply);

    QByteArray payload = d->message.payload();

    maxSize = qMin(maxSize, qint64(payload.size()) - pos());
    if (maxSize <= 0)
        return qint64(0);

    // Explicitly account for platform size_t limitations
    size_t len = static_cast<size_t>(maxSize);
    if (sizeof(qint64) > sizeof(size_t)
            && maxSize > static_cast<qint64>(std::numeric_limits<size_t>::max())) {
        qCWarning(lcCoapExchange) << "Cannot read more than"
                                  << std::numeric_limits<size_t>::max()
                                  << "at a time";
        len = std::numeric_limits<size_t>::max();
    }

    memcpy(data, payload.constData() + pos(), len);

    return static_cast<qint64>(len);
}

/*!
  \internal

  \overload
*/
qint64 QCoapReply::writeData(const char *data, qint64 maxSize)
{
    // The user cannot write to the reply
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

/*!
    Returns the response code of the request.
*/
QtCoap::ResponseCode QCoapReply::responseCode() const
{
    Q_D(const QCoapReply);
    return d->responseCode;
}

/*!
    Returns the contained message.
*/
QCoapMessage QCoapReply::message() const
{
    Q_D(const QCoapReply);
    return d->message;
}

/*!
    Returns the associated request.
*/
QCoapRequest QCoapReply::request() const
{
    Q_D(const QCoapReply);
    return d->request;
}

/*!
    Returns \c true if the request is finished.

    \sa finished()
*/
bool QCoapReply::isFinished() const
{
    Q_D(const QCoapReply);
    return d->isFinished || d->isAborted;
}

/*!
    Returns \c true if the request is running.
*/
bool QCoapReply::isRunning() const
{
    Q_D(const QCoapReply);
    return d->isRunning && !isFinished();
}

/*!
    Returns \c true if the request has been aborted.
*/
bool QCoapReply::isAborted() const
{
    Q_D(const QCoapReply);
    return d->isAborted;
}

/*!
    Returns \c true if the request finished with no error.
*/
bool QCoapReply::isSuccessful() const
{
    Q_D(const QCoapReply);
    return d->isFinished && !QtCoap::isError(d->responseCode)
            && d->error == QtCoap::Error::Ok;
}

/*!
    Returns the target uri of the associated request.
*/
QUrl QCoapReply::url() const
{
    Q_D(const QCoapReply);
    return d->request.url();
}

/*!
    Returns the method of the associated request.
*/
QtCoap::Method QCoapReply::method() const
{
    Q_D(const QCoapReply);
    return d->request.method();
}

/*!
    Returns the error of the reply or QCoapReply::NoError if there is no error.
*/
QtCoap::Error QCoapReply::errorReceived() const
{
    Q_D(const QCoapReply);
    return d->error;
}

/*!
    Aborts the request immediately and emits the
    \l{QCoapReply::aborted(const QCoapToken &token)}{aborted(const QCoapToken &token)}
    signal if the request was not finished.
*/
void QCoapReply::abortRequest()
{
    Q_D(QCoapReply);

    if (isFinished())
        return;

    d->isAborted = true;
    d->isFinished = true;
    d->isRunning = false;
    emit aborted(request().token());
    emit finished(this);
}

/*!
    \internal

    Creates a new instance of QCoapReply and returns a pointer to it.
*/
QCoapReply *QCoapReplyPrivate::createCoapReply(const QCoapRequest &request, QObject *parent)
{
    return new QCoapReply(*new QCoapReplyPrivate(request), parent);
}

QT_END_NAMESPACE
