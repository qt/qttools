// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
QString asAsciiPrintable(const QString &name);
QStringList getInternalIncludePaths(const QString &compiler);
}

QT_END_NAMESPACE

#endif // UTILITIES_H
