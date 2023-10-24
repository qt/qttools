// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

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
