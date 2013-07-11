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

#include <cstdio>

enum QtModule {
    QtCoreModule = 0x1,
    QtGuiModule = 0x2,
    QtSqlModule = 0x4,
    QtNetworkModule = 0x8,
    QtMultimediaModule = 0x10,
    QtMultimediaWidgetsModule = 0x20,
    QtOpenGlModule = 0x40,
    QtPrintSupportModule = 0x80,
    QtDeclarativeModule = 0x100,
    QtQmlModule = 0x200,
    QtQuickModule = 0x400,
    QtQuickParticlesModule = 0x800,
    QtScriptModule = 0x1000,
    QtSvgModule = 0x2000,
    QtXmlModule = 0x4000,
    QtXmlPatternsModule = 0x8000,
    QtHelpModule = 0x10000,
    QtSensorsModule = 0x20000,
    QtV8Module = 0x40000,
    QtWidgetsModule = 0x80000,
    QtWebKitModule = 0x100000,
    QtWebKitWidgetsModule = 0x200000
};

struct QtModuleEntry {
    unsigned module;
    const char *option;
    const char *libraryName;
    const char *translation;
};

QtModuleEntry qtModuleEntries[] = {
    { QtCoreModule, "core", "Qt5Core", "qtbase" },
    { QtGuiModule, "gui", "Qt5Gui", "qtbase" },
    { QtSqlModule, "sql", "Qt5Sql", "qtbase" },
    { QtNetworkModule, "network", "Qt5Network", "qtbase" },
    { QtMultimediaModule, "multimedia", "Qt5Multimedia", "qtmultimedia" },
    { QtMultimediaWidgetsModule, "multimediawidgets", "Qt5MultimediaWidgets", "qtmultimedia" },
    { QtOpenGlModule, "opengl", "Qt5OpenGL", 0 },
    { QtPrintSupportModule, "printsupport", "Qt5PrintSupport", 0 },
    { QtDeclarativeModule, "declarative", "Qt5Declarative", "qtquick1" },
    { QtQmlModule, "qml", "Qt5Qml", "qtdeclarative" },
    { QtQuickModule, "quick", "Qt5Quick", "qtdeclarative" },
    { QtQuickParticlesModule, "quickparticles", "Qt5QuickParticles", 0 },
    { QtScriptModule, "script", "Qt5Script", "qtscript" },
    { QtXmlModule, "xml", "Qt5Xml", 0 },
    { QtXmlPatternsModule, "xmlpatterns", "Qt5XmlPatterns", "qtxmlpatterns" },
    { QtHelpModule, "help", "Qt5Help", "qt_help" },
    { QtSensorsModule, "sensors", "Qt5Sensors", 0 },
    { QtSvgModule, "svg", "Qt5Svg", 0 },
    { QtV8Module, "v8", "Qt5V8", 0 },
    { QtWebKitModule, "webkit", "Qt5WebKit", 0 },
    { QtWebKitWidgetsModule, "webkitwidgets", "Qt5WebKitWidgets", 0 },
    { QtWidgetsModule, "widgets", "Qt5Widgets", "qtbase" }
};

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

static const char webProcessC[] = "QtWebProcess";

static inline QString webProcessBinary(Platform p)
{
    const QString webProcess = QLatin1String(webProcessC);
    return p == Windows || p == WinRt ? webProcess + QStringLiteral(".exe") : webProcess;
}

static unsigned qtModuleByOption(const QStringRef &r)
{
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i)
        if (r == QLatin1String(qtModuleEntries[i].option))
            return qtModuleEntries[i].module;
    return 0;
}

static Platform platformFromMkSpec(const QString &xSpec)
{
    if (xSpec == QLatin1String("linux-g++"))
        return Unix;
    if (xSpec.startsWith(QLatin1String("win32-")))
        return Windows;
    if (xSpec.startsWith(QLatin1String("winrt")) || xSpec.startsWith(QLatin1String("winphone")))
        return WinRt;
    return UnknownPlatform;
}

bool optHelp = false;
int optWebKit2 = 0;

struct Options {
    Options() : plugins(true), libraries(true), quickImports(true), translations(true)
              , platform(Windows), additionalLibraries(0), disabledLibraries(0) {}

    bool plugins;
    bool libraries;
    bool quickImports;
    bool translations;
    Platform platform;
    unsigned additionalLibraries;
    unsigned disabledLibraries;
    QString directory;
    QString libraryDirectory;
    QString binary;
};

static QByteArray usage()
{
    QByteArray result = QByteArrayLiteral("Usage: windeployqt file [options]\n\n"
"Copies/updates the dependent Qt libraries and plugins required for\n"
"a Windows/WinRT application to the build-directory.\n\n"
"<file> is an executable file or build directory.\n\n"
"Options:\n"
"         -libdir <path>     : Copy libraries to <path>\n"
"         -no-plugins        : Skip plugin deployment\n"
"         -no-libraries      : Skip library deployment\n"
"         -no-quick-imports  : Skip deployment of Qt Quick imports\n"
"         -no-translations   : Skip deployment of the translations\n"
"         -webkit2           : Deployment of WebKit2 (web process)\n"
"         -no-webkit2        : Skip deployment of WebKit2\n"
"         -h                 : Display help\n"
"         -verbose=<0-3>     : 0 = no output, 1 = progress (default),\n"
"                              2 = normal, 3 = debug\n"
"\nLibraries can be added by passing their name (-xml) or disabled by passing\n"
"the name prepended by -no- (-no-xml).\nAvailable libraries: ");
    result += formatQtModules(0xFFFFFFFF, true);
    return result;
}

static inline bool parseOption(QStringList::ConstIterator &it,
                               const QStringList::ConstIterator &end,
                               Options *options)
{
    const QString &option = *it;
    if (option == QLatin1String("-libdir")) {
        if (++it == end)
            return false;
        options->libraryDirectory = *it;
        return true;
    }
    if (option == QLatin1String("-no-plugins")) {
        options->plugins = false;
        return true;
    }
    if (option == QLatin1String("-no-libraries")) {
       options->libraries = false;
       return true;
    }
    if (option == QLatin1String("-no-quick-imports")) {
       options->quickImports = false;
       return true;
    }
    if (option == QLatin1String("-no-translations")) {
        options->translations = false;
        return true;
    }
    if (option == QLatin1String("-h")) {
        optHelp = true;
        return true;
    }
    if (option == QLatin1String("-h")) {
        optHelp = true;
        return true;
    }
    if (option == QLatin1String("-webkit2")) {
        optWebKit2 = 1;
        return true;
    }
    if (option == QLatin1String("-no-webkit2")) {
        optWebKit2 = -1;
        return true;
    }
    if (option.startsWith(QLatin1String("-verbose"))) {
        const int index = option.indexOf(QLatin1Char('='));
        bool ok = false;
        optVerboseLevel = option.mid(index + 1).toInt(&ok);
        if (!ok) {
            std::fprintf(stderr, "Could not parse verbose level.\n");
            return false;
        }
        return true;
    }
    // Enabled, disabled module
    if (option.startsWith(QLatin1String("-no-"))) {
        if (const unsigned qtModule = qtModuleByOption(option.rightRef(it->size() - 4))) {
            options->disabledLibraries |= qtModule;
            return true;
        }
    }
    if (const unsigned qtModule = qtModuleByOption(option.rightRef(option.size() - 1))) {
        options->additionalLibraries |= qtModule;
        return true;
    }
    std::fprintf(stderr, "Unhandled option '%s'.\n", qPrintable(option));
    return false;
}

// Return binary from folder
static inline QString findBinary(const QString &directory, Platform platform)
{
    QDir dir(QDir::cleanPath(directory));

    const QStringList nameFilters = platform == Windows || platform == WinRt ?
        QStringList(QStringLiteral("*.exe")) : QStringList();
    foreach (const QString &binary, dir.entryList(nameFilters, QDir::Files | QDir::Executable))
        if (!binary.contains(QLatin1String(webProcessC), Qt::CaseInsensitive))
            return dir.filePath(binary);
    return QString();
}

static inline bool parseArguments(const QStringList &arguments, Options *options)
{
    const QStringList::ConstIterator end = arguments.end();
    QStringList::ConstIterator it = arguments.begin();
    for (++it ; it != end ; ++it) {
        if (it->startsWith(QLatin1Char('-'))) {
            if (!parseOption(it, end, options))
                return false;
        } else {
            if (!options->directory.isEmpty())
                return false;
            const QFileInfo fi(QDir::cleanPath(*it));
            if (!fi.exists()) {
                std::fprintf(stderr, "'%s' does not exist.\n", qPrintable(*it));
                return false;
            }
            if (fi.isFile()) {
                options->binary = fi.absoluteFilePath();
                options->directory = fi.absolutePath();
            } else {
                options->binary = findBinary(fi.absoluteFilePath(), options->platform);
                if (options->binary.isEmpty()) {
                    std::fprintf(stderr, "Unable to find binary in %s.\n", qPrintable(*it));
                    return false;
                }
                options->directory = fi.absoluteFilePath();
            } // directory.
        }     // argument
    }         // for
    return !options->binary.isEmpty();
}

// Find the latest D3D compiler DLL in path
static inline QString findD3dCompiler()
{
    for (int i = 46 ; i >= 40 ; --i) {
        const QString dll = findInPath(QStringLiteral("D3Dcompiler_") + QString::number(i) + QStringLiteral(".dll"));
        if (!dll.isEmpty())
            return dll;
    }
    return QString();
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
    explicit DllDirectoryFileEntryFunction(bool debug, const QString &prefix = QStringLiteral("*")) :
        m_nameFilter(QStringList(prefix  + (debug ? QStringLiteral("d.dll") : QStringLiteral(".dll")))),
        m_dllDebug(debug) {}

    QStringList operator()(const QDir &dir) const
    {
        QStringList result;
        QString errorMessage;
        foreach (const QString &dll, m_nameFilter(dir)) {
            const QString dllPath = dir.absoluteFilePath(dll);
            bool debugDll;
            if (readPeExecutable(dllPath, &errorMessage, 0, 0, &debugDll)) {
                if (debugDll == m_dllDebug) {
                    result.push_back(dll);
                }
            } else {
                std::fprintf(stderr, "Warning: Unable to read %s: %s",
                             qPrintable(QDir::toNativeSeparators(dllPath)), qPrintable(errorMessage));
            }
        }
        return result;
    }

private:
    const NameFilterFileEntryFunction m_nameFilter;
    const bool m_dllDebug;
};

// File entry filter function for updateFile() that returns a list of files for
// QML import trees: DLLs (matching debgug) and .qml/,js, etc.
class QmlDirectoryFileEntryFunction {
public:
    explicit QmlDirectoryFileEntryFunction(bool debug)
        : m_qmlNameFilter(QStringList() << QStringLiteral("*.js") << QStringLiteral("qmldir") << QStringLiteral("*.qmltypes") << QStringLiteral("*.png"))
        , m_dllFilter(debug)
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
                case WinRt:
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
            const QStringList plugins = platform == Unix ?
                        NameFilterFileEntryFunction(QStringList(filter + QStringLiteral(".so")))(subDir) :
                        DllDirectoryFileEntryFunction(debug, filter)(subDir);
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
    const size_t qtModulesCount = sizeof(qtModuleEntries)/sizeof(QtModuleEntry);
    for (size_t i = 0; i < qtModulesCount; ++i)
        if (module.contains(QLatin1String(qtModuleEntries[i].libraryName), Qt::CaseInsensitive))
            return qtModuleEntries[i].module;
    return 0;
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
            std::fprintf(stderr, "Creating %s...\n", qPrintable(targetFile));
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
    switch (platform) {
    case Windows:
    case WinRt:
        result += QLatin1String(name);
        if (debug)
            result += QLatin1Char('d');
        result += QStringLiteral(".dll");
        break;
    case Unix:
        result += QStringLiteral("lib");
        result += QLatin1String(name);
        result += QStringLiteral(".so");
        break;
    default:
        break;
    }
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
        std::fprintf(stderr, "Qt binaries in %s\n", qPrintable(QDir::toNativeSeparators(qtBinDir)));

    QStringList dependentQtLibs;
    bool isDebug;
    unsigned wordSize;
    int directDependencyCount;
    if (!findDependentQtLibraries(libraryLocation, options.binary, options.platform, errorMessage, &dependentQtLibs, &wordSize, &isDebug, &directDependencyCount))
        return result;

    std::printf("%s: %ubit, %s executable.\n", qPrintable(QDir::toNativeSeparators(options.binary)),
                wordSize, isDebug ? "debug" : "release");

    if (dependentQtLibs.isEmpty()) {
        *errorMessage = QDir::toNativeSeparators(options.binary) +  QStringLiteral(" does not seem to be a Qt executable.");
        return result;
    }

    // Some Windows-specific checks in QtCore: ICU
    if (options.platform  == Windows || options.platform == WinRt)  {
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
                        std::fprintf(stderr, "Adding ICU version %s\n", qPrintable(icuVersion));
                    icuLibs.push_back(QStringLiteral("icudt") + icuVersion + QStringLiteral(".dll"));
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
        std::fprintf(stderr, "Direct dependencies: %s\nAll dependencies   : %s\nTo be deployed     : %s\n",
                     formatQtModules(result.directlyUsedQtLibraries).constData(),
                     formatQtModules(result.usedQtLibraries).constData(),
                     formatQtModules(result.deployedQtLibraries).constData());
    }

    const QStringList plugins = findQtPlugins(result.deployedQtLibraries, qmakeVariables.value(QStringLiteral("QT_INSTALL_PLUGINS")),
                                              isDebug, options.platform, &platformPlugin);
    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Plugins: %s\n", qPrintable(plugins.join(QLatin1Char(','))));

    if (plugins.isEmpty())
        return result;

    if (platformPlugin.isEmpty()) {
        *errorMessage =QStringLiteral("Unable to find the platform plugin.");
        return result;
    }

    // Check for ANGLE on the platform plugin.
    if (options.platform  == Windows || options.platform == WinRt)  {
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
            const QString d3dCompiler = findD3dCompiler();
            if (d3dCompiler.isEmpty()) {
                std::fprintf(stderr, "Warning: Cannot find any version of the d3dcompiler DLL.\n");
            } else {
                deployedQtLibraries.push_back(d3dCompiler);
            }
        } // !libEgl.isEmpty()
    } // Windows

    // Update libraries
    if (options.libraries) {
        QString targetPath = options.directory;
        if (!options.libraryDirectory.isEmpty()) {
            if (!createDirectory(options.libraryDirectory, errorMessage))
                return result;
            targetPath = options.libraryDirectory;
        }
        foreach (const QString &qtLib, deployedQtLibraries) {
            if (!updateFile(qtLib, targetPath, errorMessage))
                return result;
        }
    } // optLibraries

    // Update plugins
    if (options.plugins) {
        QDir dir(options.directory);
        foreach (const QString &plugin, plugins) {
            const QString targetDirName = plugin.section(slash, -2, -2);
            if (!dir.exists(targetDirName)) {
                std::printf("Creating directory %s.\n", qPrintable(targetDirName));
                if (!dir.mkdir(targetDirName)) {
                    std::fprintf(stderr, "Cannot create %s.\n",  qPrintable(targetDirName));
                    *errorMessage = QStringLiteral("Cannot create ") + targetDirName +  QLatin1Char('.');
                    return result;
                }
            }
            if (!updateFile(plugin, options.directory + slash + targetDirName, errorMessage))
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
        const QmlDirectoryFileEntryFunction qmlFileEntryFunction(isDebug);
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
                if (!updateFile(quick2ImportPath + slash + quick2Import, qmlFileEntryFunction, options.directory, errorMessage))
                    return result;
            }
        } // Quick 2
        if (usesQuick1) {
            const QString quick1ImportPath = qmakeVariables.value(QStringLiteral("QT_INSTALL_IMPORTS"));
            QStringList quick1Imports(QStringLiteral("Qt"));
            if (result.deployedQtLibraries & QtWebKitModule)
                quick1Imports << QStringLiteral("QtWebKit");
            foreach (const QString &quick1Import, quick1Imports) {
                if (!updateFile(quick1ImportPath + slash + quick1Import, qmlFileEntryFunction, options.directory, errorMessage))
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
    if (!updateFile(webProcessSource, sourceOptions.directory, errorMessage))
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


    Options options;
    QString errorMessage;
    const QMap<QString, QString> qmakeVariables = queryQMakeAll(&errorMessage);
    const QString xSpec = qmakeVariables.value(QStringLiteral("QMAKE_XSPEC"));
    options.platform = platformFromMkSpec(xSpec);

    if (!parseArguments(QCoreApplication::arguments(), &options) || optHelp) {
        std::printf("\nwindeployqt based on Qt %s\n\n%s", QT_VERSION_STR, usage().constData());
        return optHelp ? 0 : 1;
    }

    if (qmakeVariables.isEmpty() || xSpec.isEmpty() || !qmakeVariables.contains(QStringLiteral("QT_INSTALL_BINS"))) {
        std::fprintf(stderr, "Unable to query qmake: %s\n", qPrintable(errorMessage));
        return 1;
    }

    if (options.platform == UnknownPlatform) {
        std::fprintf(stderr, "Unsupported platform %s\n", qPrintable(xSpec));
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
            std::fprintf(stderr, "Deploying: %s...\n", webProcessC);
        if (!deployWebKit2(qmakeVariables, options, &errorMessage)) {
            std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
            return 1;
        }
    }

    return 0;
}
