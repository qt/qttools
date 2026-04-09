// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPCLIENT_P_H
#define QCOAPCLIENT_P_H

#include <QtCoap/qcoapclient.h>
#include <QtCore/qthread.h>
#include <QtCore/qpointer.h>
#include <private/qobject_p.h>

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

class Q_AUTOTEST_EXPORT QCoapClientPrivate : public QObjectPrivate
{
public:
    QCoapClientPrivate(QCoapProtocol *protocol, QCoapConnection *connection);
    ~QCoapClientPrivate();

    QCoapProtocol *protocol = nullptr;
    QCoapConnection *connection = nullptr;
    QThread *workerThread = nullptr;
#if QT_CONFIG(networkinterface)
    QNetworkInterface interface;
#endif

    QCoapReply *sendRequest(const QCoapRequest &request);
    QCoapResourceDiscoveryReply *sendDiscovery(const QCoapRequest &request);
    bool send(QCoapReply *reply);

    void setConnection(QCoapConnection *customConnection);

    Q_DECLARE_PUBLIC(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_P_H
