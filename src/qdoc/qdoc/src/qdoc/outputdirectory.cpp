// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "outputdirectory.h"

#include "location.h"

#include <QtCore/qfileinfo.h>

#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class OutputDirectory
    \internal
    \brief Represents an output directory that has been verified to exist.

    OutputDirectory is a simple value type that wraps a QString representing
    a filesystem path to a directory. Instances are created via
    OutputDirectory::ensure() and OutputDirectory::ensureSubdir() after
    ensuring the directory exists.

    This class provides a type-safe way to pass around directory paths that
    are known to be valid. This reduces the need for repeated existence checks.

    \section1 Guarantees

    Instances created via ensure() or ensureSubdir() provide the following
    guarantees:

    \list
        \li path() refers to an existing directory.
        \li Directory creation failures terminate QDoc via Location::fatal().
        \li All paths are normalized using QDir::cleanPath().
        \li ensureSubdir() accepts multi-segment relative paths
            (such as "images/used-in-examples").
        \li ensureSubdir() prevents path traversal attacks using ".."
            components.
    \endlist

    \note The public constructor doesn't perform existence checks and is
          intended primarily for internal use by the factory methods. Avoid
          direct construction in favor of ensure() or ensureSubdir().

    \note These methods are designed for single-threaded use during QDoc's
          generation phase and are not synchronized.

    \sa OutputDirectory::ensure(), OutputDirectory::ensureSubdir()
*/

/*!
    Constructs an OutputDirectory for the given \a path.

    The \a path is normalized using QDir::cleanPath() and moved into the object.
    This constructor is intentionally public to allow the static factory methods
    to create instances, but users should not construct OutputDirectory
    instances directly—they should use OutputDirectory::ensure() or
    ensureSubdir() instead.
*/
OutputDirectory::OutputDirectory(QString path) noexcept
    : m_path(QDir::cleanPath(std::move(path)))
{
}

/*!
    Returns the absolute file path for a file named \a fileName within
    this output directory.

    For example, if the output directory is "/tmp/doc" and \e fileName is
    "index.html", this returns "/tmp/doc/index.html".
*/
QString OutputDirectory::absoluteFilePath(QStringView fileName) const
{
    return QDir(m_path).absoluteFilePath(fileName.toString());
}

/*!
    Ensures that an output directory exists at \a path and creates it if
    necessary.

    If \a path exists but is not a directory, or if the directory cannot be
    created, reports a fatal error using \a location and terminates QDoc
    execution.

    Returns an OutputDirectory representing the created or existing directory.

    \sa ensureSubdir()
*/
OutputDirectory OutputDirectory::ensure(QStringView path, const Location &location)
{
    const QString cleanPath = QDir::cleanPath(path.toString());

    const QFileInfo fi(cleanPath);
    if (fi.exists() && !fi.isDir())
        location.fatal(u"'%1' exists and is not a directory"_s.arg(cleanPath));

    QDir dir(cleanPath);
    if (!dir.exists() && !dir.mkpath("."_L1))
        location.fatal(u"Cannot create output directory '%1'"_s.arg(cleanPath));

    return OutputDirectory(cleanPath);
}

/*!
    Ensures that a subdirectory named \a subdirName exists within this
    output directory, creating it if necessary.

    The \a subdirName must be a relative path. Absolute paths and paths that
    traverse outside the parent directory using ".." are rejected with a fatal
    error to prevent accidental writes outside the output tree.

    If the directory cannot be created, reports a fatal error using \a location
    and terminates QDoc execution.

    Returns an OutputDirectory representing the created or existing subdirectory.

    \sa ensure()
*/
OutputDirectory OutputDirectory::ensureSubdir(QStringView subdirName,
                                               const Location &location) const
{
    const QString subdirNameStr = subdirName.toString();

    if (QDir::isAbsolutePath(subdirNameStr))
        location.fatal(u"Subdirectory name must be relative, not absolute: '%1'"_s.arg(subdirNameStr));

    const QString subdirPath = QDir(m_path).filePath(subdirNameStr);
    const QString cleanSubdirPath = QDir::cleanPath(subdirPath);

    const QString cleanParentPath = QDir::cleanPath(m_path);
    const QDir parentDir(cleanParentPath);
    const QString relativePath = parentDir.relativeFilePath(cleanSubdirPath);

    if (relativePath.startsWith(".."_L1 + u'/') || relativePath == ".."_L1) {
        location.fatal(u"Refusing to create subdirectory outside parent: '%1' (parent: '%2')"_s
                               .arg(cleanSubdirPath, cleanParentPath));
    }

    return ensure(cleanSubdirPath, location);
}

QT_END_NAMESPACE

