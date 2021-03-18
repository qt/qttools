/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <iostream>

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

class LD {
    Q_DECLARE_TR_FUNCTIONS(LProDump)
};

static void printUsage()
{
    printOut(LD::tr(
        "Usage:\n"
        "    lprodump [options] project-file...\n"
        "lprodump is part of Qt's Linguist tool chain. It extracts information\n"
        "from qmake projects to a .json file. This file can be passed to\n"
        "lupdate/lrelease using the -project option.\n\n"
        "Options:\n"
        "    -help  Display this information and exit.\n"
        "    -silent\n"
        "           Do not explain what is being done.\n"
        "    -pro <filename>\n"
        "           Name of a .pro file. Useful for files with .pro file syntax but\n"
        "           different file suffix. Projects are recursed into and merged.\n"
        "    -pro-out <directory>\n"
        "           Virtual output directory for processing subsequent .pro files.\n"
        "    -pro-debug\n"
        "           Trace processing .pro files. Specify twice for more verbosity.\n"
        "    -out <filename>\n"
        "           Name of the output file.\n"
        "    -version\n"
        "           Display the version of lprodump and exit.\n"
    ));
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
    virtual void message(int type, const QString &msg, const QString &fileName, int lineNo)
    {
        if (verbose && !(type & CumulativeEvalMessage) && (type & CategoryMask) == ErrorMessage)
            print(fileName, lineNo, msg);
    }

    virtual void fileMessage(int type, const QString &msg)
    {
        if (verbose && !(type & CumulativeEvalMessage) && (type & CategoryMask) == ErrorMessage) {
            // "Downgrade" errors, as we don't really care for them
            printErr(QLatin1String("WARNING: ") + msg + QLatin1Char('\n'));
        }
    }

    virtual void aboutToEval(ProFile *, ProFile *, EvalFileType) {}
    virtual void doneWithEval(ProFile *) {}

    bool verbose = true;
};

static EvalHandler evalHandler;

static bool isSupportedExtension(const QString &ext)
{
    return ext == QLatin1String("qml")
        || ext == QLatin1String("js") || ext == QLatin1String("qs")
        || ext == QLatin1String("ui") || ext == QLatin1String("jui");
}

static QStringList getResources(const QString &resourceFile, QMakeVfs *vfs)
{
    Q_ASSERT(vfs);
    if (!vfs->exists(resourceFile, QMakeVfs::VfsCumulative))
        return QStringList();
    QString content;
    QString errStr;
    if (vfs->readFile(vfs->idForFileName(resourceFile, QMakeVfs::VfsCumulative),
                      &content, &errStr) != QMakeVfs::ReadOk) {
        printErr(LD::tr("lprodump error: Cannot read %1: %2\n").arg(resourceFile, errStr));
        return QStringList();
    }
    const ReadQrcResult rqr = readQrcFile(resourceFile, content);
    if (rqr.hasError()) {
        printErr(LD::tr("lprodump error: %1:%2: %3\n")
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

    QStringList resourceFiles = getSources("RESOURCES", "VPATH_RESOURCES", baseVPaths, projectDir, visitor);
    foreach (const QString &resource, resourceFiles)
        sourceFiles += getResources(resource, vfs);

    QStringList installs = visitor.values(QLatin1String("INSTALLS"))
                         + visitor.values(QLatin1String("DEPLOYMENT"));
    installs.removeDuplicates();
    QDir baseDir(projectDir);
    foreach (const QString inst, installs) {
        foreach (const QString &file, visitor.values(inst + QLatin1String(".files"))) {
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

    foreach (const QString &ex, excludes) {
        // TODO: take advantage of the file list being sorted
        QRegExp rx(ex, Qt::CaseSensitive, QRegExp::Wildcard);
        for (QStringList::Iterator it = sourceFiles.begin(); it != sourceFiles.end(); ) {
            if (rx.exactMatch(*it))
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
    foreach (const QString &ex, visitor.values(QLatin1String("TR_EXCLUDE"))) {
        QRegExp rx(ex, Qt::CaseSensitive, QRegExp::Wildcard);
        for (QStringList::Iterator it = subProjects->begin(); it != subProjects->end(); ) {
            if (rx.exactMatch(*it))
                it = subProjects->erase(it);
            else
                ++it;
        }
    }
}

static QJsonArray processProjects(bool topLevel, const QStringList &proFiles,
        const QHash<QString, QString> &outDirMap,
        ProFileGlobals *option, QMakeVfs *vfs, QMakeParser *parser,
        bool *fail);

static QJsonObject processProject(const QString &proFile, ProFileGlobals *option, QMakeVfs *vfs,
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
        foreach (const QString &subdir, subProjects) {
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
        QJsonArray subResults = processProjects(false, subProFiles,
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
        const QHash<QString, QString> &outDirMap,
        ProFileGlobals *option, QMakeVfs *vfs, QMakeParser *parser, bool *fail)
{
    QJsonArray result;
    foreach (const QString &proFile, proFiles) {
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

        QJsonObject prj = processProject(proFile, option, vfs, parser, visitor);
        setValue(prj, "projectFile", proFile);
        if (visitor.contains(QLatin1String("TRANSLATIONS"))) {
            QStringList tsFiles;
            QDir proDir(QFileInfo(proFile).path());
            const QStringList translations = visitor.values(QLatin1String("TRANSLATIONS"));
            for (const QString &tsFile : translations)
                tsFiles << proDir.filePath(tsFile);
            setValue(prj, "translations", tsFiles);
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
                printErr(LD::tr("The option -out requires a parameter.\n"));
                return 1;
            }
            outputFilePath = args[i];
        } else if (arg == QLatin1String("-silent")) {
            evalHandler.verbose = false;
        } else if (arg == QLatin1String("-pro-debug")) {
            proDebug++;
        } else if (arg == QLatin1String("-version")) {
            printOut(LD::tr("lprodump version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == QLatin1String("-pro")) {
            ++i;
            if (i == argc) {
                printErr(LD::tr("The -pro option should be followed by a filename of .pro file.\n"));
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            outDirMap[file] = outDir;
        } else if (arg == QLatin1String("-pro-out")) {
            ++i;
            if (i == argc) {
                printErr(LD::tr("The -pro-out option should be followed by a directory name.\n"));
                return 1;
            }
            outDir = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
        } else if (arg.startsWith(QLatin1String("-")) && arg != QLatin1String("-")) {
            printErr(LD::tr("Unrecognized option '%1'.\n").arg(arg));
            return 1;
        } else {
            QFileInfo fi(arg);
            if (!fi.exists()) {
                printErr(LD::tr("lprodump error: File '%1' does not exist.\n").arg(arg));
                return 1;
            }
            if (!isProOrPriFile(arg)) {
                printErr(LD::tr("lprodump error: '%1' is neither a .pro nor a .pri file.\n")
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
    if (option.qmake_abslocation.isEmpty())
        option.qmake_abslocation = app.applicationDirPath() + QLatin1String("/qmake");
    option.debugLevel = proDebug;
    option.initProperties();
    option.setCommandLineArguments(QDir::currentPath(),
                                   QStringList() << QLatin1String("CONFIG+=lupdate_run"));
    QMakeVfs vfs;
    QMakeParser parser(0, &vfs, &evalHandler);

    QJsonArray results = processProjects(true, proFiles, outDirMap, &option, &vfs,
                                         &parser, &fail);
    if (fail)
        return 1;

    const QByteArray output = QJsonDocument(results).toJson(QJsonDocument::Compact);
    if (outputFilePath.isEmpty()) {
        puts(output.constData());
    } else {
        QFile f(outputFilePath);
        if (!f.open(QIODevice::WriteOnly)) {
            printErr(LD::tr("lprodump error: Cannot open %1 for writing.\n").arg(outputFilePath));
            return 1;
        }
        f.write(output);
        f.write("\n");
    }
    return 0;
}
