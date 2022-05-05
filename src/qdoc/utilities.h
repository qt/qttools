/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore/qstring.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQdoc)
Q_DECLARE_LOGGING_CATEGORY(lcQdocClang)

namespace Utilities {
void startDebugging(const QString &message);
void stopDebugging(const QString &message);
bool debugging();

QString separator(qsizetype wordPosition, qsizetype numberOfWords);
QString comma(qsizetype wordPosition, qsizetype numberOfWords);
}

QT_END_NAMESPACE

#endif // UTILITIES_H
