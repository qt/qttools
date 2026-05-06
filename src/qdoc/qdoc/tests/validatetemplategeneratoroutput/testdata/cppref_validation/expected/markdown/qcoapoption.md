[QtCoap](qtcoap-module.md)> QCoapOption

**Contents**

- [Public Types](#public-types)
- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Member Type Documentation](#member-type-documentation)
- [Member Function Documentation](#member-function-documentation)

# QCoapOption

class QCoapOption

The QCoapOption class holds data about CoAP options.

| Key | Value |
| --- | --- |
| Header | `QCoapOption` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |

- [List of all members, including inherited members](qcoapoption-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Public Types

| Member | Description |
| --- | --- |
| `enum OptionName` |  |

## Public Functions

| Member | Description |
| --- | --- |
| `QCoapOption(QCoapOption::OptionName name = Invalid, const QByteArray &opaqueValue = QByteArray())` |  |
| `QCoapOption(QCoapOption::OptionName name, const QString &stringValue)` |  |
| `QCoapOption(QCoapOption::OptionName name, quint32 intValue)` |  |
| `QCoapOption(const QCoapOption &other)` |  |
| `QCoapOption(QCoapOption &&other)` |  |
| `~QCoapOption()` |  |
| `bool isValid() const` |  |
| `int length() const` |  |
| `QCoapOption::OptionName name() const` |  |
| `QByteArray opaqueValue() const` |  |
| `QString stringValue() const` |  |
| `void swap(QCoapOption &other)` |  |
| `quint32 uintValue() const` |  |
| `bool operator!=(const QCoapOption &other) const` |  |
| `QCoapOption & operator=(QCoapOption &&other)` |  |
| `QCoapOption & operator=(const QCoapOption &other)` |  |
| `bool operator==(const QCoapOption &other) const` |  |

## Detailed Description
CoAP defines a number of options that can be included in a message. Both requests and responses may include a list of one or more options. For example, the URI in a request is transported in several options, and metadata that would be carried in an HTTP header in HTTP is supplied as options as well.
An option contains a name, related to an option ID, and a value. The name is one of the values from the [OptionName](qcoapoption.md#OptionName-enum) enumeration.

## Member Type Documentation

<a id="OptionName-enum"></a>
### enum OptionName

Indicates the name of an option. The value of each ID is as specified by the CoAP standard, with the exception of Invalid. You can refer to [RFC 7252](https://tools.ietf.org/html/rfc7252#section-5.10) and [RFC 7959](https://tools.ietf.org/html/rfc7959#section-2.1) for more details.
| Constant | Description |
| --- | --- |
| `Invalid` | An invalid option.|
| `IfMatch` | If-Match option.|
| `UriHost` | Uri-Host option.|
| `Etag` | Etag option.|
| `IfNoneMatch` | If-None-Match option.|
| `Observe` | Observe option.|
| `UriPort` | Uri-Port option.|
| `LocationPath` | Location-path option.|
| `UriPath` | Uri-Path option.|
| `ContentFormat` | Content-Format option.|
| `MaxAge` | Max-Age option.|
| `UriQuery` | Uri-Query option.|
| `Accept` | Accept option.|
| `LocationQuery` | Location-Query option.|
| `Block2` | Block2 option.|
| `Block1` | Block1 option.|
| `Size2` | Size2 option.|
| `ProxyUri` | Proxy-Uri option.|
| `ProxyScheme` | Proxy-Scheme option.|
| `Size1` | Size1 option.|

## Member Function Documentation

<a id="QCoapOption-2"></a>
### QCoapOption(QCoapOption::OptionName name = Invalid, const QByteArray &opaqueValue = QByteArray())

Constructs a new CoAP option with the given _name_ and QByteArray _opaqueValue_. If no parameters are passed, constructs an Invalid object.
**See also** [isValid()](qcoapoption.md#isValid).

<a id="QCoapOption-3"></a>
### QCoapOption(QCoapOption::OptionName name, const QString &stringValue)

Constructs a new CoAP option with the given _name_ and the QString _stringValue_.
**See also** [isValid()](qcoapoption.md#isValid).

<a id="QCoapOption-4"></a>
### QCoapOption(QCoapOption::OptionName name, quint32 intValue)

Constructs a new CoAP option with the given _name_ and the unsigned integer _intValue_.
**See also** [isValid()](qcoapoption.md#isValid).

<a id="QCoapOption-1"></a>
### QCoapOption(const QCoapOption &other)

Constructs a new CoAP option as a copy of _other_, making the two options identical.
**See also** [isValid()](qcoapoption.md#isValid).

<a id="QCoapOption"></a>
### QCoapOption(QCoapOption &&other)

Move-constructs a [QCoapOption](qcoapoption.md), making it point to the same object as _other_ was pointing to.
<a id="dtor.QCoapOption"></a>
### ~QCoapOption()

Destroys the [QCoapOption](qcoapoption.md) object.
<a id="isValid"></a>
### bool isValid() const

Returns `true` if the option is valid.
<a id="length"></a>
### int length() const

Returns the length of the value of the option.
<a id="name"></a>
### QCoapOption::OptionName name() const

Returns the name of the option.
<a id="opaqueValue"></a>
### QByteArray opaqueValue() const

Returns the value of the option.
<a id="stringValue"></a>
### QString stringValue() const

Returns the QString value of the option.
<a id="swap"></a>
### void swap(QCoapOption &other)

Swaps this option with _other_. This operation is very fast and never fails.
<a id="uintValue"></a>
### quint32 uintValue() const

Returns the integer value of the option.
<a id="operator-not-eq"></a>
### bool operator!=(const QCoapOption &other) const

Returns `true` if this [QCoapOption](qcoapoption.md) and _other_ are different.
<a id="operator-eq"></a>
### QCoapOption & operator=(QCoapOption &&other)

Moves _other_ into this option and returns a reference to this [QCoapOption](qcoapoption.md).
<a id="operator-eq-1"></a>
### QCoapOption & operator=(const QCoapOption &other)

Copies _other_ into this option, making the two options identical. Returns a reference to this [QCoapOption](qcoapoption.md).
<a id="operator-eq-eq"></a>
### bool operator==(const QCoapOption &other) const

Returns `true` if this [QCoapOption](qcoapoption.md) and _other_ are equals.

---

*Built with QDoc's template engine.*
