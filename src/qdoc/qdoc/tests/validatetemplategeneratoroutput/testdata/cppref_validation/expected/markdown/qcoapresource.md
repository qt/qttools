[QtCoap](qtcoap-module.md)> QCoapResource
**Contents**

- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# QCoapResource

class QCoapResource

The QCoapResource class holds information about a discovered resource.

| Key | Value |
| --- | --- |
| Header | `QCoapResource` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |

- [List of all members, including inherited members](qcoapresource-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Public Functions

| Member | Description |
| --- | --- |
| `QCoapResource()` |  |
| `QCoapResource(const QCoapResource &other)` |  |
| `~QCoapResource()` |  |
| `uint contentFormat() const` |  |
| `QHostAddress host() const` |  |
| `QString interface() const` |  |
| `int maximumSize() const` |  |
| `bool observable() const` |  |
| `QString path() const` |  |
| `QString resourceType() const` |  |
| `void setContentFormat(uint contentFormat)` |  |
| `void setHost(const QHostAddress &host)` |  |
| `void setInterface(const QString &interface)` |  |
| `void setMaximumSize(int maximumSize)` |  |
| `void setObservable(bool observable)` |  |
| `void setPath(const QString &path)` |  |
| `void setResourceType(const QString &resourceType)` |  |
| `void setTitle(const QString &title)` |  |
| `void swap(QCoapResource &other)` |  |
| `QString title() const` |  |
| `QCoapResource & operator=(const QCoapResource &other)` |  |

## Detailed Description
The [QCoapRequest](qcoaprequest.md) contains data as the path and title of the resource and other ancillary information.

## Member Function Documentation

<a id="QCoapResource"></a>
### QCoapResource()

Constructs a new [QCoapResource](qcoapresource.md).
<a id="QCoapResource-1"></a>
### QCoapResource(const QCoapResource &other)

Constructs a new CoAP resource as a copy of _other_, making the two resources identical.
<a id="dtor.QCoapResource"></a>
### ~QCoapResource()

Destroy the [QCoapResource](qcoapresource.md).
<a id="contentFormat"></a>
### uint contentFormat() const

Returns the Content-Format code of the resource.
The Content-Format code corresponds to the 'ct' attribute and provides a hint about the Content-Formats this resource returns. It is specified in [RFC 7252](https://tools.ietf.org/html/rfc7252#section-7.2.1).
**See also** [setContentFormat()](qcoapresource.md#setContentFormat).

<a id="host"></a>
### QHostAddress host() const

Returns the host of the resource.
**See also** [setHost()](qcoapresource.md#setHost).

<a id="interface"></a>
### QString interface() const

Returns the interface description of the resource.
The Interface Description 'if' attribute is an opaque string used to provide a name or URI indicating a specific interface definition used to interact with the target resource. It is specified in [RFC 6690](https://tools.ietf.org/html/rfc6690#section-3.2).
**See also** [setInterface()](qcoapresource.md#setInterface).

<a id="maximumSize"></a>
### int maximumSize() const

Returns the maximum size of the resource.
The maximum size estimate attribute 'sz' gives an indication of the maximum size of the resource representation returned by performing a GET on the target URI. It is specified in [RFC 6690](https://tools.ietf.org/html/rfc6690#section-3.3).
**See also** [setMaximumSize()](qcoapresource.md#setMaximumSize).

<a id="observable"></a>
### bool observable() const

Returns `true` if the resource is observable
**See also** [setObservable()](qcoapresource.md#setObservable).

<a id="path"></a>
### QString path() const

Returns the path of the resource.
**See also** [setPath()](qcoapresource.md#setPath).

<a id="resourceType"></a>
### QString resourceType() const

Returns the type of the resource.
**See also** [setResourceType()](qcoapresource.md#setResourceType).

<a id="setContentFormat"></a>
### void setContentFormat(uint contentFormat)

Sets the content format of the resource to _contentFormat_. The content format can be one of the content formats defined in [CoAP Content-Formats Registry](https://www.iana.org/assignments/core-parameters/core-parameters.xhtml#content-formats).
> **Note:** CoAP supports common content formats such as XML, JSON, and so on, but these are text based and consequently heavy both in payload and in processing. One of the recommended content formats to use with CoAP is CBOR, which is designed to be used in such contexts.

**See also** [contentFormat()](qcoapresource.md#contentFormat), [QCborStreamWriter](https://doc.qt.io/qt-6/qcborstreamwriter.html), and [QCborStreamReader](https://doc.qt.io/qt-6/qcborstreamreader.html).

<a id="setHost"></a>
### void setHost(const QHostAddress &host)

Sets the host of the resource to _host_.
**See also** [host()](qcoapresource.md#host).

<a id="setInterface"></a>
### void setInterface(const QString &interface)

Sets the interface of the resource to _interface_.
**See also** [interface()](qcoapresource.md#interface).

<a id="setMaximumSize"></a>
### void setMaximumSize(int maximumSize)

Sets the maximum size of the resource to _maximumSize_.
**See also** [maximumSize()](qcoapresource.md#maximumSize).

<a id="setObservable"></a>
### void setObservable(bool observable)

Makes the resource observable if the _observable_ parameter is `true`.
**See also** [observable()](qcoapresource.md#observable).

<a id="setPath"></a>
### void setPath(const QString &path)

Sets the path of the resource to _path_.
**See also** [path()](qcoapresource.md#path).

<a id="setResourceType"></a>
### void setResourceType(const QString &resourceType)

Sets the resource type to _resourceType_.
**See also** [resourceType()](qcoapresource.md#resourceType).

<a id="setTitle"></a>
### void setTitle(const QString &title)

Sets the title of the resource to _title_.
**See also** [title()](qcoapresource.md#title).

<a id="swap"></a>
### void swap(QCoapResource &other)

Swaps this resource with _other_. This operation is very fast and never fails.
<a id="title"></a>
### QString title() const

Returns the title of the resource.
**See also** [setTitle()](qcoapresource.md#setTitle).

<a id="operator-eq"></a>
### QCoapResource & operator=(const QCoapResource &other)

Copies _other_ into this resource, making the two resources identical. Returns a reference to this [QCoapResource](qcoapresource.md).

---

*Built with QDoc's template engine.*
