// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPRESOURCE_H
#define QCOAPRESOURCE_H

#include <QtCoap/qcoapglobal.h>
#include <QtCore/qshareddata.h>
#include <QtNetwork/qhostaddress.h>

#ifdef interface
#  undef interface
#endif

QT_BEGIN_NAMESPACE

class QCoapResourcePrivate;

class Q_COAP_EXPORT QCoapResource
{
public:
    QCoapResource();
    QCoapResource(const QCoapResource &other);
    ~QCoapResource();
    QCoapResource &operator =(const QCoapResource &other);

    void swap(QCoapResource &other) noexcept;

    QHostAddress host() const;
    QString path() const;
    QString title() const;
    bool observable() const;
    QString resourceType() const;
    QString interface() const;
    int maximumSize() const;
    uint contentFormat() const;

    void setHost(const QHostAddress &host);
    void setPath(const QString &path);
    void setTitle(const QString &title);
    void setObservable(bool observable);
    void setResourceType(const QString &resourceType);
    void setInterface(const QString &interface);
    void setMaximumSize(int maximumSize);
    void setContentFormat(uint contentFormat);

private:
    QSharedDataPointer<QCoapResourcePrivate> d;
};

Q_DECLARE_SHARED(QCoapResource)

QT_END_NAMESPACE

#endif // QCOAPRESOURCE_H
