# List of All Members for QCoapRequest

This is the complete list of members for [QCoapRequest](qcoaprequest.md), including inherited members.

- [`enum class Type`](qcoapmessage.md#Type-enum)
- [`QCoapRequest(const char *, QCoapMessage::Type)`](qcoaprequest.md#QCoapRequest-1)
- [`QCoapRequest(const QUrl &, QCoapMessage::Type, const QUrl &)`](qcoaprequest.md#QCoapRequest-2)
- [`QCoapRequest(const QCoapRequest &)`](qcoaprequest.md#QCoapRequest)
- [`~QCoapRequest()`](qcoaprequest.md#dtor.QCoapRequest)
- [`addOption(const QCoapOption &)`](qcoapmessage.md#addOption)
- [`addOption(QCoapOption::OptionName, const QByteArray &)`](qcoapmessage.md#addOption-1)
- [`clearOptions()`](qcoapmessage.md#clearOptions)
- [`enableObserve()`](qcoaprequest.md#enableObserve)
- [`hasOption(QCoapOption::OptionName) const : bool`](qcoapmessage.md#hasOption)
- [`isObserve() const : bool`](qcoaprequest.md#isObserve)
- [`messageId() const : quint16`](qcoapmessage.md#messageId)
- [`method() const : QtCoap::Method`](qcoaprequest.md#method)
- [`option(QCoapOption::OptionName) const : QCoapOption`](qcoapmessage.md#option)
- [`optionAt(int) const : QCoapOption`](qcoapmessage.md#optionAt)
- [`optionCount() const : int`](qcoapmessage.md#optionCount)
- [`options() const : const QList<QCoapOption> &`](qcoapmessage.md#options)
- [`options(QCoapOption::OptionName) const : QList<QCoapOption>`](qcoapmessage.md#options-1)
- [`payload() const : QByteArray`](qcoapmessage.md#payload)
- [`proxyUrl() const : QUrl`](qcoaprequest.md#proxyUrl)
- [`removeOption(QCoapOption::OptionName)`](qcoapmessage.md#removeOption)
- [`removeOption(const QCoapOption &)`](qcoapmessage.md#removeOption-1)
- [`setMessageId(quint16)`](qcoapmessage.md#setMessageId)
- [`setOptions(const QList<QCoapOption> &)`](qcoapmessage.md#setOptions)
- [`setPayload(const QByteArray &)`](qcoapmessage.md#setPayload)
- [`setProxyUrl(const QUrl &)`](qcoaprequest.md#setProxyUrl)
- [`setToken(const QByteArray &)`](qcoapmessage.md#setToken)
- [`setType(const QCoapMessage::Type &)`](qcoapmessage.md#setType)
- [`setUrl(const QUrl &)`](qcoaprequest.md#setUrl)
- [`setVersion(quint8)`](qcoapmessage.md#setVersion)
- [`swap(QCoapMessage &)`](qcoapmessage.md#swap)
- [`token() const : QByteArray`](qcoapmessage.md#token)
- [`tokenLength() const : quint8`](qcoapmessage.md#tokenLength)
- [`type() const : QCoapMessage::Type`](qcoapmessage.md#type)
- [`url() const : QUrl`](qcoaprequest.md#url)
- [`version() const : quint8`](qcoapmessage.md#version)
- [`operator=(const QCoapRequest &) : QCoapRequest &`](qcoaprequest.md#operator-eq)
- [`operator=(QCoapMessage &&) : QCoapMessage &`](qcoapmessage.md#operator-eq)
- [`operator=(const QCoapMessage &) : QCoapMessage &`](qcoapmessage.md#operator-eq-1)


---

*Built with QDoc's template engine.*
