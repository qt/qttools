// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef OUTPUTDIRECTORY_H
#define OUTPUTDIRECTORY_H

#include <QtCore/qdir.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringview.h>

QT_BEGIN_NAMESPACE

class Location;

class OutputDirectory
{
public:
    explicit OutputDirectory(QString path) noexcept;

    OutputDirectory(const OutputDirectory &) = default;
    OutputDirectory(OutputDirectory &&) noexcept = default;
    OutputDirectory &operator=(const OutputDirectory &) = default;
    OutputDirectory &operator=(OutputDirectory &&) noexcept = default;
    ~OutputDirectory() = default;

    [[nodiscard]] static OutputDirectory ensure(QStringView path, const Location &location);
    [[nodiscard]] OutputDirectory ensureSubdir(QStringView subdirName,
                                                const Location &location) const;

    [[nodiscard]] const QString &path() const noexcept { return m_path; }
    [[nodiscard]] QString absoluteFilePath(QStringView fileName) const;
    [[nodiscard]] QDir toQDir() const { return QDir(m_path); }

    [[nodiscard]] friend bool operator==(const OutputDirectory &lhs,
                                         const OutputDirectory &rhs) noexcept
    {
        return lhs.m_path == rhs.m_path;
    }
    [[nodiscard]] friend bool operator!=(const OutputDirectory &lhs,
                                         const OutputDirectory &rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    QString m_path;
};

QT_END_NAMESPACE

#endif // OUTPUTDIRECTORY_H

