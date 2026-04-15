# QCoapClient

class QCoapClient

The QCoapClient class allows the application to send CoAP requests and receive replies.

| Key | Value |
| --- | --- |
| Header | `QCoapClient` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |

- [List of all members, including inherited members](qcoapclient-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Properties

| Member | Description |
| --- | --- |
| `bindInterface : QNetworkInterface` | the network interface to be used by the socket |

## Public Functions

| Member | Description |
| --- | --- |
| `QCoapClient(QtCoap::SecurityMode securityMode = QtCoap::SecurityMode::NoSecurity, QObject *parent = nullptr)` |  |
| `~QCoapClient()` |  |
| `QNetworkInterface bindInterface() const` |  |
| `void cancelObserve(QCoapReply *notifiedReply)` |  |
| `void cancelObserve(const QUrl &url)` |  |
| `QCoapReply * deleteResource(const QCoapRequest &request)` |  |
| `QCoapReply * deleteResource(const QUrl &url)` |  |
| `void disconnect()` |  |
| `QCoapResourceDiscoveryReply * discover(const QUrl &url, const QString &discoveryPath = QLatin1String("/.well-known/core"))` |  |
| `QCoapResourceDiscoveryReply * discover(QtCoap::MulticastGroup group = QtCoap::MulticastGroup::AllCoapNodesIPv4, int port = QtCoap::DefaultPort, const QString &discoveryPath = QLatin1String("/.well-known/core"))` |  |
| `QCoapReply * get(const QCoapRequest &request)` |  |
| `QCoapReply * get(const QUrl &url)` |  |
| `QCoapReply * observe(const QCoapRequest &request)` |  |
| `QCoapReply * observe(const QUrl &url)` |  |
| `QCoapReply * post(const QCoapRequest &request, const QByteArray &data = QByteArray())` |  |
| `QCoapReply * post(const QCoapRequest &request, QIODevice *device)` |  |
| `QCoapReply * post(const QUrl &url, const QByteArray &data = QByteArray())` |  |
| `QCoapReply * put(const QCoapRequest &request, const QByteArray &data = QByteArray())` |  |
| `QCoapReply * put(const QCoapRequest &request, QIODevice *device)` |  |
| `QCoapReply * put(const QUrl &url, const QByteArray &data = QByteArray())` |  |
| `void setAckRandomFactor(double ackRandomFactor)` |  |
| `void setAckTimeout(uint ackTimeout)` |  |
| `void setBindInterface(const QNetworkInterface &iface)` |  |
| `void setBlockSize(quint16 blockSize)` |  |
| `void setMaximumRetransmitCount(uint maximumRetransmitCount)` |  |
| `void setMaximumServerResponseDelay(uint responseDelay)` |  |
| `void setMinimumTokenSize(int tokenSize)` |  |
| `void setSecurityConfiguration(const QCoapSecurityConfiguration &configuration)` |  |
| `void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)` |  |

## Signals

| Member | Description |
| --- | --- |
| `void bindInterfaceChanged(const QNetworkInterface &iface)` |  |
| `void error(QCoapReply *reply, QtCoap::Error error)` |  |
| `void finished(QCoapReply *reply)` |  |
| `void responseToMulticastReceived(QCoapReply *reply, const QCoapMessage &message, const QHostAddress &sender)` |  |

The [QCoapClient](qcoapclient.md) class contains signals that get triggered when the reply of a sent request has arrived.
The application can use a [QCoapClient](qcoapclient.md) to send requests over a CoAP network. It provides functions for standard requests: each returns a [QCoapReply](qcoapreply.md) object, to which the response data shall be delivered; this can be read when the [finished()](qcoapclient.md#finished) signal arrives.
A simple request can be sent with:
```cpp
QCoapClient *client = new QCoapClient(this);
connect(client, &QCoapClient::finished, this, &TestClass::slotFinished);
client->get(QCoapRequest(Qurl("coap://coap.me/test")));

```

> **Note:** After processing of the request has finished, it is the responsibility of the user to delete the QCoapReply object at an appropriate time. Do not directly delete it inside the slot connected to finished(). You can use the deleteLater() function.

You can also use an _observe_ request. This can be used as above, or more conveniently with the [QCoapReply::notified()](qcoapreply.md#notified) signal:
```cpp
QCoapRequest request = QCoapRequest(Qurl("coap://coap.me/obs"));
QCoapReply *reply = client->observe(request);
connect(reply, &QCoapReply::notified, this, &TestClass::slotNotified);

```

And the observation can be cancelled with:
```cpp
client->cancelObserve(reply);

```

When a reply arrives, the [QCoapClient](qcoapclient.md) emits a [finished()](qcoapclient.md#finished) signal.
> **Note:** For a discovery request, the returned object is a QCoapResourceDiscoveryReply. It can be used the same way as a QCoapReply but contains also a list of resources.


## Property Documentation

<a id="bindInterface-prop"></a>
### bindInterface : QNetworkInterface

the network interface to be used by the socket
The default value is an [invalid](https://doc.qt.io/qt-6/qnetworkinterface.html#isValid) QNetworkInterface object, meaning that incoming packets will be accepted from all network interfaces. Similarly, all network interfaces can be used to send outgoing packets.
When a valid network interface is specified, incoming packets will only be accepted from that interface. Similarly, outgoing packets will only be sent using that interface.
Changing the property only has an effect the next time the client binds to the socket, so make sure to call [disconnect()](qcoapclient.md#disconnect) if there was any prior communication.
This property was introduced in Qt 6.11.

## Member Function Documentation

<a id="QCoapClient"></a>
### QCoapClient(QtCoap::SecurityMode securityMode = QtCoap::SecurityMode::NoSecurity, QObject *parent = nullptr)

Constructs a [QCoapClient](qcoapclient.md) object for the given _securityMode_ and sets _parent_ as the parent object.
The default for _securityMode_ is QtCoap::NoSecurity, which disables security.
<a id="dtor.QCoapClient"></a>
### ~QCoapClient()

Destroys the [QCoapClient](qcoapclient.md) object and frees up any resources. Note that [QCoapReply](qcoapreply.md) objects that are returned from this class have the [QCoapClient](qcoapclient.md) set as their parents, which means that they will be deleted along with it.
<a id="cancelObserve"></a>
### void cancelObserve(QCoapReply *notifiedReply)

Cancels the observation of a resource using the reply _notifiedReply_ returned by the [observe()](qcoapclient.md#observe) method.
**See also** [observe()](qcoapclient.md#observe).

<a id="cancelObserve-1"></a>
### void cancelObserve(const QUrl &url)

Cancels the observation of a resource identified by the _url_.
**See also** [observe()](qcoapclient.md#observe).

<a id="deleteResource"></a>
### QCoapReply * deleteResource(const QCoapRequest &request)

Sends the _request_ using the DELETE method and returns a new [QCoapReply](qcoapreply.md) object.
**See also** [get()](qcoapclient.md#get)[put()](qcoapclient.md#put)[post()](qcoapclient.md#post)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="deleteResource-1"></a>
### QCoapReply * deleteResource(const QUrl &url)

Sends a DELETE request to the target _url_.
**See also** [get()](qcoapclient.md#get)[put()](qcoapclient.md#put)[post()](qcoapclient.md#post)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="disconnect"></a>
### void disconnect()

Closes the open sockets and connections to free the transport.
> **Note:** In the secure mode this needs to be called before changing the security configuration or connecting to another server.

**See also** [setSecurityConfiguration()](qcoapclient.md#setSecurityConfiguration).

<a id="discover"></a>
### QCoapResourceDiscoveryReply * discover(const QUrl &url, const QString &discoveryPath = QLatin1String("/.well-known/core"))

Discovers the resources available at the given _url_ and returns a new [QCoapResourceDiscoveryReply](qcoapresourcediscoveryreply.md) object which emits the [QCoapResourceDiscoveryReply::discovered()](qcoapresourcediscoveryreply.md#discovered) signal whenever the response arrives.
Discovery path defaults to "/.well-known/core", but can be changed by passing a different path to _discoveryPath_. Discovery is described in [RFC 6690](https://tools.ietf.org/html/rfc6690#section-1.2.1).
**See also** [get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe).

<a id="discover-1"></a>
### QCoapResourceDiscoveryReply * discover(QtCoap::MulticastGroup group = QtCoap::MulticastGroup::AllCoapNodesIPv4, int port = QtCoap::DefaultPort, const QString &discoveryPath = QLatin1String("/.well-known/core"))

Discovers the resources available at the endpoints which have joined the _group_ at the given _port_. Returns a new [QCoapResourceDiscoveryReply](qcoapresourcediscoveryreply.md) object which emits the [QCoapResourceDiscoveryReply::discovered()](qcoapresourcediscoveryreply.md#discovered) signal whenever a response arrives. The _group_ is one of the CoAP multicast group addresses and defaults to QtCoap::AllCoapNodesIPv4.
Discovery path defaults to "/.well-known/core", but can be changed by passing a different path to _discoveryPath_. Discovery is described in [RFC 6690](https://tools.ietf.org/html/rfc6690#section-1.2.1).
**See also** [get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe).

<a id="error"></a>
### void error(QCoapReply *reply, QtCoap::Error error)

This signal is emitted whenever an error occurs. The _reply_ parameter can be `nullptr` if the error is not related to a specific [QCoapReply](qcoapreply.md). The _error_ parameter contains the error code.
**See also** [finished()](qcoapclient.md#finished)[QCoapReply::error()](qcoapreply.md#error)[QCoapReply::finished()](qcoapreply.md#finished).

<a id="finished"></a>
### void finished(QCoapReply *reply)

This signal is emitted along with the [QCoapReply::finished()](qcoapreply.md#finished) signal whenever a CoAP reply is received, after either a success or an error. The _reply_ parameter will contain a pointer to the reply that has just been received.
**See also** [error()](qcoapclient.md#error)[QCoapReply::finished()](qcoapreply.md#finished)[QCoapReply::error()](qcoapreply.md#error).

<a id="get"></a>
### QCoapReply * get(const QCoapRequest &request)

Sends the _request_ using the GET method and returns a new [QCoapReply](qcoapreply.md) object.
**See also** [post()](qcoapclient.md#post)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="get-1"></a>
### QCoapReply * get(const QUrl &url)

Sends a GET request to _url_ and returns a new [QCoapReply](qcoapreply.md) object.
**See also** [post()](qcoapclient.md#post)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="observe"></a>
### QCoapReply * observe(const QCoapRequest &request)

Sends a request to observe the target _request_ and returns a new [QCoapReply](qcoapreply.md) object which emits the [QCoapReply::notified()](qcoapreply.md#notified) signal whenever a new notification arrives.
**See also** [cancelObserve()](qcoapclient.md#cancelObserve)[get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[discover()](qcoapclient.md#discover).

<a id="observe-1"></a>
### QCoapReply * observe(const QUrl &url)

Sends a request to observe the target _url_ and returns a new [QCoapReply](qcoapreply.md) object which emits the [QCoapReply::notified()](qcoapreply.md#notified) signal whenever a new notification arrives.
**See also** [cancelObserve()](qcoapclient.md#cancelObserve)[get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[discover()](qcoapclient.md#discover).

<a id="post"></a>
### QCoapReply * post(const QCoapRequest &request, const QByteArray &data = QByteArray())

Sends the _request_ using the POST method and returns a new [QCoapReply](qcoapreply.md) object. Uses _data_ as the payload for this request. If _data_ is empty, the payload of the _request_ will be used.
**See also** [get()](qcoapclient.md#get)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="post-1"></a>
### QCoapReply * post(const QCoapRequest &request, QIODevice *device)

Sends the _request_ using the POST method and returns a new [QCoapReply](qcoapreply.md) object. Uses _device_ content as the payload for this request. A null device is treated as empty content, in which case the payload of the _request_ will be used.
> **Note:** The device has to be open and readable before calling this function.

**See also** [get()](qcoapclient.md#get)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="post-2"></a>
### QCoapReply * post(const QUrl &url, const QByteArray &data = QByteArray())

Sends a POST request to _url_ and returns a new [QCoapReply](qcoapreply.md) object. Uses _data_ as the payload for this request.
**See also** [get()](qcoapclient.md#get)[put()](qcoapclient.md#put)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="put"></a>
### QCoapReply * put(const QCoapRequest &request, const QByteArray &data = QByteArray())

Sends the _request_ using the PUT method and returns a new [QCoapReply](qcoapreply.md) object. Uses _data_ as the payload for this request. If _data_ is empty, the payload of the _request_ will be used.
**See also** [get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="put-1"></a>
### QCoapReply * put(const QCoapRequest &request, QIODevice *device)

Sends the _request_ using the PUT method and returns a new [QCoapReply](qcoapreply.md) object. Uses _device_ content as the payload for this request. A null device is treated as empty content, in which case the payload of the _request_ will be used.
> **Note:** The device has to be open and readable before calling this function.

**See also** [get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="put-2"></a>
### QCoapReply * put(const QUrl &url, const QByteArray &data = QByteArray())

Sends a PUT request to _url_ and returns a new [QCoapReply](qcoapreply.md) object. Uses _data_ as the payload for this request.
**See also** [get()](qcoapclient.md#get)[post()](qcoapclient.md#post)[deleteResource()](qcoapclient.md#deleteResource)[observe()](qcoapclient.md#observe)[discover()](qcoapclient.md#discover).

<a id="responseToMulticastReceived"></a>
### void responseToMulticastReceived(QCoapReply *reply, const QCoapMessage &message, const QHostAddress &sender)

This signal is emitted when a unicast response to a multicast request arrives. The _reply_ parameter contains a pointer to the reply that has just been received, _message_ contains the payload and the message details, and _sender_ contains the sender address.
**See also** [error()](qcoapclient.md#error)[QCoapReply::finished()](qcoapreply.md#finished)[QCoapReply::error()](qcoapreply.md#error).

<a id="setAckRandomFactor"></a>
### void setAckRandomFactor(double ackRandomFactor)

Sets the `ACK_RANDOM_FACTOR` value defined in [RFC 7252 - Section 4.2](https://datatracker.ietf.org/doc/html/rfc7252#section-4.2), to _ackRandomFactor_. This value should be greater than or equal to 1. The default is 1.5.
**See also** [setAckTimeout()](qcoapclient.md#setAckTimeout).

<a id="setAckTimeout"></a>
### void setAckTimeout(uint ackTimeout)

Sets the `ACK_TIMEOUT` value defined in [RFC 7252 - Section 4.2](https://datatracker.ietf.org/doc/html/rfc7252#section-4.2) to _ackTimeout_ in milliseconds. The default is 2000 ms.
This timeout only applies to confirmable messages. The actual timeout for reliable transmissions is a random value between `ACK_TIMEOUT` and `ACK_TIMEOUT * ACK_RANDOM_FACTOR`.
**See also** [setAckRandomFactor()](qcoapclient.md#setAckRandomFactor).

<a id="setBlockSize"></a>
### void setBlockSize(quint16 blockSize)

Sets the maximum block size used by the protocol to _blockSize_ when sending requests and receiving replies. The block size must be a power of two.
<a id="setMaximumRetransmitCount"></a>
### void setMaximumRetransmitCount(uint maximumRetransmitCount)

Sets the `MAX_RETRANSMIT` value defined in [RFC 7252 - Section 4.2](https://datatracker.ietf.org/doc/html/rfc7252#section-4.2) to _maximumRetransmitCount_. This value should be less than or equal to 25. The default is 4.
<a id="setMaximumServerResponseDelay"></a>
### void setMaximumServerResponseDelay(uint responseDelay)

Sets the `MAX_SERVER_RESPONSE_DELAY` value to _responseDelay_ in milliseconds. The default is 250 seconds.
As defined in [RFC 7390 - Section 2.5](https://datatracker.ietf.org/doc/html/rfc7390#section-2.5), `MAX_SERVER_RESPONSE_DELAY` is the expected maximum response delay over all servers that the client can send a multicast request to.
<a id="setMinimumTokenSize"></a>
### void setMinimumTokenSize(int tokenSize)

Sets the minimum token size to _tokenSize_ in bytes. For security reasons it is recommended to use tokens with a length of at least 4 bytes. The default value for this parameter is 4 bytes.
<a id="setSecurityConfiguration"></a>
### void setSecurityConfiguration(const QCoapSecurityConfiguration &configuration)

Sets the security configuration parameters from _configuration_. Configuration will be ignored if the QtCoap::NoSecurity mode is used.
> **Note:** This method must be called before the handshake starts. If you need to change the security configuration after establishing a secure connection with the server, the client needs to be disconnected first.

**See also** [disconnect()](qcoapclient.md#disconnect).

<a id="setSocketOption"></a>
### void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)

Sets the QUdpSocket socket _option_ to _value_.
---

*Built with QDoc's template engine.*
