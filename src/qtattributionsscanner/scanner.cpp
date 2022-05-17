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
#include <optional>

namespace Scanner {

static void missingPropertyWarning(const QString &filePath, const QString &property)
{
    std::cerr << qPrintable(tr("File %1: Missing mandatory property '%2'.").arg(
                                QDir::toNativeSeparators(filePath), property)) << std::endl;
}

static void validatePackage(Package &p, const QString &filePath, LogLevel logLevel)
{
    if (p.qtParts.isEmpty())
        p.qtParts << QStringLiteral("libs");

    if (logLevel != SilentLog) {
        if (p.name.isEmpty()) {
            if (p.id.startsWith(QLatin1String("chromium-"))) // Ignore invalid README.chromium files
                return;

            missingPropertyWarning(filePath, QStringLiteral("Name"));
        }

        if (p.id.isEmpty())
            missingPropertyWarning(filePath, QStringLiteral("Id"));
        if (p.license.isEmpty())
            missingPropertyWarning(filePath, QStringLiteral("License"));

        if (!p.copyright.isEmpty() && !p.copyrightFile.isEmpty()) {
            std::cerr << qPrintable(tr("File %1: Properties 'Copyright' and 'CopyrightFile' are "
                                       "mutually exclusive.")
                                            .arg(QDir::toNativeSeparators(filePath)))
                      << std::endl;
        }

        for (const QString &part : qAsConst(p.qtParts)) {
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
            }
        }
    }
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

// Transforms a JSON object into a Package object
static Package readPackage(const QJsonObject &object, const QString &filePath, LogLevel logLevel)
{
    Package p;
    const QString directory = QFileInfo(filePath).absolutePath();
    p.path = directory;

    for (auto iter = object.constBegin(); iter != object.constEnd(); ++iter) {
        const QString key = iter.key();

        if (!iter.value().isString() && key != QLatin1String("QtParts")
            && key != QLatin1String("LicenseFiles")) {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("File %1: Expected JSON string as value of %2.").arg(
                                            QDir::toNativeSeparators(filePath), key)) << std::endl;
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
            if (!strings && (logLevel != SilentLog))
                std::cerr << qPrintable(tr("File %1: Expected JSON array of strings in %2.")
                                                .arg(QDir::toNativeSeparators(filePath), key))
                          << std::endl;
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
            if (!parts && (logLevel != SilentLog))
                std::cerr << qPrintable(tr("File %1: Expected JSON array of strings in %2.")
                                                .arg(QDir::toNativeSeparators(filePath), key))
                          << std::endl;
            p.qtParts = parts.value();
        } else {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("File %1: Unknown key %2.").arg(
                                            QDir::toNativeSeparators(filePath), key)) << std::endl;
        }
    }

    validatePackage(p, filePath, logLevel);

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

        if (parts.count() < 2)
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

    validatePackage(p, filePath, logLevel);

    return p;
}

QList<Package> readFile(const QString &filePath, LogLevel logLevel)
{
    QList<Package> packages;

    if (logLevel == VerboseLog) {
        std::cerr << qPrintable(tr("Reading file %1...").arg(
                                    QDir::toNativeSeparators(filePath))) << std::endl;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (logLevel != SilentLog)
            std::cerr << qPrintable(tr("Could not open file %1.").arg(
                                        QDir::toNativeSeparators(file.fileName()))) << std::endl;
        return QList<Package>();
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
            return QList<Package>();
        }

        if (document.isObject()) {
            packages << readPackage(document.object(), file.fileName(), logLevel);
        } else if (document.isArray()) {
            QJsonArray array = document.array();
            for (int i = 0, size = array.size(); i < size; ++i) {
                QJsonValue value = array.at(i);
                if (value.isObject()) {
                    packages << readPackage(value.toObject(), file.fileName(), logLevel);
                } else {
                    if (logLevel != SilentLog)
                        std::cerr << qPrintable(tr("File %1: Expecting JSON object in array.")
                                        .arg(QDir::toNativeSeparators(file.fileName())))
                                  << std::endl;
                }
            }
        } else {
            if (logLevel != SilentLog)
                std::cerr << qPrintable(tr("File %1: Expecting JSON object in array.").arg(
                                            QDir::toNativeSeparators(file.fileName()))) << std::endl;
        }
    } else if (filePath.endsWith(QLatin1String(".chromium"))) {
        Package chromiumPackage = parseChromiumFile(file, filePath, logLevel);
        if (!chromiumPackage.name.isEmpty()) // Skip invalid README.chromium files
            packages << chromiumPackage;
    } else {
        if (logLevel != SilentLog)
            std::cerr << qPrintable(tr("File %1: Unsupported file type.")
                            .arg(QDir::toNativeSeparators(file.fileName())))
                      << std::endl;
    }

    return packages;
}

QList<Package> scanDirectory(const QString &directory, InputFormats inputFormats, LogLevel logLevel)
{
    QDir dir(directory);
    QList<Package> packages;

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
            packages += scanDirectory(info.filePath(), inputFormats, logLevel);
        } else {
            packages += readFile(info.filePath(), logLevel);
        }
    }

    return packages;
}

} // namespace Scanner
