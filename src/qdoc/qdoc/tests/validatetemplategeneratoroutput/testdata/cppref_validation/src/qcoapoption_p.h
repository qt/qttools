// Copyright (C) 2017 Witekio.
// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPOPTION_P_H
#define QCOAPOPTION_P_H

#include <QtCoap/qcoapoption.h>
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

class Q_AUTOTEST_EXPORT QCoapOptionPrivate
{
public:
    QCoapOptionPrivate() = default;

    void setValue(const QByteArray &opaqueValue);
    void setValue(const QString &value);
    void setValue(quint32 value);

    QCoapOption::OptionName name = QCoapOption::Invalid;
    QByteArray value;
};

QT_END_NAMESPACE

#endif // QCOAPOPTION_P_H
