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
bool optHelp = false;

Platform platform = Windows;

QString optDirectory;

static const char usageC[] =
"Usage: winrtdeployqt build-directory [options]\n\n"
"Copies/updates the dependent Qt libraries and plugins required for\n"
"a WinRT application to the build-directory.\n\n"
"Options: -no-plugins        : Skip plugin deployment\n"
"         -no-libraries      : Skip library deployment\n"
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

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    const QChar slash = QLatin1Char('/');

    if (!parseArguments(QCoreApplication::arguments()) || optHelp) {
        std::printf("\nwinrtdeployqt based on Qt %s\n\n%s", QT_VERSION_STR, usageC);
        return optHelp ? 0 : 1;
    }

    const QString binary = findBinary(optDirectory);
    if (binary.isEmpty()) {
        std::fprintf(stderr, "Unable to find binary in %s.\n", qPrintable(optDirectory));
        return 1;
    }
    QString errorMessage;
    const QString qtBinDir = queryQMake(QStringLiteral("QT_INSTALL_BINS"), &errorMessage);

    if (qtBinDir.isEmpty()) {
        std::fprintf(stderr, "Unable to find Qt bin directory: %s\n", qPrintable(errorMessage));
        return 1;
    }

    platform = queryQMake(QStringLiteral("QMAKE_XSPEC"), &errorMessage).startsWith(QLatin1String("winrt")) ? WinRt : Windows;

    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Qt binaries in %s\n", qPrintable(QDir::toNativeSeparators(qtBinDir)));

    const QStringList dependentLibs = findDependentLibraries(binary, &errorMessage);
    if (dependentLibs.isEmpty()) {
        std::fprintf(stderr, "Unable to find dependent libraries of %s: %s\n",
                     qPrintable(binary), qPrintable(errorMessage));
        return 1;
    }

    // Filter out the Qt libraries. Note that depends.exe finds libs from optDirectory if we
    // are run the 2nd time (updating). We want to check against the Qt bin dir libraries
    QStringList dependentQtLibs;
    const QRegExp filterRegExp(QRegExp(QStringLiteral("Qt5"), Qt::CaseInsensitive, QRegExp::FixedString));
    foreach (const QString &qtLib, dependentLibs.filter(filterRegExp))
        dependentQtLibs.push_back(normalizeFileName(qtBinDir + slash + QFileInfo(qtLib).fileName()));

    // Some checks in QtCore: Debug, ICU
    bool isDebug = false;
    const QStringList qt5Core = dependentQtLibs.filter(QStringLiteral("Qt5Core"), Qt::CaseInsensitive);
    if (!qt5Core.isEmpty()) {
        isDebug = qt5Core.front().contains(QStringLiteral("Qt5Cored.dll"), Qt::CaseInsensitive);
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

    if (dependentQtLibs.isEmpty()) {
            std::fprintf(stderr, "%s does not seem to be a Qt executable\n",
                         qPrintable(QDir::toNativeSeparators(binary)));
        return 1;
    }

    // Find the plugins and check whether ANGLE, D3D are required on the platform plugin.
    QString platformPlugin;
    const QStringList plugins = findQtPlugins(isDebug, platform, &platformPlugin, &errorMessage);
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
    return 0;
}
