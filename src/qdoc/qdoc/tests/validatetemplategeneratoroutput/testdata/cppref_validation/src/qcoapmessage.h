// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapoption.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate;
class Q_COAP_EXPORT QCoapMessage
{
public:
    enum class Type : quint8 {
        Confirmable,
        NonConfirmable,
        Acknowledgment,
        Reset
    };

    QCoapMessage();
    QCoapMessage(const QCoapMessage &other);
    ~QCoapMessage();

    void swap(QCoapMessage &other) noexcept;
    QCoapMessage &operator=(const QCoapMessage &other);
    QCoapMessage &operator=(QCoapMessage &&other) noexcept;

    quint8 version() const;
    Type type() const;
    QByteArray token() const;
    quint8 tokenLength() const;
    quint16 messageId() const;
    QByteArray payload() const;
    void setVersion(quint8 version);
    void setType(const Type &type);
    void setToken(const QByteArray &token);
    void setMessageId(quint16);
    void setPayload(const QByteArray &payload);
    void setOptions(const QList<QCoapOption> &options);

    QCoapOption optionAt(int index) const;
    QCoapOption option(QCoapOption::OptionName name) const;
    bool hasOption(QCoapOption::OptionName name) const;
    const QList<QCoapOption> &options() const;
    QList<QCoapOption> options(QCoapOption::OptionName name) const;
    int optionCount() const;
    void addOption(QCoapOption::OptionName name, const QByteArray &value = QByteArray());
    void addOption(const QCoapOption &option);
    void removeOption(const QCoapOption &option);
    void removeOption(QCoapOption::OptionName name);
    void clearOptions();

protected:
    explicit QCoapMessage(QCoapMessagePrivate &dd);

    QSharedDataPointer<QCoapMessagePrivate> d_ptr;

    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    inline QCoapMessagePrivate *d_func();
    const QCoapMessagePrivate *d_func() const { return d_ptr.constData(); }
};

Q_DECLARE_SHARED(QCoapMessage)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCoapMessage)
Q_DECLARE_METATYPE(QCoapMessage::Type)

#endif // QCOAPMESSAGE_H
