// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPSECURITYCONFIGURATION_H
#define QCOAPSECURITYCONFIGURATION_H

#include <QtCoap/qcoapglobal.h>

#include <QtCore/qglobal.h>
#include <QtCore/QSharedDataPointer>
#include <QtNetwork/QSslCertificate>

QT_BEGIN_NAMESPACE

class QCoapPrivateKeyPrivate;
class Q_COAP_EXPORT QCoapPrivateKey
{
public:
    QCoapPrivateKey();
    QCoapPrivateKey(const QByteArray &key, QSsl::KeyAlgorithm algorithm,
                    QSsl::EncodingFormat format = QSsl::Pem,
                    const QByteArray &passPhrase = QByteArray());
    QCoapPrivateKey(const Qt::HANDLE &handle);

    QCoapPrivateKey(const QCoapPrivateKey& other);
    QCoapPrivateKey(QCoapPrivateKey&& other) noexcept;
    ~QCoapPrivateKey();

    QCoapPrivateKey &operator=(QCoapPrivateKey &&other) noexcept
    { swap(other); return *this; }
    QCoapPrivateKey &operator=(const QCoapPrivateKey &other);

    void swap(QCoapPrivateKey &other) noexcept
    { d.swap(other.d); }

    bool isNull() const;

    QByteArray key() const;
    Qt::HANDLE handle() const;
    QSsl::KeyAlgorithm algorithm() const;
    QSsl::EncodingFormat encodingFormat() const;
    QByteArray passPhrase() const;
private:
     QSharedDataPointer<QCoapPrivateKeyPrivate> d;
};

class QCoapSecurityConfigurationPrivate;
class Q_COAP_EXPORT QCoapSecurityConfiguration
{
public:
    QCoapSecurityConfiguration();
    QCoapSecurityConfiguration(const QCoapSecurityConfiguration &other);
    ~QCoapSecurityConfiguration();

    QCoapSecurityConfiguration(QCoapSecurityConfiguration &&other) noexcept;
    QCoapSecurityConfiguration &operator=(QCoapSecurityConfiguration &&other) noexcept
    { swap(other); return *this; }
    QCoapSecurityConfiguration &operator=(const QCoapSecurityConfiguration &other);

    void swap(QCoapSecurityConfiguration &other) noexcept
    { d.swap(other.d); }

    void setPreSharedKeyIdentity(const QByteArray &preSharedKeyIdentity);
    QByteArray preSharedKeyIdentity() const;

    void setPreSharedKey(const QByteArray &preSharedKey);
    QByteArray preSharedKey() const;

    void setDefaultCipherString(const QString &cipherString);
    QString defaultCipherString() const;

    void setCaCertificates(const QList<QSslCertificate> &certificates);
    QList<QSslCertificate> caCertificates() const;

    void setLocalCertificateChain(const QList<QSslCertificate> &localChain);
    QList<QSslCertificate> localCertificateChain() const;

    void setPrivateKey(const QCoapPrivateKey &key);
    QCoapPrivateKey privateKey() const;

private:
     QSharedDataPointer<QCoapSecurityConfigurationPrivate> d;
};

Q_DECLARE_SHARED(QCoapSecurityConfiguration)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCoapSecurityConfiguration)

#endif // QCOAPSECURITYCONFIGURATION_H
