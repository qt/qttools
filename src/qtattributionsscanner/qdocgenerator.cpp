// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdocgenerator.h"

#include <QtCore/qdir.h>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

namespace QDocGenerator {

// See definition of idstring and licenseid in https://spdx.org/spdx-specification-21-web-version
static bool isSpdxLicenseId(const QString &str) {
    if (str.isEmpty())
        return false;
    for (auto iter(str.cbegin()); iter != str.cend(); ++iter) {
        const QChar c = *iter;
        if (!((c >= QLatin1Char('A') && c <= QLatin1Char('Z'))
              || (c >= QLatin1Char('a') && c <= QLatin1Char('z'))
              || (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
              || (c == QLatin1Char('-')) || (c == QLatin1Char('.'))))
            return false;
    }
    return true;
}

static QString languageJoin(const QStringList &list)
{
    QString result;
    for (int i = 0; i < list.size(); ++i) {
        QString delimiter = u", "_s;
        if (i == list.size() - 1) // last item
            delimiter.clear();
        else if (list.size() == 2)
            delimiter = u" and "_s;
        else if (list.size() > 2 && i == list.size() - 2)
            delimiter = u", and "_s; // oxford comma
        result += list[i] + delimiter;
    }

    return result;
}

// Embed source code between \badcode ... \endbadcode
// Also, avoid '*/' breaking qdoc by passing the star as argument
static void sourceCode(QTextStream &out, const QString &src)
{
    out << "\\badcode *\n";
    out << QString(src).replace(u"*/"_s, u"\\1/"_s);
    out << "\n\\endcode\n\n";
}

static void generate(QTextStream &out, const Package &package, const QDir &baseDir)
{
    out << "/*!\n\n";
    for (const QString &part : package.qtParts) {
        out << "\\ingroup attributions-" << package.qdocModule << "-" << part << "\n";
        out << "\\ingroup attributions-" << part << "\n";
    }

    if (package.qtParts.contains("libs"_L1)) {
        // show up in xxx-index.html page of module
        out << "\\ingroup attributions-" << package.qdocModule << "\n";
        // include in '\generatelist annotatedattributions'
        out << "\\page " << package.qdocModule << "-attribution-" << package.id
            << ".html\n";
        out << "\\attribution\n";
    } else {
        out << "\\page " << package.qdocModule << "-attribution-" << package.id
            << ".html \n";
    }

    out << "\\target " << package.id << "\n\n";
    out << "\\title " << package.name;
    if (!package.version.isEmpty())
        out << ", version " << package.version;
    out << "\n\n\\brief " << package.license << "\n\n";

    if (!package.description.isEmpty())
        out << package.description << "\n\n";

    if (!package.qtUsage.isEmpty())
        out << package.qtUsage << "\n\n";

    QStringList sourcePaths;
    if (package.files.isEmpty()) {
        sourcePaths << baseDir.relativeFilePath(package.path);
    } else {
        const QDir packageDir(package.path);
        for (const QString &filePath: package.files) {
            const QString absolutePath = packageDir.absoluteFilePath(filePath);
            sourcePaths << baseDir.relativeFilePath(absolutePath);
        }
    }

    out << "The sources can be found in " << languageJoin(sourcePaths) << ".\n\n";

    const bool hasPackageVersion = !package.version.isEmpty();
    const bool hasPackageDownloadLocation = !package.downloadLocation.isEmpty();
    if (!package.homepage.isEmpty()) {
        out << "\\l{" << package.homepage << "}{Project Homepage}";
        if (hasPackageVersion)
            out << ", ";
    }
    if (hasPackageVersion) {
        out << "upstream version: ";
        if (hasPackageDownloadLocation)
            out << "\\l{" << package.downloadLocation << "}{";
        out << package.version;
        if (hasPackageDownloadLocation)
            out << "}";
    }

    out << "\n\n";

    QString copyright;
    if (!package.copyright.isEmpty())
        copyright = package.copyright;
    else if (!package.copyrightFileContents.isEmpty())
        copyright = package.copyrightFileContents;

    if (!copyright.isEmpty()) {
        out << "\n";
        sourceCode(out, copyright);
    }

    if (isSpdxLicenseId(package.licenseId) && package.licenseId != "NONE"_L1) {
        out << "\\l{https://spdx.org/licenses/" << package.licenseId << ".html}"
            << "{" << package.license << "}.\n\n";
    } else if (package.licenseId.startsWith("urn:dje:license:"_L1)) {
        out << "\\l{https://enterprise.dejacode.com/licenses/public/" << package.licenseId.mid(16)
            << "/}{" << package.license << "}.\n\n";
    } else {
        out << package.license << ".\n\n";
    }

    foreach (const QString &license, package.licenseFilesContents)
        sourceCode(out, license);

    out << "*/\n";
}

void generate(QTextStream &out, const QList<Package> &packages, const QString &baseDirectory,
              LogLevel logLevel)
{
    if (logLevel == VerboseLog)
        std::cerr << qPrintable(tr("Generating qdoc file...")) << std::endl;

    QDir baseDir(baseDirectory);
    for (const Package &package : packages)
        generate(out, package, baseDir);
}

} // namespace QDocGenerator
