// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "filesignificancecheck.h"

QT_BEGIN_NAMESPACE

FileSignificanceCheck *FileSignificanceCheck::m_instance = nullptr;

void FileSignificanceCheck::setRootDirectories(const QStringList &paths)
{
    const size_t pathsSize = static_cast<size_t>(paths.size());
    m_rootDirs.resize(pathsSize);
    for (size_t i = 0; i < pathsSize; ++i)
        m_rootDirs[i].setPath(paths.at(i));
}

void FileSignificanceCheck::setExclusionRegExes(const QVector<QRegularExpression> &expressions)
{
    m_exclusionRegExes = expressions;
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
    {
        QReadLocker locker(&m_cacheLock);
        auto it = m_cache.find(filePath);
        if (it != m_cache.end())
            return it->second;
    }

    // cache miss
    QWriteLocker locker(&m_cacheLock);
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
