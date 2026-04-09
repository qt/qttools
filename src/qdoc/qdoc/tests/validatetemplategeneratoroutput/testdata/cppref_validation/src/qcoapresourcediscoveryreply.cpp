// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qcoapresourcediscoveryreply_p.h"
#include "qcoapinternalreply_p.h"
#include "qcoapnamespace_p.h"

QT_BEGIN_NAMESPACE

QCoapResourceDiscoveryReplyPrivate::QCoapResourceDiscoveryReplyPrivate(const QCoapRequest &request) :
    QCoapReplyPrivate(request)
{
}

/*!
    \internal

    Updates the QCoapResourceDiscoveryReply object, its message and list of resources
    with data of the internal reply \a internalReply.
*/
void QCoapResourceDiscoveryReplyPrivate::_q_setContent(const QHostAddress &sender,
                                                       const QCoapMessage &msg,
                                                       QtCoap::ResponseCode code)
{
    Q_Q(QCoapResourceDiscoveryReply);

    if (q->isFinished())
        return;

    message = msg;
    responseCode = code;

    if (QtCoap::isError(responseCode)) {
        _q_setError(responseCode);
    } else {
        auto res = QCoapResourceDiscoveryReplyPrivate::resourcesFromCoreLinkList(sender,
                                                                                 message.payload());
        resources.append(res);
        emit q->discovered(q, res);
    }
}

/*!
    \class QCoapResourceDiscoveryReply
    \inmodule QtCoap

    \brief The QCoapResourceDiscoveryReply class holds the data of a CoAP reply
    for a resource discovery request.

    \reentrant

    This class is used for discovery requests. It emits the discovered()
    signal if and when resources are discovered. When using a multicast
    address for discovery, the discovered() signal will be emitted once
    for each response received.

    \note A QCoapResourceDiscoveryReply is a QCoapReply that stores also a list
    of QCoapResources.

    \sa QCoapClient, QCoapRequest, QCoapReply, QCoapResource
*/

/*!
    \fn void QCoapResourceDiscoveryReply::discovered(QCoapResourceDiscoveryReply *reply,
                                                     QList<QCoapResource> resources);

    This signal is emitted whenever a CoAP resource is discovered.

    The \a reply parameter contains a pointer to the reply that has just been
    received, and \a resources contains a list of resources that were discovered.

    \sa QCoapReply::finished()
*/

/*!
    \internal

    Constructs a new CoAP discovery reply from the \a request and sets \a parent
    as its parent.
*/
QCoapResourceDiscoveryReply::QCoapResourceDiscoveryReply(const QCoapRequest &request, QObject *parent) :
    QCoapReply(*new QCoapResourceDiscoveryReplyPrivate(request), parent)
{
}

/*!
    Returns the list of resources.
*/
QList<QCoapResource> QCoapResourceDiscoveryReply::resources() const
{
    Q_D(const QCoapResourceDiscoveryReply);
    return d->resources;
}

/*!
    \internal

    Decodes the \a data received from the \a sender to a list of QCoapResource
    objects. The \a data byte array contains the frame returned by the
    discovery request.
*/
QList<QCoapResource>
QCoapResourceDiscoveryReplyPrivate::resourcesFromCoreLinkList(const QHostAddress &sender,
                                                              const QByteArray &data)
{
    QList<QCoapResource> resourceList;

    QLatin1String quote = QLatin1String("\"");
    const QList<QByteArray> links = data.split(',');
    for (const QByteArray &link : links) {
        QCoapResource resource;
        resource.setHost(sender);

        const QList<QByteArray> parameterList = link.split(';');
        for (const QByteArray &parameter : parameterList) {
            QString parameterString = QString::fromUtf8(parameter);
            const qsizetype length = parameterString.size();
            if (parameter.startsWith('<'))
                resource.setPath(parameterString.mid(1, length - 2));
            else if (parameter.startsWith("title="))
                resource.setTitle(parameterString.mid(6).remove(quote));
            else if (parameter.startsWith("rt="))
                resource.setResourceType(parameterString.mid(3).remove(quote));
            else if (parameter.startsWith("if="))
                resource.setInterface(parameterString.mid(3).remove(quote));
            else if (parameter.startsWith("sz="))
                resource.setMaximumSize(parameterString.mid(3).remove(quote).toInt());
            else if (parameter.startsWith("ct="))
                resource.setContentFormat(parameterString.mid(3).remove(quote).toUInt());
            else if (parameter == "obs")
                resource.setObservable(true);
        }

        if (!resource.path().isEmpty())
            resourceList.push_back(resource);
    }

    return resourceList;
}

QT_END_NAMESPACE
