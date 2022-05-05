/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef SCANNER_H
#define SCANNER_H

#include "logging.h"
#include "package.h"

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

namespace Scanner {

enum class InputFormat {
    QtAttributions = 0x1, // qt_attributions.json
    ChromiumAttributions = 0x2, // README.chromium
};
Q_DECLARE_FLAGS(InputFormats, InputFormat)
Q_DECLARE_OPERATORS_FOR_FLAGS(InputFormats)

QList<Package> readFile(const QString &filePath, LogLevel logLevel);
QList<Package> scanDirectory(const QString &directory, InputFormats inputFormats, LogLevel logLevel);

}

#endif // SCANNER_H
