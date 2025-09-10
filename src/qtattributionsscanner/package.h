// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
    bool securityCritical = false; // Whether code is security critical in the Qt module. Optional.
    QStringList qtParts; // Possible values are "examples", "tests", "tools", or "libs".
                         // "libs" is the default.

    QString description; // A short description of what the package is and is used for. Optional.
    QString homepage; // Homepage of the upstream project. Optional.
    QString version; // Version used from the upstream project. Optional.
    QString downloadLocation; // Link to exact upstream version. Optional.

    QString license; // The license under which the package is distributed. Mandatory.
    QString licenseId; // see https://spdx.org/licenses/. Optional.
    QStringList licenseFiles; // path to files containing the license text. Optional.
    QStringList licenseFilesContents;

    QString copyright; // Text with copyright owner. Optional.
    QString copyrightFile; // Path to file containing copyright owners. Optional.
    QString copyrightFileContents;

    QStringList cpeList; // List of CPE values. Optional.
    QStringList purlList; // List of PURL values. Optional.

    QString packageComment; // Further comments about the package. Optional.
};

#endif // PACKAGE_H
