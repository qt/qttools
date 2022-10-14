// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef SCANNER_H
#define SCANNER_H

#include "logging.h"
#include "package.h"

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

#include <optional>

namespace Scanner {

enum class InputFormat {
    QtAttributions = 0x1, // qt_attributions.json
    ChromiumAttributions = 0x2, // README.chromium
};
Q_DECLARE_FLAGS(InputFormats, InputFormat)
Q_DECLARE_OPERATORS_FOR_FLAGS(InputFormats)

std::optional<QList<Package>> readFile(const QString &filePath, LogLevel logLevel);
std::optional<QList<Package>> scanDirectory(const QString &directory, InputFormats inputFormats,
                                            LogLevel logLevel);
}

#endif // SCANNER_H
