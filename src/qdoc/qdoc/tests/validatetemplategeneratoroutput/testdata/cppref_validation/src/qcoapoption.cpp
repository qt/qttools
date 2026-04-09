// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcoapoption_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcCoapOption, "qt.coap.option")

/*!
    \class QCoapOption
    \inmodule QtCoap

    \brief The QCoapOption class holds data about CoAP options.

    \reentrant

    CoAP defines a number of options that can be included in a message.
    Both requests and responses may include a list of one or more
    options.  For example, the URI in a request is transported in several
    options, and metadata that would be carried in an HTTP header in HTTP
    is supplied as options as well.

    An option contains a name, related to an option ID, and a value.
    The name is one of the values from the OptionName enumeration.
*/

/*!
    \enum QCoapOption::OptionName

    Indicates the name of an option.
    The value of each ID is as specified by the CoAP standard, with the
    exception of Invalid. You can refer to
    \l{https://tools.ietf.org/html/rfc7252#section-5.10}{RFC 7252} and
    \l{https://tools.ietf.org/html/rfc7959#section-2.1}{RFC 7959} for more details.

    \value Invalid                  An invalid option.
    \value IfMatch                  If-Match option.
    \value UriHost                  Uri-Host option.
    \value Etag                     Etag option.
    \value IfNoneMatch              If-None-Match option.
    \value Observe                  Observe option.
    \value UriPort                  Uri-Port option.
    \value LocationPath             Location-path option.
    \value UriPath                  Uri-Path option.
    \value ContentFormat            Content-Format option.
    \value MaxAge                   Max-Age option.
    \value UriQuery                 Uri-Query option.
    \value Accept                   Accept option.
    \value LocationQuery            Location-Query option.
    \value Block2                   Block2 option.
    \value Block1                   Block1 option.
    \value Size2                    Size2 option.
    \value ProxyUri                 Proxy-Uri option.
    \value ProxyScheme              Proxy-Scheme option.
    \value Size1                    Size1 option.
*/

/*!
    Constructs a new CoAP option with the given \a name
    and QByteArray \a opaqueValue.
    If no parameters are passed, constructs an Invalid object.

    \sa isValid()
 */
QCoapOption::QCoapOption(OptionName name, const QByteArray &opaqueValue) :
    d_ptr(new QCoapOptionPrivate)
{
    Q_D(QCoapOption);
    d->name = name;
    d->setValue(opaqueValue);
}

/*!
    Constructs a new CoAP option with the given \a name
    and the QString \a stringValue.

    \sa isValid()
 */
QCoapOption::QCoapOption(OptionName name, const QString &stringValue) :
    d_ptr(new QCoapOptionPrivate)
{
    Q_D(QCoapOption);
    d->name = name;
    d->setValue(stringValue);
}

/*!
    Constructs a new CoAP option with the given \a name
    and the unsigned integer \a intValue.

    \sa isValid()
 */
QCoapOption::QCoapOption(OptionName name, quint32 intValue) :
    d_ptr(new QCoapOptionPrivate)
{
    Q_D(QCoapOption);
    d->name = name;
    d->setValue(intValue);
}

/*!
    Constructs a new CoAP option as a copy of \a other, making the two
    options identical.

    \sa isValid()
 */
QCoapOption::QCoapOption(const QCoapOption &other) :
    d_ptr(new QCoapOptionPrivate(*other.d_ptr))
{
}

/*!
    Move-constructs a QCoapOption, making it point to the same object
    as \a other was pointing to.
 */
QCoapOption::QCoapOption(QCoapOption &&other) :
    d_ptr(other.d_ptr)
{
    other.d_ptr = nullptr;
}

/*!
    Destroys the QCoapOption object.
 */
QCoapOption::~QCoapOption()
{
    delete d_ptr;
}

/*!
    Copies \a other into this option, making the two options identical.
    Returns a reference to this QCoapOption.
 */
QCoapOption &QCoapOption::operator=(const QCoapOption &other)
{
    QCoapOption copy(other);
    swap(copy);
    return *this;
}

/*!
    Moves \a other into this option and returns a reference to this QCoapOption.
 */
QCoapOption &QCoapOption::operator=(QCoapOption &&other) noexcept
{
    swap(other);
    return *this;
}

/*!
    Swaps this option with \a other. This operation is very fast and never fails.
 */
void QCoapOption::swap(QCoapOption &other) noexcept
{
    qt_ptr_swap(d_ptr, other.d_ptr);
}

/*!
    Returns the value of the option.
 */
QByteArray QCoapOption::opaqueValue() const
{
    Q_D(const QCoapOption);
    return d->value;
}

/*!
    Returns the integer value of the option.
 */
quint32 QCoapOption::uintValue() const
{
    Q_D(const QCoapOption);

    quint32 intValue = 0;
    for (int i = 0; i < d->value.size(); i++)
        intValue |= static_cast<quint8>(d->value.at(i)) << (8 * i);

    return intValue;
}

/*!
    Returns the QString value of the option.
*/
QString QCoapOption::stringValue() const
{
    Q_D(const QCoapOption);
    return QString::fromUtf8(d->value);
}

/*!
    Returns the length of the value of the option.
 */
int QCoapOption::length() const
{
    Q_D(const QCoapOption);
    return d->value.size();
}

/*!
    Returns the name of the option.
 */
QCoapOption::OptionName QCoapOption::name() const
{
    Q_D(const QCoapOption);
    return d->name;
}

/*!
    Returns \c true if the option is valid.
 */
bool QCoapOption::isValid() const
{
    Q_D(const QCoapOption);
    return d->name != QCoapOption::Invalid;
}

/*!
    Returns \c true if this QCoapOption and \a other are equals.
 */
bool QCoapOption::operator==(const QCoapOption &other) const
{
    Q_D(const QCoapOption);
    return (d->name == other.d_ptr->name
            && d->value == other.d_ptr->value);
}

/*!
    Returns \c true if this QCoapOption and \a other are different.
 */
bool QCoapOption::operator!=(const QCoapOption &other) const
{
    return !(*this == other);
}

/*!
    \internal

    Sets the \a value for the option.
 */
void QCoapOptionPrivate::setValue(const QByteArray &opaqueValue)
{
    bool oversized = false;

    // Check for value maximum size, according to section 5.10 of RFC 7252
    // https://tools.ietf.org/html/rfc7252#section-5.10
    switch (name) {
    case QCoapOption::IfNoneMatch:
        if (opaqueValue.size() > 0)
            oversized = true;
        break;

    case QCoapOption::UriPort:
    case QCoapOption::ContentFormat:
    case QCoapOption::Accept:
        if (opaqueValue.size() > 2)
            oversized = true;
        break;

    case QCoapOption::MaxAge:
    case QCoapOption::Size1:
        if (opaqueValue.size() > 4)
            oversized = true;
        break;

    case QCoapOption::IfMatch:
    case QCoapOption::Etag:
        if (opaqueValue.size() > 8)
            oversized = true;
        break;

    case QCoapOption::UriHost:
    case QCoapOption::LocationPath:
    case QCoapOption::UriPath:
    case QCoapOption::UriQuery:
    case QCoapOption::LocationQuery:
    case QCoapOption::ProxyScheme:
        if (opaqueValue.size() > 255)
            oversized = true;
        break;

    case QCoapOption::ProxyUri:
        if (opaqueValue.size() > 1034)
            oversized = true;
        break;

    case QCoapOption::Observe:
    case QCoapOption::Block2:
    case QCoapOption::Block1:
    case QCoapOption::Size2:
    default:
        break;
    }

    if (oversized)
        qCWarning(lcCoapOption) << "Value" << opaqueValue << "is probably too big for option" << name;

    value = opaqueValue;
}

/*!
    \internal
    \overload

    Sets the \a value for the option.
 */
void QCoapOptionPrivate::setValue(const QString &value)
{
    setValue(value.toUtf8());
}

/*!
    \internal
    \overload

    Sets the \a value for the option.
 */
void QCoapOptionPrivate::setValue(quint32 value)
{
    QByteArray data;
    for (; value; value >>= 8)
        data.append(static_cast<qint8>(value & 0xFF));

    setValue(data);
}

/*!
    \internal

    For QSharedDataPointer.
*/
QCoapOptionPrivate *QCoapOption::d_func()
{
    return d_ptr;
}

QT_END_NAMESPACE
