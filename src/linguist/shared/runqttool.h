/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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
