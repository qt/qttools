// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "scanner.h"
#include "logging.h"

#include <QtCore/qdir.h>
#include <QtCore/qhash.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qvariant.h>

#include <iostream>

using namespace Qt::Literals::StringLiterals;

namespace Scanner {

static void missingPropertyWarning(const QString &filePath, const QString &property)
{
    std::cerr << qPrintable(tr("File %1: Missing mandatory property '%2'.").arg(
                                QDir::toNativeSeparators(filePath), property)) << std::endl;
}

static bool validatePackage(Package &p, const QString &filePath, LogLevel logLevel)
{
    bool validPackage = true;

    if (p.qtParts.isEmpty())
        p.qtParts << QStringLiteral("libs");

    if (p.name.isEmpty()) {
        if (p.id.startsWith(QLatin1String("chromium-"))) // Ignore invalid README.chromium files
            return false;

        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, QStringLiteral("Name"));
        validPackage = false;
    }

    if (p.id.isEmpty()) {
        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, QStringLiteral("Id"));
        validPackage = false;
    }
    if (p.license.isEmpty()) {
        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, QStringLiteral("License"));
        validPackage = false;
    }

    if (!p.copyright.isEmpty() && !p.copyrightFile.isEmpty()) {
        if (logLevel != SilentLog) {
            std::cerr << qPrintable(tr("File %1: Properties 'Copyright' and 'CopyrightFile' are "
                                       "mutually exclusive.")
                                            .arg(QDir::toNativeSeparators(filePath)))
                      << std::endl;
        }
        validPackage = false;
    }

    for (const QString &part : std::as_const(p.qtParts)) {
        if (part != QLatin1String("examples")
            && part != QLatin1String("tests")
            && part != QLatin1String("tools")
            && part != QLatin1String("libs")
            && logLevel != SilentLog) {
            std::cerr << qPrintable(tr("File %1: Property 'QtPart' contains unknown element "
                                       "'%2'. Valid entries are 'examples', 'tests', 'tools' "
                                       "and 'libs'.").arg(
                                           QDir::toNativeSeparators(filePath), part))
                      << std::endl;
            validPackage = false;
        }
    }

    return validPackage;
}

static std::optional<QStringList> toStringList(const QJsonValue &value)
{
    if (!value.isArray())
        return std::nullopt;
    QStringList result;
    for (auto iter : value.toArray()) {
        if (iter.type() != QJsonValue::String)
            return std::nullopt;
        result.push_back(iter.toString());
    }
    return result;
}

// Extracts SPDX license ids from a SPDX license expression.
// For "(BSD-3-Clause AND BeerWare)" this function returns { "BSD-3-Clause", "BeerWare" }.
static QStringList extractLicenseIdsFromSPDXExpression(QString expression)
{
    const QStringList spdxOperators = {
        u"AND"_s,
        u"OR"_s,
        u"WITH"_s
    };

    // Replace parentheses with spaces. We're not interested in grouping.
    const QRegularExpression parensRegex(u"[()]"_s);
    expression.replace(parensRegex, u" "_s);

    // Split the string at space boundaries to extract tokens.
    QStringList result;
    for (const QString &token : expression.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
        if (spdxOperators.contains(token))
            continue;

        // Remove the unary + operator, if present.
        if (token.endsWith(QLatin1Char('+')))
            result.append(token.mid(0, token.length() - 1));
        else
            result.append(token);
    }
    return result;
}

// Starting at packageDir, look for a LICENSES subdirectory in the directory hierarchy upwards.
// Return a default-constructed QString if the directory was not found.
static QString locateLicensesDir(const QString &packageDir)
{
    static const QString licensesSubDir = u"LICENSES"_s;
    QDir dir(packageDir);
    while (true) {
        if (dir.cd(licensesSubDir))
            return dir.path();
        if (dir.isRoot())
            break;
        dir.cdUp();
    }
    return {};
}

// Locates the license files that belong to the licenses mentioned in LicenseId and stores them in
// the specified package object.
static bool autoDetectLicenseFiles(Package &p)
{
    const QString licensesDirPath = locateLicensesDir(p.path);
    const QStringList licenseIds = extractLicenseIdsFromSPDXExpression(p.licenseId);
    if (!licenseIds.isEmpty() && licensesDirPath.isEmpty()) {
        std::cerr << qPrintable(tr("LICENSES directory could not be located.")) << std::endl;
        return false;
    }

    bool success = true;
    QDir licensesDir(licensesDirPath);
    for (const QString &id : licenseIds) {
        QString fileName = id + u".txt";
        if (licensesDir.exists(fileName)) {
            p.licenseFiles.append(licensesDir.filePath(fileName));
        } else {
            std::cerr << qPrintable(tr("Expected license file not found: %1").arg(
                                        QDir::toNativeSeparators(licensesDir.filePath(fileName))))
                      << std::endl;
            success = false;
        }
    }

    return success;
}

// Transforms a JSON object into a Package object
static std::optional<Package> readPackage(const QJsonObject &object, const QString &filePath,
                                          LogLevel logLevel)
{
    Package p;
    bool validPackage = true;
    const QString directory = QFileInfo(filePath).absolutePath();
    p.path = directory;

    for (auto iter = object.constBegin(); iter != object.constEnd(); ++iter) {
        const QString key = iter.key();

        if (!iter.value().isString() && key != QLatin1String("QtParts")
            && key != QLatin1String("LicenseFiles")) {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("File %1: Expected JSON string as value of %2.").arg(
                                            QDir::toNativeSeparators(filePath), key)) << std::endl;
            validPackage = false;
            continue;
        }
        const QString value = iter.value().toString();
        if (key == QLatin1String("Name")) {
            p.name = value;
        } else if (key == QLatin1String("Path")) {
            p.path = QDir(directory).absoluteFilePath(value);
        } else if (key == QLatin1String("Files")) {
            p.files = value.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
        } else if (key == QLatin1String("Id")) {
            p.id = value;
        } else if (key == QLatin1String("Homepage")) {
            p.homepage = value;
        } else if (key == QLatin1String("Version")) {
            p.version = value;
        } else if (key == QLatin1String("DownloadLocation")) {
            p.downloadLocation = value;
        } else if (key == QLatin1String("License")) {
            p.license = value;
        } else if (key == QLatin1String("LicenseId")) {
            p.licenseId = value;
        } else if (key == QLatin1String("LicenseFile")) {
            p.licenseFiles = QStringList(QDir(directory).absoluteFilePath(value));
        } else if (key == QLatin1String("LicenseFiles")) {
            auto strings = toStringList(iter.value());
            if (!strings) {
                if (logLevel != SilentLog)
                    std::cerr << qPrintable(tr("File %1: Expected JSON array of strings in %2.")
                                                    .arg(QDir::toNativeSeparators(filePath), key))
                              << std::endl;
                validPackage = false;
                continue;
            }
            const QDir dir(directory);
            for (auto iter : strings.value())
                p.licenseFiles.push_back(dir.absoluteFilePath(iter));
        } else if (key == QLatin1String("Copyright")) {
            p.copyright = value;
        } else if (key == QLatin1String("CopyrightFile")) {
            p.copyrightFile = QDir(directory).absoluteFilePath(value);
        } else if (key == QLatin1String("PackageComment")) {
            p.packageComment = value;
        } else if (key == QLatin1String("QDocModule")) {
            p.qdocModule = value;
        } else if (key == QLatin1String("Description")) {
            p.description = value;
        } else if (key == QLatin1String("QtUsage")) {
            p.qtUsage = value;
        } else if (key == QLatin1String("QtParts")) {
            auto parts = toStringList(iter.value());
            if (!parts) {
                if (logLevel != SilentLog) {
                    std::cerr << qPrintable(tr("File %1: Expected JSON array of strings in %2.")
                                                    .arg(QDir::toNativeSeparators(filePath), key))
                              << std::endl;
                }
                validPackage = false;
                continue;
            }
            p.qtParts = parts.value();
        } else {
            if (logLevel != SilentLog) {
                std::cerr << qPrintable(tr("File %1: Unknown key %2.").arg(
                                            QDir::toNativeSeparators(filePath), key)) << std::endl;
            }
            validPackage = false;
        }
    }

    if (!p.copyrightFile.isEmpty()) {
        QFile file(p.copyrightFile);
        if (!file.open(QIODevice::ReadOnly)) {
            std::cerr << qPrintable(tr("File %1: Cannot open 'CopyrightFile' %2.\n")
                                            .arg(QDir::toNativeSeparators(filePath))
                                            .arg(QDir::toNativeSeparators(p.copyrightFile)));
            validPackage = false;
        }
        p.copyrightFileContents = QString::fromUtf8(file.readAll());
    }

    foreach (const QString &licenseFile, p.licenseFiles) {
        QFile file(licenseFile);
        if (!file.open(QIODevice::ReadOnly)) {
            if (logLevel != SilentLog) {
                std::cerr << qPrintable(tr("File %1: Cannot open 'LicenseFile' %2.\n")
                                                .arg(QDir::toNativeSeparators(filePath))
                                                .arg(QDir::toNativeSeparators(licenseFile)));
            }
            validPackage = false;
        }
        p.licenseFilesContents << QString::fromUtf8(file.readAll()).trimmed();
    }

    if (p.licenseFiles.isEmpty() && !autoDetectLicenseFiles(p))
        return std::nullopt;

    if (!validatePackage(p, filePath, logLevel) || !validPackage)
        return std::nullopt;

    return p;
}

// Parses a package's details from a README.chromium file
static Package parseChromiumFile(QFile &file, const QString &filePath, LogLevel logLevel)
{
    const QString directory = QFileInfo(filePath).absolutePath();

    // Parse the fields in the file
    QHash<QString, QString> fields;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList parts = line.split(QStringLiteral(":"));

        if (parts.size() < 2)
            continue;

        QString key = parts.at(0);
        parts.removeFirst();
        QString value = parts.join(QString()).trimmed();

        fields[key] = value;

        if (line == QLatin1String("Description:")) { // special field : should handle multi-lines values
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();

                if (line.startsWith(QLatin1String("Local Modifications:"))) // Don't include this part
                    break;

                fields[key] += line + QStringLiteral("\n");
            }

            break;
        }
    }

    // Construct the Package object
    Package p;

    QString shortName = fields.contains(QLatin1String("Short Name"))
            ? fields[QLatin1String("Short Name")]
            : fields[QLatin1String("Name")];
    QString version = fields[QStringLiteral("Version")];

    p.id =  QStringLiteral("chromium-") + shortName.toLower().replace(QChar::Space, QStringLiteral("-"));
    p.name = fields[QStringLiteral("Name")];
    if (version != QLatin1Char('0')) // "0" : not applicable
        p.version = version;
    p.license = fields[QStringLiteral("License")];
    p.homepage = fields[QStringLiteral("URL")];
    p.qdocModule = QStringLiteral("qtwebengine");
    p.qtUsage = QStringLiteral("Used in Qt WebEngine");
    p.description = fields[QStringLiteral("Description")].trimmed();
    p.path = directory;

    QString licenseFile = fields[QStringLiteral("License File")];
    if (licenseFile != QString() && licenseFile != QLatin1String("NOT_SHIPPED")) {
        p.licenseFiles = QStringList(QDir(directory).absoluteFilePath(licenseFile));
    } else {
        // Look for a LICENSE or COPYING file as a fallback
        QDir dir = directory;

        dir.setNameFilters({ QStringLiteral("LICENSE"), QStringLiteral("COPYING") });
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        const QFileInfoList entries = dir.entryInfoList();
        if (!entries.empty())
            p.licenseFiles = QStringList(entries.at(0).absoluteFilePath());
    }

    // let's ignore warnings regarding Chromium files for now
    Q_UNUSED(validatePackage(p, filePath, logLevel));

    return p;
}

std::optional<QList<Package>> readFile(const QString &filePath, LogLevel logLevel)
{
    QList<Package> packages;
    bool errorsFound = false;

    if (logLevel == VerboseLog) {
        std::cerr << qPrintable(tr("Reading file %1...").arg(
                                    QDir::toNativeSeparators(filePath))) << std::endl;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (logLevel != SilentLog)
            std::cerr << qPrintable(tr("Could not open file %1.").arg(
                                        QDir::toNativeSeparators(file.fileName()))) << std::endl;
        return std::nullopt;
    }

    if (filePath.endsWith(QLatin1String(".json"))) {
        QJsonParseError jsonParseError;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &jsonParseError);
        if (document.isNull()) {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("Could not parse file %1: %2").arg(
                                            QDir::toNativeSeparators(file.fileName()),
                                            jsonParseError.errorString()))
                          << std::endl;
            return std::nullopt;
        }

        if (document.isObject()) {
            std::optional<Package> p = readPackage(document.object(), file.fileName(), logLevel);
            if (p) {
                packages << *p;
            } else {
                errorsFound = true;
            }
        } else if (document.isArray()) {
            QJsonArray array = document.array();
            for (int i = 0, size = array.size(); i < size; ++i) {
                QJsonValue value = array.at(i);
                if (value.isObject()) {
                    std::optional<Package> p =
                            readPackage(value.toObject(), file.fileName(), logLevel);
                    if (p) {
                        packages << *p;
                    } else {
                        errorsFound = true;
                    }
                } else {
                    if (logLevel != SilentLog) {
                        std::cerr << qPrintable(tr("File %1: Expecting JSON object in array.")
                                        .arg(QDir::toNativeSeparators(file.fileName())))
                                  << std::endl;
                    }
                    errorsFound = true;
                }
            }
        } else {
            if (logLevel != SilentLog) {
                std::cerr << qPrintable(tr("File %1: Expecting JSON object in array.").arg(
                                            QDir::toNativeSeparators(file.fileName()))) << std::endl;
            }
            errorsFound = true;
        }
    } else if (filePath.endsWith(QLatin1String(".chromium"))) {
        Package chromiumPackage = parseChromiumFile(file, filePath, logLevel);
        if (!chromiumPackage.name.isEmpty()) // Skip invalid README.chromium files
            packages << chromiumPackage;
    } else {
        if (logLevel != SilentLog) {
            std::cerr << qPrintable(tr("File %1: Unsupported file type.")
                            .arg(QDir::toNativeSeparators(file.fileName())))
                      << std::endl;
        }
        errorsFound = true;
    }

    if (errorsFound)
        return std::nullopt;
    return packages;
}

std::optional<QList<Package>> scanDirectory(const QString &directory, InputFormats inputFormats,
                                            LogLevel logLevel)
{
    QDir dir(directory);
    QList<Package> packages;
    bool errorsFound = false;

    QStringList nameFilters = QStringList();
    if (inputFormats & InputFormat::QtAttributions)
        nameFilters << QStringLiteral("qt_attribution.json");
    if (inputFormats & InputFormat::ChromiumAttributions)
        nameFilters << QStringLiteral("README.chromium");
    if (qEnvironmentVariableIsSet("QT_ATTRIBUTIONSSCANNER_TEST")) {
        nameFilters
                << QStringLiteral("qt_attribution_test.json")
                << QStringLiteral("README_test.chromium");
    }

    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    const QFileInfoList entries = dir.entryInfoList();
    for (const QFileInfo &info : entries) {
        if (info.isDir()) {
            std::optional<QList<Package>> ps =
                    scanDirectory(info.filePath(), inputFormats, logLevel);
            if (!ps)
                errorsFound = true;
            else
                packages += *ps;
        } else {
            std::optional p = readFile(info.filePath(), logLevel);
            if (!p)
                errorsFound = true;
            else
                packages += *p;
        }
    }

    if (errorsFound)
        return std::nullopt;
    return packages;
}

} // namespace Scanner
