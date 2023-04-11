// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "fileresolver.h"

#include "boundaries/filesystem/filepath.h"

#include <QDir>

#include <iostream>
#include <algorithm>

/*!
 * \class FileResolver
 * \brief Encapsulate the logic that QDoc uses to find files whose
 * path is provided by the user and that are relative to the current
 * configuration.
 *
 * A FileResolver instance is configured during creation, defining the
 * root directories that the search should be performed on.
 *
 * Afterwards, it can be used to resolve paths relative to those
 * directories, by querying through the resolve() method.
 *
 * Queries are resolved through a linear search through root
 * directories, finding at most one file each time.
 * A file is considered to be resolved if, from any root directory,
 * the query represents an existing file.
 *
 * For example, consider the following directory structure on some
 * filesystem:
 *
 * \badcode
 * foo/
 * |
 * |-bar/
 * |-|
 * | |-anotherfile.txt
 * |-file.txt
 * \endcode
 *
 * And consider an instance of FileResolver tha considers \e{foo/} to
 * be a root directory for search.
 *
 * Then, queries such as \e {bar/anotherfile.txt} and \e {file.txt}
 * will be resolved.
 *
 * Instead, queries such as \e {foobar.cpp}, \e {bar}, and \e
 * {foo/bar/anotherfile.txt} will not be resolved, as they do not
 * represent any file reachable from a root directory for search.
 *
 * It is important to note that FileResolver always searches its root
 * directories in an order that is based on the lexicographic ordering
 * of the path of its root directories.
 *
 * For example, consider the following directory structure on some
 * filesystem:
 *
 * \badcode
 * foo/
 * |
 * |-bar/
 * |-|
 * | |-file.txt
 * |-foobar/
 * |-|
 * | |-file.txt
 * \endcode
 *
 * And consider an instance of FileResolver that considers \e
 * {foo/bar/} and \e {foo/foobar/} to be root directories for search.
 *
 * Then, when the query \e {file.txt} is resolved, it will always
 * resolve to the file in \e {bar}, as \e {bar} will be searched
 * before \e {foobar}.
 *
 * We say that \e {foobar/file.txt} is shadowed by \e {bar/file.txt}.
 *
 * Currently, if this is an issue, it is possible to resolve it by
 * using a common ancestor as a root directory instead of using
 * multiples directories.
 *
 * In the previous example, if \e {foo} is instead chosen as the root
 * directory for search, then queries \e {bar/file.txt} and \e
 * {foobar/file.txt} can be used to uniquely resolve the two files,
 * removing the shadowing.
 * */

/*!
 * Constructs an instance of FileResolver with the directories in \a
 * search_directories as root directories for searching.
 *
 * Duplicates in \a search_directories do not affect the resolution of
 * files for the instance.
 *
 * For example, if \a search_directories contains some directory D
 * more than once, the constructed instance will resolve files
 * equivalently to an instance constructed with a single appearance of
 * D.
 *
 * The order of \a search_directories does not affect the resolution
 * of files for an instance.
 *
 * For example, if \a search_directories contains a permutation of
 * directories D1, D2, ..., Dn, then the constructed instance will
 * resolve files equivalently to an instance constructed from a
 * difference permutation of the same directories.
 */
FileResolver::FileResolver(std::vector<DirectoryPath>&& search_directories)
    : search_directories{std::move(search_directories)}
{
    std::sort(this->search_directories.begin(), this->search_directories.end());
    this->search_directories.erase (
        std::unique(this->search_directories.begin(), this->search_directories.end()),
        this->search_directories.end()
    );
}

// REMARK: Note that we do not treat absolute path specially.
// This will in general mean that they cannot get resolved (albeit
// there is a peculiar instance in which they can considering that
// most path formats treat multiple adjacent separators as one).
//
// While we need to treat them at some point with a specific
// intention, this was avoided at the current moment as it is
// unrequired to build the actual documentation.
//
// In particular, avoiding this choice now allows us to move it to a
// later stage where we can work with the origin of the data itself.
// User-inputted paths come into the picture during the configuration
// process and when parsing qdoc comments, there is a good chance that
// some amount of sophistication will be required to handle this data
// at the code level, for example to ensure that multiplatform
// handling of paths is performed correctly.
//
// This will then define how we should handle absolute paths, if we
// can receive them at all and so on.

/*!
* Returns a ResolvedFile if \a query can be resolved or std::nullopt
* otherwise.
*
* The returned ResolvedFile, if any, will contain the provided \a
* query and the path that the \a query was resolved to.
*/
[[nodiscard]] std::optional<ResolvedFile> FileResolver::resolve(QString query) const {
    for (auto& directory_path : search_directories) {
        auto maybe_filepath = FilePath::refine(QDir(directory_path.value() + "/" + query).path());
        if (maybe_filepath) return ResolvedFile{std::move(query), std::move(*maybe_filepath)};
    }

    return std::nullopt;
}

/*!
 * \fn FileResolver::get_search_directories() const
 *
 * Returns a const-reference to a collection of root search
 * directories that this instance will use during the resolution of
 * files.
 */
