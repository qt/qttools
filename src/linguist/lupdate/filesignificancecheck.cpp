/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

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
