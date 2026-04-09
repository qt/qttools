// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapsecurityconfiguration.h"

#include <QtCore/QByteArray>
#include <QtCore/QIODevice>
#include <QtCore/QList>
#include <QtNetwork/QSslCertificate>

QT_BEGIN_NAMESPACE

class QCoapPrivateKeyPrivate : public QSharedData
{
public:
    QByteArray key;
    Qt::HANDLE opaqueKey = nullptr;
    QSsl::KeyAlgorithm algorithm = QSsl::Opaque;
    QSsl::EncodingFormat encodingFormat;
    QByteArray passPhrase;
};

class QCoapSecurityConfigurationPrivate : public QSharedData
{
public:
    QByteArray identity;
    QByteArray preSharedKey;
    QString defaultCipherString;
    QList<QSslCertificate> caCertificates;
    QList<QSslCertificate> localCertificateChain;
    QCoapPrivateKey privateKey;
};

/*!
    \class QCoapPrivateKey
    \inmodule QtCoap

    \brief The QCoapPrivateKey class provides an interface for managing
    CoAP security keys.

    A QCoapPrivateKey packages a private key used in negotiating CoAP connections
    securely. It holds the information required for authentication using
    \c pre-shared keys and X.509 certificates.
*/

/*!
    \fn void QCoapPrivateKey::swap(QCoapPrivateKey &other)

    Swaps this private key with \a other. This operation is very fast and never fails.
*/

/*!
    Constructs an empty instance of QCoapPrivateKey.
*/
QCoapPrivateKey::QCoapPrivateKey()
    : d(new QCoapPrivateKeyPrivate)
{
}

/*!
    Constructs a QCoapPrivateKey from the byte array \a key using
    the specified \a algorithm and encoding \a format.

    If the key is encrypted then \a passPhrase is required to decrypt it.
*/
QCoapPrivateKey::QCoapPrivateKey(const QByteArray &key, QSsl::KeyAlgorithm algorithm,
                                 QSsl::EncodingFormat format, const QByteArray &passPhrase)
    : d(new QCoapPrivateKeyPrivate)
{
    d->key = key;
    d->opaqueKey = nullptr;
    d->algorithm = algorithm;
    d->encodingFormat = format;
    d->passPhrase = passPhrase;
}

/*!
    Constructs a QCoapPrivateKey from a native key \a handle.
*/
QCoapPrivateKey::QCoapPrivateKey(const Qt::HANDLE &handle)
    : d(new QCoapPrivateKeyPrivate)
{
    d->opaqueKey = handle;
    d->algorithm = QSsl::Opaque;
}

/*!
    Copies the contents of \a other into this key, making the two keys
    identical.
*/
QCoapPrivateKey::QCoapPrivateKey(const QCoapPrivateKey &other)
    : d(other.d)
{
}

/*!
    Move-constructs a QCoapPrivateKey, making it point to the same
    object as \a other was pointing to.
*/
QCoapPrivateKey::QCoapPrivateKey(QCoapPrivateKey &&other) noexcept
    : d(other.d)
{
    other.d = nullptr;
}

/*!
    Releases any resources held by QCoapPrivateKey.
*/
QCoapPrivateKey::~QCoapPrivateKey()
{
}

/*!
    Copies the contents of \a other into this key, making the two keys
    identical.

    Returns a reference to this QCoapPrivateKey.
*/
QCoapPrivateKey &QCoapPrivateKey::operator=(const QCoapPrivateKey &other)
{
    d = other.d;
    return *this;
}

/*!
    Returns \c true if the private key is null, returns \c false otherwise.
*/
bool QCoapPrivateKey::isNull() const
{
    return d->algorithm == QSsl::Opaque ? !d->opaqueKey : d->key.isEmpty();
}

/*!
    Returns the encoded private key.
*/
QByteArray QCoapPrivateKey::key() const
{
    return d->key;
}

/*!
    Returns a pointer to the native key handle.
*/
Qt::HANDLE QCoapPrivateKey::handle() const
{
    return d->opaqueKey;
}

/*!
    Returns the key algorithm.
*/
QSsl::KeyAlgorithm QCoapPrivateKey::algorithm() const
{
    return d->algorithm;
}

/*!
    Returns the encoding format of the key.
*/
QSsl::EncodingFormat QCoapPrivateKey::encodingFormat() const
{
    return d->encodingFormat;
}

/*!
    Returns the passphrase for the key.
*/
QByteArray QCoapPrivateKey::passPhrase() const
{
    return  d->passPhrase;
}

/*!
    \class QCoapSecurityConfiguration
    \inmodule QtCoap

    \brief The QCoapSecurityConfiguration class holds configuration
    options during the authentication process.

    It holds information such as client identity, pre shared key, information
    about certificates, and so on.
*/


/*!
    \fn void QCoapSecurityConfiguration::swap(QCoapSecurityConfiguration &other)

    Swaps this security configuration with \a other. This operation is very fast
    and never fails.
*/

/*!
    Constructs a new QCoapSecurityConfiguration.
*/
QCoapSecurityConfiguration::QCoapSecurityConfiguration()
    : d(new QCoapSecurityConfigurationPrivate)
{
}

/*!
    Copies the configuration and state of \a other.
*/
QCoapSecurityConfiguration::QCoapSecurityConfiguration(const QCoapSecurityConfiguration &other)
    : d(other.d)
{
}

/*!
    Move-constructs a QCoapSecurityConfiguration, making it point to the same
    object as \a other was pointing to.
*/
QCoapSecurityConfiguration::QCoapSecurityConfiguration(
        QCoapSecurityConfiguration &&other) noexcept
    : d(other.d)
{
    other.d = nullptr;
}

/*!
    Copies the configuration and state of \a other.
*/
QCoapSecurityConfiguration &QCoapSecurityConfiguration::operator=(
        const QCoapSecurityConfiguration &other)
{
    d = other.d;
    return *this;
}

/*!
    Releases any resources held by QCoapSecurityConfiguration.
*/
QCoapSecurityConfiguration::~QCoapSecurityConfiguration()
{
}

/*!
    Sets the PSK client identity (to be advised to the server) to \a identity.

    \sa preSharedKeyIdentity()
*/
void QCoapSecurityConfiguration::setPreSharedKeyIdentity(const QByteArray &identity)
{
    d->identity = identity;
}

/*!
    Returns the PSK client identity.

    \sa setPreSharedKeyIdentity()
*/
QByteArray QCoapSecurityConfiguration::preSharedKeyIdentity() const
{
    return d->identity;
}

/*!
    Sets the pre shared key to \a preSharedKey.

    \sa preSharedKey()
*/
void QCoapSecurityConfiguration::setPreSharedKey(const QByteArray &preSharedKey)
{
    d->preSharedKey = preSharedKey;
}

/*!
    Returns the pre shared key.

    \sa setPreSharedKey()
*/
QByteArray QCoapSecurityConfiguration::preSharedKey() const
{
    return d->preSharedKey;
}

/*!
    Sets the SSL cipher string to \a cipherString.

    The security back-end (for example OpenSSL) might not include ciphers required
    for \l{https://tools.ietf.org/html/rfc7252#section-9}{RFC 7252} by default.
    This method specifies which ciphers the back-end should use.
    For example to enable CCM ciphers required by RFC, "AESCCM" can be passed
    as \a cipherString.

    See the \l{https://www.openssl.org/docs/manmaster/man1/ciphers.html#CIPHER-STRINGS}
    {OpenSSL docs} for more information about cipher strings.

    \sa defaultCipherString()
*/
void QCoapSecurityConfiguration::setDefaultCipherString(const QString &cipherString)
{
    d->defaultCipherString = cipherString;
}

/*!
    Returns the default cipher string.

    \sa setDefaultCipherString()
*/
QString QCoapSecurityConfiguration::defaultCipherString() const
{
    return d->defaultCipherString;
}

/*!
    Sets \a certificates as the certificate authority database for the connection.

    \sa caCertificates()
*/
void QCoapSecurityConfiguration::setCaCertificates(const QList<QSslCertificate> &certificates)
{
    d->caCertificates = certificates;
}

/*!
    Returns this connection's certificate authority certificate database.

    \sa setCaCertificates()
*/
QList<QSslCertificate> QCoapSecurityConfiguration::caCertificates() const
{
    return d->caCertificates;
}

/*!
    Sets \a localChain as the certificate chain to present to the peer
    during the handshake.

    \sa localCertificateChain()
 */
void QCoapSecurityConfiguration::setLocalCertificateChain(const QList<QSslCertificate> &localChain)
{
    d->localCertificateChain = localChain;
}

/*!
    Returns the certificate chain to be presented to the peer during the handshake.

    \sa setLocalCertificateChain()
*/
QList<QSslCertificate> QCoapSecurityConfiguration::localCertificateChain() const
{
    return  d->localCertificateChain;
}

/*!
    Sets the connection's private key to \a key.

    \sa privateKey(), setLocalCertificateChain()
*/
void QCoapSecurityConfiguration::setPrivateKey(const QCoapPrivateKey &key)
{
    d->privateKey = key;
}

/*!
    Returns the private key assigned to the connection.

    \sa setPrivateKey(), localCertificateChain()
*/
QCoapPrivateKey QCoapSecurityConfiguration::privateKey() const
{
    return d->privateKey;
}

QT_END_NAMESPACE
