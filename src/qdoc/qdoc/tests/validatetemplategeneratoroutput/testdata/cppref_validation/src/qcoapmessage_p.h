// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPMESSAGE_P_H
#define QCOAPMESSAGE_P_H

#include <QtCoap/qcoapmessage.h>
#include <QtCore/qshareddata.h>
#include <private/qobject_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QCoapMessagePrivate : public QSharedData
{
public:
    QCoapMessagePrivate(QCoapMessage::Type type = QCoapMessage::Type::NonConfirmable);
    virtual ~QCoapMessagePrivate();

    virtual QCoapMessagePrivate *clone() const;

    QList<QCoapOption>::const_iterator findOption(QCoapOption::OptionName name) const;

    quint8 version = 1;
    QCoapMessage::Type type = QCoapMessage::Type::NonConfirmable;
    quint16 messageId = 0;
    QByteArray token;
    QList<QCoapOption> options;
    QByteArray payload;

protected:
    QCoapMessagePrivate(const QCoapMessagePrivate &other);
};

// don't use the copy constructor when detaching from a QSharedDataPointer,
// use virtual clone() call instead.
template <>
Q_INLINE_TEMPLATE QCoapMessagePrivate *QSharedDataPointer<QCoapMessagePrivate>::clone()
{
    return d->clone();
}

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_P_H
