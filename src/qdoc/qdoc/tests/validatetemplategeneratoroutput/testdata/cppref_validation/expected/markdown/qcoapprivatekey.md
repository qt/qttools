[QtCoap](qtcoap-module.md)> QCoapPrivateKey
**Contents**

- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# QCoapPrivateKey

class QCoapPrivateKey

The QCoapPrivateKey class provides an interface for managing CoAP security keys.

| Key | Value |
| --- | --- |
| Header | `QCoapPrivateKey` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |

- [List of all members, including inherited members](qcoapprivatekey-members.md)


## Public Functions

| Member | Description |
| --- | --- |
| `QCoapPrivateKey()` |  |
| `QCoapPrivateKey(const Qt::HANDLE &handle)` |  |
| `QCoapPrivateKey(const QByteArray &key, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format = QSsl::Pem, const QByteArray &passPhrase = QByteArray())` |  |
| `QCoapPrivateKey(const QCoapPrivateKey &other)` |  |
| `QCoapPrivateKey(QCoapPrivateKey &&other)` |  |
| `~QCoapPrivateKey()` |  |
| `QSsl::KeyAlgorithm algorithm() const` |  |
| `QSsl::EncodingFormat encodingFormat() const` |  |
| `Qt::HANDLE handle() const` |  |
| `bool isNull() const` |  |
| `QByteArray key() const` |  |
| `QByteArray passPhrase() const` |  |
| `void swap(QCoapPrivateKey &other)` |  |
| `QCoapPrivateKey & operator=(const QCoapPrivateKey &other)` |  |

## Detailed Description
A [QCoapPrivateKey](qcoapprivatekey.md) packages a private key used in negotiating CoAP connections securely. It holds the information required for authentication using `pre-shared` keys and X.509 certificates.

## Member Function Documentation

<a id="QCoapPrivateKey"></a>
### QCoapPrivateKey()

Constructs an empty instance of [QCoapPrivateKey](qcoapprivatekey.md).
<a id="QCoapPrivateKey-3"></a>
### QCoapPrivateKey(const Qt::HANDLE &handle)

Constructs a [QCoapPrivateKey](qcoapprivatekey.md) from a native key _handle_.
<a id="QCoapPrivateKey-4"></a>
### QCoapPrivateKey(const QByteArray &key, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format = QSsl::Pem, const QByteArray &passPhrase = QByteArray())

Constructs a [QCoapPrivateKey](qcoapprivatekey.md) from the byte array _key_ using the specified _algorithm_ and encoding _format_.
If the key is encrypted then _passPhrase_ is required to decrypt it.
<a id="QCoapPrivateKey-2"></a>
### QCoapPrivateKey(const QCoapPrivateKey &other)

Copies the contents of _other_ into this key, making the two keys identical.
<a id="QCoapPrivateKey-1"></a>
### QCoapPrivateKey(QCoapPrivateKey &&other)

Move-constructs a [QCoapPrivateKey](qcoapprivatekey.md), making it point to the same object as _other_ was pointing to.
<a id="dtor.QCoapPrivateKey"></a>
### ~QCoapPrivateKey()

Releases any resources held by [QCoapPrivateKey](qcoapprivatekey.md).
<a id="algorithm"></a>
### QSsl::KeyAlgorithm algorithm() const

Returns the key algorithm.
<a id="encodingFormat"></a>
### QSsl::EncodingFormat encodingFormat() const

Returns the encoding format of the key.
<a id="handle"></a>
### Qt::HANDLE handle() const

Returns a pointer to the native key handle.
<a id="isNull"></a>
### bool isNull() const

Returns `true` if the private key is null, returns `false` otherwise.
<a id="key"></a>
### QByteArray key() const

Returns the encoded private key.
<a id="passPhrase"></a>
### QByteArray passPhrase() const

Returns the passphrase for the key.
<a id="swap"></a>
### void swap(QCoapPrivateKey &other)

Swaps this private key with _other_. This operation is very fast and never fails.
<a id="operator-eq"></a>
### QCoapPrivateKey & operator=(const QCoapPrivateKey &other)

Copies the contents of _other_ into this key, making the two keys identical.
Returns a reference to this [QCoapPrivateKey](qcoapprivatekey.md).

---

*Built with QDoc's template engine.*
