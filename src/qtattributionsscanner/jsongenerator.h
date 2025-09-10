// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef JSONGENERATOR
#define JSONGENERATOR

#include "logging.h"
#include "package.h"

#include <QtCore/qtextstream.h>
#include <QtCore/qlist.h>

namespace JsonGenerator {

void generate(QTextStream &out, const QList<Package> &packages, LogLevel logLevel);

} // namespace JsonGenerator

#endif // JSONGENERATOR
