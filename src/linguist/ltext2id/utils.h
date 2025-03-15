// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UTILS_H
#define UTILS_H

#include <QMultiHash>

QT_BEGIN_NAMESPACE

class QFileInfo;

namespace Utils {

void printErr(const QString &out);
void printOut(const QString &out);

QMultiHash<QString, QString> getIncludeOptions(const QFileInfo &root, const QStringList &paths);
QString getIndentation(const QString &line);

QStringList readLines(const QString &filename);
void writeLines(const QString &filename, const QStringList &lines);

} // namespace Utils

QT_END_NAMESPACE

#endif // UTILS_H
