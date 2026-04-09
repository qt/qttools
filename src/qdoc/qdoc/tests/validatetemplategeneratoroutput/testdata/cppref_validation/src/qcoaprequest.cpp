// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoaprequest_p.h"

#include <QtCore/qmath.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/QDebug>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

namespace {
const auto CoapScheme = QLatin1String("coap");
const auto CoapSecureScheme = QLatin1String("coaps");
}

QCoapRequestPrivate::QCoapRequestPrivate(const QUrl &url, QCoapMessage::Type type,
                                         const QUrl &proxyUrl) :
    QCoapMessagePrivate(type),
    proxyUri(proxyUrl)
{
    setUrl(url);
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
}

QCoapRequestPrivate *QCoapRequestPrivate::clone() const
{
    return new QCoapRequestPrivate(*this);
}

/*!
    \internal

    \brief Sets the url after adjusting it, and asserting its validity.
*/
void QCoapRequestPrivate::setUrl(const QUrl &url)
{
    // Print no warning when clearing URL
    if (url.isEmpty()) {
        uri = url;
        return;
    }

    // Make first checks before editing the URL, to avoid editing it
    // in a wrong way (e.g. when adding the scheme)
    if (!url.isValid()) {
        qCWarning(lcCoapExchange) << "Invalid CoAP url" << url.toString();
        return;
    }

    // If the port is unknown, try to set it based on scheme
    QUrl finalizedUrl = url;
    if (!url.scheme().isEmpty()) {
        if (url.scheme() == CoapScheme) {
            if (url.port() == -1)
                finalizedUrl.setPort(QtCoap::DefaultPort);
        } else if (url.scheme() == CoapSecureScheme) {
            if (url.port() == -1)
                finalizedUrl.setPort(QtCoap::DefaultSecurePort);
        } else {
            qCWarning(lcCoapExchange) << "QCoapRequest: Request URL's scheme" << url.scheme()
                                      << "isn't valid for CoAP";
            return;
        }
    }

    uri = finalizedUrl;
}

/*!
    \class QCoapRequest
    \inmodule QtCoap

    \brief The QCoapRequest class holds a CoAP request. This request
    can be sent with QCoapClient.

    \reentrant

    The QCoapRequest contains data needed to make CoAP frames that can be
    sent to the URL it holds.

    \sa QCoapClient, QCoapReply, QCoapResourceDiscoveryReply
*/

/*!
    Constructs a QCoapRequest object with the target \a url,
    the proxy URL \a proxyUrl and the \a type of the message.
*/
QCoapRequest::QCoapRequest(const QUrl &url, Type type, const QUrl &proxyUrl) :
    QCoapMessage(*new QCoapRequestPrivate(url, type, proxyUrl))
{
}

/*!
    Constructs a QCoapRequest from a string literal
*/
QCoapRequest::QCoapRequest(const char *url, Type type) :
    QCoapMessage(*new QCoapRequestPrivate(QUrl(QString::fromUtf8(url)), type))
{
}

/*!
    Constructs a copy of the \a other QCoapRequest.
*/
QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    //! No private data sharing, as QCoapRequestPrivate!=QCoapMessagePrivate
    //! and the d_ptr is a QSharedDataPointer<QCoapMessagePrivate>
    QCoapMessage(*other.d_func()->clone())
{
}

/*!
    Destroys the QCoapRequest.
*/
QCoapRequest::~QCoapRequest()
{
}

/*!
    Returns the target URI of the request.

    \sa setUrl()
*/
QUrl QCoapRequest::url() const
{
    Q_D(const QCoapRequest);
    return d->uri;
}

/*!
    Returns the proxy URI of the request.
    The request shall be sent directly if this is invalid.

    \sa setProxyUrl()
*/
QUrl QCoapRequest::proxyUrl() const
{
    Q_D(const QCoapRequest);
    return d->proxyUri;
}

/*!
    Returns the method of the request.
*/
QtCoap::Method QCoapRequest::method() const
{
    Q_D(const QCoapRequest);
    return d->method;
}

/*!
    Returns \c true if the request is an observe request.

    \sa enableObserve()
*/
bool QCoapRequest::isObserve() const
{
    return hasOption(QCoapOption::Observe);
}

/*!
    Sets the target URI of the request to the given \a url.

    If not indicated, the scheme of the URL will default to 'coap', and its
    port will default to 5683.

    \sa url()
*/
void QCoapRequest::setUrl(const QUrl &url)
{
    Q_D(QCoapRequest);
    d->setUrl(url);
}

/*!
    Sets the proxy URI of the request to the given \a proxyUrl.

    \sa proxyUrl()
*/
void QCoapRequest::setProxyUrl(const QUrl &proxyUrl)
{
    Q_D(QCoapRequest);
    d->proxyUri = proxyUrl;
}

/*!
    Sets the observe to \c true to make an observe request.

    \sa isObserve()
*/
void QCoapRequest::enableObserve()
{
    if (isObserve())
        return;

    addOption(QCoapOption::Observe);
}

/*!
    \internal

    Adjusts the request URL by setting the correct default scheme and port
    (if not indicated) based on the \a secure parameter.

    In non-secure mode the scheme of request URL will default to \c coap, and
    its port will default to \e 5683. In secure mode the scheme will default to
    \c coaps, and the port will default to \e 5684.
*/
void QCoapRequestPrivate::adjustUrl(bool secure)
{
    uri = adjustedUrl(uri, secure);
}

/*!
    Creates a copy of \a other.
*/
QCoapRequest &QCoapRequest::operator=(const QCoapRequest &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \internal

    Returns \c true if the \a url is a valid CoAP URL.
*/
bool QCoapRequestPrivate::isUrlValid(const QUrl &url)
{
    return (url.isValid() && !url.isLocalFile() && !url.isRelative()
            && (url.scheme() == CoapScheme || url.scheme() == CoapSecureScheme)
            && !url.hasFragment());
}

/*!
    \internal

    Adjusts the \a url by setting the correct default scheme and port
    (if not indicated) based on the \a secure parameter. Returns the
    adjusted URL.

    In non-secure mode the scheme of request URL will default to \c coap, and
    its port will default to \e 5683. In secure mode the scheme will default to
    \c coaps, and the port will default to \e 5684.
*/
QUrl QCoapRequestPrivate::adjustedUrl(const QUrl &url, bool secure)
{
    if (url.isEmpty() || !url.isValid())
        return QUrl();

    QUrl finalizedUrl = url;
    const auto scheme = secure ? CoapSecureScheme : CoapScheme;
    if (url.host().isEmpty() && url.isRelative()) {
        // In some cases host address is mistaken for part of the relative path,
        // prepending the scheme fixes this.
        finalizedUrl = QUrl{scheme + "://"_L1 + url.toString()};
    } else if (url.scheme().isEmpty()) {
        finalizedUrl.setScheme(scheme);
    }

    if (finalizedUrl.host().isEmpty()) {
        qCWarning(lcCoapExchange) << "The requested URL" << url << "is not a valid CoAP URL.";
        return QUrl();
    }

    if (url.port() == -1) {
        const auto port = secure ? QtCoap::DefaultSecurePort : QtCoap::DefaultPort;
        finalizedUrl.setPort(port);
    }

    return finalizedUrl;
}

/*!
    \internal

    For QSharedDataPointer.
*/
QCoapRequestPrivate* QCoapRequest::d_func()
{
    return static_cast<QCoapRequestPrivate*>(d_ptr.data());
}

/*!
    \internal

    Creates a copy of \a other request and sets \a method as its request method.
    Adjusts the request URL based on \a isSecure parameter.
*/
QCoapRequest
QCoapRequestPrivate::createRequest(const QCoapRequest &other, QtCoap::Method method, bool isSecure)
{
    QCoapRequest request(other);
    request.d_func()->method = method;
    request.d_func()->adjustUrl(isSecure);
    return request;
}

QT_END_NAMESPACE
