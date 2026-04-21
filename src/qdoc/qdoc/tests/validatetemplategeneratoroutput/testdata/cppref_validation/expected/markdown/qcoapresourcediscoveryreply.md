[QtCoap](qtcoap-module.md)> QCoapResourceDiscoveryReply
**Contents**

- [Public Functions](#public-functions)
- [Signals](#signals)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# QCoapResourceDiscoveryReply

class QCoapResourceDiscoveryReply

The QCoapResourceDiscoveryReply class holds the data of a CoAP reply for a resource discovery request.

| Key | Value |
| --- | --- |
| Header | `QCoapResourceDiscoveryReply` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |
| Inherits | [QCoapReply](qcoapreply.md)|

- [List of all members, including inherited members](qcoapresourcediscoveryreply-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Public Functions

| Member | Description |
| --- | --- |
| `QList<QCoapResource> resources() const` |  |

## Signals

| Member | Description |
| --- | --- |
| `void discovered(QCoapResourceDiscoveryReply *reply, QList<QCoapResource> resources)` |  |

## Detailed Description
This class is used for discovery requests. It emits the [discovered()](qcoapresourcediscoveryreply.md#discovered) signal if and when resources are discovered. When using a multicast address for discovery, the [discovered()](qcoapresourcediscoveryreply.md#discovered) signal will be emitted once for each response received.
> **Note:** A QCoapResourceDiscoveryReply is a QCoapReply that stores also a list of QCoapResources.


## Member Function Documentation

<a id="discovered"></a>
### void discovered(QCoapResourceDiscoveryReply *reply, QList<QCoapResource> resources)

This signal is emitted whenever a CoAP resource is discovered.
The _reply_ parameter contains a pointer to the reply that has just been received, and _resources_ contains a list of resources that were discovered.
**See also** [QCoapReply::finished()](qcoapreply.md#finished).

<a id="resources"></a>
### QList<QCoapResource> resources() const

Returns the list of resources.

---

*Built with QDoc's template engine.*
