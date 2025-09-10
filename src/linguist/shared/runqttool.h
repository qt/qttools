// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef RUNQTTOOL_H
#define RUNQTTOOL_H

#include <QtCore/qlibraryinfo.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qtemporaryfile.h>

#include <memory>

void runQtTool(const QString &toolName, const QStringList &arguments,
               QLibraryInfo::LibraryPath location = QLibraryInfo::BinariesPath);
void runInternalQtTool(const QString &toolName, const QStringList &arguments);
std::unique_ptr<QTemporaryFile> createProjectDescription(QStringList args);

#endif // RUNQTTOOL_H
