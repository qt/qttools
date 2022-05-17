// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDOCGENERATOR_H
#define QDOCGENERATOR_H

#include "logging.h"
#include "package.h"

#include <QtCore/qtextstream.h>
#include <QtCore/qlist.h>

namespace QDocGenerator {

void generate(QTextStream &out, const QList<Package> &packages, const QString &baseDirectory,
              LogLevel logLevel);

} // namespace QDocGenerator

#endif // QDOCGENERATOR_H
