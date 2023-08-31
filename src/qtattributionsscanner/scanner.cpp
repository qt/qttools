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

static bool validatePackage(Package &p, const QString &filePath, Checks checks, LogLevel logLevel)
{
    bool validPackage = true;

    if (p.qtParts.isEmpty())
        p.qtParts << u"libs"_s;

    if (p.name.isEmpty()) {
        if (p.id.startsWith("chromium-"_L1)) // Ignore invalid README.chromium files
            return false;

        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, u"Name"_s);
        validPackage = false;
    }

    if (p.id.isEmpty()) {
        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, u"Id"_s);
        validPackage = false;
    }
    if (p.license.isEmpty()) {
        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, u"License"_s);
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

    if (p.securityCritical && p.downloadLocation.isEmpty()) {
        if (logLevel != SilentLog)
            missingPropertyWarning(filePath, u"DownloadLocation"_s);
        validPackage = false;
    }

    for (const QString &part : std::as_const(p.qtParts)) {
        if (part != "examples"_L1 && part != "tests"_L1
            && part != "tools"_L1 && part != "libs"_L1) {

            if (logLevel != SilentLog) {
                std::cerr << qPrintable(tr("File %1: Property 'QtPart' contains unknown element "
                                           "'%2'. Valid entries are 'examples', 'tests', 'tools' "
                                           "and 'libs'.").arg(
                                            QDir::toNativeSeparators(filePath), part))
                          << std::endl;
            }
            validPackage = false;
        }
    }

    if (!(checks & Check::Paths))
        return validPackage;

    const QDir dir = p.path;
    if (!dir.exists()) {
        std::cerr << qPrintable(
                tr("File %1: Directory '%2' does not exist.")
                        .arg(QDir::toNativeSeparators(filePath), QDir::toNativeSeparators(p.path)))
                  << std::endl;
        validPackage = false;
    } else {
        for (const QString &file : std::as_const(p.files)) {
            if (!dir.exists(file)) {
                if (logLevel != SilentLog) {
                    std::cerr << qPrintable(
                            tr("File %1: Path '%2' does not exist in directory '%3'.")
                                    .arg(QDir::toNativeSeparators(filePath),
                                         QDir::toNativeSeparators(file),
                                         QDir::toNativeSeparators(p.path)))
                              << std::endl;
                }
                validPackage = false;
            }
        }
    }

    return validPackage;
}

static std::optional<QStringList> toStringList(const QJsonValue &value)
{
    if (!value.isArray())
        return std::nullopt;
    QStringList result;
    for (const auto &iter : value.toArray()) {
        if (iter.type() != QJsonValue::String)
            return std::nullopt;
        result.push_back(iter.toString());
    }
    return result;
}

static std::optional<QString> arrayToMultiLineString(const QJsonValue &value)
{
    if (!value.isArray())
        return std::nullopt;
    QString result;
    for (const auto &iter : value.toArray()) {
        if (iter.type() != QJsonValue::String)
            return std::nullopt;
        result.append(iter.toString());
        result.append(QLatin1StringView("\n"));
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
            result.append(token.mid(0, token.size() - 1));
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
        if (!dir.exists())
            break;
        if (dir.cd(licensesSubDir))
            return dir.path();
        if (dir.isRoot() || !dir.cdUp())
            break;
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
                                          Checks checks, LogLevel logLevel)
{
    Package p;
    bool validPackage = true;
    const QString directory = QFileInfo(filePath).absolutePath();
    p.path = directory;

    for (auto iter = object.constBegin(); iter != object.constEnd(); ++iter) {
        const QString key = iter.key();

        if (!iter.value().isString() && key != "QtParts"_L1 && key != "SecurityCritical"_L1
            && key != "Files"_L1 && key != "LicenseFiles"_L1 && key != "Comment"_L1
            && key != "Copyright"_L1) {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("File %1: Expected JSON string as value of %2.").arg(
                                            QDir::toNativeSeparators(filePath), key)) << std::endl;
            validPackage = false;
            continue;
        }
        const QString value = iter.value().toString();
        if (key == "Name"_L1) {
            p.name = value;
        } else if (key == "Path"_L1) {
            p.path = QDir(directory).absoluteFilePath(value);
        } else if (key == "Files"_L1) {
            QJsonValueConstRef jsonValue = iter.value();
            if (jsonValue.isArray()) {
                auto maybeStringList = toStringList(jsonValue);
                if (maybeStringList)
                    p.files = maybeStringList.value();
            } else if (jsonValue.isString()) {
                // Legacy format: multiple values separated by space in one string.
                p.files = value.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
            } else {
                if (logLevel != SilentLog) {
                    std::cerr << qPrintable(tr("File %1: Expected JSON array of strings as value "
                                               "of Files."));
                    validPackage = false;
                    continue;
                }
            }
        } else if (key == "Comment"_L1) {
            // Accepted purely to record details of potential interest doing
            // updates in future. Value is an arbitrary object. Any number of
            // Comment entries may be present: JSON doesn't require names to be
            // unique, albeit some linters may kvetch.
        } else if (key == "Id"_L1) {
            p.id = value;
        } else if (key == "Homepage"_L1) {
            p.homepage = value;
        } else if (key == "Version"_L1) {
            p.version = value;
        } else if (key == "DownloadLocation"_L1) {
            p.downloadLocation = value;
        } else if (key == "License"_L1) {
            p.license = value;
        } else if (key == "LicenseId"_L1) {
            p.licenseId = value;
        } else if (key == "LicenseFile"_L1) {
            p.licenseFiles = QStringList(QDir(directory).absoluteFilePath(value));
        } else if (key == "LicenseFiles"_L1) {
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
            for (const auto &iter : std::as_const(strings.value()))
                p.licenseFiles.push_back(dir.absoluteFilePath(iter));
        } else if (key == "Copyright"_L1) {
            QJsonValueConstRef jsonValue = iter.value();
            if (jsonValue.isArray()) {
                // Array joined with new lines
                auto maybeString = arrayToMultiLineString(jsonValue);
                if (maybeString)
                    p.copyright = maybeString.value();
            } else if (jsonValue.isString()) {
                // Legacy format: multiple values separated by space in one string.
                p.copyright = value;
            } else {
                if (logLevel != SilentLog) {
                    std::cerr << qPrintable(tr("File %1: Expected JSON array of string or"
                                               "string as value of %2.").arg(
                                                QDir::toNativeSeparators(filePath), key)) << std::endl;
                    validPackage = false;
                    continue;
                }
            }
        } else if (key == "CopyrightFile"_L1) {
            p.copyrightFile = QDir(directory).absoluteFilePath(value);
        } else if (key == "PackageComment"_L1) {
            p.packageComment = value;
        } else if (key == "QDocModule"_L1) {
            p.qdocModule = value;
        } else if (key == "Description"_L1) {
            p.description = value;
        } else if (key == "QtUsage"_L1) {
            p.qtUsage = value;
        } else if (key == "SecurityCritical"_L1) {
            if (!iter.value().isBool()) {
                std::cerr << qPrintable(tr("File %1: Expected JSON boolean in %2.")
                                                .arg(QDir::toNativeSeparators(filePath), key))
                          << std::endl;
                validPackage = false;
                continue;
            }
            p.securityCritical = iter.value().toBool();
        } else if (key == "QtParts"_L1) {
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
                                            .arg(QDir::toNativeSeparators(filePath),
                                                 QDir::toNativeSeparators(p.copyrightFile)));
            validPackage = false;
        }
        p.copyrightFileContents = QString::fromUtf8(file.readAll());
    }

    foreach (const QString &licenseFile, p.licenseFiles) {
        QFile file(licenseFile);
        if (!file.open(QIODevice::ReadOnly)) {
            if (logLevel != SilentLog) {
                std::cerr << qPrintable(tr("File %1: Cannot open 'LicenseFile' %2.\n")
                                                .arg(QDir::toNativeSeparators(filePath),
                                                     QDir::toNativeSeparators(licenseFile)));
            }
            validPackage = false;
        }
        p.licenseFilesContents << QString::fromUtf8(file.readAll()).trimmed();
    }

    if (p.licenseFiles.isEmpty() && !autoDetectLicenseFiles(p))
        return std::nullopt;

    if (!validatePackage(p, filePath, checks, logLevel) || !validPackage)
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
        QStringList parts = line.split(u":"_s);

        if (parts.size() < 2)
            continue;

        QString key = parts.at(0);
        parts.removeFirst();
        QString value = parts.join(QString()).trimmed();

        fields[key] = value;

        if (line == "Description:"_L1) { // special field : should handle multi-lines values
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();

                if (line.startsWith("Local Modifications:"_L1)) // Don't include this part
                    break;

                fields[key] += line + u"\n"_s;
            }

            break;
        }
    }

    // Construct the Package object
    Package p;

    QString shortName = fields.contains("Short Name"_L1)
            ? fields["Short Name"_L1]
            : fields["Name"_L1];
    QString version = fields[u"Version"_s];

    p.id =  u"chromium-"_s + shortName.toLower().replace(QChar::Space, u"-"_s);
    p.name = fields[u"Name"_s];
    if (version != QLatin1Char('0')) // "0" : not applicable
        p.version = version;
    p.license = fields[u"License"_s];
    p.homepage = fields[u"URL"_s];
    p.qdocModule = u"qtwebengine"_s;
    p.qtUsage = u"Used in Qt WebEngine"_s;
    p.description = fields[u"Description"_s].trimmed();
    p.path = directory;

    QString licenseFile = fields[u"License File"_s];
    if (licenseFile != QString() && licenseFile != "NOT_SHIPPED"_L1) {
        p.licenseFiles = QStringList(QDir(directory).absoluteFilePath(licenseFile));
    } else {
        // Look for a LICENSE or COPYING file as a fallback
        QDir dir = directory;

        dir.setNameFilters({ u"LICENSE"_s, u"COPYING"_s });
        dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        const QFileInfoList entries = dir.entryInfoList();
        if (!entries.empty())
            p.licenseFiles = QStringList(entries.at(0).absoluteFilePath());
    }

    // let's ignore warnings regarding Chromium files for now
    Q_UNUSED(validatePackage(p, filePath, {}, logLevel));

    return p;
}

std::optional<QList<Package>> readFile(const QString &filePath, Checks checks, LogLevel logLevel)
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

    if (filePath.endsWith(".json"_L1)) {
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
            std::optional<Package> p =
                    readPackage(document.object(), file.fileName(), checks, logLevel);
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
                            readPackage(value.toObject(), file.fileName(), checks, logLevel);
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
    } else if (filePath.endsWith(".chromium"_L1)) {
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
                                            Checks checks, LogLevel logLevel)
{
    QDir dir(directory);
    QList<Package> packages;
    bool errorsFound = false;

    QStringList nameFilters = QStringList();
    if (inputFormats & InputFormat::QtAttributions)
        nameFilters << u"qt_attribution.json"_s;
    if (inputFormats & InputFormat::ChromiumAttributions)
        nameFilters << u"README.chromium"_s;
    if (qEnvironmentVariableIsSet("QT_ATTRIBUTIONSSCANNER_TEST"))
        nameFilters << u"qt_attribution_test.json"_s << u"README_test.chromium"_s;

    dir.setNameFilters(nameFilters);
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

    const QFileInfoList entries = dir.entryInfoList();
    for (const QFileInfo &info : entries) {
        if (info.isDir()) {
            std::optional<QList<Package>> ps =
                    scanDirectory(info.filePath(), inputFormats, checks, logLevel);
            if (!ps)
                errorsFound = true;
            else
                packages += *ps;
        } else {
            std::optional p = readFile(info.filePath(), checks, logLevel);
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
