// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "qmlcoapmulticastclient.h"

#include <QCoapResourceDiscoveryReply>
#include <QLoggingCategory>

Q_STATIC_LOGGING_CATEGORY(lcCoapClient, "qt.coap.client")

//! [ctor]
QmlCoapMulticastClient::QmlCoapMulticastClient(QObject *parent)
    : QCoapClient(QtCoap::SecurityMode::NoSecurity, parent)
{
    connect(this, &QCoapClient::finished, this,
            [this](QCoapReply *reply) {
                if (reply) {
                    emit finished(static_cast<int>(reply->errorReceived()));
                    reply->deleteLater();
                    if (m_reply == reply) {
                        m_reply = nullptr;
                        emit isDiscoveringChanged();
                    }
                } else {
                    qCWarning(lcCoapClient, "Something went wrong, received a null reply");
                }
            });

    connect(this, &QCoapClient::error, this,
            [this](QCoapReply *, QtCoap::Error err) {
                emit finished(static_cast<int>(err));
            });
}
//! [ctor]

//! [discover_custom]
void QmlCoapMulticastClient::discover(const QString &host, int port, const QString &discoveryPath)
{
    QUrl url;
    url.setHost(host);
    url.setPort(port);

    m_reply = QCoapClient::discover(url, discoveryPath);
    if (m_reply) {
        connect(m_reply, &QCoapResourceDiscoveryReply::discovered,
                this, &QmlCoapMulticastClient::onDiscovered);
        emit isDiscoveringChanged();
    } else {
        qCWarning(lcCoapClient, "Discovery request failed.");
    }
}
//! [discover_custom]

//! [discover_group]
void QmlCoapMulticastClient::discover(QtCoap::MulticastGroup group, int port,
                                      const QString &discoveryPath)
{
    m_reply = QCoapClient::discover(group, port, discoveryPath);
    if (m_reply) {
        connect(m_reply, &QCoapResourceDiscoveryReply::discovered,
                this, &QmlCoapMulticastClient::onDiscovered);
        emit isDiscoveringChanged();
    } else {
        qCWarning(lcCoapClient, "Discovery request failed.");
    }
}
//! [discover_group]

//! [stop_discovery]
void QmlCoapMulticastClient::stopDiscovery()
{
    if (m_reply)
        m_reply->abortRequest();
}
//! [stop_discovery]

bool QmlCoapMulticastClient::isDiscovering() const
{
    return m_reply && !m_reply->isFinished();
}

//! [on_discovered]
void QmlCoapMulticastClient::onDiscovered(QCoapResourceDiscoveryReply *reply,
                                          const QList<QCoapResource> &resources)
{
    Q_UNUSED(reply)
    for (const auto &resource : resources)
        emit discovered(resource);
}
//! [on_discovered]

#include "moc_qmlcoapmulticastclient.cpp"
