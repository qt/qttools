// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPRESOURCEDISCOVERYREPLY_H
#define QCOAPRESOURCEDISCOVERYREPLY_H

#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapresource.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QCoapResourceDiscoveryReplyPrivate;
class Q_COAP_EXPORT QCoapResourceDiscoveryReply : public QCoapReply
{
    Q_OBJECT

public:
    QList<QCoapResource> resources() const;

Q_SIGNALS:
    void discovered(QCoapResourceDiscoveryReply *reply, QList<QCoapResource> resources);

private:
    explicit QCoapResourceDiscoveryReply(const QCoapRequest &request, QObject *parent = nullptr);
    friend class QCoapClientPrivate;

    Q_DECLARE_PRIVATE(QCoapResourceDiscoveryReply)
};

QT_END_NAMESPACE

#endif // QCOAPRESOURCEDISCOVERYREPLY_H
