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

bool optPlugins = true;
bool optLibraries = true;
bool optQuickImports = true;
bool optHelp = false;

Platform platform = Windows;

QString optDirectory;

static const char usageC[] =
"Usage: windeployqt build-directory [options]\n\n"
"Copies/updates the dependent Qt libraries and plugins required for\n"
"a Windows/WinRT application to the build-directory.\n\n"
"Options: -no-plugins        : Skip plugin deployment\n"
"         -no-libraries      : Skip library deployment\n"
"         -no-quick-imports  : Skip deployment of Qt Quick imports\n"
"         -h                 : Display help\n"
"         -verbose=<0-3>     : 0 = no output, 1 = progress (default),\n"
"                              2 = normal, 3 = debug\n";

static inline bool parseArguments(const QStringList &arguments)
{
    for (int i = 1; i < arguments.size(); ++i) {
        const QString argument = arguments.at(i);
        if (argument == QLatin1String("-no-plugins")) {
            optPlugins = false;
        } else if (argument == QLatin1String("-no-libraries")) {
           optLibraries = false;
        } else if (argument == QLatin1String("-no-quick-imports")) {
           optQuickImports = false;
        } else if (argument.startsWith(QLatin1String("-h"))) {
            optHelp = true;
        } else if (argument.startsWith(QLatin1String("-verbose"))) {
            const int index = argument.indexOf(QLatin1Char('='));
            bool ok = false;
            optVerboseLevel = argument.mid(index + 1).toInt(&ok);
            if (!ok) {
                std::fprintf(stderr, "Could not parse verbose level.\n");
                return false;
            }
        } else {
            if (!optDirectory.isEmpty())
                return false;
            optDirectory = QDir::cleanPath(argument);
            if (optDirectory.endsWith(QLatin1Char('/')))
                optDirectory.chop(1);
        }
    }
    return !optDirectory.isEmpty();
}

// Return binary from folder
static inline QString findBinary(const QString &directory)
{
    QDir dir(QDir::cleanPath(directory));
    const QStringList exes = dir.entryList(QStringList(QLatin1String("*.exe")));
    return exes.isEmpty() ? QString() : dir.filePath(exes.front());
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
static bool findDependentQtLibraries(const QString &qtBinDir, const QString &binary,
                                     QString *errorMessage, QStringList *result,
                                     unsigned *wordSize = 0, bool *isDebug = 0)
{
    QStringList dependentLibs;
    if (!readPeExecutable(binary, errorMessage, &dependentLibs, wordSize, isDebug)) {
        errorMessage->prepend(QLatin1String("Unable to find dependent libraries of ") +
                              QDir::toNativeSeparators(binary) + QLatin1String(" :"));
        return false;
    }
    // Filter out the Qt libraries. Note that depends.exe finds libs from optDirectory if we
    // are run the 2nd time (updating). We want to check against the Qt bin dir libraries
    const QRegExp filterRegExp(QStringLiteral("Qt5"), Qt::CaseInsensitive, QRegExp::FixedString);
    foreach (const QString &qtLib, dependentLibs.filter(filterRegExp)) {
        const QString path = normalizeFileName(qtBinDir + QLatin1Char('/') + QFileInfo(qtLib).fileName());
        if (!result->contains(path)) {
            result->append(path);
            if (!findDependentQtLibraries(qtBinDir, path, errorMessage, result))
                return false;
        }
    }
    return true;
}

// Base class to filter debug/release DLLs for functions to be passed to updateFile().
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
        : m_qmlNameFilter(QStringList() << QStringLiteral("*.js") << QStringLiteral("qmldir") << QStringLiteral("*.qmltypes"))
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
        return GuiModule;
    }
    if (subDirName == QLatin1String("bearer"))
        return NetworkModule;
    if (subDirName == QLatin1String("sqldrivers"))
        return SqlModule;
    if (subDirName == QLatin1String("mediaservice") || subDirName == QLatin1String("playlistformats"))
        return MultimediaModule;
    if (subDirName == QLatin1String("printsupport"))
        return PrintSupportModule;
    if (subDirName == QLatin1String("qmltooling"))
        return Quick1Module | Quick2Module;
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
                filter = platform == WinRt ? QStringLiteral("qwinrt") : QStringLiteral("qwindows");
            } else {
                filter  = QLatin1String("*");
            }
            foreach (const QString &plugin, DllDirectoryFileEntryFunction(debug, filter)(subDir)) {
                const QString pluginPath = subDir.absoluteFilePath(plugin);
                if (isPlatformPlugin)
                    *platformPlugin = pluginPath;
                result.push_back(pluginPath);
            } // for filter
        } // type matches
    } // for plugin folder
    return result;
}

static unsigned qtModules(const QStringList &qtLibraries)
{
    unsigned result = 0;
    if (!qtLibraries.filter(QStringLiteral("Qt5Gui"), Qt::CaseInsensitive).isEmpty())
        result |= GuiModule;
    if (!qtLibraries.filter(QStringLiteral("Qt5Sql"), Qt::CaseInsensitive).isEmpty())
        result |= SqlModule;
    if (!qtLibraries.filter(QStringLiteral("Qt5Network"), Qt::CaseInsensitive).isEmpty())
        result |= NetworkModule;
    if (!qtLibraries.filter(QStringLiteral("Qt5PrintSupport"), Qt::CaseInsensitive).isEmpty())
        result |= PrintSupportModule;
    if (!qtLibraries.filter(QStringLiteral("Qt5Multimedia"), Qt::CaseInsensitive).isEmpty())
        result |= MultimediaModule;
    if (!qtLibraries.filter(QStringLiteral("Qt5Sensors"), Qt::CaseInsensitive).isEmpty())
        result |= SensorsModule;
    if (!qtLibraries.filter(QStringLiteral("Qt5Quick"), Qt::CaseInsensitive).isEmpty())
        result |= Quick2Module;
    if (!qtLibraries.filter(QStringLiteral("Qt5Declarative"), Qt::CaseInsensitive).isEmpty())
        result |= Quick1Module;
    if (!qtLibraries.filter(QStringLiteral("Qt5WebKit"), Qt::CaseInsensitive).isEmpty())
        result |= WebKitModule;
    return result;
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    const QChar slash = QLatin1Char('/');

    if (!parseArguments(QCoreApplication::arguments()) || optHelp) {
        std::printf("\nwindeployqt based on Qt %s\n\n%s", QT_VERSION_STR, usageC);
        return optHelp ? 0 : 1;
    }

    const QString binary = findBinary(optDirectory);
    if (binary.isEmpty()) {
        std::fprintf(stderr, "Unable to find binary in %s.\n", qPrintable(optDirectory));
        return 1;
    }
    QString errorMessage;

    const QMap<QString, QString> qmakeVariables = queryQMakeAll(&errorMessage);
    const QString qtBinDir = qmakeVariables.value(QStringLiteral("QT_INSTALL_BINS"));
    if (qmakeVariables.isEmpty() || qtBinDir.isEmpty()) {
        std::fprintf(stderr, "Unable to query qmake: %s\n", qPrintable(errorMessage));
        return 1;
    }

    const QString xSpec = qmakeVariables.value(QStringLiteral("QMAKE_XSPEC"));
    if (xSpec.startsWith(QLatin1String("winrt")) || xSpec.startsWith(QLatin1String("winphone")))
        platform = WinRt;
    else
        platform = Windows;

    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Qt binaries in %s\n", qPrintable(QDir::toNativeSeparators(qtBinDir)));

    QStringList dependentQtLibs;
    bool isDebug;
    unsigned wordSize;
    if (!findDependentQtLibraries(qtBinDir, binary, &errorMessage, &dependentQtLibs, &wordSize, &isDebug)) {
        std::fputs(qPrintable(errorMessage), stderr);
        return 1;
    }

    std::printf("%s: %ubit, %s executable.\n", qPrintable(QDir::toNativeSeparators(binary)),
                wordSize, isDebug ? "debug" : "release");

    if (dependentQtLibs.isEmpty()) {
            std::fprintf(stderr, "%s does not seem to be a Qt executable\n",
                         qPrintable(QDir::toNativeSeparators(binary)));
        return 1;
    }

    // Some checks in QtCore: ICU
    const QStringList qt5Core = dependentQtLibs.filter(QStringLiteral("Qt5Core"), Qt::CaseInsensitive);
    if (!qt5Core.isEmpty()) {
        QStringList icuLibs = findDependentLibraries(qt5Core.front(), &errorMessage).filter(QStringLiteral("ICU"), Qt::CaseInsensitive);
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
                    std::fprintf(stderr, "Unable to locate ICU library %s\n", qPrintable(icuLib));
                    return -1;
                }
                dependentQtLibs.push_back(icuPath);
            } // foreach icuLib
        } // !icuLibs.isEmpty()
    } // Qt5Core

    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Qt libraries required: %s\n", qPrintable(dependentQtLibs.join(QLatin1Char(','))));


    // Find the plugins and check whether ANGLE, D3D are required on the platform plugin.
    QString platformPlugin;
    const unsigned usedQtModules = qtModules(dependentQtLibs);
    const QStringList plugins = findQtPlugins(usedQtModules, qmakeVariables.value(QStringLiteral("QT_INSTALL_PLUGINS")),
                                              isDebug, platform, &platformPlugin);
    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Plugins: %s\n", qPrintable(plugins.join(QLatin1Char(','))));

    if (plugins.isEmpty()) {
        std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
        return 1;
    }

    if (platformPlugin.isEmpty()) {
        std::fprintf(stderr, "Unable to find the platform plugin.\n");
        return 1;
    }

    // Check for ANGLE on the platform plugin.
    const QStringList platformPluginLibraries = findDependentLibraries(platformPlugin, &errorMessage);
    const QStringList libEgl = platformPluginLibraries.filter(QStringLiteral("libegl"), Qt::CaseInsensitive);
    if (!libEgl.isEmpty()) {
        const QString libEglFullPath = qtBinDir + slash + QFileInfo(libEgl.front()).fileName();
        dependentQtLibs.push_back(libEglFullPath);
        const QStringList libGLESv2 = findDependentLibraries(libEglFullPath, &errorMessage).filter(QStringLiteral("libGLESv2"), Qt::CaseInsensitive);
        if (!libGLESv2.isEmpty()) {
            const QString libGLESv2FullPath = qtBinDir + slash + QFileInfo(libGLESv2.front()).fileName();
            dependentQtLibs.push_back(libGLESv2FullPath);
        }
        // Find the D3d Compiler matching the D3D library.
        const QString d3dCompiler = findD3dCompiler();
        if (d3dCompiler.isEmpty()) {
            std::fprintf(stderr, "Warning: Cannot find any version of the d3dcompiler DLL.\n");
        } else {
            dependentQtLibs.push_back(d3dCompiler);
        }
    } // !libEgl.isEmpty()

    // Update libraries
    if (optLibraries) {
        foreach (const QString &qtLib, dependentQtLibs) {
            if (!updateFile(qtLib, optDirectory, &errorMessage)) {
                std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
                return 1;
            }
        }
    } // optLibraries

    // Update plugins
    if (optPlugins) {
        QDir dir(optDirectory);
        foreach (const QString &plugin, plugins) {
            const QString targetDirName = plugin.section(slash, -2, -2);
            if (!dir.exists(targetDirName)) {
                std::printf("Creating directory %s.\n", qPrintable(targetDirName));
                if (!dir.mkdir(targetDirName)) {
                    std::fprintf(stderr, "Cannot create %s.\n",  qPrintable(targetDirName));
                    return -1;
                }
            }
            if (!updateFile(plugin, optDirectory + slash + targetDirName, &errorMessage)) {
                std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
                return 1;
            }
        }
    } // optPlugins

    // Update Quick imports
    if (optQuickImports && (usedQtModules & (Quick1Module | Quick2Module))) {
        const QmlDirectoryFileEntryFunction qmlFileEntryFunction(isDebug);
        if (usedQtModules & Quick2Module) {
            const QString quick2ImportPath = qmakeVariables.value(QStringLiteral("QT_INSTALL_QML"));
            QStringList quick2Imports;
            quick2Imports << QStringLiteral("QtQml") << QStringLiteral("QtQuick") << QStringLiteral("QtQuick.2");
            if (usedQtModules & MultimediaModule)
                quick2Imports << QStringLiteral("QtMultimedia");
            if (usedQtModules & SensorsModule)
                quick2Imports << QStringLiteral("QtSensors");
            if (usedQtModules & WebKitModule)
                quick2Imports << QStringLiteral("QtWebKit");
            foreach (const QString &quick2Import, quick2Imports) {
                if (!updateFile(quick2ImportPath + slash + quick2Import, qmlFileEntryFunction, optDirectory, &errorMessage)) {
                    std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
                    return 1;
                }
            }
        } // Quick 2
        if (usedQtModules & Quick1Module) {
            const QString quick1ImportPath = qmakeVariables.value(QStringLiteral("QT_INSTALL_IMPORTS"));
            QStringList quick1Imports(QStringLiteral("Qt"));
            if (usedQtModules & WebKitModule)
                quick1Imports << QStringLiteral("QtWebKit");
            foreach (const QString &quick1Import, quick1Imports) {
                if (!updateFile(quick1ImportPath + slash + quick1Import, qmlFileEntryFunction, optDirectory, &errorMessage)) {
                    std::fprintf(stderr, "%s\n", qPrintable(errorMessage));
                    return 1;
                }
            }
        } // Quick 1
    } // optQuickImports

    return 0;
}
