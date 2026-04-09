// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapmessage_p.h"

QT_BEGIN_NAMESPACE

QCoapMessagePrivate::QCoapMessagePrivate(QCoapMessage::Type _type) :
    type(_type)
{
}

QCoapMessagePrivate::QCoapMessagePrivate(const QCoapMessagePrivate &other) :
    QSharedData(other),
    version(other.version), type(other.type), messageId(other.messageId),
    token(other.token), options(other.options), payload(other.payload)
{
}

QCoapMessagePrivate::~QCoapMessagePrivate()
{
}

QCoapMessagePrivate *QCoapMessagePrivate::clone() const
{
    return new QCoapMessagePrivate(*this);
}

/*!
    \class QCoapMessage
    \inmodule QtCoap

    \brief The QCoapMessage class holds information about a CoAP message that
    can be a request or a reply.

    \reentrant

    It holds information such as the message type, message id, token and other
    ancillary data.

    \sa QCoapOption, QCoapReply, QCoapRequest
*/

/*!
    \enum QCoapMessage::Type

    Indicates the type of the message.

    \value Confirmable                  A Confirmable message. The destination
                                        endpoint needs to acknowledge the
                                        message.
    \value NonConfirmable               A Non-Confirmable message. The
                                        destination endpoint does not need to
                                        acknowledge the message.
    \value Acknowledgment               An Acknowledgment message. A message
                                        sent or received in reply to a
                                        Confirmable message.
    \value Reset                        A Reset message. This message type is used
                                        in case of errors or to stop the ongoing
                                        transmission. (For example, it is used
                                        to cancel an observation).
*/

/*!
    \fn void QCoapMessage::swap(QCoapMessage &other)

    Swaps this message with \a other. This operation is very fast and never fails.
*/

/*!
    Constructs a new QCoapMessage.
*/
QCoapMessage::QCoapMessage() :
    d_ptr(new QCoapMessagePrivate)
{
}

/*!
    Destroys the QCoapMessage.
*/
QCoapMessage::~QCoapMessage()
{
}

/*!
    Constructs a shallow copy of \a other.
*/
QCoapMessage::QCoapMessage(const QCoapMessage &other) :
    d_ptr(other.d_ptr)
{
}

/*!
    \internal
    Constructs a new QCoapMessage with \a dd as the d_ptr.
    This constructor must be used internally when subclassing
    the QCoapMessage class.
*/
QCoapMessage::QCoapMessage(QCoapMessagePrivate &dd) :
    d_ptr(&dd)
{
}

/*!
    \overload

    Adds the CoAP option with the given \a name and \a value.
*/
void QCoapMessage::addOption(QCoapOption::OptionName name, const QByteArray &value)
{
    QCoapOption option(name, value);
    addOption(option);
}

/*!
    Adds the given CoAP \a option.
*/
void QCoapMessage::addOption(const QCoapOption &option)
{
    Q_D(QCoapMessage);

    const auto it = std::upper_bound(d->options.begin(), d->options.end(), option,
                                     [](const QCoapOption &a, const QCoapOption &b) -> bool {
                                         return a.name() < b.name();
                                     });
    const auto idx = std::distance(d->options.begin(), it);

    // Sort options by ascending order while inserting
    d->options.insert(idx, option);
}

/*!
    Removes the given \a option.
*/
void QCoapMessage::removeOption(const QCoapOption &option)
{
    Q_D(QCoapMessage);
    d->options.removeOne(option);
}

/*!
    Removes all options with the given \a name.
    The CoAP protocol allows for the same option to repeat.
*/
void QCoapMessage::removeOption(QCoapOption::OptionName name)
{
    Q_D(QCoapMessage);
    auto namesMatch = [name](const QCoapOption &option) {
        return option.name() == name;
    };

    auto &options = d->options;
    options.erase(std::remove_if(options.begin(), options.end(), namesMatch),
                  options.end());
}

/*!
    Removes all options.
*/
void QCoapMessage::clearOptions()
{
    Q_D(QCoapMessage);
    d->options.clear();
}

/*!
    Returns the CoAP version.

    \sa setVersion()
*/
quint8 QCoapMessage::version() const
{
    Q_D(const QCoapMessage);
    return d->version;
}

/*!
    Returns the message type.

    \sa setType()
*/
QCoapMessage::Type QCoapMessage::type() const
{
    Q_D(const QCoapMessage);
    return d->type;
}

/*!
    Returns the message token.

    \sa setToken()
*/
QByteArray QCoapMessage::token() const
{
    Q_D(const QCoapMessage);
    return d->token;
}

/*!
    Returns the token length.
*/
quint8 QCoapMessage::tokenLength() const
{
    Q_D(const QCoapMessage);
    return static_cast<quint8>(d->token.size());
}

/*!
    Returns the message id.

    \sa setMessageId()
*/
quint16 QCoapMessage::messageId() const
{
    Q_D(const QCoapMessage);
    return d->messageId;
}

/*!
    Returns the payload.

    \sa setPayload()
*/
QByteArray QCoapMessage::payload() const
{
    Q_D(const QCoapMessage);
    return d->payload;
}

/*!
    Returns the option at \a index position.
*/
QCoapOption QCoapMessage::optionAt(int index) const
{
    Q_D(const QCoapMessage);
    return d->options.at(index);
}

/*!
    Finds and returns the first option with the given \a name.
    If there is no such option, returns an invalid QCoapOption with an empty value.
*/
QCoapOption QCoapMessage::option(QCoapOption::OptionName name) const
{
    Q_D(const QCoapMessage);

    auto it = d->findOption(name);
    return it != d->options.end() ? *it : QCoapOption();
}

/*!
    \internal

    Finds and returns a constant iterator to the first option
    with the given \a name.
    If there is no such option, returns \c d->options.end().
*/
QList<QCoapOption>::const_iterator
QCoapMessagePrivate::findOption(QCoapOption::OptionName name) const
{
    return std::find_if(options.begin(), options.end(), [name](const QCoapOption &option) {
        return option.name() == name;
    });
}

/*!
    Returns \c true if the message contains at last one option
    with \a name.
*/
bool QCoapMessage::hasOption(QCoapOption::OptionName name) const
{
    Q_D(const QCoapMessage);
    return d->findOption(name) != d->options.end();
}

/*!
    Returns the list of options.
*/
const QList<QCoapOption> &QCoapMessage::options() const
{
    Q_D(const QCoapMessage);
    return d->options;
}

/*!
    Finds and returns the list of options with the given \a name.
*/
QList<QCoapOption> QCoapMessage::options(QCoapOption::OptionName name) const
{
    Q_D(const QCoapMessage);

    QList<QCoapOption> result;
    std::copy_if(d->options.cbegin(), d->options.cend(), std::back_inserter(result),
                 [name](const QCoapOption &option) {
                            return option.name() == name;
                 });
    return result;
}

/*!
    Returns the number of options.
*/
int QCoapMessage::optionCount() const
{
    Q_D(const QCoapMessage);
    return d->options.size();
}

/*!
    Sets the CoAP version to \a version.

    \sa version()
*/
void QCoapMessage::setVersion(quint8 version)
{
    Q_D(QCoapMessage);
    d->version = version;
}

/*!
    Sets the message type to \a type.

    \sa type()
*/
void QCoapMessage::setType(const Type &type)
{
    Q_D(QCoapMessage);
    d->type = type;
}

/*!
    Sets the message token to \a token.

    \sa token()
*/
void QCoapMessage::setToken(const QByteArray &token)
{
    Q_D(QCoapMessage);
    d->token = token;
}

/*!
    Sets the message ID to \a id.

    \sa messageId()
*/
void QCoapMessage::setMessageId(quint16 id)
{
    Q_D(QCoapMessage);
    d->messageId = id;
}

/*!
    Sets the message payload to \a payload. The payload can be represented in
    one of the content formats defined in \l {CoAP Content-Formats Registry}.

    \note CoAP supports common content formats such as XML, JSON, and so on, but
    these are text based and consequently heavy both in payload and in processing.
    One of the recommended content formats to use with CoAP is CBOR, which is
    designed to be used in such contexts.

    \sa payload(), QCborStreamWriter, QCborStreamReader
*/
void QCoapMessage::setPayload(const QByteArray &payload)
{
    Q_D(QCoapMessage);
    d->payload = payload;
}

/*!
    Sets the message options to \a options.
*/
void QCoapMessage::setOptions(const QList<QCoapOption> &options)
{
    Q_D(QCoapMessage);
    d->options = options;
}

void QCoapMessage::swap(QCoapMessage &other) noexcept
{
    qSwap(d_ptr, other.d_ptr);
}

/*!
    Moves \a other into this message and returns a reference to this QCoapMessage.
 */
QCoapMessage &QCoapMessage::operator=(QCoapMessage &&other) noexcept
{
    swap(other);
    return *this;
}

/*!
    Copies the contents of \a other into this message.
    Returns a reference to this QCoapMessage.
 */
QCoapMessage &QCoapMessage::operator=(const QCoapMessage &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    \internal

    For QSharedDataPointer.
*/
QCoapMessagePrivate* QCoapMessage::d_func()
{
    return d_ptr.data();
}

/*!
    \variable QCoapMessage::d_ptr
    \internal

    Pointer to private data structure.
*/

QT_END_NAMESPACE
