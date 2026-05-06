[QtCoap](qtcoap-module.md)> QCoapReply

**Contents**

- [Public Functions](#public-functions)
- [Signals](#signals)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# QCoapReply

class QCoapReply

The QCoapReply class holds the data of a CoAP reply.

| Key | Value |
| --- | --- |
| Header | `QCoapReply` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |
| Inherited By | [QCoapResourceDiscoveryReply](qcoapresourcediscoveryreply.md)|

- [List of all members, including inherited members](qcoapreply-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Public Functions

| Member | Description |
| --- | --- |
| `~QCoapReply()` |  |
| `void abortRequest()` |  |
| `QtCoap::Error errorReceived() const` |  |
| `bool isAborted() const` |  |
| `bool isFinished() const` |  |
| `bool isRunning() const` |  |
| `bool isSuccessful() const` |  |
| `QCoapMessage message() const` |  |
| `QtCoap::Method method() const` |  |
| `QCoapRequest request() const` |  |
| `QtCoap::ResponseCode responseCode() const` |  |
| `QUrl url() const` |  |

## Signals

| Member | Description |
| --- | --- |
| `void aborted(const QCoapToken &token)` |  |
| `void error(QCoapReply *reply, QtCoap::Error error)` |  |
| `void finished(QCoapReply *reply)` |  |
| `void notified(QCoapReply *reply, const QCoapMessage &message)` |  |

## Detailed Description
The [QCoapReply](qcoapreply.md) contains data related to a request sent with the [QCoapClient](qcoapclient.md).
The [finished()](qcoapreply.md#finished) signal is emitted when the response is fully received or when the request fails.
For _Observe_ requests specifically, the [notified()](qcoapreply.md#notified) signal is emitted whenever a notification is received.

## Member Function Documentation

<a id="dtor.QCoapReply"></a>
### ~QCoapReply()

Destroys the [QCoapReply](qcoapreply.md) and aborts the request if its response has not yet been received.
<a id="abortRequest"></a>
### void abortRequest()

Aborts the request immediately and emits the [aborted(const QCoapToken &token)](qcoapreply.md#aborted) signal if the request was not finished.
<a id="aborted"></a>
### void aborted(const QCoapToken &token)

This signal is emitted when the request is aborted or the reply is deleted. Its _token_ parameter is the token of the exchange that has been aborted.
> **Note:** If the QCoapReply is deleted while not finished, both aborted() and finished() signal will be emitted immediately before the QCoapReply is destroyed. Given the QCoapReply may have been deleted when receiving the signal, you should not rely on the sender() object to be still valid.

**See also** [finished()](qcoapreply.md#finished) and [error()](qcoapreply.md#error).

<a id="error"></a>
### void error(QCoapReply *reply, QtCoap::Error error)

This signal is emitted whenever an error occurs and is followed by the [finished()](qcoapreply.md#finished) signal.
Its _reply_ parameters is the [QCoapReply](qcoapreply.md) itself for convenience, and the _error_ parameter is the error received.
**See also** [finished()](qcoapreply.md#finished) and [aborted()](qcoapreply.md#aborted).

<a id="errorReceived"></a>
### QtCoap::Error errorReceived() const

Returns the error of the reply or QCoapReply::NoError if there is no error.
<a id="finished"></a>
### void finished(QCoapReply *reply)

This signal is emitted whenever the corresponding request finished, whether successfully or not. When a resource is observed, this signal will only be emitted once, when the observation ends.
The _reply_ parameter is the [QCoapReply](qcoapreply.md) itself for convenience.
> **Note:** If the QCoapReply is deleted while not finished, both aborted() and finished() signal will be emitted immediately before the QCoapReply is destroyed. Given the QCoapReply may have been deleted when receiving the signal, you should not rely on the reply to be still valid.

**See also** [QCoapClient::finished()](qcoapclient.md#finished), [isFinished()](qcoapreply.md#isFinished), [notified()](qcoapreply.md#notified), and [aborted()](qcoapreply.md#aborted).

<a id="isAborted"></a>
### bool isAborted() const

Returns `true` if the request has been aborted.
<a id="isFinished"></a>
### bool isFinished() const

Returns `true` if the request is finished.
**See also** [finished()](qcoapreply.md#finished).

<a id="isRunning"></a>
### bool isRunning() const

Returns `true` if the request is running.
<a id="isSuccessful"></a>
### bool isSuccessful() const

Returns `true` if the request finished with no error.
<a id="message"></a>
### QCoapMessage message() const

Returns the contained message.
<a id="method"></a>
### QtCoap::Method method() const

Returns the method of the associated request.
<a id="notified"></a>
### void notified(QCoapReply *reply, const QCoapMessage &message)

This signal is emitted whenever a notification is received from an observed resource.
Its _message_ parameter is a [QCoapMessage](qcoapmessage.md) containing the payload and the message details. The _reply_ parameter is the [QCoapReply](qcoapreply.md) itself for convenience.
**See also** [QCoapClient::finished()](qcoapclient.md#finished), [isFinished()](qcoapreply.md#isFinished), [finished()](qcoapreply.md#finished), and [notified()](qcoapreply.md#notified).

<a id="request"></a>
### QCoapRequest request() const

Returns the associated request.
<a id="responseCode"></a>
### QtCoap::ResponseCode responseCode() const

Returns the response code of the request.
<a id="url"></a>
### QUrl url() const

Returns the target uri of the associated request.

---

*Built with QDoc's template engine.*
