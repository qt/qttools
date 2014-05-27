/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "utils.h"
#include "qmlutils.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QVector>

#include <iostream>

QT_BEGIN_NAMESPACE

enum QtModule
#if defined(Q_COMPILER_CLASS_ENUM) || defined(Q_CC_MSVC)
    : quint64
#endif
{
    QtBluetoothModule = 0x1,
    QtCLuceneModule = 0x2,
    QtConcurrentModule = 0x4,
    QtCoreModule = 0x8,
    QtDeclarativeModule = 0x10,
    QtDesignerComponents = 0x20,
    QtDesignerModule = 0x40,
    QtGuiModule = 0x80,
    QtCluceneModule = 0x100,
    QtHelpModule = 0x200,
    QtMultimediaModule = 0x400,
    QtMultimediaWidgetsModule = 0x800,
    QtMultimediaQuickModule = 0x1000,
    QtNetworkModule = 0x2000,
    QtNfcModule = 0x4000,
    QtOpenGLModule = 0x8000,
    QtPositioningModule = 0x10000,
    QtPrintSupportModule = 0x20000,
    QtQmlModule = 0x40000,
    QtQuickModule = 0x80000,
    QtQuickParticlesModule = 0x100000,
    QtScriptModule = 0x200000,
    QtScriptToolsModule = 0x400000,
    QtSensorsModule = 0x800000,
    QtSerialPortModule = 0x1000000,
    QtSqlModule = 0x2000000,
    QtSvgModule = 0x4000000,
    QtTestModule = 0x8000000,
    QtWidgetsModule = 0x10000000,
    QtWinExtrasModule = 0x20000000,
    QtXmlModule = 0x40000000,
    QtXmlPatternsModule = 0x80000000,
    QtWebKitModule = 0x100000000,
    QtWebKitWidgetsModule = 0x200000000
};

struct QtModuleEntry {
    quint64 module;
    const char *option;
    const char *libraryName;
    const char *translation;
};

QtModuleEntry qtModuleEntries[] = {
    { QtBluetoothModule, "bluetooth", "Qt5Bluetooth", 0 },
    { QtCLuceneModule, "clucene", "Qt5CLucene", "qt_help" },
    { QtConcurrentModule, "concurrent", "Qt5Concurrent", "qtbase" },
    { QtCoreModule, "core", "Qt5Core", "qtbase" },
    { QtDeclarativeModule, "declarative", "Qt5Declarative", "qtquick1" },
    { QtDesignerComponents, "designercomponents", "Qt5DesignerComponents", 0 },
    { QtDesignerModule, "designer", "Qt5Designer", 0 },
    { QtGuiModule, "gui", "Qt5Gui", "qtbase" },
    { QtCluceneModule, "clucene", "Qt5CLucene", 0 },
    { QtHelpModule, "qthelp", "Qt5Help", "qt_help" },
    { QtMultimediaModule, "multimedia", "Qt5Multimedia", "qtmultimedia" },
    { QtMultimediaWidgetsModule, "multimediawidgets", "Qt5MultimediaWidgets", "qtmultimedia" },
    { QtMultimediaQuickModule, "multimediaquick", "Qt5MultimediaQuick_p", "qtmultimedia" },
    { QtNetworkModule, "network", "Qt5Network", "qtbase" },
    { QtNfcModule, "nfc", "Qt5Nfc", 0 },
    { QtOpenGLModule, "opengl", "Qt5OpenGL", 0 },
    { QtPositioningModule, "positioning", "Qt5Positioning", 0 },
    { QtPrintSupportModule, "printsupport", "Qt5PrintSupport", 0 },
    { QtQmlModule, "qml", "Qt5Qml", "qtdeclarative" },
    { QtQuickModule, "quick", "Qt5Quick", "qtdeclarative" },
    { QtQuickParticlesModule, "quickparticles", "Qt5QuickParticles", 0 },
    { QtScriptModule, "script", "Qt5Script", "qtscript" },
    { QtScriptToolsModule, "scripttools", "Qt5ScriptTools", "qtscript" },
    { QtSensorsModule, "sensors", "Qt5Sensors", 0 },
    { QtSerialPortModule, "serialport", "Qt5SerialPort", 0 },
    { QtSqlModule, "sql", "Qt5Sql", "qtbase" },
    { QtSvgModule, "svg", "Qt5Svg", 0 },
    { QtTestModule, "test", "Qt5Test", "qtbase" },
    { QtWidgetsModule, "widgets", "Qt5Widgets", "qtbase" },
    { QtWinExtrasModule, "winextras", "Qt5WinExtras", 0 },
    { QtXmlModule, "xml", "Qt5Xml", "qtbase" },
    { QtXmlPatternsModule, "xmlpatterns", "Qt5XmlPatterns", "qtxmlpatterns" },
    { QtWebKitModule, "webkit", "Qt5WebKit", 0 },
    { QtWebKitWidgetsModule, "webkitwidgets", "Qt5WebKitWidgets", 0 }
};

static const char webProcessC[] = "QtWebProcess";

static inline QString webProcessBinary(Platform p)
{
    const QString webProcess = QLatin1String(webProcessC);
    return (p & WindowsBased) ? webProcess + QStringLiteral(".exe") : webProcess;
}

static QByteArray formatQtModules(unsigned mask, bool option = false)
{
    QByteArray result;
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i) {
        if (mask & qtModuleEntries[i].module) {
            if (!result.isEmpty())
                result.append(' ');
            result.append(option ? qtModuleEntries[i].option : qtModuleEntries[i].libraryName);
        }
    }
    return result;
}

static Platform platformFromMkSpec(const QString &xSpec)
{
    if (xSpec == QLatin1String("linux-g++"))
        return Unix;
    if (xSpec.startsWith(QLatin1String("win32-")))
        return xSpec.contains(QLatin1String("g++")) ? WindowsMinGW : Windows;
    if (xSpec.startsWith(QLatin1String("winrt-x")))
        return WinRtIntel;
    if (xSpec.startsWith(QLatin1String("winrt-arm")))
        return WinRtArm;
    if (xSpec.startsWith(QLatin1String("winphone-x")))
        return WinPhoneIntel;
    if (xSpec.startsWith(QLatin1String("winphone-arm")))
        return WinPhoneArm;
    return UnknownPlatform;
}

bool optHelp = false;
int optWebKit2 = 0;

struct Options {
    enum DebugDetection {
        DebugDetectionAuto,
        DebugDetectionForceDebug,
        DebugDetectionForceRelease
    };

    Options() : plugins(true), libraries(true), quickImports(true), translations(true), systemD3dCompiler(true), compilerRunTime(false)
              , platform(Windows), additionalLibraries(0), disabledLibraries(0)
              , updateFileFlags(0), json(0), list(ListNone), debugDetection(DebugDetectionAuto) {}

    bool plugins;
    bool libraries;
    bool quickImports;
    bool translations;
    bool systemD3dCompiler;
    bool compilerRunTime;
    Platform platform;
    unsigned additionalLibraries;
    unsigned disabledLibraries;
    unsigned updateFileFlags;
    QStringList qmlDirectories; // Project's QML files.
    QString directory;
    QString libraryDirectory;
    QString binary;
    JsonOutput *json;
    ListOption list;
    DebugDetection debugDetection;
};

// Return binary from folder
static inline QString findBinary(const QString &directory, Platform platform)
{
    QDir dir(QDir::cleanPath(directory));

    const QStringList nameFilters = (platform & WindowsBased) ?
        QStringList(QStringLiteral("*.exe")) : QStringList();
    foreach (const QString &binary, dir.entryList(nameFilters, QDir::Files | QDir::Executable))
        if (!binary.contains(QLatin1String(webProcessC), Qt::CaseInsensitive))
            return dir.filePath(binary);
    return QString();
}

enum CommandLineParseFlag {
    CommandLineParseError = 0x1,
    CommandLineParseHelpRequested = 0x2
};

static inline int parseArguments(const QStringList &arguments, QCommandLineParser *parser,
                                 Options *options, QString *errorMessage)
{
    typedef QSharedPointer<QCommandLineOption> CommandLineOptionPtr;
    typedef QPair<CommandLineOptionPtr, unsigned> OptionMaskPair;
    typedef QVector<OptionMaskPair> OptionMaskVector;

    parser->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser->setApplicationDescription(QStringLiteral("Qt Deploy Tool ") + QLatin1String(QT_VERSION_STR)
        + QLatin1String("\n\nThe simplest way to use windeployqt is to add the bin directory of your Qt\n"
        "installation (e.g. <QT_DIR\\bin>) to the PATH variable and then run:\n  windeployqt <path-to-app-binary>\n"
        "If ICU, ANGLE, etc. are not in the bin directory, they need to be in the PATH\nvariable. "
        "If your application uses Qt Quick, run:\n  windeployqt --qmldir <path-to-app-qml-files> <path-to-app-binary>"));
    const QCommandLineOption helpOption = parser->addHelpOption();
    parser->addVersionOption();

    QCommandLineOption dirOption(QStringLiteral("dir"),
                                 QStringLiteral("Use directory instead of binary directory."),
                                 QStringLiteral("directory"));
    parser->addOption(dirOption);

    QCommandLineOption libDirOption(QStringLiteral("libdir"),
                                    QStringLiteral("Copy libraries to path."),
                                    QStringLiteral("path"));
    parser->addOption(libDirOption);

    QCommandLineOption debugOption(QStringLiteral("debug"),
                                   QStringLiteral("Assume debug binaries."));
    parser->addOption(debugOption);
    QCommandLineOption releaseOption(QStringLiteral("release"),
                                   QStringLiteral("Assume release binaries."));
    parser->addOption(releaseOption);

    QCommandLineOption forceOption(QStringLiteral("force"),
                                    QStringLiteral("Force updating files."));
    parser->addOption(forceOption);

    QCommandLineOption dryRunOption(QStringLiteral("dry-run"),
                                    QStringLiteral("Simulation mode. Behave normally, but do not copy/update any files."));
    parser->addOption(dryRunOption);

    QCommandLineOption noPluginsOption(QStringLiteral("no-plugins"),
                                       QStringLiteral("Skip plugin deployment."));
    parser->addOption(noPluginsOption);

    QCommandLineOption noLibraryOption(QStringLiteral("no-libraries"),
                                       QStringLiteral("Skip library deployment."));
    parser->addOption(noLibraryOption);

    QCommandLineOption qmlDirOption(QStringLiteral("qmldir"),
                                    QStringLiteral("Scan for QML-imports starting from directory."),
                                    QStringLiteral("directory"));
    parser->addOption(qmlDirOption);

    QCommandLineOption noQuickImportOption(QStringLiteral("no-quick-import"),
                                           QStringLiteral("Skip deployment of Qt Quick imports."));
    parser->addOption(noQuickImportOption);

    QCommandLineOption noTranslationOption(QStringLiteral("no-translations"),
                                           QStringLiteral("Skip deployment of translations."));
    parser->addOption(noTranslationOption);

    QCommandLineOption noSystemD3DCompilerOption(QStringLiteral("no-system-d3d-compiler"),
                                                 QStringLiteral("Skip deployment of the system D3D compiler."));
    parser->addOption(noSystemD3DCompilerOption);


    QCommandLineOption compilerRunTimeOption(QStringLiteral("compiler-runtime"),
                                             QStringLiteral("Deploy compiler runtime (Desktop only)."));
    parser->addOption(compilerRunTimeOption);

    QCommandLineOption noCompilerRunTimeOption(QStringLiteral("no-compiler-runtime"),
                                             QStringLiteral("Do not deploy compiler runtime (Desktop only)."));
    parser->addOption(noCompilerRunTimeOption);

    QCommandLineOption webKitOption(QStringLiteral("webkit2"),
                                    QStringLiteral("Deployment of WebKit2 (web process)."));
    parser->addOption(webKitOption);

    QCommandLineOption noWebKitOption(QStringLiteral("no-webkit2"),
                                      QStringLiteral("Skip deployment of WebKit2."));
    parser->addOption(noWebKitOption);

    QCommandLineOption jsonOption(QStringLiteral("json"),
                                  QStringLiteral("Print to stdout in JSON format."));
    parser->addOption(jsonOption);

    QCommandLineOption listOption(QStringLiteral("list"),
                                  QLatin1String("Print only the names of the files copied.\n"
                                                "Available options:\n"
                                                "  source:   absolute path of the source files\n"
                                                "  target:   absolute path of the target files\n"
                                                "  relative: paths of the target files, relative\n"
                                                "            to the target directory\n"
                                                "  mapping:  outputs the source and the relative\n"
                                                "            target, suitable for use within an\n"
                                                "            Appx mapping file"),
                                  QStringLiteral("option"));
    parser->addOption(listOption);

    QCommandLineOption verboseOption(QStringLiteral("verbose"),
                                     QStringLiteral("Verbose level."),
                                     QStringLiteral("level"));
    parser->addOption(verboseOption);

    parser->addPositionalArgument(QStringLiteral("[file]"),
                                  QStringLiteral("Binary or directory containing the binary."));

    OptionMaskVector enabledModules;
    OptionMaskVector disabledModules;
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i) {
        const QString option = QLatin1String(qtModuleEntries[i].option);
        const QString name = QLatin1String(qtModuleEntries[i].libraryName);
        const QString enabledDescription = QStringLiteral("Add ") + name + QStringLiteral(" module.");
        CommandLineOptionPtr enabledOption(new QCommandLineOption(option, enabledDescription));
        parser->addOption(*enabledOption.data());
        enabledModules.push_back(OptionMaskPair(enabledOption, qtModuleEntries[i].module));

        const QString disabledDescription = QStringLiteral("Remove ") + name + QStringLiteral(" module.");
        CommandLineOptionPtr disabledOption(new QCommandLineOption(QStringLiteral("no-") + option,
                                                                   disabledDescription));
        disabledModules.push_back(OptionMaskPair(disabledOption, qtModuleEntries[i].module));
        parser->addOption(*disabledOption.data());
    }

    const bool success = parser->parse(arguments);
    if (parser->isSet(helpOption))
        return CommandLineParseHelpRequested;
    if (!success) {
        *errorMessage = parser->errorText();
        return CommandLineParseError;
    }

    options->libraryDirectory = parser->value(libDirOption);
    options->plugins = !parser->isSet(noPluginsOption);
    options->libraries = !parser->isSet(noLibraryOption);
    options->translations = !parser->isSet(noTranslationOption);
    options->systemD3dCompiler = !parser->isSet(noSystemD3DCompilerOption);
    options->quickImports = !parser->isSet(noQuickImportOption);

    if (parser->isSet(compilerRunTimeOption))
        options->compilerRunTime = true;
    else if (parser->isSet(noCompilerRunTimeOption))
        options->compilerRunTime = false;

    if (options->compilerRunTime && options->platform != WindowsMinGW && options->platform != Windows) {
        *errorMessage = QStringLiteral("Deployment of the compiler runtime is implemented for Desktop only.");
        return CommandLineParseError;
    }

    const bool forceDebug = parser->isSet(debugOption);
    const bool forceRelease = parser->isSet(releaseOption);
    if (forceDebug && forceRelease)
        std::wcerr << "Warning: both -debug and -release were specified, defaulting to debug.\n";
    if (forceDebug)
        options->debugDetection = Options::DebugDetectionForceDebug;
    else if (forceRelease)
        options->debugDetection = Options::DebugDetectionForceRelease;

    if (parser->isSet(forceOption))
        options->updateFileFlags |= ForceUpdateFile;
    if (parser->isSet(dryRunOption))
        options->updateFileFlags |= SkipUpdateFile;

    for (size_t i = 0; i < qtModulesCount; ++i) {
        if (parser->isSet(*enabledModules.at(int(i)).first.data()))
            options->additionalLibraries |= enabledModules.at(int(i)).second;
        if (parser->isSet(*disabledModules.at(int(i)).first.data()))
            options->disabledLibraries |= disabledModules.at(int(i)).second;
    }

    // Add some dependencies
    if (options->additionalLibraries & QtQuickModule)
        options->additionalLibraries |= QtQmlModule;
    if (options->additionalLibraries & QtHelpModule)
        options->additionalLibraries |= QtCLuceneModule;
    if (options->additionalLibraries & QtDesignerComponents)
        options->additionalLibraries |= QtDesignerModule;

    if (parser->isSet(listOption)) {
        const QString value = parser->value(listOption);
        if (value == QStringLiteral("source")) {
            options->list = ListSource;
        } else if (value == QStringLiteral("target")) {
            options->list = ListTarget;
        } else if (value == QStringLiteral("relative")) {
            options->list = ListRelative;
        } else if (value == QStringLiteral("mapping")) {
            options->list = ListMapping;
        } else {
            *errorMessage = QStringLiteral("Please specify a valid option for -list (source, target, relative, mapping).");
            return CommandLineParseError;
        }
    }

    if (parser->isSet(jsonOption) || options->list) {
        optVerboseLevel = 0;
        options->json = new JsonOutput;
    } else {
        if (parser->isSet(verboseOption)) {
            bool ok;
            const QString value = parser->value(verboseOption);
            optVerboseLevel = value.toInt(&ok);
            if (!ok || optVerboseLevel < 0) {
                *errorMessage = QStringLiteral("Invalid value \"%1\" passed for verbose level.").arg(value);
                return CommandLineParseError;
            }
        }
    }

    const QStringList posArgs = parser->positionalArguments();
    if (posArgs.isEmpty()) {
        *errorMessage = QStringLiteral("Please specify the binary or folder.");
        return CommandLineParseError | CommandLineParseHelpRequested;
    }
    if (posArgs.size() > 1) {
        QStringList superfluousArguments = posArgs;
        superfluousArguments.pop_front();
        *errorMessage = QStringLiteral("Superfluous arguments specified: ") + superfluousArguments.join(QLatin1Char(','));
        return CommandLineParseError;
    }

    if (parser->isSet(dirOption))
        options->directory = parser->value(dirOption);

    if (parser->isSet(qmlDirOption))
        options->qmlDirectories = parser->values(qmlDirOption);

    const QString &file = posArgs.front();
    const QFileInfo fi(QDir::cleanPath(file));
    if (!fi.exists()) {
        *errorMessage = QLatin1Char('"') + file + QStringLiteral("\" does not exist.");
        return CommandLineParseError;
    }

    if (!options->directory.isEmpty() && !fi.isFile()) { // -dir was specified - expecting file.
        *errorMessage = QLatin1Char('"') + file + QStringLiteral("\" is not an executable file.");
        return CommandLineParseError;
    }

    if (fi.isFile()) {
        options->binary = fi.absoluteFilePath();
        if (options->directory.isEmpty())
            options->directory = fi.absolutePath();
    } else {
        options->binary = findBinary(fi.absoluteFilePath(), options->platform);
        if (options->binary.isEmpty()) {
            *errorMessage = QStringLiteral("Unable to find binary in \"") + file + QLatin1Char('"');
            return CommandLineParseError;
        }
        options->directory = fi.absoluteFilePath();
    } // directory.
    return 0;
}

// Simple line wrapping at 80 character boundaries.
static inline QString lineBreak(QString s)
{
    for (int i = 80; i < s.size(); i += 80) {
        const int lastBlank = s.lastIndexOf(QLatin1Char(' '), i);
        if (lastBlank >= 0) {
            s[lastBlank] = QLatin1Char('\n');
            i = lastBlank + 1;
        }
    }
    return s;
}

static inline QString helpText(const QCommandLineParser &p)
{
    QString result = p.helpText();
    // Replace the default-generated text which is too long by a short summary
    // explaining how to enable single libraries.
    const int moduleStart = result.indexOf(QLatin1String("\n  --bluetooth"));
    const int argumentsStart = result.lastIndexOf(QLatin1String("\nArguments:"));
    if (moduleStart >= argumentsStart)
        return result;
    QString moduleHelp = QLatin1String(
        "\n\nQt libraries can be added by passing their name (-xml) or removed by passing\n"
        "the name prepended by --no- (--no-xml). Available libraries:\n");
    moduleHelp += lineBreak(QString::fromLatin1(formatQtModules(0xFFFFFFFF, true)));
    moduleHelp += QLatin1Char('\n');
    result.replace(moduleStart, argumentsStart - moduleStart, moduleHelp);
    return result;
}

// Helper for recursively finding all dependent Qt libraries.
static bool findDependentQtLibraries(const QString &qtBinDir, const QString &binary, Platform platform,
                                     QString *errorMessage, QStringList *result,
                                     unsigned *wordSize = 0, bool *isDebug = 0,
                                     int *directDependencyCount = 0, int recursionDepth = 0)
{
    QStringList dependentLibs;
    if (directDependencyCount)
        *directDependencyCount = 0;
    if (!readExecutable(binary, platform, errorMessage, &dependentLibs, wordSize, isDebug)) {
        errorMessage->prepend(QLatin1String("Unable to find dependent libraries of ") +
                              QDir::toNativeSeparators(binary) + QLatin1String(" :"));
        return false;
    }
    // Filter out the Qt libraries. Note that depends.exe finds libs from optDirectory if we
    // are run the 2nd time (updating). We want to check against the Qt bin dir libraries
    const int start = result->size();
    const QRegExp filterRegExp(QStringLiteral("Qt5"), Qt::CaseInsensitive, QRegExp::FixedString);
    foreach (const QString &qtLib, dependentLibs.filter(filterRegExp)) {
        const QString path = normalizeFileName(qtBinDir + QLatin1Char('/') + QFileInfo(qtLib).fileName());
        if (!result->contains(path))
            result->append(path);
    }
    const int end = result->size();
    if (directDependencyCount)
        *directDependencyCount = end - start;
    // Recurse
    for (int i = start; i < end; ++i)
        if (!findDependentQtLibraries(qtBinDir, result->at(i), platform, errorMessage, result, 0, 0, 0, recursionDepth + 1))
            return false;
    return true;
}

// Base class to filter debug/release Windows DLLs for functions to be passed to updateFile().
// Tries to pre-filter by namefilter and does check via PE.
class DllDirectoryFileEntryFunction {
public:
    explicit DllDirectoryFileEntryFunction(Platform platform, bool debug, const QString &prefix = QString()) :
        m_platform(platform), m_dllDebug(debug), m_prefix(prefix) {}

    QStringList operator()(const QDir &dir) const
        { return findSharedLibraries(dir, m_platform, m_dllDebug, m_prefix); }

private:
    const Platform m_platform;
    const bool m_dllDebug;
    const QString m_prefix;
};

// File entry filter function for updateFile() that returns a list of files for
// QML import trees: DLLs (matching debgug) and .qml/,js, etc.
class QmlDirectoryFileEntryFunction {
public:
    explicit QmlDirectoryFileEntryFunction(Platform platform, bool debug, bool skipQmlSources = false)
        : m_qmlNameFilter(QmlDirectoryFileEntryFunction::qmlNameFilters(skipQmlSources))
        , m_dllFilter(platform, debug)
    {}

    QStringList operator()(const QDir &dir) const { return m_dllFilter(dir) + m_qmlNameFilter(dir);  }

private:
    static inline QStringList qmlNameFilters(bool skipQmlSources)
    {
        QStringList result;
        result << QStringLiteral("qmldir") << QStringLiteral("*.qmltypes");
        if (!skipQmlSources)
            result << QStringLiteral("*.js") <<  QStringLiteral("*.qml") << QStringLiteral("*.png");
        return result;
    }

    NameFilterFileEntryFunction m_qmlNameFilter;
    DllDirectoryFileEntryFunction m_dllFilter;
};

static inline unsigned qtModuleForPlugin(const QString &subDirName)
{
    if (subDirName == QLatin1String("accessible") || subDirName == QLatin1String("iconengines")
        || subDirName == QLatin1String("imageformats") || subDirName == QLatin1String("platforms")) {
        return QtGuiModule;
    }
    if (subDirName == QLatin1String("bearer"))
        return QtNetworkModule;
    if (subDirName == QLatin1String("sqldrivers"))
        return QtSqlModule;
    if (subDirName == QLatin1String("audio") || subDirName == QLatin1String("mediaservice") || subDirName == QLatin1String("playlistformats"))
        return QtMultimediaModule;
    if (subDirName == QLatin1String("printsupport"))
        return QtPrintSupportModule;
    if (subDirName == QLatin1String("qmltooling"))
        return QtDeclarativeModule | QtQuickModule;
    if (subDirName == QLatin1String("position"))
        return QtPositioningModule;
    if (subDirName == QLatin1String("sensors") || subDirName == QLatin1String("sensorgestures"))
        return QtSensorsModule;
    return 0; // "designer"
}

static unsigned qtModule(const QString &module)
{
    unsigned bestMatch = 0;
    int bestMatchLength = 0;
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i) {
        const QString libraryName = QLatin1String(qtModuleEntries[i].libraryName);
        if (libraryName.size() > bestMatchLength && module.contains(libraryName, Qt::CaseInsensitive)) {
            bestMatch = qtModuleEntries[i].module;
            bestMatchLength = libraryName.size();
        }
    }
    return bestMatch;
}

QStringList findQtPlugins(unsigned *usedQtModules, unsigned disabledQtModules,
                          const QString &qtPluginsDirName, const QString &libraryLocation,
                          bool debug, Platform platform, QString *platformPlugin)
{
    QString errorMessage;
    if (qtPluginsDirName.isEmpty())
        return QStringList();
    QDir pluginsDir(qtPluginsDirName);
    QStringList result;
    foreach (const QString &subDirName, pluginsDir.entryList(QStringList(QLatin1String("*")), QDir::Dirs | QDir::NoDotAndDotDot)) {
        const unsigned module = qtModuleForPlugin(subDirName);
        if (module & *usedQtModules) {
            const QString subDirPath = qtPluginsDirName + QLatin1Char('/') + subDirName;
            QDir subDir(subDirPath);
            // Filter for platform or any.
            QString filter;
            const bool isPlatformPlugin = subDirName == QLatin1String("platforms");
            if (isPlatformPlugin) {
                switch (platform) {
                case Windows:
                case WindowsMinGW:
                    filter = QStringLiteral("qwindows");
                    break;
                case WinRtIntel:
                case WinRtArm:
                case WinPhoneIntel:
                case WinPhoneArm:
                    filter = QStringLiteral("qwinrt");
                    break;
                case Unix:
                    filter = QStringLiteral("libqxcb");
                    break;
                case UnknownPlatform:
                    break;
                }
            } else {
                filter  = QLatin1String("*");
            }
            const QStringList plugins = findSharedLibraries(subDir, platform ,debug, filter);
            foreach (const QString &plugin, plugins) {
                const QString pluginPath = subDir.absoluteFilePath(plugin);
                if (isPlatformPlugin)
                    *platformPlugin = pluginPath;
                QStringList dependentQtLibs;
                unsigned neededModules = 0;
                if (findDependentQtLibraries(libraryLocation, pluginPath, platform, &errorMessage, &dependentQtLibs)) {
                    for (int d = 0; d < dependentQtLibs.size(); ++ d)
                        neededModules |= qtModule(dependentQtLibs.at(d));
                } else {
                    std::wcerr << "Warning: Cannot determine dependencies of "
                        << QDir::toNativeSeparators(pluginPath) << ": " << errorMessage << '\n';
                }
                if (neededModules & disabledQtModules) {
                    if (optVerboseLevel)
                        std::wcout << "Skipping plugin " << plugin << " due to disabled dependencies.\n";
                } else {
                    if (const unsigned missingModules = (neededModules & ~*usedQtModules)) {
                        *usedQtModules |= missingModules;
                        if (optVerboseLevel)
                            std::wcout << "Adding " << formatQtModules(missingModules).constData() << " for " << plugin << '\n';
                    }
                    result.append(pluginPath);
                }
            } // for filter
        } // type matches
    } // for plugin folder
    return result;
}

static QStringList translationNameFilters(unsigned modules, const QString &prefix)
{
    QStringList result;
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i) {
        if ((qtModuleEntries[i].module & modules) && qtModuleEntries[i].translation) {
            const QString name = QLatin1String(qtModuleEntries[i].translation) +
                                 QLatin1Char('_') +  prefix + QStringLiteral(".qm");
            if (!result.contains(name))
                result.push_back(name);
        }
    }
    return result;
}

static bool deployTranslations(const QString &sourcePath, unsigned usedQtModules,
                               const QString &target, unsigned flags, QString *errorMessage)
{
    // Find available languages prefixes by checking on qtbase.
    QStringList prefixes;
    QDir sourceDir(sourcePath);
    const QStringList qmFilter = QStringList(QStringLiteral("qtbase_*.qm"));
    foreach (QString qmFile, sourceDir.entryList(qmFilter)) {
       qmFile.chop(3);
       qmFile.remove(0, 7);
       prefixes.push_back(qmFile);
    }
    if (prefixes.isEmpty()) {
        std::wcerr << "Warning: Could not find any translations in "
                   << QDir::toNativeSeparators(sourcePath) << " (developer build?)\n.";
        return true;
    }
    // Run lconvert to concatenate all files into a single named "qt_<prefix>.qm" in the application folder
    // Use QT_INSTALL_TRANSLATIONS as working directory to keep the command line short.
    const QString absoluteTarget = QFileInfo(target).absoluteFilePath();
    const QString binary = QStringLiteral("lconvert");
    QStringList arguments;
    foreach (const QString &prefix, prefixes) {
        const QString targetFile = QStringLiteral("qt_") + prefix + QStringLiteral(".qm");
        arguments.append(QStringLiteral("-o"));
        arguments.append(QDir::toNativeSeparators(absoluteTarget + QLatin1Char('/') + targetFile));
        foreach (const QString &qmFile, sourceDir.entryList(translationNameFilters(usedQtModules, prefix)))
            arguments.append(qmFile);
        if (optVerboseLevel)
            std::wcout << "Creating " << targetFile << "...\n";
        unsigned long exitCode;
        if (!(flags & SkipUpdateFile)
            && (!runProcess(binary, arguments, sourcePath, &exitCode, 0, 0, errorMessage) || exitCode)) {
            return false;
        }
    } // for prefixes.
    return true;
}

struct DeployResult
{
    DeployResult() : success(false), directlyUsedQtLibraries(0), usedQtLibraries(0), deployedQtLibraries(0) {}
    operator bool() const { return success; }

    bool success;
    unsigned directlyUsedQtLibraries;
    unsigned usedQtLibraries;
    unsigned deployedQtLibraries;
};

static QString libraryPath(const QString &libraryLocation, const char *name,
                           const QString &qtLibInfix, Platform platform, bool debug)
{
    QString result = libraryLocation + QLatin1Char('/');
    if (platform & WindowsBased) {
        result += QLatin1String(name);
        result += qtLibInfix;
        if (debug)
            result += QLatin1Char('d');
    } else if (platform & UnixBased) {
        result += QStringLiteral("lib");
        result += QLatin1String(name);
        result += qtLibInfix;
    }
    result += sharedLibrarySuffix(platform);
    return result;
}

static QStringList compilerRunTimeLibs(Platform platform, unsigned wordSize)
{
    QStringList result;
    switch (platform) {
    case WindowsMinGW: { // MinGW: Add runtime libraries
        static const char *minGwRuntimes[] = {"*gcc_", "*stdc++", "*winpthread"};
        const QString gcc = findInPath(QStringLiteral("g++.exe"));
        if (gcc.isEmpty())
            break;
        const QString binPath = QFileInfo(gcc).absolutePath();
        QDir dir(binPath);
        QStringList filters;
        const QString suffix = QLatin1Char('*') + sharedLibrarySuffix(platform);
        const size_t count = sizeof(minGwRuntimes) / sizeof(minGwRuntimes[0]);
        for (size_t i = 0; i < count; ++i)
            filters.append(QLatin1String(minGwRuntimes[i]) + suffix);
        foreach (const QString &dll, dir.entryList(filters, QDir::Files))
                result.append(binPath + QLatin1Char('/') + dll);
    }
        break;
    case Windows: { // MSVC/Desktop: Add redistributable packages.
        const char vcDirVar[] = "VCINSTALLDIR";
        const QChar slash(QLatin1Char('/'));
        QString vcRedistDirName = QDir::cleanPath(QFile::decodeName(qgetenv(vcDirVar)));
        if (vcRedistDirName.isEmpty()) {
             std::wcerr << "Warning: Cannot find Visual Studio installation directory, " << vcDirVar << " is not set.\n";
             break;
        }
        if (!vcRedistDirName.endsWith(slash))
            vcRedistDirName.append(slash);
        vcRedistDirName.append(QStringLiteral("redist"));
        QDir vcRedistDir(vcRedistDirName);
        if (!vcRedistDir.exists()) {
            std::wcerr << "Warning: Cannot find Visual Studio redist directory, "
                       << QDir::toNativeSeparators(vcRedistDirName).toStdWString() << ".\n";
            break;
        }
        const QStringList countryCodes = vcRedistDir.entryList(QStringList(QStringLiteral("[0-9]*")), QDir::Dirs);
        QString redist;
        if (!countryCodes.isEmpty()) {
            const QFileInfo fi(vcRedistDirName + slash + countryCodes.first() + slash
                               + QStringLiteral("vcredist_x") + QLatin1String(wordSize > 32 ? "64" : "86")
                               + QStringLiteral(".exe"));
            if (fi.isFile())
                redist = fi.absoluteFilePath();
        }
        if (redist.isEmpty()) {
            std::wcerr << "Warning: Cannot find Visual Studio redistributable in "
                       << QDir::toNativeSeparators(vcRedistDirName).toStdWString() << ".\n";
            break;
        }
        result.append(redist);
    }
    default:
        break;
    }
    return result;
}

static inline int qtVersion(const QMap<QString, QString> &qmakeVariables)
{
    const QString versionString = qmakeVariables.value(QStringLiteral("QT_VERSION"));
    const QChar dot = QLatin1Char('.');
    const int majorVersion = versionString.section(dot, 0, 0).toInt();
    const int minorVersion = versionString.section(dot, 1, 1).toInt();
    const int patchVersion = versionString.section(dot, 2, 2).toInt();
    return (majorVersion << 16) | (minorVersion << 8) | patchVersion;
}

// Determine the Qt lib infix from the library path of "Qt5Core<qtblibinfix>[d].dll".
static inline QString qtlibInfixFromCoreLibName(const QString &path, bool isDebug, Platform platform)
{
    const int startPos = path.lastIndexOf(QLatin1Char('/')) + 8;
    int endPos = path.lastIndexOf(QLatin1Char('.'));
    if (isDebug && (platform & WindowsBased))
        endPos--;
    return endPos > startPos ? path.mid(startPos, endPos - startPos) : QString();
}

static DeployResult deploy(const Options &options,
                           const QMap<QString, QString> &qmakeVariables,
                           QString *errorMessage)
{
    DeployResult result;

    const QChar slash = QLatin1Char('/');

    const QString qtBinDir = qmakeVariables.value(QStringLiteral("QT_INSTALL_BINS"));
    const QString libraryLocation = options.platform == Unix ? qmakeVariables.value(QStringLiteral("QT_INSTALL_LIBS")) : qtBinDir;
    const int version = qtVersion(qmakeVariables);

    if (optVerboseLevel > 1)
        std::wcout << "Qt binaries in " << QDir::toNativeSeparators(qtBinDir) << '\n';

    QStringList dependentQtLibs;
    bool detectedDebug;
    unsigned wordSize;
    int directDependencyCount;
    if (!findDependentQtLibraries(libraryLocation, options.binary, options.platform, errorMessage, &dependentQtLibs, &wordSize,
                                  &detectedDebug, &directDependencyCount))
        return result;

    const bool isDebug = options.debugDetection == Options::DebugDetectionAuto ? detectedDebug: options.debugDetection == Options::DebugDetectionForceDebug;

    // Determine application type, check Quick2 is used by looking at the
    // direct dependencies (do not be fooled by QtWebKit depending on it).
    QString qtLibInfix;
    for (int m = 0; m < directDependencyCount; ++m) {
        const unsigned module = qtModule(dependentQtLibs.at(m));
        result.directlyUsedQtLibraries |= module;
        if (module == QtCoreModule)
            qtLibInfix = qtlibInfixFromCoreLibName(dependentQtLibs.at(m), isDebug, options.platform);
    }

    const bool usesQml2 = !(options.disabledLibraries & QtQmlModule)
                            && ((result.directlyUsedQtLibraries & QtQmlModule)
                                || (options.additionalLibraries & QtQmlModule));

    if (optVerboseLevel) {
        std::wcout << QDir::toNativeSeparators(options.binary) << ' '
                   << wordSize << " bit, " << (isDebug ? "debug" : "release")
                   << " executable";
        if (usesQml2)
            std::wcout << " [QML]";
        std::wcout << '\n';
    }

    if (dependentQtLibs.isEmpty()) {
        *errorMessage = QDir::toNativeSeparators(options.binary) +  QStringLiteral(" does not seem to be a Qt executable.");
        return result;
    }

    // Some Windows-specific checks in QtCore: ICU
    if (options.platform & WindowsBased)  {
        const QStringList qt5Core = dependentQtLibs.filter(QStringLiteral("Qt5Core"), Qt::CaseInsensitive);
        if (!qt5Core.isEmpty()) {
            QStringList icuLibs = findDependentLibraries(qt5Core.front(), options.platform, errorMessage).filter(QStringLiteral("ICU"), Qt::CaseInsensitive);
            if (!icuLibs.isEmpty()) {
                // Find out the ICU version to add the data library icudtXX.dll, which does not show
                // as a dependency.
                QRegExp numberExpression(QStringLiteral("\\d+"));
                Q_ASSERT(numberExpression.isValid());
                const int index = numberExpression.indexIn(icuLibs.front());
                if (index >= 0)  {
                    const QString icuVersion = icuLibs.front().mid(index, numberExpression.matchedLength());
                    if (optVerboseLevel > 1)
                        std::wcout << "Adding ICU version " << icuVersion << '\n';
                    icuLibs.push_back(QStringLiteral("icudt") + icuVersion + QLatin1String(windowsSharedLibrarySuffix));
                }
                foreach (const QString &icuLib, icuLibs) {
                    const QString icuPath = findInPath(icuLib);
                    if (icuPath.isEmpty()) {
                        *errorMessage = QStringLiteral("Unable to locate ICU library ") + icuLib;
                        return result;
                    }
                    dependentQtLibs.push_back(icuPath);
                } // foreach icuLib
            } // !icuLibs.isEmpty()
        } // Qt5Core
    } // Windows

    // Scan Quick2 imports
    QmlImportScanResult qmlScanResult;
    if (options.quickImports && usesQml2) {
        QStringList qmlDirectories = options.qmlDirectories;
        if (qmlDirectories.isEmpty()) {
            const QString qmlDirectory = findQmlDirectory(options.platform, options.directory);
            if (!qmlDirectory.isEmpty())
                qmlDirectories.append(qmlDirectory);
        }
        foreach (const QString &qmlDirectory, qmlDirectories) {
            if (optVerboseLevel >= 1)
                std::wcout << "Scanning " << QDir::toNativeSeparators(qmlDirectory) << ":\n";
            const QmlImportScanResult scanResult = runQmlImportScanner(qmlDirectory, qmakeVariables.value(QStringLiteral("QT_INSTALL_QML")), options.platform, isDebug, errorMessage);
            if (!scanResult.ok)
                return result;
            qmlScanResult.append(scanResult);
            // Additional dependencies of QML plugins.
            foreach (const QString &plugin, qmlScanResult.plugins) {
                if (!findDependentQtLibraries(libraryLocation, plugin, options.platform, errorMessage, &dependentQtLibs, &wordSize, &detectedDebug))
                    return result;
            }
            if (optVerboseLevel >= 1) {
                std::wcout << "QML imports:\n";
                foreach (const QmlImportScanResult::Module &mod, qmlScanResult.modules) {
                    std::wcout << "  '" << mod.name << "' "
                               << QDir::toNativeSeparators(mod.sourcePath) << '\n';
                }
                if (optVerboseLevel >= 2) {
                    std::wcout << "QML plugins:\n";
                    foreach (const QString &p, qmlScanResult.plugins)
                        std::wcout << "  " << QDir::toNativeSeparators(p) << '\n';
                }
            }
        }
    }

    // Find the plugins and check whether ANGLE, D3D are required on the platform plugin.
    QString platformPlugin;
    // Sort apart Qt 5 libraries in the ones that are represented by the
    // QtModule enumeration (and thus controlled by flags) and others.
    QStringList deployedQtLibraries;
    for (int i = 0 ; i < dependentQtLibs.size(); ++i)  {
        if (const unsigned qtm = qtModule(dependentQtLibs.at(i)))
            result.usedQtLibraries |= qtm;
        else
            deployedQtLibraries.push_back(dependentQtLibs.at(i)); // Not represented by flag.
    }
    result.deployedQtLibraries = (result.usedQtLibraries | options.additionalLibraries) & ~options.disabledLibraries;

    const QStringList plugins =
        findQtPlugins(&result.deployedQtLibraries,
                      // For non-QML applications, disable QML to prevent it from being pulled in by the qtaccessiblequick plugin.
                      options.disabledLibraries | (usesQml2 ? 0 : (QtQmlModule | QtQuickModule)),
                      qmakeVariables.value(QStringLiteral("QT_INSTALL_PLUGINS")), libraryLocation,
                      isDebug, options.platform, &platformPlugin);

    // Apply options flags and re-add library names.
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i)
        if (result.deployedQtLibraries & qtModuleEntries[i].module)
            deployedQtLibraries.push_back(libraryPath(libraryLocation, qtModuleEntries[i].libraryName, qtLibInfix, options.platform, isDebug));

    if (optVerboseLevel >= 1) {
        std::wcout << "Direct dependencies: " << formatQtModules(result.directlyUsedQtLibraries).constData()
                   << "\nAll dependencies   : " << formatQtModules(result.usedQtLibraries).constData()
                   << "\nTo be deployed     : " << formatQtModules(result.deployedQtLibraries).constData() << '\n';
    }

    if (optVerboseLevel > 1)
        std::wcout << "Plugins: " << plugins.join(QLatin1Char(',')) << '\n';

    if ((result.deployedQtLibraries & QtGuiModule) && platformPlugin.isEmpty()) {
        *errorMessage =QStringLiteral("Unable to find the platform plugin.");
        return result;
    }

    // Check for ANGLE on the platform plugin.
    if (options.platform & WindowsBased)  {
        const QStringList platformPluginLibraries = findDependentLibraries(platformPlugin, options.platform, errorMessage);
        const QStringList libEgl = platformPluginLibraries.filter(QStringLiteral("libegl"), Qt::CaseInsensitive);
        if (!libEgl.isEmpty()) {
            const QString libEglFullPath = qtBinDir + slash + QFileInfo(libEgl.front()).fileName();
            deployedQtLibraries.push_back(libEglFullPath);
            const QStringList libGLESv2 = findDependentLibraries(libEglFullPath, options.platform, errorMessage).filter(QStringLiteral("libGLESv2"), Qt::CaseInsensitive);
            if (!libGLESv2.isEmpty()) {
                const QString libGLESv2FullPath = qtBinDir + slash + QFileInfo(libGLESv2.front()).fileName();
                deployedQtLibraries.push_back(libGLESv2FullPath);
            }
            // Find the system D3d Compiler matching the D3D library.
            if (options.systemD3dCompiler && options.platform != WinPhoneArm && options.platform != WinPhoneIntel
                    && options.platform != WinRtArm && options.platform != WinRtIntel) {
                const QString d3dCompiler = findD3dCompiler(options.platform, qtBinDir, wordSize);
                if (d3dCompiler.isEmpty()) {
                    std::wcerr << "Warning: Cannot find any version of the d3dcompiler DLL.\n";
                } else {
                    deployedQtLibraries.push_back(d3dCompiler);
                }
            }
            // Deploy Qt's D3D compiler starting from 5.3 onwards.
            if (version >= 0x050300) {
                QString d3dCompilerQt = qtBinDir + QStringLiteral("/d3dcompiler_qt");
                if (isDebug)
                    d3dCompilerQt += QLatin1Char('d');
                d3dCompilerQt += QLatin1String(windowsSharedLibrarySuffix);
                if (QFileInfo(d3dCompilerQt).exists())
                    deployedQtLibraries.push_back(d3dCompilerQt);
            }
        } // !libEgl.isEmpty()
    } // Windows

    // Update libraries
    if (options.libraries) {
        const QString targetPath = options.libraryDirectory.isEmpty() ?
            options.directory : options.libraryDirectory;
        QStringList libraries = deployedQtLibraries;
        if (options.compilerRunTime)
            libraries.append(compilerRunTimeLibs(options.platform, wordSize));
        foreach (const QString &qtLib, libraries) {
            if (!updateFile(qtLib, targetPath, options.updateFileFlags, options.json, errorMessage))
                return result;
        }
    } // optLibraries

    // Update plugins
    if (options.plugins) {
        QDir dir(options.directory);
        foreach (const QString &plugin, plugins) {
            const QString targetDirName = plugin.section(slash, -2, -2);
            if (!dir.exists(targetDirName)) {
                if (optVerboseLevel)
                    std::wcout << "Creating directory " << targetDirName << ".\n";
                if (!(options.updateFileFlags & SkipUpdateFile) && !dir.mkdir(targetDirName)) {
                    std::wcerr << "Cannot create " << targetDirName << ".\n";
                    *errorMessage = QStringLiteral("Cannot create ") + targetDirName +  QLatin1Char('.');
                    return result;
                }
            }
            const QString targetPath = options.directory + slash + targetDirName;
            if (!updateFile(plugin, targetPath, options.updateFileFlags, options.json, errorMessage))
                return result;
        }
    } // optPlugins

    // Update Quick imports
    const bool usesQuick1 = result.deployedQtLibraries & QtDeclarativeModule;
    // Do not be fooled by QtWebKit.dll depending on Quick into always installing Quick imports
    // for WebKit1-applications. Check direct dependency only.
    if (options.quickImports && (usesQuick1 || usesQml2)) {
        const QmlDirectoryFileEntryFunction qmlFileEntryFunction(options.platform, isDebug);
        if (usesQml2) {
            foreach (const QmlImportScanResult::Module &module, qmlScanResult.modules) {
                const QString installPath = module.installPath(options.directory);
                if (optVerboseLevel > 1)
                    std::wcout << "Installing: '" << module.name
                               << "' from " << module.sourcePath << " to "
                               << QDir::toNativeSeparators(installPath) << '\n';
                if (installPath != options.directory && !createDirectory(installPath, errorMessage))
                    return result;
                const bool updateResult = module.sourcePath.contains(QLatin1String("QtQuick/Controls"))
                    || module.sourcePath.contains(QLatin1String("QtQuick/Dialogs")) ?
                    updateFile(module.sourcePath, QmlDirectoryFileEntryFunction(options.platform, isDebug, true),
                               installPath, options.updateFileFlags | RemoveEmptyQmlDirectories,
                               options.json, errorMessage) :
                    updateFile(module.sourcePath, qmlFileEntryFunction, installPath, options.updateFileFlags,
                               options.json, errorMessage);
                if (!updateResult)
                    return result;
            }
        } // Quick 2
        if (usesQuick1) {
            const QString quick1ImportPath = qmakeVariables.value(QStringLiteral("QT_INSTALL_IMPORTS"));
            QStringList quick1Imports(QStringLiteral("Qt"));
            if (result.deployedQtLibraries & QtWebKitModule)
                quick1Imports << QStringLiteral("QtWebKit");
            foreach (const QString &quick1Import, quick1Imports) {
                const QString sourceFile = quick1ImportPath + slash + quick1Import;
                if (!updateFile(sourceFile, qmlFileEntryFunction, options.directory, options.updateFileFlags, options.json, errorMessage))
                    return result;
            }
        } // Quick 1
    } // optQuickImports

    if (options.translations
        && !deployTranslations(qmakeVariables.value(QStringLiteral("QT_INSTALL_TRANSLATIONS")),
                               result.deployedQtLibraries, options.directory,
                               options.updateFileFlags, errorMessage)) {
        return result;
    }

    result.success = true;
    return result;
}

static bool deployWebKit2(const QMap<QString, QString> &qmakeVariables,
                          const Options &sourceOptions, QString *errorMessage)
{
    // Copy the web process and its dependencies
    const QString webProcess = webProcessBinary(sourceOptions.platform);
    const QString webProcessSource = qmakeVariables.value(QStringLiteral("QT_INSTALL_LIBEXECS")) +
                                     QLatin1Char('/') + webProcess;
    if (!updateFile(webProcessSource, sourceOptions.directory, sourceOptions.updateFileFlags, sourceOptions.json, errorMessage))
        return false;
    Options options(sourceOptions);
    options.binary = options.directory + QLatin1Char('/') + webProcess;
    options.quickImports = false;
    options.translations = false;
    return deploy(options, qmakeVariables, errorMessage);
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));

    Options options;
    QString errorMessage;
    const QMap<QString, QString> qmakeVariables = queryQMakeAll(&errorMessage);
    const QString xSpec = qmakeVariables.value(QStringLiteral("QMAKE_XSPEC"));
    options.platform = platformFromMkSpec(xSpec);
    if (options.platform == WindowsMinGW || options.platform == Windows)
        options.compilerRunTime = true;

    {   // Command line
        QCommandLineParser parser;
        QString errorMessage;
        const int result = parseArguments(QCoreApplication::arguments(), &parser, &options, &errorMessage);
        if (result & CommandLineParseError)
            std::wcerr << errorMessage << "\n\n";
        if (result & CommandLineParseHelpRequested)
            std::fputs(qPrintable(helpText(parser)), stdout);
        if (result & CommandLineParseError)
            return 1;
        if (result & CommandLineParseHelpRequested)
            return 0;
    }

    if (qmakeVariables.isEmpty() || xSpec.isEmpty() || !qmakeVariables.contains(QStringLiteral("QT_INSTALL_BINS"))) {
        std::wcerr << "Unable to query qmake: " << errorMessage << '\n';
        return 1;
    }

    if (options.platform == UnknownPlatform) {
        std::wcerr << "Unsupported platform " << xSpec << '\n';
        return 1;
    }

    // Create directories
    if (!createDirectory(options.directory, &errorMessage)) {
        std::wcerr << errorMessage << '\n';
        return 1;
    }
    if (!options.libraryDirectory.isEmpty() && options.libraryDirectory != options.directory
        && !createDirectory(options.libraryDirectory, &errorMessage)) {
        std::wcerr << errorMessage << '\n';
        return 1;
    }

    if (optWebKit2)
        options.additionalLibraries |= QtWebKitModule;


    const DeployResult result = deploy(options, qmakeVariables, &errorMessage);
    if (!result) {
        std::wcerr << errorMessage << '\n';
        return 1;
    }

    if ((optWebKit2 != -1)
        && (optWebKit2 == 1
            || ((result.deployedQtLibraries & QtWebKitModule)
                && (result.directlyUsedQtLibraries & QtQuickModule)))) {
        if (optVerboseLevel)
            std::wcout << "Deploying: " << webProcessC << "...\n";
        if (!deployWebKit2(qmakeVariables, options, &errorMessage)) {
            std::wcerr << errorMessage << '\n';
            return 1;
        }
    }

    if (options.json) {
        if (options.list)
            std::fputs(options.json->toList(options.list, options.directory).constData(), stdout);
        else
            std::fputs(options.json->toJson().constData(), stdout);
        delete options.json;
        options.json = 0;
    }

    return 0;
}

QT_END_NAMESPACE
