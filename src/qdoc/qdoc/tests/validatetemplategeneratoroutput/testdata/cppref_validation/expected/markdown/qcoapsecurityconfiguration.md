[QtCoap](qtcoap-module.md)> QCoapSecurityConfiguration
**Contents**

- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# QCoapSecurityConfiguration

class QCoapSecurityConfiguration

The QCoapSecurityConfiguration class holds configuration options during the authentication process.

| Key | Value |
| --- | --- |
| Header | `QCoapSecurityConfiguration` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |

- [List of all members, including inherited members](qcoapsecurityconfiguration-members.md)


## Public Functions

| Member | Description |
| --- | --- |
| `QCoapSecurityConfiguration()` |  |
| `QCoapSecurityConfiguration(const QCoapSecurityConfiguration &other)` |  |
| `QCoapSecurityConfiguration(QCoapSecurityConfiguration &&other)` |  |
| `~QCoapSecurityConfiguration()` |  |
| `QList<QSslCertificate> caCertificates() const` |  |
| `QString defaultCipherString() const` |  |
| `QList<QSslCertificate> localCertificateChain() const` |  |
| `QByteArray preSharedKey() const` |  |
| `QByteArray preSharedKeyIdentity() const` |  |
| `QCoapPrivateKey privateKey() const` |  |
| `void setCaCertificates(const QList<QSslCertificate> &certificates)` |  |
| `void setDefaultCipherString(const QString &cipherString)` |  |
| `void setLocalCertificateChain(const QList<QSslCertificate> &localChain)` |  |
| `void setPreSharedKey(const QByteArray &preSharedKey)` |  |
| `void setPreSharedKeyIdentity(const QByteArray &identity)` |  |
| `void setPrivateKey(const QCoapPrivateKey &key)` |  |
| `void swap(QCoapSecurityConfiguration &other)` |  |
| `QCoapSecurityConfiguration & operator=(const QCoapSecurityConfiguration &other)` |  |

## Detailed Description
It holds information such as client identity, pre shared key, information about certificates, and so on.

## Member Function Documentation

<a id="QCoapSecurityConfiguration"></a>
### QCoapSecurityConfiguration()

Constructs a new [QCoapSecurityConfiguration](qcoapsecurityconfiguration.md).
<a id="QCoapSecurityConfiguration-2"></a>
### QCoapSecurityConfiguration(const QCoapSecurityConfiguration &other)

Copies the configuration and state of _other_.
<a id="QCoapSecurityConfiguration-1"></a>
### QCoapSecurityConfiguration(QCoapSecurityConfiguration &&other)

Move-constructs a [QCoapSecurityConfiguration](qcoapsecurityconfiguration.md), making it point to the same object as _other_ was pointing to.
<a id="dtor.QCoapSecurityConfiguration"></a>
### ~QCoapSecurityConfiguration()

Releases any resources held by [QCoapSecurityConfiguration](qcoapsecurityconfiguration.md).
<a id="caCertificates"></a>
### QList<QSslCertificate> caCertificates() const

Returns this connection's certificate authority certificate database.
**See also** [setCaCertificates()](qcoapsecurityconfiguration.md#setCaCertificates).

<a id="defaultCipherString"></a>
### QString defaultCipherString() const

Returns the default cipher string.
**See also** [setDefaultCipherString()](qcoapsecurityconfiguration.md#setDefaultCipherString).

<a id="localCertificateChain"></a>
### QList<QSslCertificate> localCertificateChain() const

Returns the certificate chain to be presented to the peer during the handshake.
**See also** [setLocalCertificateChain()](qcoapsecurityconfiguration.md#setLocalCertificateChain).

<a id="preSharedKey"></a>
### QByteArray preSharedKey() const

Returns the pre shared key.
**See also** [setPreSharedKey()](qcoapsecurityconfiguration.md#setPreSharedKey).

<a id="preSharedKeyIdentity"></a>
### QByteArray preSharedKeyIdentity() const

Returns the PSK client identity.
**See also** [setPreSharedKeyIdentity()](qcoapsecurityconfiguration.md#setPreSharedKeyIdentity).

<a id="privateKey"></a>
### QCoapPrivateKey privateKey() const

Returns the private key assigned to the connection.
**See also** [setPrivateKey()](qcoapsecurityconfiguration.md#setPrivateKey)[localCertificateChain()](qcoapsecurityconfiguration.md#localCertificateChain).

<a id="setCaCertificates"></a>
### void setCaCertificates(const QList<QSslCertificate> &certificates)

Sets _certificates_ as the certificate authority database for the connection.
**See also** [caCertificates()](qcoapsecurityconfiguration.md#caCertificates).

<a id="setDefaultCipherString"></a>
### void setDefaultCipherString(const QString &cipherString)

Sets the SSL cipher string to _cipherString_.
The security back-end (for example OpenSSL) might not include ciphers required for [RFC 7252](https://tools.ietf.org/html/rfc7252#section-9) by default. This method specifies which ciphers the back-end should use. For example to enable CCM ciphers required by RFC, "AESCCM" can be passed as _cipherString_.
See the [OpenSSL docs](https://www.openssl.org/docs/manmaster/man1/ciphers.html#CIPHER-STRINGS) for more information about cipher strings.
**See also** [defaultCipherString()](qcoapsecurityconfiguration.md#defaultCipherString).

<a id="setLocalCertificateChain"></a>
### void setLocalCertificateChain(const QList<QSslCertificate> &localChain)

Sets _localChain_ as the certificate chain to present to the peer during the handshake.
**See also** [localCertificateChain()](qcoapsecurityconfiguration.md#localCertificateChain).

<a id="setPreSharedKey"></a>
### void setPreSharedKey(const QByteArray &preSharedKey)

Sets the pre shared key to _preSharedKey_.
**See also** [preSharedKey()](qcoapsecurityconfiguration.md#preSharedKey).

<a id="setPreSharedKeyIdentity"></a>
### void setPreSharedKeyIdentity(const QByteArray &identity)

Sets the PSK client identity (to be advised to the server) to _identity_.
**See also** [preSharedKeyIdentity()](qcoapsecurityconfiguration.md#preSharedKeyIdentity).

<a id="setPrivateKey"></a>
### void setPrivateKey(const QCoapPrivateKey &key)

Sets the connection's private key to _key_.
**See also** [privateKey()](qcoapsecurityconfiguration.md#privateKey)[setLocalCertificateChain()](qcoapsecurityconfiguration.md#setLocalCertificateChain).

<a id="swap"></a>
### void swap(QCoapSecurityConfiguration &other)

Swaps this security configuration with _other_. This operation is very fast and never fails.
<a id="operator-eq"></a>
### QCoapSecurityConfiguration & operator=(const QCoapSecurityConfiguration &other)

Copies the configuration and state of _other_.

---

*Built with QDoc's template engine.*
