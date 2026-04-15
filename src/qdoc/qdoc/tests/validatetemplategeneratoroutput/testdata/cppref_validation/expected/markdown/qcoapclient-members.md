<!--
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-->

# List of All Members for QCoapClient

This is the complete list of members for [QCoapClient](qcoapclient.md), including inherited members.

- [`QCoapClient(QtCoap::SecurityMode, QObject *)`](qcoapclient.md#QCoapClient)
- [`~QCoapClient()`](qcoapclient.md#dtor.QCoapClient)
- [`bindInterface() const : QNetworkInterface`](qcoapclient.md#bindInterface-prop)
- [`bindInterfaceChanged(const QNetworkInterface &)`](qcoapclient.md#bindInterface-prop)
- [`cancelObserve(QCoapReply *)`](qcoapclient.md#cancelObserve)
- [`cancelObserve(const QUrl &)`](qcoapclient.md#cancelObserve-1)
- [`deleteResource(const QCoapRequest &) : QCoapReply *`](qcoapclient.md#deleteResource)
- [`deleteResource(const QUrl &) : QCoapReply *`](qcoapclient.md#deleteResource-1)
- [`disconnect()`](qcoapclient.md#disconnect)
- [`discover(const QUrl &, const QString &) : QCoapResourceDiscoveryReply *`](qcoapclient.md#discover)
- [`discover(QtCoap::MulticastGroup, int, const QString &) : QCoapResourceDiscoveryReply *`](qcoapclient.md#discover-1)
- [`error(QCoapReply *, QtCoap::Error)`](qcoapclient.md#error)
- [`finished(QCoapReply *)`](qcoapclient.md#finished)
- [`get(const QCoapRequest &) : QCoapReply *`](qcoapclient.md#get)
- [`get(const QUrl &) : QCoapReply *`](qcoapclient.md#get-1)
- [`observe(const QCoapRequest &) : QCoapReply *`](qcoapclient.md#observe)
- [`observe(const QUrl &) : QCoapReply *`](qcoapclient.md#observe-1)
- [`post(const QCoapRequest &, const QByteArray &) : QCoapReply *`](qcoapclient.md#post)
- [`post(const QCoapRequest &, QIODevice *) : QCoapReply *`](qcoapclient.md#post-1)
- [`post(const QUrl &, const QByteArray &) : QCoapReply *`](qcoapclient.md#post-2)
- [`put(const QCoapRequest &, const QByteArray &) : QCoapReply *`](qcoapclient.md#put)
- [`put(const QCoapRequest &, QIODevice *) : QCoapReply *`](qcoapclient.md#put-1)
- [`put(const QUrl &, const QByteArray &) : QCoapReply *`](qcoapclient.md#put-2)
- [`responseToMulticastReceived(QCoapReply *, const QCoapMessage &, const QHostAddress &)`](qcoapclient.md#responseToMulticastReceived)
- [`setAckRandomFactor(double)`](qcoapclient.md#setAckRandomFactor)
- [`setAckTimeout(uint)`](qcoapclient.md#setAckTimeout)
- [`setBindInterface(const QNetworkInterface &)`](qcoapclient.md#bindInterface-prop)
- [`setBlockSize(quint16)`](qcoapclient.md#setBlockSize)
- [`setMaximumRetransmitCount(uint)`](qcoapclient.md#setMaximumRetransmitCount)
- [`setMaximumServerResponseDelay(uint)`](qcoapclient.md#setMaximumServerResponseDelay)
- [`setMinimumTokenSize(int)`](qcoapclient.md#setMinimumTokenSize)
- [`setSecurityConfiguration(const QCoapSecurityConfiguration &)`](qcoapclient.md#setSecurityConfiguration)
- [`setSocketOption(QAbstractSocket::SocketOption, const QVariant &)`](qcoapclient.md#setSocketOption)

---

*Built with QDoc's template engine.*
