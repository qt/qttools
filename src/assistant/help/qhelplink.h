// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef QHELPLINK_H
#define QHELPLINK_H

#include <QtHelp/qhelp_global.h>

#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

struct QHELP_EXPORT QHelpLink final
{
    QUrl url;
    QString title;
};

QT_END_NAMESPACE

#endif // QHELPLINK_H
