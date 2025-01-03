// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "boundaries/filesystem/directorypath.h"
#include "boundaries/filesystem/resolvedfile.h"

#include <optional>
#include <vector>

#include <QtCore/qstring.h>

class FileResolver {
public:
    FileResolver(std::vector<DirectoryPath>&& search_directories);

    [[nodiscard]] std::optional<ResolvedFile> resolve(QString filename) const;

    [[nodiscard]] const std::vector<DirectoryPath>& get_search_directories() const { return search_directories; }

private:
    std::vector<DirectoryPath> search_directories;
};
