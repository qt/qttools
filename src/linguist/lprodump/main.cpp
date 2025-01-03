// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <profileevaluator.h>
#include <profileutils.h>
#include <qmakeparser.h>
#include <qmakevfs.h>
#include <qrcreader.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <iostream>

using namespace Qt::StringLiterals;

static void printOut(const QString &out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString &out)
{
    std::cerr << qPrintable(out);
}

static QJsonValue toJsonValue(const QJsonValue &v)
{
    return v;
}

static QJsonValue toJsonValue(const QString &s)
{
    return QJsonValue(s);
}

static QJsonValue toJsonValue(const QStringList &lst)
{
    return QJsonArray::fromStringList(lst);
}

template <class T>
void setValue(QJsonObject &obj, const char *key, T value)
{
    obj[QLatin1String(key)] = toJsonValue(value);
}

static void printUsage()
{
    printOut(uR"(Usage:
    lprodump [options] project-file...
lprodump is part of Qt's Linguist tool chain. It extracts information
from qmake projects to a .json file. This file can be passed to
lupdate/lrelease using the -project option.

Options:
    -help  Display this information and exit.
    -silent
           Do not explain what is being done.
    -pro <filename>
           Name of a .pro file. Useful for files with .pro file syntax but
           different file suffix. Projects are recursed into and merged.
    -pro-out <directory>
           Virtual output directory for processing subsequent .pro files.
    -pro-debug
           Trace processing .pro files. Specify twice for more verbosity.
    -out <filename>
           Name of the output file.
    -translations-variables <variable_1>[,<variable_2>,...]
           Comma-separated list of QMake variables containing .ts files.
    -version
           Display the version of lprodump and exit.
)"_s);
}

static void print(const QString &fileName, int lineNo, const QString &msg)
{
    if (lineNo > 0)
        printErr(QString::fromLatin1("WARNING: %1:%2: %3\n").arg(fileName, QString::number(lineNo), msg));
    else if (lineNo)
        printErr(QString::fromLatin1("WARNING: %1: %2\n").arg(fileName, msg));
    else
        printErr(QString::fromLatin1("WARNING: %1\n").arg(msg));
}

class EvalHandler : public QMakeHandler {
public:
    void message(int type, const QString &msg, const QString &fileName, int lineNo) override
    {
        if (verbose && !(type & CumulativeEvalMessage) && (type & CategoryMask) == ErrorMessage)
            print(fileName, lineNo, msg);
    }

    void fileMessage(int type, const QString &msg) override
    {
        if (verbose && !(type & CumulativeEvalMessage) && (type & CategoryMask) == ErrorMessage) {
            // "Downgrade" errors, as we don't really care for them
            printErr(QLatin1String("WARNING: ") + msg + QLatin1Char('\n'));
        }
    }

    void aboutToEval(ProFile *, ProFile *, EvalFileType) override {}
    void doneWithEval(ProFile *) override {}

    bool verbose = true;
};

static EvalHandler evalHandler;

static QStringList getResources(const QString &resourceFile, QMakeVfs *vfs)
{
    Q_ASSERT(vfs);
    if (!vfs->exists(resourceFile, QMakeVfs::VfsCumulative))
        return QStringList();
    QString content;
    QString errStr;
    if (vfs->readFile(vfs->idForFileName(resourceFile, QMakeVfs::VfsCumulative),
                      &content, &errStr) != QMakeVfs::ReadOk) {
        printErr(QStringLiteral("lprodump error: Cannot read %1: %2\n").arg(resourceFile, errStr));
        return QStringList();
    }
    const ReadQrcResult rqr = readQrcFile(resourceFile, content);
    if (rqr.hasError()) {
        printErr(QStringLiteral("lprodump error: %1:%2: %3\n")
                 .arg(resourceFile, QString::number(rqr.line), rqr.errorString));
    }
    return rqr.files;
}

static QStringList getSources(const char *var, const char *vvar, const QStringList &baseVPaths,
                              const QString &projectDir, const ProFileEvaluator &visitor)
{
    QStringList vPaths = visitor.absolutePathValues(QLatin1String(vvar), projectDir);
    vPaths += baseVPaths;
    vPaths.removeDuplicates();
    return visitor.absoluteFileValues(QLatin1String(var), projectDir, vPaths, 0);
}

static QStringList getSources(const ProFileEvaluator &visitor, const QString &projectDir,
                              const QStringList &excludes, QMakeVfs *vfs)
{
    QStringList baseVPaths;
    baseVPaths += visitor.absolutePathValues(QLatin1String("VPATH"), projectDir);
    baseVPaths << projectDir; // QMAKE_ABSOLUTE_SOURCE_PATH
    baseVPaths.removeDuplicates();

    QStringList sourceFiles;

    // app/lib template
    sourceFiles += getSources("SOURCES", "VPATH_SOURCES", baseVPaths, projectDir, visitor);
    sourceFiles += getSources("HEADERS", "VPATH_HEADERS", baseVPaths, projectDir, visitor);

    sourceFiles += getSources("FORMS", "VPATH_FORMS", baseVPaths, projectDir, visitor);

    const QStringList resourceFiles = getSources("RESOURCES", "VPATH_RESOURCES", baseVPaths, projectDir, visitor);
    for (const QString &resource : resourceFiles)
        sourceFiles += getResources(resource, vfs);

    QStringList installs = visitor.values(QLatin1String("INSTALLS"))
                         + visitor.values(QLatin1String("DEPLOYMENT"));
    installs.removeDuplicates();
    QDir baseDir(projectDir);
    for (const QString &inst : std::as_const(installs)) {
        for (const QString &file : visitor.values(inst + QLatin1String(".files"))) {
            QFileInfo info(file);
            if (!info.isAbsolute())
                info.setFile(baseDir.absoluteFilePath(file));
            QStringList nameFilter;
            QString searchPath;
            if (info.isDir()) {
                nameFilter << QLatin1String("*");
                searchPath = info.filePath();
            } else {
                nameFilter << info.fileName();
                searchPath = info.path();
            }

            QDirIterator iterator(searchPath, nameFilter,
                                  QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                                  QDirIterator::Subdirectories);
            while (iterator.hasNext()) {
                iterator.next();
                QFileInfo cfi = iterator.fileInfo();
                if (isSupportedExtension(cfi.suffix()))
                    sourceFiles << cfi.filePath();
            }
        }
    }

    sourceFiles.removeDuplicates();
    sourceFiles.sort();

    for (const QString &ex : excludes) {
        // TODO: take advantage of the file list being sorted
        QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(ex));
        for (auto it = sourceFiles.begin(); it != sourceFiles.end(); ) {
            if (rx.match(*it).hasMatch())
                it = sourceFiles.erase(it);
            else
                ++it;
        }
    }

    return sourceFiles;
}

QStringList getExcludes(const ProFileEvaluator &visitor, const QString &projectDirPath)
{
    const QStringList trExcludes = visitor.values(QLatin1String("TR_EXCLUDE"));
    QStringList excludes;
    excludes.reserve(trExcludes.size());
    const QDir projectDir(projectDirPath);
    for (const QString &ex : trExcludes)
        excludes << QDir::cleanPath(projectDir.absoluteFilePath(ex));
    return excludes;
}

static void excludeProjects(const ProFileEvaluator &visitor, QStringList *subProjects)
{
    for (const QString &ex : visitor.values(QLatin1String("TR_EXCLUDE"))) {
        QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(ex));
        for (auto it = subProjects->begin(); it != subProjects->end(); ) {
            if (rx.match(*it).hasMatch())
                it = subProjects->erase(it);
            else
                ++it;
        }
    }
}

static QJsonArray processProjects(bool topLevel, const QStringList &proFiles,
        const QStringList &translationsVariables,
        const QHash<QString, QString> &outDirMap,
        ProFileGlobals *option, QMakeVfs *vfs, QMakeParser *parser,
        bool *fail);

static QJsonObject processProject(const QString &proFile, const QStringList &translationsVariables,
                                  ProFileGlobals *option, QMakeVfs *vfs,
                                  QMakeParser *parser, ProFileEvaluator &visitor)
{
    QJsonObject result;
    QStringList tmp = visitor.values(QLatin1String("CODECFORSRC"));
    if (!tmp.isEmpty())
        result[QStringLiteral("codec")] = tmp.last();
    QString proPath = QFileInfo(proFile).path();
    if (visitor.templateType() == ProFileEvaluator::TT_Subdirs) {
        QStringList subProjects = visitor.values(QLatin1String("SUBDIRS"));
        excludeProjects(visitor, &subProjects);
        QStringList subProFiles;
        QDir proDir(proPath);
        for (const QString &subdir : std::as_const(subProjects)) {
            QString realdir = visitor.value(subdir + QLatin1String(".subdir"));
            if (realdir.isEmpty())
                realdir = visitor.value(subdir + QLatin1String(".file"));
            if (realdir.isEmpty())
                realdir = subdir;
            QString subPro = QDir::cleanPath(proDir.absoluteFilePath(realdir));
            QFileInfo subInfo(subPro);
            if (subInfo.isDir()) {
                subProFiles << (subPro + QLatin1Char('/')
                                + subInfo.fileName() + QLatin1String(".pro"));
            } else {
                subProFiles << subPro;
            }
        }
        QJsonArray subResults = processProjects(false, subProFiles, translationsVariables,
                                                QHash<QString, QString>(), option, vfs, parser,
                                                nullptr);
        if (!subResults.isEmpty())
            setValue(result, "subProjects", subResults);
    } else {
        const QStringList excludes = getExcludes(visitor, proPath);
        const QStringList sourceFiles = getSources(visitor, proPath, excludes, vfs);
        setValue(result, "includePaths",
                 visitor.absolutePathValues(QLatin1String("INCLUDEPATH"), proPath));
        setValue(result, "excluded", excludes);
        setValue(result, "sources", sourceFiles);
    }
    return result;
}

static QJsonArray processProjects(bool topLevel, const QStringList &proFiles,
        const QStringList &translationsVariables,
        const QHash<QString, QString> &outDirMap,
        ProFileGlobals *option, QMakeVfs *vfs, QMakeParser *parser, bool *fail)
{
    QJsonArray result;
    for (const QString &proFile : proFiles) {
        if (!outDirMap.isEmpty())
            option->setDirectories(QFileInfo(proFile).path(), outDirMap[proFile]);

        ProFile *pro;
        if (!(pro = parser->parsedProFile(proFile, topLevel ? QMakeParser::ParseReportMissing
                                                            : QMakeParser::ParseDefault))) {
            if (topLevel)
                *fail = true;
            continue;
        }
        ProFileEvaluator visitor(option, parser, vfs, &evalHandler);
        visitor.setCumulative(true);
        visitor.setOutputDir(option->shadowedPath(pro->directoryName()));
        if (!visitor.accept(pro)) {
            if (topLevel)
                *fail = true;
            pro->deref();
            continue;
        }

        QJsonObject prj = processProject(proFile, translationsVariables, option, vfs, parser,
                                         visitor);
        setValue(prj, "projectFile", proFile);
        QStringList tsFiles;
        for (const QString &varName : translationsVariables) {
            if (!visitor.contains(varName))
                continue;
            QDir proDir(QFileInfo(proFile).path());
            const QStringList translations = visitor.values(varName);
            for (const QString &tsFile : translations)
                tsFiles << proDir.filePath(tsFile);
        }
        if (!tsFiles.isEmpty())
            setValue(prj, "translations", tsFiles);
        if (visitor.contains(QLatin1String("LUPDATE_COMPILE_COMMANDS_PATH"))) {
            const QStringList thepathjson = visitor.values(
                QLatin1String("LUPDATE_COMPILE_COMMANDS_PATH"));
            setValue(prj, "compileCommands", thepathjson.value(0));
        }
        result.append(prj);
        pro->deref();
    }
    return result;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    QStringList proFiles;
    QStringList translationsVariables = { u"TRANSLATIONS"_s };
    QString outDir = QDir::currentPath();
    QHash<QString, QString> outDirMap;
    QString outputFilePath;
    int proDebug = 0;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == QLatin1String("-help")
                || arg == QLatin1String("--help")
                || arg == QLatin1String("-h")) {
            printUsage();
            return 0;
        } else if (arg == QLatin1String("-out")) {
            ++i;
            if (i == argc) {
                printErr(u"The option -out requires a parameter.\n"_s);
                return 1;
            }
            outputFilePath = args[i];
        } else if (arg == QLatin1String("-silent")) {
            evalHandler.verbose = false;
        } else if (arg == QLatin1String("-pro-debug")) {
            proDebug++;
        } else if (arg == QLatin1String("-version")) {
            printOut(QStringLiteral("lprodump version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == QLatin1String("-pro")) {
            ++i;
            if (i == argc) {
                printErr(QStringLiteral("The -pro option should be followed by a filename of .pro file.\n"));
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            outDirMap[file] = outDir;
        } else if (arg == QLatin1String("-pro-out")) {
            ++i;
            if (i == argc) {
                printErr(QStringLiteral("The -pro-out option should be followed by a directory name.\n"));
                return 1;
            }
            outDir = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
        } else if (arg == u"-translations-variables"_s) {
            ++i;
            if (i == argc) {
                printErr(u"The -translations-variables option must be followed by a "_s
                         u"comma-separated list of variable names.\n"_s);
                return 1;
            }
            translationsVariables = args.at(i).split(QLatin1Char(','));
        } else if (arg.startsWith(QLatin1String("-")) && arg != QLatin1String("-")) {
            printErr(QStringLiteral("Unrecognized option '%1'.\n").arg(arg));
            return 1;
        } else {
            QFileInfo fi(arg);
            if (!fi.exists()) {
                printErr(QStringLiteral("lprodump error: File '%1' does not exist.\n").arg(arg));
                return 1;
            }
            if (!isProOrPriFile(arg)) {
                printErr(QStringLiteral("lprodump error: '%1' is neither a .pro nor a .pri file.\n")
                         .arg(arg));
                return 1;
            }
            QString cleanFile = QDir::cleanPath(fi.absoluteFilePath());
            proFiles << cleanFile;
            outDirMap[cleanFile] = outDir;
        }
    } // for args

    if (proFiles.isEmpty()) {
        printUsage();
        return 1;
    }

    bool fail = false;
    ProFileGlobals option;
    option.qmake_abslocation = QString::fromLocal8Bit(qgetenv("QMAKE"));
    if (option.qmake_abslocation.isEmpty()) {
        option.qmake_abslocation = QLibraryInfo::path(QLibraryInfo::BinariesPath)
            + QLatin1String("/qmake");
    }
    option.debugLevel = proDebug;
    option.initProperties();
    option.setCommandLineArguments(QDir::currentPath(),
                                   QStringList() << QLatin1String("CONFIG+=lupdate_run"));
    QMakeVfs vfs;
    QMakeParser parser(0, &vfs, &evalHandler);

    QJsonArray results = processProjects(true, proFiles, translationsVariables, outDirMap, &option,
                                         &vfs, &parser, &fail);
    if (fail)
        return 1;

    const QByteArray output = QJsonDocument(results).toJson(QJsonDocument::Compact);
    if (outputFilePath.isEmpty()) {
        puts(output.constData());
    } else {
        QFile f(outputFilePath);
        if (!f.open(QIODevice::WriteOnly)) {
            printErr(QStringLiteral("lprodump error: Cannot open %1 for writing.\n").arg(outputFilePath));
            return 1;
        }
        f.write(output);
        f.write("\n");
    }
    return 0;
}
