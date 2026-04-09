// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCOAPNAMESPACE_P_H
#define QCOAPNAMESPACE_P_H

#include "qcoapnamespace.h"
#include "private/qglobal_p.h"
#include <QtCore/qloggingcategory.h>


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

Q_DECLARE_LOGGING_CATEGORY(lcCoapExchange)
Q_DECLARE_LOGGING_CATEGORY(lcCoapConnection)

namespace QtCoap
{
    bool Q_AUTOTEST_EXPORT isError(QtCoap::ResponseCode code);
    Error Q_AUTOTEST_EXPORT errorForResponseCode(QtCoap::ResponseCode code);
    QRandomGenerator Q_AUTOTEST_EXPORT &randomGenerator();
}

QT_END_NAMESPACE

#endif // QCOAPNAMESPACE_P_H
