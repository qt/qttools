/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
****************************************************************************/

#include "utilities.h"
#include "loggingcategory.h"

/*!
    \namespace Utilities
    \internal
    \brief This namespace holds QDoc-internal utility methods.
 */
namespace Utilities {
static inline void setDebugEnabled(bool value)
{
    const_cast<QLoggingCategory &>(lcQdoc()).setEnabled(QtDebugMsg, value);
}

void startDebugging(const QString &message)
{
    setDebugEnabled(true);
    qCDebug(lcQdoc, "START DEBUGGING: %ls", qUtf16Printable(message));
}

void stopDebugging(const QString &message)
{
    qCDebug(lcQdoc, "STOP DEBUGGING: %ls", qUtf16Printable(message));
    setDebugEnabled(false);
}

bool debugging()
{
    return lcQdoc().isEnabled(QtDebugMsg);
}
}
