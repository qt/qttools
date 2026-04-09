// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPINTERNALREPLY_H
#define QCOAPINTERNALREPLY_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <private/qcoapinternalmessage_p.h>
#include <QtNetwork/qhostaddress.h>

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

class QCoapInternalReplyPrivate;
class Q_AUTOTEST_EXPORT QCoapInternalReply : public QCoapInternalMessage
{
    Q_OBJECT
public:
    explicit QCoapInternalReply(QObject *parent = nullptr);

    static QCoapInternalReply *createFromFrame(const QByteArray &frame, QObject *parent = nullptr);
    void appendData(const QByteArray &data);
    bool hasMoreBlocksToSend() const;
    int nextBlockToSend() const;

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption &option) override;
    void setSenderAddress(const QHostAddress &address);

    QtCoap::ResponseCode responseCode() const;
    QHostAddress senderAddress() const;

private:
    Q_DECLARE_PRIVATE(QCoapInternalReply)
};

class Q_AUTOTEST_EXPORT QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate() = default;

    QtCoap::ResponseCode responseCode = QtCoap::ResponseCode::InvalidCode;
    QHostAddress senderAddress;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
