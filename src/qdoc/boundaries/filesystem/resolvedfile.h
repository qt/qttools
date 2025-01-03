// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "boundaries/filesystem/filepath.h"

#include <QString>

struct ResolvedFile {
public:
    ResolvedFile(QString query, FilePath filepath) : query{query}, filepath{filepath} {}

    [[nodiscard]] const QString& get_query() const { return query; }
    [[nodiscard]] const QString& get_path() const { return filepath.value(); }

private:
    QString query;
    FilePath filepath;
};
