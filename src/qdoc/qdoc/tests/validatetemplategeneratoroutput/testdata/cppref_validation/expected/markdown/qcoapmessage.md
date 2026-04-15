# QCoapMessage

class QCoapMessage

The QCoapMessage class holds information about a CoAP message that can be a request or a reply.

| Key | Value |
| --- | --- |
| Header | `QCoapMessage` |
| CMake | `find_package(Qt6 REQUIRED COMPONENTS Coap)` `target_link_libraries(mytarget PRIVATE Qt6::Coap)` |
| qmake | `QT += coap` |
| Inherited By | [QCoapRequest](qcoaprequest.md)|

- [List of all members, including inherited members](qcoapmessage-members.md)

> **Note:** All functions in this class are [reentrant](threads-reentrancy.md).

## Public Types

| Member | Description |
| --- | --- |
| `enum class Type` |  |

## Public Functions

| Member | Description |
| --- | --- |
| `QCoapMessage()` |  |
| `QCoapMessage(const QCoapMessage &other)` |  |
| `~QCoapMessage()` |  |
| `void addOption(const QCoapOption &option)` |  |
| `void addOption(QCoapOption::OptionName name, const QByteArray &value = QByteArray())` |  |
| `void clearOptions()` |  |
| `bool hasOption(QCoapOption::OptionName name) const` |  |
| `quint16 messageId() const` |  |
| `QCoapOption option(QCoapOption::OptionName name) const` |  |
| `QCoapOption optionAt(int index) const` |  |
| `int optionCount() const` |  |
| `const QList<QCoapOption> & options() const` |  |
| `QList<QCoapOption> options(QCoapOption::OptionName name) const` |  |
| `QByteArray payload() const` |  |
| `void removeOption(QCoapOption::OptionName name)` |  |
| `void removeOption(const QCoapOption &option)` |  |
| `void setMessageId(quint16 id)` |  |
| `void setOptions(const QList<QCoapOption> &options)` |  |
| `void setPayload(const QByteArray &payload)` |  |
| `void setToken(const QByteArray &token)` |  |
| `void setType(const QCoapMessage::Type &type)` |  |
| `void setVersion(quint8 version)` |  |
| `void swap(QCoapMessage &other)` |  |
| `QByteArray token() const` |  |
| `quint8 tokenLength() const` |  |
| `QCoapMessage::Type type() const` |  |
| `quint8 version() const` |  |
| `QCoapMessage & operator=(QCoapMessage &&other)` |  |
| `QCoapMessage & operator=(const QCoapMessage &other)` |  |

It holds information such as the message type, message id, token and other ancillary data.

## Member Type Documentation

<a id="Type-enum"></a>
### enum class Type

Indicates the type of the message.
| Constant | Description |
| --- | --- |
| `Confirmable` | A Confirmable message. The destination endpoint needs to acknowledge the message.|
| `NonConfirmable` | A Non-Confirmable message. The destination endpoint does not need to acknowledge the message.|
| `Acknowledgment` | An Acknowledgment message. A message sent or received in reply to a Confirmable message.|
| `Reset` | A Reset message. This message type is used in case of errors or to stop the ongoing transmission. (For example, it is used to cancel an observation).|

## Member Function Documentation

<a id="QCoapMessage"></a>
### QCoapMessage()

Constructs a new [QCoapMessage](qcoapmessage.md).
<a id="QCoapMessage-1"></a>
### QCoapMessage(const QCoapMessage &other)

Constructs a shallow copy of _other_.
<a id="dtor.QCoapMessage"></a>
### ~QCoapMessage()

Destroys the [QCoapMessage](qcoapmessage.md).
<a id="addOption"></a>
### void addOption(const QCoapOption &option)

Adds the given CoAP _option_.
<a id="addOption-1"></a>
### void addOption(QCoapOption::OptionName name, const QByteArray &value = QByteArray())

Adds the CoAP option with the given _name_ and _value_.
<a id="clearOptions"></a>
### void clearOptions()

Removes all options.
<a id="hasOption"></a>
### bool hasOption(QCoapOption::OptionName name) const

Returns `true` if the message contains at last one option with _name_.
<a id="messageId"></a>
### quint16 messageId() const

Returns the message id.
**See also** [setMessageId()](qcoapmessage.md#setMessageId).

<a id="option"></a>
### QCoapOption option(QCoapOption::OptionName name) const

Finds and returns the first option with the given _name_. If there is no such option, returns an invalid [QCoapOption](qcoapoption.md) with an empty value.
<a id="optionAt"></a>
### QCoapOption optionAt(int index) const

Returns the option at _index_ position.
<a id="optionCount"></a>
### int optionCount() const

Returns the number of options.
<a id="options"></a>
### const QList<QCoapOption> & options() const

Returns the list of options.
<a id="options-1"></a>
### QList<QCoapOption> options(QCoapOption::OptionName name) const

Finds and returns the list of options with the given _name_.
<a id="payload"></a>
### QByteArray payload() const

Returns the payload.
**See also** [setPayload()](qcoapmessage.md#setPayload).

<a id="removeOption"></a>
### void removeOption(QCoapOption::OptionName name)

Removes all options with the given _name_. The CoAP protocol allows for the same option to repeat.
<a id="removeOption-1"></a>
### void removeOption(const QCoapOption &option)

Removes the given _option_.
<a id="setMessageId"></a>
### void setMessageId(quint16 id)

Sets the message ID to _id_.
**See also** [messageId()](qcoapmessage.md#messageId).

<a id="setOptions"></a>
### void setOptions(const QList<QCoapOption> &options)

Sets the message options to _options_.
<a id="setPayload"></a>
### void setPayload(const QByteArray &payload)

Sets the message payload to _payload_. The payload can be represented in one of the content formats defined in [CoAP Content-Formats Registry](https://www.iana.org/assignments/core-parameters/core-parameters.xhtml#content-formats).
> **Note:** CoAP supports common content formats such as XML, JSON, and so on, but these are text based and consequently heavy both in payload and in processing. One of the recommended content formats to use with CoAP is CBOR, which is designed to be used in such contexts.

**See also** [payload()](qcoapmessage.md#payload)[QCborStreamWriter](https://doc.qt.io/qt-6/qcborstreamwriter.html)[QCborStreamReader](https://doc.qt.io/qt-6/qcborstreamreader.html).

<a id="setToken"></a>
### void setToken(const QByteArray &token)

Sets the message token to _token_.
**See also** [token()](qcoapmessage.md#token).

<a id="setType"></a>
### void setType(const QCoapMessage::Type &type)

Sets the message type to _type_.
**See also** [type()](qcoapmessage.md#type).

<a id="setVersion"></a>
### void setVersion(quint8 version)

Sets the CoAP version to _version_.
**See also** [version()](qcoapmessage.md#version).

<a id="swap"></a>
### void swap(QCoapMessage &other)

Swaps this message with _other_. This operation is very fast and never fails.
<a id="token"></a>
### QByteArray token() const

Returns the message token.
**See also** [setToken()](qcoapmessage.md#setToken).

<a id="tokenLength"></a>
### quint8 tokenLength() const

Returns the token length.
<a id="type"></a>
### QCoapMessage::Type type() const

Returns the message type.
**See also** [setType()](qcoapmessage.md#setType).

<a id="version"></a>
### quint8 version() const

Returns the CoAP version.
**See also** [setVersion()](qcoapmessage.md#setVersion).

<a id="operator-eq"></a>
### QCoapMessage & operator=(QCoapMessage &&other)

Moves _other_ into this message and returns a reference to this [QCoapMessage](qcoapmessage.md).
<a id="operator-eq-1"></a>
### QCoapMessage & operator=(const QCoapMessage &other)

Copies the contents of _other_ into this message. Returns a reference to this [QCoapMessage](qcoapmessage.md).
---

*Built with QDoc's template engine.*
