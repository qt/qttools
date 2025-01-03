// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "filesignificancecheck.h"

#include <mutex>

QT_BEGIN_NAMESPACE

FileSignificanceCheck *FileSignificanceCheck::m_instance = nullptr;

void FileSignificanceCheck::setRootDirectories(const QStringList &paths)
{
    const size_t pathsSize = static_cast<size_t>(paths.size());
    m_rootDirs.resize(pathsSize);
    for (size_t i = 0; i < pathsSize; ++i)
        m_rootDirs[i].setPath(paths.at(i));
}

void FileSignificanceCheck::setExclusionPatterns(const QStringList &patterns)
{
    const size_t patternsSize = static_cast<size_t>(patterns.size());
    m_exclusionRegExes.resize(patternsSize);
    for (size_t i = 0; i < patternsSize; ++i)
        m_exclusionRegExes[i] = QRegularExpression::fromWildcard(patterns.at(i));
}

/*
 * Return true if the given source file is significant for lupdate.
 * A file is considered insignificant if
 *   - it's not within any project root
 *   - it's excluded
 *
 * This method is called from multiple threads.
 * Results are cached.
 */
bool FileSignificanceCheck::isFileSignificant(const std::string &filePath) const
{
    // cache lookup
    std::shared_lock<std::shared_mutex> readLock(m_cacheMutex);
    auto it = m_cache.find(filePath);
    if (it != m_cache.end())
        return it->second;

    // cache miss
    readLock.unlock();
    std::unique_lock<std::shared_mutex> writeLock(m_cacheMutex);
    QString file = QString::fromUtf8(filePath);
    QString cleanFile = QDir::cleanPath(file);
    for (const QRegularExpression &rx : m_exclusionRegExes) {
        if (rx.match(cleanFile).hasMatch()) {
            m_cache.insert({filePath, false});
            return false;
        }
    }

    for (const QDir &rootDir : m_rootDirs) {
        QString relativeFilePath = rootDir.relativeFilePath(file);
        if (!relativeFilePath.startsWith(QLatin1String("../"))
            && QFileInfo(relativeFilePath).isRelative()) {
            m_cache.insert({filePath, true});
            return true;
        }
    }

    m_cache.insert({filePath, false});
    return false;
}

QT_END_NAMESPACE
