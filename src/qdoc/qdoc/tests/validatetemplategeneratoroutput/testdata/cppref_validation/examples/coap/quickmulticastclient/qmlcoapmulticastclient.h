// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef QMLCOAPMULTICASTCLIENT_H
#define QMLCOAPMULTICASTCLIENT_H

#include <QtCoap/qcoapnamespace.h>
#include <QCoapClient>
#include <QCoapResource>

#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE
class QCoapResourceDiscoveryReply;
QT_END_NAMESPACE

//! [coap_resource]
class QmlCoapResource : public QCoapResource
{
    Q_GADGET
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString host READ hostStr)
    Q_PROPERTY(QString path READ path)

    QML_ANONYMOUS
public:
    QmlCoapResource() : QCoapResource() {}
    QmlCoapResource(const QCoapResource &resource)
        : QCoapResource(resource) {}

    QString hostStr() const { return host().toString(); }
};
//! [coap_resource]
Q_DECLARE_METATYPE(QmlCoapResource)

//! [coap_client]
class QmlCoapMulticastClient : public QCoapClient
{
    Q_OBJECT

    Q_PROPERTY(bool isDiscovering READ isDiscovering NOTIFY isDiscoveringChanged)

    QML_NAMED_ELEMENT(CoapMulticastClient)
public:
    QmlCoapMulticastClient(QObject *parent = nullptr);

    Q_INVOKABLE void discover(const QString &host, int port, const QString &discoveryPath);
    Q_INVOKABLE void discover(QtCoap::MulticastGroup group, int port, const QString &discoveryPath);
    Q_INVOKABLE void stopDiscovery();

    bool isDiscovering() const;

Q_SIGNALS:
    void discovered(const QmlCoapResource &resource);
    void finished(int error);
    // The bool parameter is not provided, because the signal is only used by
    // the QML property system, and it does not use the passed value anyway.
    void isDiscoveringChanged();

public slots:
    void onDiscovered(QCoapResourceDiscoveryReply *reply, const QList<QCoapResource> &resources);

private:
    QCoapResourceDiscoveryReply *m_reply = nullptr;
};
//! [coap_client]

//! [coap_namespace]
namespace QCoapForeignNamespace
{
    Q_NAMESPACE
    QML_FOREIGN_NAMESPACE(QtCoap)
    QML_NAMED_ELEMENT(QtCoap)
}
//! [coap_namespace]

#endif // QMLCOAPMULTICASTCLIENT_H
