# QCoapRequest

class QCoapRequest

The QCoapRequest class holds a CoAP request. This request can be sent with QCoapClient.

| Key | Value |
| --- | --- |
| Header | `QCoapRequest` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |
| Inherits | [QCoapMessage](qcoapmessage.md)|

- [List of all members, including inherited members](qcoaprequest-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Public Functions

| Member | Description |
| --- | --- |
| `QCoapRequest(const char *url, QCoapMessage::Type type = Type::NonConfirmable)` |  |
| `QCoapRequest(const QUrl &url = QUrl(), QCoapMessage::Type type = Type::NonConfirmable, const QUrl &proxyUrl = QUrl())` |  |
| `QCoapRequest(const QCoapRequest &other)` |  |
| `~QCoapRequest()` |  |
| `void enableObserve()` |  |
| `bool isObserve() const` |  |
| `QtCoap::Method method() const` |  |
| `QUrl proxyUrl() const` |  |
| `void setProxyUrl(const QUrl &proxyUrl)` |  |
| `void setUrl(const QUrl &url)` |  |
| `QUrl url() const` |  |
| `QCoapRequest & operator=(const QCoapRequest &other)` |  |

The [QCoapRequest](qcoaprequest.md) contains data needed to make CoAP frames that can be sent to the URL it holds.

## Member Function Documentation

<a id="QCoapRequest-1"></a>
### QCoapRequest(const char *url, QCoapMessage::Type type = Type::NonConfirmable)

Constructs a [QCoapRequest](qcoaprequest.md) from a string literal
<a id="QCoapRequest-2"></a>
### QCoapRequest(const QUrl &url = QUrl(), QCoapMessage::Type type = Type::NonConfirmable, const QUrl &proxyUrl = QUrl())

Constructs a [QCoapRequest](qcoaprequest.md) object with the target _url_, the proxy URL _proxyUrl_ and the _type_ of the message.
<a id="QCoapRequest"></a>
### QCoapRequest(const QCoapRequest &other)

Constructs a copy of the _other_ [QCoapRequest](qcoaprequest.md).
<a id="dtor.QCoapRequest"></a>
### ~QCoapRequest()

Destroys the [QCoapRequest](qcoaprequest.md).
<a id="enableObserve"></a>
### void enableObserve()

Sets the observe to `true` to make an observe request.
**See also** [isObserve()](qcoaprequest.md#isObserve).

<a id="isObserve"></a>
### bool isObserve() const

Returns `true` if the request is an observe request.
**See also** [enableObserve()](qcoaprequest.md#enableObserve).

<a id="method"></a>
### QtCoap::Method method() const

Returns the method of the request.
<a id="proxyUrl"></a>
### QUrl proxyUrl() const

Returns the proxy URI of the request. The request shall be sent directly if this is invalid.
**See also** [setProxyUrl()](qcoaprequest.md#setProxyUrl).

<a id="setProxyUrl"></a>
### void setProxyUrl(const QUrl &proxyUrl)

Sets the proxy URI of the request to the given _proxyUrl_.
**See also** [proxyUrl()](qcoaprequest.md#proxyUrl).

<a id="setUrl"></a>
### void setUrl(const QUrl &url)

Sets the target URI of the request to the given _url_.
If not indicated, the scheme of the URL will default to 'coap', and its port will default to 5683.
**See also** [url()](qcoaprequest.md#url).

<a id="url"></a>
### QUrl url() const

Returns the target URI of the request.
**See also** [setUrl()](qcoaprequest.md#setUrl).

<a id="operator-eq"></a>
### QCoapRequest & operator=(const QCoapRequest &other)

Creates a copy of _other_.
---

*Built with QDoc's template engine.*
