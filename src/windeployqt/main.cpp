/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QVector>

#include <cstdio>

QT_BEGIN_NAMESPACE

enum QtModule
#ifdef Q_COMPILER_CLASS_ENUM
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
    QtHelpModule = 0x100,
    QtMultimediaModule = 0x200,
    QtMultimediaWidgetsModule = 0x400,
    QtNetworkModule = 0x800,
    QtNfcModule = 0x1000,
    QtOpenGLModule = 0x2000,
    QtPositioningModule = 0x4000,
    QtPrintSupportModule = 0x8000,
    QtQmlModule = 0x10000,
    QtQuickModule = 0x20000,
    QtQuickParticlesModule = 0x40000,
    QtScriptModule = 0x80000,
    QtScriptToolsModule = 0x100000,
    QtSensorsModule = 0x200000,
    QtSerialPortModule = 0x400000,
    QtSqlModule = 0x800000,
    QtSvgModule = 0x1000000,
    QtTestModule = 0x2000000,
    QtWidgetsModule = 0x4000000,
    QtWinExtrasModule = 0x8000000,
    QtXmlModule = 0x10000000,
    QtXmlPatternsModule = 0x20000000,
    QtWebKitModule = 0x40000000,
    QtWebKitWidgetsModule = 0x80000000
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
    { QtHelpModule, "help", "Qt5Help", "qt_help" },
    { QtMultimediaModule, "multimedia", "Qt5Multimedia", "qtmultimedia" },
    { QtMultimediaWidgetsModule, "multimediawidgets", "Qt5MultimediaWidgets", "qtmultimedia" },
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
        return Windows;
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
    Options() : plugins(true), libraries(true), quickImports(true), translations(true)
              , platform(Windows), additionalLibraries(0), disabledLibraries(0)
              , updateFileFlags(0), json(0) {}

    bool plugins;
    bool libraries;
    bool quickImports;
    bool translations;
    Platform platform;
    unsigned additionalLibraries;
    unsigned disabledLibraries;
    unsigned updateFileFlags;
    QString directory;
    QString libraryDirectory;
    QString binary;
    JsonOutput *json;
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
    parser->setApplicationDescription(QStringLiteral("Qt Deploy Tool ") + QLatin1String(QT_VERSION_STR));
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

    QCommandLineOption forceOption(QStringLiteral("force"),
                                    QStringLiteral("Force updating files."));
    parser->addOption(forceOption);

    QCommandLineOption noPluginsOption(QStringLiteral("no-plugins"),
                                       QStringLiteral("Skip plugin deployment."));
    parser->addOption(noPluginsOption);

    QCommandLineOption noLibraryOption(QStringLiteral("no-libraries"),
                                       QStringLiteral("Skip library deployment."));
    parser->addOption(noLibraryOption);

    QCommandLineOption noQuickImportOption(QStringLiteral("no-quick-import"),
                                           QStringLiteral("Skip deployment of Qt Quick imports."));
    parser->addOption(noQuickImportOption);

    QCommandLineOption noTranslationOption(QStringLiteral("no-translations"),
                                           QStringLiteral("Skip deployment of translations."));
    parser->addOption(noTranslationOption);

    QCommandLineOption webKitOption(QStringLiteral("webkit2"),
                                    QStringLiteral("Deployment of WebKit2 (web process)."));
    parser->addOption(webKitOption);

    QCommandLineOption noWebKitOption(QStringLiteral("no-webkit2"),
                                      QStringLiteral("Skip deployment of WebKit2."));
    parser->addOption(noWebKitOption);

    QCommandLineOption jsonOption(QStringLiteral("json"),
                                  QStringLiteral("Print to stdout in JSON format."));
    parser->addOption(jsonOption);

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
    options->quickImports = !parser->isSet(noQuickImportOption);
    if (parser->isSet(forceOption))
        options->updateFileFlags |= ForceUpdateFile;

    for (size_t i = 0; i < qtModulesCount; ++i) {
        if (parser->isSet(*enabledModules.at(int(i)).first.data()))
            options->additionalLibraries |= enabledModules.at(int(i)).second;
        if (parser->isSet(*disabledModules.at(int(i)).first.data()))
            options->disabledLibraries |= disabledModules.at(int(i)).second;
    }

    if (parser->isSet(jsonOption)) {
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
    const int moduleStart = result.indexOf(QLatin1String("\n  --core"));
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
    explicit QmlDirectoryFileEntryFunction(Platform platform, bool debug)
        : m_qmlNameFilter(QStringList() << QStringLiteral("*.js") << QStringLiteral("qmldir") << QStringLiteral("*.qmltypes") << QStringLiteral("*.png"))
        , m_dllFilter(platform, debug)
    {}

    QStringList operator()(const QDir &dir) const { return m_dllFilter(dir) + m_qmlNameFilter(dir);  }

private:
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
    if (subDirName == QLatin1String("mediaservice") || subDirName == QLatin1String("playlistformats"))
        return QtMultimediaModule;
    if (subDirName == QLatin1String("printsupport"))
        return QtPrintSupportModule;
    if (subDirName == QLatin1String("qmltooling"))
        return QtDeclarativeModule | QtQuickModule;
    return 0; // "designer"
}

QStringList findQtPlugins(unsigned usedQtModules,
                          const QString qtPluginsDirName,
                          bool debug, Platform platform,
                          QString *platformPlugin)
{
    if (qtPluginsDirName.isEmpty())
        return QStringList();
    QDir pluginsDir(qtPluginsDirName);
    QStringList result;
    foreach (const QString &subDirName, pluginsDir.entryList(QStringList(QLatin1String("*")), QDir::Dirs | QDir::NoDotAndDotDot)) {
        const unsigned module = qtModuleForPlugin(subDirName);
        if (module & usedQtModules) {
            const QString subDirPath = qtPluginsDirName + QLatin1Char('/') + subDirName;
            QDir subDir(subDirPath);
            // Filter for platform or any.
            QString filter;
            const bool isPlatformPlugin = subDirName == QLatin1String("platforms");
            if (isPlatformPlugin) {
                switch (platform) {
                case Windows:
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
                result.push_back(pluginPath);
            } // for filter
        } // type matches
    } // for plugin folder
    return result;
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
                               const QString &target, QString *errorMessage)
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
        fprintf(stderr, "Warning: Could not find any translations in %s (developer build?)\n.",
                qPrintable(QDir::toNativeSeparators(sourcePath)));
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
            std::printf("Creating %s...\n", qPrintable(targetFile));
        unsigned long exitCode;
        if (!runProcess(binary, arguments, sourcePath, &exitCode, 0, 0, errorMessage) || exitCode)
            return false;
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

static QString libraryPath(const QString &libraryLocation, const char *name, Platform platform, bool debug)
{
    QString result = libraryLocation + QLatin1Char('/');
    if (platform & WindowsBased) {
        result += QLatin1String(name);
        if (debug)
            result += QLatin1Char('d');
    } else if (platform & UnixBased) {
        result += QStringLiteral("lib");
        result += QLatin1String(name);
    }
    result += sharedLibrarySuffix(platform);
    return result;
}

static DeployResult deploy(const Options &options,
                           const QMap<QString, QString> &qmakeVariables,
                           QString *errorMessage)
{
    DeployResult result;

    const QChar slash = QLatin1Char('/');

    const QString qtBinDir = qmakeVariables.value(QStringLiteral("QT_INSTALL_BINS"));
    const QString libraryLocation = options.platform == Unix ? qmakeVariables.value(QStringLiteral("QT_INSTALL_LIBS")) : qtBinDir;

    if (optVerboseLevel > 1)
        std::printf("Qt binaries in %s\n", qPrintable(QDir::toNativeSeparators(qtBinDir)));

    QStringList dependentQtLibs;
    bool isDebug;
    unsigned wordSize;
    int directDependencyCount;
    if (!findDependentQtLibraries(libraryLocation, options.binary, options.platform, errorMessage, &dependentQtLibs, &wordSize, &isDebug, &directDependencyCount))
        return result;

    if (optVerboseLevel) {
        std::printf("%s: %ubit, %s executable.\n", qPrintable(QDir::toNativeSeparators(options.binary)),
                    wordSize, isDebug ? "debug" : "release");
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
                        std::printf("Adding ICU version %s\n", qPrintable(icuVersion));
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

    // Find the plugins and check whether ANGLE, D3D are required on the platform plugin.
    QString platformPlugin;
    // Sort apart Qt 5 libraries in the ones that are represented by the
    // QtModule enumeration (and thus controlled by flags) and others.
    QStringList deployedQtLibraries;
    for (int i = 0 ; i < dependentQtLibs.size(); ++i)  {
        if (const unsigned qtm = qtModule(dependentQtLibs.at(i))) {
            result.usedQtLibraries |= qtm;
            if (i < directDependencyCount)
                result.directlyUsedQtLibraries |= qtm;
        } else {
            deployedQtLibraries.push_back(dependentQtLibs.at(i)); // Not represented by flag.
        }
    }
    result.deployedQtLibraries = (result.usedQtLibraries | options.additionalLibraries) & ~options.disabledLibraries;
    // Apply options flags and re-add library names.
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i)
        if (result.deployedQtLibraries & qtModuleEntries[i].module)
            deployedQtLibraries.push_back(libraryPath(libraryLocation, qtModuleEntries[i].libraryName, options.platform, isDebug));

    if (optVerboseLevel >= 1) {
        std::printf("Direct dependencies: %s\nAll dependencies   : %s\nTo be deployed     : %s\n",
                    formatQtModules(result.directlyUsedQtLibraries).constData(),
                    formatQtModules(result.usedQtLibraries).constData(),
                    formatQtModules(result.deployedQtLibraries).constData());
    }

    const QStringList plugins = findQtPlugins(result.deployedQtLibraries, qmakeVariables.value(QStringLiteral("QT_INSTALL_PLUGINS")),
                                              isDebug, options.platform, &platformPlugin);
    if (optVerboseLevel > 1)
        std::printf("Plugins: %s\n", qPrintable(plugins.join(QLatin1Char(','))));

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
            // Find the D3d Compiler matching the D3D library.
            const QString d3dCompiler = findD3dCompiler(options.platform, wordSize);
            if (d3dCompiler.isEmpty()) {
                std::fprintf(stderr, "Warning: Cannot find any version of the d3dcompiler DLL.\n");
            } else {
                deployedQtLibraries.push_back(d3dCompiler);
            }
        } // !libEgl.isEmpty()
    } // Windows

    // Update libraries
    if (options.libraries) {
        const QString targetPath = options.libraryDirectory.isEmpty() ?
            options.directory : options.libraryDirectory;
        foreach (const QString &qtLib, deployedQtLibraries) {
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
                    std::printf("Creating directory %s.\n", qPrintable(targetDirName));
                if (!dir.mkdir(targetDirName)) {
                    std::fprintf(stderr, "Cannot create %s.\n",  qPrintable(targetDirName));
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
    const bool usesQuick2 = (result.directlyUsedQtLibraries & QtQuickModule)
                            || (options.additionalLibraries & QtQuickModule);
    if (options.quickImports && (usesQuick1 || usesQuick2)) {
        const QmlDirectoryFileEntryFunction qmlFileEntryFunction(options.platform, isDebug);
        if (usesQuick2) {
            const QString quick2ImportPath = qmakeVariables.value(QStringLiteral("QT_INSTALL_QML"));
            QStringList quick2Imports;
            quick2Imports << QStringLiteral("QtQml") << QStringLiteral("QtQuick") << QStringLiteral("QtQuick.2");
            if (result.deployedQtLibraries & QtMultimediaModule)
                quick2Imports << QStringLiteral("QtMultimedia");
            if (result.deployedQtLibraries & QtSensorsModule)
                quick2Imports << QStringLiteral("QtSensors");
            if (result.deployedQtLibraries & QtWebKitModule)
                quick2Imports << QStringLiteral("QtWebKit");
            foreach (const QString &quick2Import, quick2Imports) {
                const QString sourceFile = quick2ImportPath + slash + quick2Import;
                if (!updateFile(sourceFile, qmlFileEntryFunction, options.directory, options.updateFileFlags, options.json, errorMessage))
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
                               result.deployedQtLibraries, options.directory, errorMessage)) {
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

    {   // Command line
        QCommandLineParser parser;
        QString errorMessage;
        const int result = parseArguments(QCoreApplication::arguments(), &parser, &options, &errorMessage);
        if (result & CommandLineParseError)
            std::fprintf(stderr, "%s\n\n", qPrintable(errorMessage));
        if (result & CommandLineParseHelpRequested)
            std::fputs(qPrintable(helpText(parser)), stdout);
        if (result & CommandLineParseError)
            return 1;
        if (result & CommandLineParseHelpRequested)
            return 0;
    }

    if (qmakeVariables.isEmpty() || xSpec.isEmpty() || !qmakeVariables.contains(QStringLiteral("QT_INSTALL_BINS"))) {
        std::fprintf(stderr, "Unable to query qmake: %s\n", qPrintable(errorMessage));
        return 1;
    }

    if (options.platform == UnknownPlatform) {
        std::fprintf(stderr, "Unsupported platform %s\n", qPrintable(xSpec));
        return 1;
    }

    // Create directories
    if (!createDirectory(options.directory, &errorMessage)) {
        std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
        return 1;
    }
    if (!options.libraryDirectory.isEmpty() && options.libraryDirectory != options.directory
        && !createDirectory(options.libraryDirectory, &errorMessage)) {
        std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
        return 1;
    }

    if (optWebKit2)
        options.additionalLibraries |= QtWebKitModule;


    const DeployResult result = deploy(options, qmakeVariables, &errorMessage);
    if (!result) {
        std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
        return 1;
    }

    if ((optWebKit2 != -1)
        && (optWebKit2 == 1
            || ((result.deployedQtLibraries & QtWebKitModule)
                && (result.directlyUsedQtLibraries & QtQuickModule)))) {
        if (optVerboseLevel)
            std::printf("Deploying: %s...\n", webProcessC);
        if (!deployWebKit2(qmakeVariables, options, &errorMessage)) {
            std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
            return 1;
        }
    }

    if (options.json) {
        std::fputs(options.json->toJson().constData(), stdout);
        delete options.json;
        options.json = 0;
    }

    return 0;
}

QT_END_NAMESPACE
