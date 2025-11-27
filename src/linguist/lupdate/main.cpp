// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Marc Mutz <marc.mutz@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <linguistproject/profileutils.h>
#include <linguistproject/projectdescriptionreader.h>
#include <linguistproject/projsongenerator.h>
#include <linguistproject/projectprocessor.h>
#include <trlib/trparser.h>
#include <translator.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>

#include <iostream>

using namespace Qt::StringLiterals;

static QString m_defaultExtensions;

static void printOut(const QString & out)
{
    std::cout << qPrintable(out);
}

static void printErr(const QString & out)
{
    std::cerr << qPrintable(out);
}

static void printWarning(UpdateOptions options,
                         const QString &msg,
                         const QString &warningMsg = {},
                         const QString &errorMsg = {})
{
    QString text = msg;
    if (options & Werror) {
        text.prepend("lupdate error: "_L1);
        if (!errorMsg.isEmpty())
            text.append(" "_L1).append(errorMsg);
    } else {
        text.prepend("lupdate warning: "_L1);
        if (!warningMsg.isEmpty())
            text.append(" "_L1).append(warningMsg);
    }

    printErr(text);
}

static void recursiveFileInfoList(const QDir &dir,
    const QSet<QString> &nameFilters, QDir::Filters filter,
    QFileInfoList *fileinfolist)
{
    for (const QFileInfo &fi : dir.entryInfoList(filter))
        if (fi.isDir())
            recursiveFileInfoList(QDir(fi.absoluteFilePath()), nameFilters, filter, fileinfolist);
        else if (nameFilters.contains(fi.suffix()))
            fileinfolist->append(fi);
}

static void printUsage()
{
    printOut(
            "Usage:\n"
            "    lupdate [options] [project-file]...\n"
            "    lupdate [options] [source-file|path|@lst-file]... -ts ts-files|@lst-file\n\n"
            "lupdate is part of Qt's Linguist tool chain. It extracts translatable\n"
            "messages from Qt UI files, C++, Java and JavaScript/QtScript source code.\n"
            "Extracted messages are stored in textual translation source files (typically\n"
            "Qt TS XML). New and modified messages can be merged into existing TS files.\n\n"
            "Passing .pro files to lupdate is deprecated.\n"
            "Please use the lupdate-pro tool instead.\n\n"
            "Options:\n"
            "    -help  Display this information and exit.\n"
            "    -no-obsolete\n"
            "           Drop all obsolete and vanished strings.\n"
            "    -extensions <ext>[,<ext>]...\n"
            "           Process files with the given extensions only.\n"
            "           The extension list must be separated with commas, not with whitespace.\n"
            "           Default: '%1'.\n"
            "    -pluralonly\n"
            "           Only include plural form messages.\n"
            "    -silent\n"
            "           Do not explain what is being done.\n"
            "    -no-sort\n"
            "           Do not sort contexts in TS files.\n"
            "    -sort-messages\n"
            "           Sort messages in a context alphabetically in TS files.\n"
            "    -no-recursive\n"
            "           Do not recursively scan directories.\n"
            "    -recursive\n"
            "           Recursively scan directories (default).\n"
            "    -warnings-are-errors\n"
            "           Treat warnings as errors.\n"
            "    -I <includepath> or -I<includepath>\n"
            "           Additional location to look for include files.\n"
            "           May be specified multiple times.\n"
            "    -locations {absolute|relative|none}\n"
            "           Specify/override how source code references are saved in TS files.\n"
            "           absolute: Source file path is relative to target file. Absolute line\n"
            "                     number is stored.\n"
            "           relative: Source file path is relative to target file. Line number is\n"
            "                     relative to other entries in the same source file.\n"
            "           none: no information about source location is stored.\n"
            "           Guessed from existing TS files if not specified.\n"
            "           Default is absolute for new files.\n"
            "    -no-ui-lines\n"
            "           Do not record line numbers in references to UI files.\n"
            "    -disable-heuristic {sametext|similartext}\n"
            "           Disable the named merge heuristic. Can be specified multiple times.\n"
            "    -project <filename>\n"
            "           Name of a file containing the project's description in JSON format.\n"
            "           Such a file may be generated from a .pro file using lupdate-pro -dump-json.\n"
            "    -pro <filename>\n"
            "           Name of a .pro file. Useful for files with .pro file syntax but\n"
            "           different file suffix. Projects are recursed into and merged.\n"
            "           This option is deprecated. Use the lupdate-pro tool instead.\n"
            "    -pro-out <directory>\n"
            "           Virtual output directory for processing subsequent .pro files.\n"
            "    -pro-debug\n"
            "           Trace processing .pro files. Specify twice for more verbosity.\n"
            "    -source-language <language>[_<region>]\n"
            "           Specify the language of the source strings for new files.\n"
            "           Defaults to POSIX if not specified.\n"
            "    -target-language <language>[_<region>]\n"
            "           Specify the language of the translations for new files.\n"
            "           Guessed from the file name if not specified.\n"
            "    -tr-function-alias <function>{+=,=}<alias>[,<function>{+=,=}<alias>]...\n"
            "           With +=, recognize <alias> as an alternative spelling of <function>.\n"
            "           With  =, recognize <alias> as the only spelling of <function>.\n"
            "           Available <function>s (with their currently defined aliases) are:\n"
            "             %2\n"
            "    -ts <ts-file>...\n"
            "           Specify the output file(s). This will override the TRANSLATIONS.\n"
            "    -version\n"
            "           Display the version of lupdate and exit.\n"
            "    @lst-file\n"
            "           Read additional file names (one per line) or includepaths (one per\n"
            "           line, and prefixed with -I) from lst-file.\n"_L1.arg(
                    m_defaultExtensions,
                    trFunctionAliasManager.availableFunctionsWithAliases().join(
                            "\n             "_L1)));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
#ifndef QT_BOOTSTRAPPED
#ifndef Q_OS_WIN32
    QTranslator translator;
    QTranslator qtTranslator;
    QString resourceDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (translator.load("linguist_en"_L1, resourceDir)
        && qtTranslator.load("qt_en"_L1, resourceDir)) {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
    }
#endif // Q_OS_WIN32
#endif

    m_defaultExtensions = QLatin1String("java,jui,ui,c,c++,cc,cpp,cxx,ch,h,"_L1
                                        "h++,hh,hpp,hxx,js,mjs,qs,qml,qrc"_L1);

    QStringList args = app.arguments();
    QStringList tsFileNames;
    QStringList proFiles;
    QString projectDescriptionFile;
    QString outDir = QDir::currentPath();
    QMultiHash<QString, QString> allCSources;
    QSet<QString> projectRoots;
    QStringList sourceFiles;
    QStringList resourceFiles;
    QStringList includePath;
    QStringList alienFiles;
    QString targetLanguage;
    QString sourceLanguage;

    UpdateOptions options =
        Verbose | // verbose is on by default starting with Qt 4.2
        HeuristicSameText | HeuristicSimilarText;
    int numFiles = 0;
    bool metTsFlag = false;
    bool metXTsFlag = false;
    bool recursiveScan = true;

    QString extensions = m_defaultExtensions;
    QSet<QString> extensionsNameFilters;

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == "-help"_L1 || arg == "--help"_L1 || arg == "-h"_L1) {
            printUsage();
            return 0;
        } else if (arg == "-list-languages"_L1) {
            printOut(getNumerusInfoString());
            return 0;
        } else if (arg == "-pluralonly"_L1) {
            options |= PluralOnly;
            continue;
        } else if (arg == "-noobsolete"_L1 || arg == "-no-obsolete"_L1) {
            options |= NoObsolete;
            continue;
        } else if (arg == "-silent"_L1) {
            options &= ~Verbose;
            continue;
        } else if (arg == "-pro-debug"_L1) {
            continue;
        } else if (arg == "-project"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -project requires a parameter.\n"_s);
                return 1;
            }
            if (!projectDescriptionFile.isEmpty()) {
                printErr(u"The option -project must appear only once.\n"_s);
                return 1;
            }
            projectDescriptionFile = args[i];
            numFiles++;
            continue;
        } else if (arg == "-target-language"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -target-language requires a parameter.\n"_s);
                return 1;
            }
            targetLanguage = args[i];
            continue;
        } else if (arg == "-source-language"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -source-language requires a parameter.\n"_s);
                return 1;
            }
            sourceLanguage = args[i];
            continue;
        } else if (arg == "-disable-heuristic"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -disable-heuristic requires a parameter.\n"_s);
                return 1;
            }
            arg = args[i];
            if (arg == "sametext"_L1) {
                options &= ~HeuristicSameText;
            } else if (arg == "similartext"_L1) {
                options &= ~HeuristicSimilarText;
            } else {
                printErr(u"Invalid heuristic name passed to -disable-heuristic.\n"_s);
                return 1;
            }
            continue;
        } else if (arg == "-locations"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The option -locations requires a parameter.\n"_s);
                return 1;
            }
            if (args[i] == "none"_L1) {
                options |= NoLocations;
            } else if (args[i] == "relative"_L1) {
                options |= RelativeLocations;
            } else if (args[i] == "absolute"_L1) {
                options |= AbsoluteLocations;
            } else {
                printErr(u"Invalid parameter passed to -locations.\n"_s);
                return 1;
            }
            continue;
        } else if (arg == "-no-ui-lines"_L1) {
            options |= NoUiLines;
            continue;
        } else if (arg == "-verbose"_L1) {
            options |= Verbose;
            continue;
        } else if (arg == "-warnings-are-errors"_L1) {
            options |= Werror;
            continue;
        } else if (arg == "-no-recursive"_L1) {
            recursiveScan = false;
            continue;
        } else if (arg == "-recursive"_L1) {
            recursiveScan = true;
            continue;
        } else if (arg == "-no-sort"_L1 || arg == "-nosort"_L1) {
            options |= NoSort;
            continue;
        } else if (arg == "-sort-messages"_L1) {
            options |= SortMessages;
            continue;
        } else if (arg == "-version"_L1) {
            printOut(QStringLiteral("lupdate version %1\n").arg(QLatin1String(QT_VERSION_STR)));
            return 0;
        } else if (arg == "-ts"_L1) {
            metTsFlag = true;
            metXTsFlag = false;
            continue;
        } else if (arg == "-xts"_L1) {
            metTsFlag = false;
            metXTsFlag = true;
            continue;
        } else if (arg == "-extensions"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -extensions option should be followed by an extension list.\n"_s);
                return 1;
            }
            extensions = args[i];
            continue;
        } else if (arg == "-tr-function-alias"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -tr-function-alias option should be followed by a list of function=alias mappings.\n"_s);
                return 1;
            }
            if (!parseTrFunctionAliasString(args[i]))
                return 1;
            continue;
        } else if (arg == "-pro"_L1) {
            printErr(u"lupdate warning: The -pro option is deprecated. "
                     u"Please use the lupdate-pro tool instead.\n"_s);
            ++i;
            if (i == argc) {
                printErr(u"The -pro option should be followed by a filename of .pro file.\n"_s);
                return 1;
            }
            QString file = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            proFiles += file;
            numFiles++;
            continue;
        } else if (arg == "-pro-out"_L1) {
            ++i;
            if (i == argc) {
                printErr(u"The -pro-out option should be followed by a directory name.\n"_s);
                return 1;
            }
            outDir = QDir::cleanPath(QFileInfo(args[i]).absoluteFilePath());
            continue;
        } else if (arg.startsWith("-I"_L1)) {
            if (arg.size() == 2) {
                ++i;
                if (i == argc) {
                    printErr(u"The -I option should be followed by a path.\n"_s);
                    return 1;
                }
                includePath += args[i];
            } else {
                includePath += args[i].mid(2);
            }
            continue;
        }
        else if (arg.startsWith("-"_L1) && arg != "-"_L1) {
            printErr("Unrecognized option '%1'.\n"_L1.arg(arg));
            return 1;
        }

        QStringList files;
        if (arg.startsWith("@"_L1)) {
            QFile lstFile(arg.mid(1));
            if (!lstFile.open(QIODevice::ReadOnly)) {
                printErr(QStringLiteral("lupdate error: List file '%1' is not readable.\n")
                         .arg(lstFile.fileName()));
                return 1;
            }
            while (!lstFile.atEnd()) {
                QString lineContent = QString::fromLocal8Bit(lstFile.readLine().trimmed());

                if (lineContent.startsWith("-I"_L1)) {
                    if (lineContent.size() == 2) {
                        printErr(u"The -I option should be followed by a path.\n"_s);
                        return 1;
                    }
                    includePath += lineContent.mid(2);
                } else {
                    files << lineContent;
                }
            }
        } else {
            files << arg;
        }
        if (metTsFlag) {
            for (const QString &file : std::as_const(files)) {
                bool found = false;
                for (const Translator::FileFormat &fmt : std::as_const(Translator::registeredFileFormats())) {
                    if (file.endsWith(u'.' + fmt.extension, Qt::CaseInsensitive)) {
                        QFileInfo fi(file);
                        if (!fi.exists() || fi.isWritable()) {
                            tsFileNames.append(QFileInfo(file).absoluteFilePath());
                        } else {
                            printWarning(options,
                                         "For some reason, '%1' is not writable.\n"_L1
                                                 .arg(file));
                            if (options & Werror)
                                return 1;
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    printErr(QStringLiteral("lupdate error: File '%1' has no recognized extension.\n")
                             .arg(file));
                    return 1;
                }
            }
            numFiles++;
        } else if (metXTsFlag) {
            alienFiles += files;
        } else {
            for (const QString &file : std::as_const(files)) {
                QFileInfo fi(file);
                if (!fi.exists()) {
                    printErr(QStringLiteral("lupdate error: File '%1' does not exist.\n").arg(file));
                    return 1;
                }
                if (isProOrPriFile(file)) {
                    printErr(u"lupdate warning: Passing .pro/.pri files to lupdate is deprecated. "
                             u"Please use the lupdate-pro tool instead.\n"_s);
                    QString cleanFile = QDir::cleanPath(fi.absoluteFilePath());
                    proFiles << cleanFile;
                } else if (fi.isDir()) {
                    if (options & Verbose)
                        printOut(QStringLiteral("Scanning directory '%1'...\n").arg(file));
                    QDir dir = QDir(fi.filePath());
                    projectRoots.insert(dir.absolutePath() + u'/');
                    if (extensionsNameFilters.isEmpty()) {
                        for (QString ext : extensions.split(u',')) {
                            ext = ext.trimmed();
                            if (ext.startsWith(u'.'))
                                ext.remove(0, 1);
                            extensionsNameFilters.insert(ext);
                        }
                    }
                    QDir::Filters filters = QDir::Files | QDir::NoSymLinks;
                    if (recursiveScan)
                        filters |= QDir::AllDirs | QDir::NoDotAndDotDot;
                    QFileInfoList fileinfolist;
                    recursiveFileInfoList(dir, extensionsNameFilters, filters, &fileinfolist);
                    int scanRootLen = dir.absolutePath().size();
                    for (const QFileInfo &fi : std::as_const(fileinfolist)) {
                        QString fn = QDir::cleanPath(fi.absoluteFilePath());
                        if (fn.endsWith(".qrc"_L1, Qt::CaseInsensitive)) {
                            resourceFiles << fn;
                        } else {
                            sourceFiles << fn;

                            if (!fn.endsWith(".java"_L1) && !fn.endsWith(".jui"_L1)
                                && !fn.endsWith(".ui"_L1) && !fn.endsWith(".js"_L1)
                                && !fn.endsWith(".mjs"_L1) && !fn.endsWith(".qs"_L1)
                                && !fn.endsWith(".qml"_L1)) {
                                int offset = 0;
                                int depth = 0;
                                do {
                                    offset = fn.lastIndexOf(u'/', offset - 1);
                                    QString ffn = fn.mid(offset + 1);
                                    allCSources.insert(ffn, fn);
                                } while (++depth < 3 && offset > scanRootLen);
                            }
                        }
                    }
                } else {
                    QString fn = QDir::cleanPath(fi.absoluteFilePath());
                    if (fn.endsWith(".qrc"_L1, Qt::CaseInsensitive))
                        resourceFiles << fn;
                    else
                        sourceFiles << fn;
                    projectRoots.insert(fi.absolutePath() + u'/');
                }
            }
            numFiles++;
        }
    } // for args

    if (numFiles == 0) {
        printUsage();
        return 1;
    }

    if (!targetLanguage.isEmpty() && tsFileNames.size() != 1) {
        printWarning(options,
                     u"-target-language usually only makes sense with exactly one TS file.\n"_s);
        if (options & Werror)
            return 1;
    }

    if (proFiles.isEmpty() && resourceFiles.isEmpty() && sourceFiles.size() == 1
        && QFileInfo(sourceFiles.first()).fileName() == u"CMakeLists.txt"_s) {
        printErr(u"lupdate error: Passing a CMakeLists.txt as project file is not supported.\n"_s
                 u"Please use the 'qt_add_lupdate' CMake command and build the "_s
                 u"'update_translations' target.\n"_s);
        return 1;
    }

    QString errorString;
    Projects projectDescription;

    if (!proFiles.isEmpty()) {
        QStringList translationsVariables = { u"TRANSLATIONS"_s };
        QHash<QString, QString> outDirMap;
        QString outDir = QDir::currentPath();
        for (const QString &proFile : std::as_const(proFiles))
            outDirMap[proFile] = outDir;

        QString errorString;
        QJsonArray results;
        projectDescription = generateProjects(proFiles, translationsVariables, outDirMap,
                                                       0, false, &errorString, &results);
        if (!errorString.isEmpty()) {
            printErr("lupdate error: %1\n"_L1.arg(errorString));
            return 1;
        }
        if (projectDescription.empty()) {
            printErr("lupdate error: No projects found in .pro files\n"_L1);
            return 1;
        }
    } else if (!projectDescriptionFile.isEmpty()) {
        projectDescription = projectDescriptionFromFile(projectDescriptionFile, &errorString);
        if (!errorString.isEmpty()) {
            printErr(QStringLiteral("lupdate error: %1\n").arg(errorString));
            return 1;
        }
        if (projectDescription.empty()) {
            printErr(QStringLiteral("lupdate error:"
                            " Could not find project descriptions in %1.\n")
                     .arg(projectDescriptionFile));
            return 1;
        }
    }

    bool ok = true;
    if (projectDescription.empty()) {   // Direct source file processing mode
        if (tsFileNames.isEmpty()) {
            printWarning(options, u"no TS files specified."_s,
                         u"Only diagnostics will be produced.\n"_s,
                         u"Terminating the operation.\n"_s);
            if (options & Werror)
                return 1;
        }

        for (const QString &resource : std::as_const(resourceFiles))
            sourceFiles << getSourceFilesFromQrc(resource);
        ok &= processSourceFiles(sourceFiles, tsFileNames, alienFiles, projectRoots, includePath,
                           allCSources, sourceLanguage, targetLanguage, options);
    } else {                            // Project-based processing mode
        if (!sourceFiles.isEmpty() || !resourceFiles.isEmpty() || !includePath.isEmpty()) {
            printErr(QStringLiteral("lupdate error:"
                            " Both project and source files / include paths specified.\n"));
            return 1;
        }

        ok &= processProjectDescription(projectDescription, tsFileNames, alienFiles,
                                  sourceLanguage, targetLanguage, options);
    }
    return ok ? 0 : 1;
}
