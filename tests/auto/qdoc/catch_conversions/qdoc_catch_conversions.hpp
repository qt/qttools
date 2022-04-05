/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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
****************************************************************************/

#pragma once

#include "qt_catch_conversions.hpp"

#include <boundaries/filesystem/directorypath.hpp>
#include <boundaries/filesystem/filepath.hpp>
#include <boundaries/filesystem/resolvedfile.hpp>

#include <ostream>

inline std::ostream& operator<<(std::ostream& os, const DirectoryPath& dirpath) {
    return os << dirpath.value().toStdString();
}

inline std::ostream& operator<<(std::ostream& os, const FilePath& filepath) {
    return os << filepath.value().toStdString();
}

inline std::ostream& operator<<(std::ostream& os, const ResolvedFile& resolved_file) {
    return os << "ResolvedFile{ query: " << resolved_file.get_query().toStdString() << ", " << "filepath: " << resolved_file.get_path() << " }";
}
