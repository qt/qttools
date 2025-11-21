// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROJSONGENERATOR_H
#define PROJSONGENERATOR_H

#include <QtCore/qjsonarray.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

std::optional<QJsonArray> generateProjectDescription(const QStringList &proFiles,
                                                     const QStringList &translationsVariables,
                                                     const QHash<QString, QString> &outDirMap,
                                                     int proDebug, bool verbose);

QT_END_NAMESPACE

#endif // PROJSONGENERATOR_H
