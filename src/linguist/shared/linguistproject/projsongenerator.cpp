// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "projsongenerator.h"
#include "projectdescriptionreader.h"

#include "qmake-parser/profileevaluator.h"
#include "qmake-parser/qmakeparser.h"
#include "qmake-parser/qmakevfs.h"
#include "qrcreader.h"

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QLibraryInfo>
#include <QtCore/QRegularExpression>

using namespace Qt::StringLiterals;

static QStringList getResources(const QString &resourceFile, QMakeVfs *vfs);
static QStringList getSources(const char *var, const char *vvar, const QStringList &baseVPaths,
                              const QString &projectDir, const ProFileEvaluator &visitor);
static QStringList getSources(const ProFileEvaluator &visitor, const QString &projectDir,
                              QMakeVfs *vfs);
static QStringList getExcludes(const ProFileEvaluator &visitor, const QString &projectDirPath);
static void excludeProjects(const ProFileEvaluator &visitor, QStringList *subProjects);

class EvalHandler : public QMakeHandler
{
public:
    void message(int type, const QString &msg, const QString &fileName, int lineNo) override
    {
        if (verbose && !(type & CumulativeEvalMessage) && (type & CategoryMask) == ErrorMessage) {
            if (lineNo > 0)
                qWarning("WARNING: %s:%d: %s", qPrintable(fileName), lineNo, qPrintable(msg));
            else if (lineNo)
                qWarning("WARNING: %s: %s", qPrintable(fileName), qPrintable(msg));
            else
                qWarning("WARNING: %s", qPrintable(msg));
        }
    }

    void fileMessage(int type, const QString &msg) override
    {
        if (verbose && !(type & CumulativeEvalMessage) && (type & CategoryMask) == ErrorMessage) {
            qWarning("WARNING: %s", qPrintable(msg));
        }
    }

    void aboutToEval(ProFile *, ProFile *, EvalFileType) override {}
    void doneWithEval(ProFile *) override {}

    bool verbose = true;
};

static QStringList getResources(const QString &resourceFile, QMakeVfs *vfs)
{
    Q_ASSERT(vfs);
    if (!vfs->exists(resourceFile, QMakeVfs::VfsCumulative))
        return QStringList();
    QString content;
    QString errStr;
    if (vfs->readFile(vfs->idForFileName(resourceFile, QMakeVfs::VfsCumulative), &content, &errStr)
        != QMakeVfs::ReadOk) {
        qWarning("Pro-JSON-generator error: Cannot read %s: %s", qPrintable(resourceFile),
                 qPrintable(errStr));
        return QStringList();
    }
    const ReadQrcResult rqr = readQrcFile(resourceFile, content);
    if (rqr.hasError()) {
        qWarning("Pro-JSON-generator error: %s:%lld: %s", qPrintable(resourceFile), rqr.line,
                 qPrintable(rqr.errorString));
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

    const QStringList resourceFiles =
            getSources("RESOURCES", "VPATH_RESOURCES", baseVPaths, projectDir, visitor);
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

static QStringList getExcludes(const ProFileEvaluator &visitor, const QString &projectDirPath)
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
static void setValue(QJsonObject &obj, const char *key, T value)
{
    obj[QLatin1String(key)] = toJsonValue(value);
}

static QJsonArray processProjects(bool topLevel, const QStringList &proFiles,
                                  const QStringList &translationsVariables,
                                  const QHash<QString, QString> &outDirMap, ProFileGlobals *option,
                                  QMakeVfs *vfs, QMakeParser *parser, EvalHandler *evalHandler,
                                  bool *fail);

static QJsonObject processProject(const QString &proFile, const QStringList &translationsVariables,
                                  ProFileGlobals *option, QMakeVfs *vfs, QMakeParser *parser,
                                  EvalHandler *evalHandler, ProFileEvaluator &visitor)
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
                                                evalHandler, nullptr);
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
                                  const QHash<QString, QString> &outDirMap, ProFileGlobals *option,
                                  QMakeVfs *vfs, QMakeParser *parser, EvalHandler *evalHandler,
                                  bool *fail)
{
    QJsonArray result;
    for (const QString &proFile : proFiles) {
        if (!outDirMap.isEmpty())
            option->setDirectories(QFileInfo(proFile).path(), outDirMap[proFile]);

        ProFile *pro;
        if (!(pro = parser->parsedProFile(proFile,
                                          topLevel ? QMakeParser::ParseReportMissing
                                                   : QMakeParser::ParseDefault))) {
            if (topLevel && fail)
                *fail = true;
            continue;
        }
        ProFileEvaluator visitor(option, parser, vfs, evalHandler);
        visitor.setCumulative(true);
        visitor.setOutputDir(option->shadowedPath(pro->directoryName()));
        if (!visitor.accept(pro)) {
            if (topLevel && fail)
                *fail = true;
            pro->deref();
            continue;
        }

        QJsonObject prj = processProject(proFile, translationsVariables, option, vfs, parser,
                                         evalHandler, visitor);
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

static std::optional<QJsonArray>
generateProjectDescription(const QStringList &proFiles, const QStringList &translationsVariables,
                           const QHash<QString, QString> &outDirMap, int proDebug, bool verbose)
{
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
    EvalHandler evalHandler;
    evalHandler.verbose = verbose;
    QMakeParser parser(0, &vfs, &evalHandler);

    QJsonArray json = processProjects(true, proFiles, translationsVariables, outDirMap, &option,
                                      &vfs, &parser, &evalHandler, &fail);
    std::optional<QJsonArray> result;
    if (!fail)
        result.emplace(std::move(json));

    return result;
}

QT_BEGIN_NAMESPACE

Projects generateProjects(const QStringList &proFiles, const QStringList &translationsVariables,
                          const QHash<QString, QString> &outDirMap, int proDebug, bool verbose,
                          QString *errorString, QJsonArray *resultJson)
{
    errorString->clear();

    std::optional<QJsonArray> jsonResults = generateProjectDescription(proFiles, translationsVariables, outDirMap,
                                                        proDebug, verbose);
    if (!jsonResults) {
        *errorString = "Failed to generate project description from .pro files"_L1;
        return {};
    }

    if (resultJson)
        *resultJson = *jsonResults;

    Projects projects = projectDescriptionFromJson(*jsonResults, errorString);
    if (!errorString->isEmpty())
        return {};

    return projects;
}

QT_END_NAMESPACE

