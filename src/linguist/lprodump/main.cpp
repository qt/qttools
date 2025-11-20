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
            printErr("WARNING: "_L1 + msg + u'\n');
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
                              QMakeVfs *vfs)
{
    QStringList baseVPaths;
    baseVPaths += visitor.absolutePathValues("VPATH"_L1, projectDir);
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

    QStringList installs = visitor.values("INSTALLS"_L1) + visitor.values("DEPLOYMENT"_L1);
    installs.removeDuplicates();
    QDir baseDir(projectDir);
    for (const QString &inst : std::as_const(installs)) {
        for (const QString &file : visitor.values(inst + ".files"_L1)) {
            QFileInfo info(file);
            if (!info.isAbsolute())
                info.setFile(baseDir.absoluteFilePath(file));
            QStringList nameFilter;
            QString searchPath;
            if (info.isDir()) {
                nameFilter << "*"_L1;
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
    return sourceFiles;
}

QStringList getExcludes(const ProFileEvaluator &visitor, const QString &projectDirPath)
{
    const QStringList trExcludes = visitor.values("TR_EXCLUDE"_L1);
    QStringList excludes;
    excludes.reserve(trExcludes.size());
    const QDir projectDir(projectDirPath);
    for (const QString &ex : trExcludes)
        excludes << QDir::cleanPath(projectDir.absoluteFilePath(ex));
    return excludes;
}

static void excludeProjects(const ProFileEvaluator &visitor, QStringList *subProjects)
{
    for (const QString &ex : visitor.values("TR_EXCLUDE"_L1)) {
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
    QStringList tmp = visitor.values("CODECFORSRC"_L1);
    if (!tmp.isEmpty())
        result[QStringLiteral("codec")] = tmp.last();
    QString proPath = QFileInfo(proFile).path();
    if (visitor.templateType() == ProFileEvaluator::TT_Subdirs) {
        QStringList subProjects = visitor.values("SUBDIRS"_L1);
        excludeProjects(visitor, &subProjects);
        QStringList subProFiles;
        QDir proDir(proPath);
        for (const QString &subdir : std::as_const(subProjects)) {
            QString realdir = visitor.value(subdir + ".subdir"_L1);
            if (realdir.isEmpty())
                realdir = visitor.value(subdir + ".file"_L1);
            if (realdir.isEmpty())
                realdir = subdir;
            QString subPro = QDir::cleanPath(proDir.absoluteFilePath(realdir));
            QFileInfo subInfo(subPro);
            if (subInfo.isDir()) {
                subProFiles << (subPro + u'/' + subInfo.fileName() + ".pro"_L1);
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
        const QStringList sourceFiles = getSources(visitor, proPath, vfs);
        setValue(result, "includePaths", visitor.absolutePathValues("INCLUDEPATH"_L1, proPath));
        setValue(result, "excluded", getExcludes(visitor, proPath));
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
        if (visitor.contains("LUPDATE_COMPILE_COMMANDS_PATH"_L1)) {
            const QStringList thepathjson = visitor.values("LUPDATE_COMPILE_COMMANDS_PATH"_L1);
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
        if (arg == "-help"_L1 || arg == "--help"_L1 || arg == "-h"_L1) {
            printUsage();
            return 0;
        } else if (arg == "-out"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -out requires a parameter.\n"_s);
                return 1;
            }
            outputFilePath = args[i];
        } else if (arg == "-silent"_L1) {
            evalHandler.verbose = false;
        } else if (arg == "-pro-debug"_L1) {
            proDebug++;
        } else if (arg == "-version"_L1) {
            printOut(QStringLiteral("lprodump version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == "-pro"_L1) {
            ++i;
            if (i == argc) {
                printErr(QStringLiteral("The -pro option should be followed by a filename of .pro file.\n"));
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            outDirMap[file] = outDir;
        } else if (arg == "-pro-out"_L1) {
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
            translationsVariables = args.at(i).split(u',');
        } else if (arg.startsWith("-"_L1) && arg != "-"_L1) {
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
        option.qmake_abslocation = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/qmake"_L1;
    }
    option.debugLevel = proDebug;
    option.initProperties();
    option.setCommandLineArguments(QDir::currentPath(), { "CONFIG+=lupdate_run"_L1 });
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
