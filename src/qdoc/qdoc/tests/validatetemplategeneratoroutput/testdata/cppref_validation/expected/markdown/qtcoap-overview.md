# Qt CoAP Overview

Provides insight into the CoAP protocol and the Qt CoAP module.

Constrained Application Protocol (CoAP) is an IoT protocol that is specifically designed for M2M data exchange between constrained devices (such as microcontrollers) in constrained networks.
The interaction model of CoAP is similar to the client/server model of HTTP, but unlike HTTP, it uses datagram-oriented connectionless transport such as UDP, which leads to a very low overhead and allows UDP broadcast and multicast to be used for addressing. At the same time it provides lightweight reliability mechanisms and security.
Qt CoAP implements the client side of CoAP. By default, the transport layer uses QUdpSocket and QDtls for security. Alternative transports can be used by implementing the QCoapConnection interface.
## Messaging Model

The CoAP messaging model is based on the exchange of messages between endpoints: clients make requests to servers; servers send back responses. Clients may _GET_, _PUT_, _POST_ and _DELETE_ resources. They can also discover resources on a server by making discovery requests, or in the local network, by making multicast discovery requests. It is also possible to subscribe to a resource, with an observe request.
Reliability of the transfer is achieved by marking a message _confirmable_ (CON). A confirmable message is retransmitted using a default timeout and exponential back-off between retransmissions, until the recipient sends an _acknowledgment_ (ACK) message. When the recipient is not able to process a confirmable message, it replies with a _reset_ message (RST) instead of an acknowledgment.
A message that does not require reliable transmission can be sent as a _non-confirmable_ (NON) message.

## Using the Qt CoAP API

The communication of the client with the CoAP server is done using the [QCoapClient](qcoapclient.md) class. It contains the methods for sending different CoAP requests and the signals that get triggered when the replies for the sent request arrive. The [QCoapRequest](qcoaprequest.md) class is used for creating CoAP requests. The response from the server is returned in a [QCoapReply](qcoapreply.md) object. For example:
```cpp
QCoapClient *client = new QCoapClient();
connect(client, &QCoapClient::finished, this, &MyClass::onFinished);
connect(client, &QCoapClient::error, this, &MyClass::onError);

QCoapRequest request(QUrl("coap://127.0.0.1/test"), QCoapMessage::Confirmable);
client->get(request);
client->put(request, QByteArray("payload"));


```


## Supported Features


### Resource Discovery

CoAP discovery requests are used to query the resources available on an endpoint or in the complete network. This is really important for M2M applications, where there are no humans in the loop. For example, for home or building automation, there is a need for local clients and servers to find and interact with each other without human intervention. Resource discovery allows clients to learn about the endpoints available in the network.
Qt CoAP supports discovery requests to a single endpoint and to multicast groups. For example, a discovery request to `/.well-known/core`, which is the default resource discovery entry point, may return something like:
```cpp
RES: 2.05 Content
</sensors/temp>;rt="temperature-c";if="sensor";obs,
</sensors/light>;rt="light-lux";if="sensor",
</firmware/v2.1>;rt="firmware";sz=262144

```

Indicating that there are resources for temperature and light sensors and a firmware resource available in the network. The reply is represented in [CoRE Link Format](https://tools.ietf.org/html/rfc6690).
The specialized [QCoapResourceDiscoveryReply](qcoapresourcediscoveryreply.md) class is used to get the discovery replies:
```cpp
// This will make a multicast discovery request to the CoAP IPv4 multicast group.
QCoapResourceDiscoveryReply *discoverReply = client->discover();
connect(discoverReply, &QCoapResourceDiscoveryReply::discovered, this, &MyClass::onDiscovered);

```

The QCoapResourceDiscoveryReply::discovered will return the list of CoAP resources found in the network.

### Resource Observation

Observe requests are used to receive automatic server notifications for a resource. The client becomes an observer of an observable resource by sending an observe request to the resource. For example, the temperature sensor from the above example is observable, because it has the `obs` attribute. So the client can subscribe to the temperature updates by sending an observe request to it.
The following example code shows how to send observe requests using Qt CoAP:
```cpp
QCoapReply *reply = client->observe(QUrl("127.0.0.1/temp"));
connect(reply, &QCoapReply::notified, this, &MyClass::onNotified);

```

For Observe requests specifically, you can use the QCoapReply::notified signal to handle notifications from the CoAP server.

### Blockwise Transfers

Since CoAP is based on datagram transports such as UDP, there are limits on how big a resource representation can be, to be transferred without fragmentation causing problems. Qt CoAP supports block-wise transfers for situations where a resource representation is larger than can be comfortably transferred in the payload of a single CoAP datagram.

### Security

The following security modes are defined for CoAP:
- **Pre-Shared Key** - in this mode the client must send to the server its identity and the pre-shared key.
- **Raw Public Key** - the client has an asymmetric key pair without a certificate (a raw public key). The client also has an identity calculated from the public key and a list of identities of the nodes it can communicate with. Qt CoAP has not implemented this mode yet.
- **Certificate** - the client has an asymmetric key pair with an X.509 certificate that is signed by some common trust root.

Since CoAP is designed to be a UDP-based protocol, Qt CoAP module implements security based on Datagram TLS (DTLS) over UDP. However, as mentioned above, it is possible to provide a custom transport with another security type.
For securing the CoAP client, one of the supported security modes should be specified when creating the client:
```cpp
QCoapClient *client = new QCoapClient(this, QtCoap::PreSharedKey);

```

The [QCoapSecurityConfiguration](qcoapsecurityconfiguration.md) class is used for specifying the security parameters. For example, in pre-shared key mode the following example code can be used:
```cpp
QCoapSecurityConfiguration config;
config.setPreSharedKey("secretPSK");
config.setIdentity("Client_identity");
client->setSecurityConfiguration(config);

```

And in certificate mode:
```cpp
QCoapClient *client = new QCoapClient(this, QtCoap::Certificate);
QList<QSslCertificate> localCertificates, caCertificates;
QCoapPrivateKey key;
// Initialize the key and certificates
QCoapSecurityConfiguration config;
config.setLocalCertificateChain(localCertificates);
config.setCaCertificates(caCertificates)
config.setPrivateKey(key);
client->setSecurityConfiguration(config);


```



---

*Built with QDoc's template engine.*
