// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "jsongenerator.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

namespace JsonGenerator {

static QJsonObject generate(Package package)
{
    QJsonObject obj;

    obj.insert(u"Id"_s, package.id);
    obj.insert(u"Path"_s, package.path);
    obj.insert(u"Files"_s, package.files.join(QLatin1Char(' ')));
    obj.insert(u"QDocModule"_s, package.qdocModule);
    obj.insert(u"Name"_s, package.name);
    obj.insert(u"QtUsage"_s, package.qtUsage);
    obj.insert(u"SecurityCritical"_s, package.securityCritical);
    obj.insert(u"QtParts"_s, QJsonArray::fromStringList(package.qtParts));

    obj.insert(u"Description"_s, package.description);
    obj.insert(u"Homepage"_s, package.homepage);
    obj.insert(u"Version"_s, package.version);
    obj.insert(u"DownloadLocation"_s, package.downloadLocation);

    obj.insert(u"License"_s, package.license);
    obj.insert(u"LicenseId"_s, package.licenseId);
    if (package.licenseFiles.isEmpty())
        obj.insert(u"LicenseFile"_s, QString());
    else if (package.licenseFiles.size() == 1)
        obj.insert(u"LicenseFile"_s, package.licenseFiles.first());
    else
        obj.insert(u"LicenseFiles"_s,
                   QJsonArray::fromStringList(package.licenseFiles));

    obj.insert(u"Copyright"_s, package.copyright);
    obj.insert(u"CopyrightFile"_s, package.copyrightFile);
    obj.insert(u"PackageComment"_s, package.packageComment);

    return obj;
}

void generate(QTextStream &out, const QList<Package> &packages, LogLevel logLevel)
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
