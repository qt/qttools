/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef PACKAGE_H
#define PACKAGE_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

struct Package {
    QString id; // Usually a lowercase, no-spaces version of the name. Mandatory.
    QString path; // Source directory. Optional.
                  // Default is the directory of the qt_attribution.json file.
    QStringList files; // Files in path. Optional.
    QString name; // Descriptive name of the package. Will be used as the title. Mandatory.
    QString qdocModule; // QDoc module where the documentation should be included. Mandatory.
    QString qtUsage; // How the package is used in Qt. Any way to disable? Mandatory.
    QStringList qtParts; // Possible values are "examples", "tests", "tools", or "libs".
                         // "libs" is the default.

    QString description; // A short description of what the package is and is used for. Optional.
    QString homepage; // Homepage of the upstream project. Optional.
    QString version; // Version used from the upstream project. Optional.
    QString downloadLocation; // Link to exact upstream version. Optional.

    QString license; // The license under which the package is distributed. Mandatory.
    QString licenseId; // see https://spdx.org/licenses/. Optional.
    QStringList licenseFiles; // path to files containing the license text. Optional.

    QString copyright; // A list of copyright owners. Mandatory.

    QString packageComment; // Further comments about the package. Optional.
};

#endif // PACKAGE_H
