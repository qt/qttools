<!--
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-->

# List of All Members for QCoapMessage

This is the complete list of members for [QCoapMessage](qcoapmessage.md), including inherited members.

- [`enum class Type`](qcoapmessage.md#Type-enum)
- [`QCoapMessage()`](qcoapmessage.md#QCoapMessage)
- [`QCoapMessage(const QCoapMessage &)`](qcoapmessage.md#QCoapMessage-1)
- [`~QCoapMessage()`](qcoapmessage.md#dtor.QCoapMessage)
- [`addOption(const QCoapOption &)`](qcoapmessage.md#addOption)
- [`addOption(QCoapOption::OptionName, const QByteArray &)`](qcoapmessage.md#addOption-1)
- [`clearOptions()`](qcoapmessage.md#clearOptions)
- [`hasOption(QCoapOption::OptionName) const : bool`](qcoapmessage.md#hasOption)
- [`messageId() const : quint16`](qcoapmessage.md#messageId)
- [`option(QCoapOption::OptionName) const : QCoapOption`](qcoapmessage.md#option)
- [`optionAt(int) const : QCoapOption`](qcoapmessage.md#optionAt)
- [`optionCount() const : int`](qcoapmessage.md#optionCount)
- [`options() const : const QList<QCoapOption> &`](qcoapmessage.md#options)
- [`options(QCoapOption::OptionName) const : QList<QCoapOption>`](qcoapmessage.md#options-1)
- [`payload() const : QByteArray`](qcoapmessage.md#payload)
- [`removeOption(QCoapOption::OptionName)`](qcoapmessage.md#removeOption)
- [`removeOption(const QCoapOption &)`](qcoapmessage.md#removeOption-1)
- [`setMessageId(quint16)`](qcoapmessage.md#setMessageId)
- [`setOptions(const QList<QCoapOption> &)`](qcoapmessage.md#setOptions)
- [`setPayload(const QByteArray &)`](qcoapmessage.md#setPayload)
- [`setToken(const QByteArray &)`](qcoapmessage.md#setToken)
- [`setType(const QCoapMessage::Type &)`](qcoapmessage.md#setType)
- [`setVersion(quint8)`](qcoapmessage.md#setVersion)
- [`swap(QCoapMessage &)`](qcoapmessage.md#swap)
- [`token() const : QByteArray`](qcoapmessage.md#token)
- [`tokenLength() const : quint8`](qcoapmessage.md#tokenLength)
- [`type() const : QCoapMessage::Type`](qcoapmessage.md#type)
- [`version() const : quint8`](qcoapmessage.md#version)
- [`operator=(QCoapMessage &&) : QCoapMessage &`](qcoapmessage.md#operator-eq)
- [`operator=(const QCoapMessage &) : QCoapMessage &`](qcoapmessage.md#operator-eq-1)

---

*Built with QDoc's template engine.*
