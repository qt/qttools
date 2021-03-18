/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PROFILEUTILS_H
#define PROFILEUTILS_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#include <algorithm>

inline bool isProOrPriFile(const QString &filePath)
{
    return filePath.endsWith(QLatin1String(".pro"), Qt::CaseInsensitive)
            || filePath.endsWith(QLatin1String(".pri"), Qt::CaseInsensitive);
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
