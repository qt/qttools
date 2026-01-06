// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpclient.h"

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QString HelpClient::designerManualUrl() const
{
    return documentUrl(u"qtdesigner"_s);
}

QT_END_NAMESPACE
