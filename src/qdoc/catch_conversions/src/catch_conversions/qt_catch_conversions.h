// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#pragma once

#include "std_catch_conversions.h"

#include <ostream>

#include <QChar>
#include <QString>

inline std::ostream& operator<<(std::ostream& os, const QChar& character) {
    return os << QString{character}.toStdString();
}

inline std::ostream& operator<<(std::ostream& os, const QString& string) {
    return os << string.toStdString();
}
