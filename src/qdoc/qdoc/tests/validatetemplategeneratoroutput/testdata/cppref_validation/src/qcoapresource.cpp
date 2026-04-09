// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapresource_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCoapResource
    \inmodule QtCoap

    \brief The QCoapResource class holds information about a discovered
    resource.

    \reentrant

    The QCoapRequest contains data as the path and title of the resource
    and other ancillary information.

    \sa QCoapResourceDiscoveryReply
*/

/*!
    Constructs a new QCoapResource.
 */
QCoapResource::QCoapResource() :
    d(new QCoapResourcePrivate)
{
}

/*!
    Constructs a new CoAP resource as a copy of \a other, making the two
    resources identical.
*/
QCoapResource::QCoapResource(const QCoapResource &other) :
    d(other.d)
{
}

/*!
    Destroy the QCoapResource.
 */
QCoapResource::~QCoapResource()
{
}

/*!
    Copies \a other into this resource, making the two resources identical.
    Returns a reference to this QCoapResource.
*/
QCoapResource &QCoapResource::operator=(const QCoapResource &other)
{
    d = other.d;
    return *this;
}

/*!
    Swaps this resource with \a other. This operation is very fast and never fails.
 */
void QCoapResource::swap(QCoapResource &other) noexcept
{
    d.swap(other.d);
}

/*!
    Returns the host of the resource.

    \sa setHost()
 */
QHostAddress QCoapResource::host() const
{
    return d->host;
}

/*!
    Returns the path of the resource.

    \sa setPath()
 */
QString QCoapResource::path() const
{
    return d->path;
}

/*!
    Returns the title of the resource.

    \sa setTitle()
 */
QString QCoapResource::title() const
{
    return d->title;
}

/*!
    Returns \c true if the resource is observable

    \sa setObservable()
 */
bool QCoapResource::observable() const
{
    return d->observable;
}

/*!
    Returns the type of the resource.

    \sa setResourceType()
 */
QString QCoapResource::resourceType() const
{
    return d->resourceType;
}

/*!
    Returns the interface description of the resource.

    The Interface Description 'if' attribute is an opaque string used to
    provide a name or URI indicating a specific interface definition used
    to interact with the target resource. It is specified in
    \l{https://tools.ietf.org/html/rfc6690#section-3.2}{RFC 6690}.

    \sa setInterface()
 */
QString QCoapResource::interface() const
{
    return d->interface;
}

/*!
    Returns the maximum size of the resource.

    The maximum size estimate attribute 'sz' gives an indication of the
    maximum size of the resource representation returned by performing a
    GET on the target URI. It is specified in
    \l{https://tools.ietf.org/html/rfc6690#section-3.3}{RFC 6690}.

    \sa setMaximumSize()
 */
int QCoapResource::maximumSize() const
{
    return d->maximumSize;
}

/*!
    Returns the Content-Format code of the resource.

    The Content-Format code corresponds to the 'ct' attribute and provides a
    hint about the Content-Formats this resource returns. It is specified
    in \l{https://tools.ietf.org/html/rfc7252#section-7.2.1}{RFC 7252}.

    \sa setContentFormat()
 */
uint QCoapResource::contentFormat() const
{
    return d->contentFormat;
}

/*!
    Sets the host of the resource to \a host.

    \sa host()
 */
void QCoapResource::setHost(const QHostAddress &host)
{
    d->host = host;
}

/*!
    Sets the path of the resource to \a path.

    \sa path()
 */
void QCoapResource::setPath(const QString &path)
{
    d->path = path;
}

/*!
    Sets the title of the resource to \a title.

    \sa title()
 */
void QCoapResource::setTitle(const QString &title)
{
    d->title = title;
}

/*!
    Makes the resource observable if the \a observable
    parameter is \c true.

    \sa observable()
 */
void QCoapResource::setObservable(bool observable)
{
    d->observable = observable;
}

/*!
    Sets the resource type to \a resourceType.

    \sa resourceType()
 */
void QCoapResource::setResourceType(const QString &resourceType)
{
    d->resourceType = resourceType;
}

/*!
    Sets the interface of the resource to \a interface.

    \sa interface()
 */
void QCoapResource::setInterface(const QString &interface)
{
    d->interface = interface;
}

/*!
    Sets the maximum size of the resource to \a maximumSize.

    \sa maximumSize()
 */
void QCoapResource::setMaximumSize(int maximumSize)
{
    d->maximumSize = maximumSize;
}

/*!
    Sets the content format of the resource to \a contentFormat. The content
    format can be one of the content formats defined in \l {CoAP Content-Formats Registry}.

    \note CoAP supports common content formats such as XML, JSON, and so on, but
    these are text based and consequently heavy both in payload and in processing.
    One of the recommended content formats to use with CoAP is CBOR, which is
    designed to be used in such contexts.

    \sa contentFormat(), QCborStreamWriter, QCborStreamReader
 */
void QCoapResource::setContentFormat(uint contentFormat)
{
    d->contentFormat = contentFormat;
}

QT_END_NAMESPACE
