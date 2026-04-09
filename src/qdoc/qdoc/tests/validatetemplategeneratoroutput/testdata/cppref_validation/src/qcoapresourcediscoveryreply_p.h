// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPRESOURCEDISCOVERYREPLY_P_H
#define QCOAPRESOURCEDISCOVERYREPLY_P_H

#include <QtCore/qlist.h>
#include <QtCoap/qcoapresourcediscoveryreply.h>
#include <QtCoap/qcoapresource.h>
#include <private/qcoapreply_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QCoapResourceDiscoveryReplyPrivate : public QCoapReplyPrivate
{
public:
    QCoapResourceDiscoveryReplyPrivate(const QCoapRequest &request);

    void _q_setContent(const QHostAddress &sender, const QCoapMessage &, QtCoap::ResponseCode) override;

    static QList<QCoapResource> resourcesFromCoreLinkList(
            const QHostAddress &sender, const QByteArray &data);

    QList<QCoapResource> resources;

    Q_DECLARE_PUBLIC(QCoapResourceDiscoveryReply)
};

QT_END_NAMESPACE

#endif // QCOAPRESOURCEDISCOVERYREPLY_P_H
