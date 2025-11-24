// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PROFILEUTILS_H
#define PROFILEUTILS_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

inline bool isProOrPriFile(const QString &filePath)
{
    return filePath.endsWith(".pro"_L1, Qt::CaseInsensitive)
            || filePath.endsWith(".pri"_L1, Qt::CaseInsensitive);
}

inline QStringList extractProFiles(QStringList *files)
{
    QStringList result;
    auto it = std::remove_if(files->begin(), files->end(), &isProOrPriFile);
    if (it == files->end())
         return result;
    std::move(it, files->end(), std::back_inserter(result));
    files->erase(it, files->end());
    return result;
}

#endif // PROFILEUTILS_H
