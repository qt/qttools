// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPREPLY_P_H
#define QCOAPREPLY_P_H

#include <QtCoap/qcoapreply.h>
#include <private/qcoapmessage_p.h>
#include <private/qiodevice_p.h>

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

class QHostAddress;
class Q_AUTOTEST_EXPORT QCoapReplyPrivate : public QIODevicePrivate
{
public:
    QCoapReplyPrivate(const QCoapRequest &request);

    void _q_setRunning(const QCoapToken &, QCoapMessageId);
    virtual void _q_setContent(const QHostAddress &sender, const QCoapMessage &, QtCoap::ResponseCode);
    void _q_setNotified();
    void _q_setObserveCancelled();
    void _q_setFinished(QtCoap::Error = QtCoap::Error::Ok);
    void _q_setError(QtCoap::ResponseCode code);
    void _q_setError(QtCoap::Error);

    static QCoapReply *createCoapReply(const QCoapRequest &request, QObject *parent = nullptr);

    QCoapRequest request;
    QCoapMessage message;
    QtCoap::ResponseCode responseCode = QtCoap::ResponseCode::InvalidCode;
    QtCoap::Error error = QtCoap::Error::Ok;
    bool isRunning = false;
    bool isFinished = false;
    bool isAborted = false;

    Q_DECLARE_PUBLIC(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_P_H
