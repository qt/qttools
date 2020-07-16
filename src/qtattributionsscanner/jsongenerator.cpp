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

#include "jsongenerator.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

#include <iostream>

namespace JsonGenerator {

static QJsonObject generate(Package package)
{
    QJsonObject obj;

    obj.insert(QStringLiteral("Id"), package.id);
    obj.insert(QStringLiteral("Path"), package.path);
    obj.insert(QStringLiteral("Files"), package.files.join(QLatin1Char(' ')));
    obj.insert(QStringLiteral("QDocModule"), package.qdocModule);
    obj.insert(QStringLiteral("Name"), package.name);
    obj.insert(QStringLiteral("QtUsage"), package.qtUsage);
    obj.insert(QStringLiteral("QtParts"), QJsonArray::fromStringList(package.qtParts));

    obj.insert(QStringLiteral("Description"), package.description);
    obj.insert(QStringLiteral("Homepage"), package.homepage);
    obj.insert(QStringLiteral("Version"), package.version);
    obj.insert(QStringLiteral("DownloadLocation"), package.downloadLocation);

    obj.insert(QStringLiteral("License"), package.license);
    obj.insert(QStringLiteral("LicenseId"), package.licenseId);
    if (package.licenseFiles.isEmpty())
        obj.insert(QStringLiteral("LicenseFile"), QString());
    else if (package.licenseFiles.size() == 1)
        obj.insert(QStringLiteral("LicenseFile"), package.licenseFiles.first());
    else
        obj.insert(QStringLiteral("LicenseFiles"),
                   QJsonArray::fromStringList(package.licenseFiles));

    obj.insert(QStringLiteral("Copyright"), package.copyright);
    obj.insert(QStringLiteral("PackageComment"), package.packageComment);

    return obj;
}

void generate(QTextStream &out, const QVector<Package> &packages, LogLevel logLevel)
{
    if (logLevel == VerboseLog)
        std::cerr << qPrintable(tr("Generating json...\n"));

    QJsonDocument document;
    QJsonArray array;
    for (const Package &package : packages)
        array.append(generate(package));
    document.setArray(array);

    out << document.toJson();
}

} // namespace JsonGenerator
