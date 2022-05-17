// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FILESIGNIFICANCECHECK_H
#define FILESIGNIFICANCECHECK_H

#include <QtCore/qdir.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstringlist.h>

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

QT_BEGIN_NAMESPACE

class FileSignificanceCheck
{
public:
    FileSignificanceCheck() = default;

    static void create()
    {
        m_instance = new FileSignificanceCheck;
    }

    static void destroy()
    {
        delete m_instance;
        m_instance = nullptr;
    }

    static FileSignificanceCheck *the()
    {
        return m_instance;
    }

    void setRootDirectories(const QStringList &paths);
    void setExclusionPatterns(const QStringList &patterns);

    bool isFileSignificant(const std::string &filePath) const;

private:
    static FileSignificanceCheck *m_instance;
    std::vector<QDir> m_rootDirs;
    std::vector<QRegularExpression> m_exclusionRegExes;
    mutable std::unordered_map<std::string, bool> m_cache;
    mutable std::shared_mutex m_cacheMutex;
};

namespace LupdatePrivate {

inline bool isFileSignificant(const std::string &filePath)
{
    return FileSignificanceCheck::the()->isFileSignificant(filePath);
}

} // namespace LupdatePrivate

QT_END_NAMESPACE

#endif // header guard
