/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "clangcodeparser.h"
#include "codemarker.h"
#include "codeparser.h"
#include "config.h"
#include "cppcodemarker.h"
#include "cppcodeparser.h"
#include "doc.h"
#include "htmlgenerator.h"
#include "docbookgenerator.h"
#include "jscodemarker.h"
#include "location.h"
#include "loggingcategory.h"
#include "puredocparser.h"
#include "qdocdatabase.h"
#include "qmlcodemarker.h"
#include "qmlcodeparser.h"
#include "utilities.h"
#include "qtranslator.h"
#include "tokenizer.h"
#include "tree.h"
#include "webxmlgenerator.h"

#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qglobal.h>
#include <QtCore/qglobalstatic.h>
#include <QtCore/qhashfunctions.h>

#ifndef QT_BOOTSTRAPPED
#    include <QtCore/qcoreapplication.h>
#endif

#include <algorithm>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQdoc, "qt.qdoc")

bool creationTimeBefore(const QFileInfo &fi1, const QFileInfo &fi2)
{
    return fi1.lastModified() < fi2.lastModified();
}

#ifndef QT_NO_TRANSLATION
typedef QPair<QString, QTranslator *> Translator;
static QVector<Translator> translators;
#endif

static ClangCodeParser *clangParser_ = nullptr;

/*!
  Read some XML indexes containing definitions from other
  documentation sets. \a config contains a variable that
  lists directories where index files can be found. It also
  contains the \c depends variable, which lists the modules
  that the current module depends on. \a formats contains
  a list of output formats; each format may have a different
  output subdirectory where index files are located.
*/
static void loadIndexFiles(const QSet<QString> &formats)
{
    Config &config = Config::instance();
    QDocDatabase *qdb = QDocDatabase::qdocDB();
    QStringList indexFiles;
    const QStringList configIndexes = config.getStringList(CONFIG_INDEXES);
    for (const auto &index : configIndexes) {
        QFileInfo fi(index);
        if (fi.exists() && fi.isFile())
            indexFiles << index;
        else
            Location().warning(QString("Index file not found: %1").arg(index));
    }

    config.dependModules() += config.getStringList(CONFIG_DEPENDS);
    config.dependModules().removeDuplicates();
    bool useNoSubDirs = false;
    QSet<QString> subDirs;

    for (const auto &format : formats) {
        if (config.getBool(format + Config::dot + "nosubdirs")) {
            useNoSubDirs = true;
            QString singleOutputSubdir = config.getString(format + Config::dot + "outputsubdir");
            if (singleOutputSubdir.isEmpty())
                singleOutputSubdir = "html";
            subDirs << singleOutputSubdir;
        }
    }

    if (config.dependModules().size() > 0) {
        if (config.indexDirs().size() > 0) {
            for (auto &dir : config.indexDirs()) {
                if (dir.startsWith("..")) {
                    const QString prefix(QDir(config.currentDir())
                                                 .relativeFilePath(config.previousCurrentDir()));
                    if (!prefix.isEmpty())
                        dir.prepend(prefix + QLatin1Char('/'));
                }
            }
            /*
              Load all dependencies:
              Either add all subdirectories of the indexdirs as dependModules,
              when an asterisk is used in the 'depends' list, or
              when <format>.nosubdirs is set, we need to look for all .index files
              in the output subdirectory instead.
            */
            bool asteriskUsed = false;
            if (config.dependModules().contains("*")) {
                config.dependModules().removeOne("*");
                asteriskUsed = true;
                if (useNoSubDirs) {
                    std::for_each(formats.begin(), formats.end(), [&](const QString &format) {
                        QDir scanDir(config.getOutputDir(format));
                        QStringList foundModules =
                                scanDir.entryList(QStringList("*.index"), QDir::Files);
                        std::transform(
                                foundModules.begin(), foundModules.end(), foundModules.begin(),
                                [](const QString &index) { return QFileInfo(index).baseName(); });
                        config.dependModules() << foundModules;
                    });
                } else {
                    for (const auto &indexDir : config.indexDirs()) {
                        QDir scanDir = QDir(indexDir);
                        scanDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
                        QFileInfoList dirList = scanDir.entryInfoList();
                        for (const auto &dir : dirList)
                            config.dependModules().append(dir.fileName());
                    }
                }
                // Remove self-dependencies and possible duplicates
                config.dependModules().removeAll(config.getString(CONFIG_PROJECT).toLower());
                config.dependModules().removeDuplicates();
                qCCritical(lcQdoc) << "qdocconf file has depends = *; loading all "
                               << config.dependModules().count()
                               << " index files found";
            }
            for (const auto &module : config.dependModules()) {
                QVector<QFileInfo> foundIndices;
                // Always look in module-specific subdir, even with *.nosubdirs config
                bool useModuleSubDir = !subDirs.contains(module);
                subDirs << module;

                for (const auto &dir : config.indexDirs()) {
                    for (const auto &subDir : subDirs) {
                        QString fileToLookFor = dir + QLatin1Char('/') + subDir + QLatin1Char('/')
                                + module + ".index";
                        if (QFile::exists(fileToLookFor)) {
                            QFileInfo tempFileInfo(fileToLookFor);
                            if (!foundIndices.contains(tempFileInfo))
                                foundIndices.append(tempFileInfo);
                        }
                    }
                }
                // Clear the temporary module-specific subdir
                if (useModuleSubDir)
                    subDirs.remove(module);
                std::sort(foundIndices.begin(), foundIndices.end(), creationTimeBefore);
                QString indexToAdd;
                if (foundIndices.size() > 1) {
                    /*
                        QDoc should always use the last entry in the multimap when there are
                        multiple index files for a module, since the last modified file has the
                        highest UNIX timestamp.
                    */
                    QStringList indexPaths;
                    indexPaths.reserve(foundIndices.size());
                    for (const auto &found : qAsConst(foundIndices))
                        indexPaths << found.absoluteFilePath();
                    Location().warning(
                            QString("Multiple index files found for dependency \"%1\":\n%2")
                                    .arg(module, indexPaths.join('\n')));
                    Location().warning(
                            QString("Using %1 as index file for dependency \"%2\"")
                                    .arg(foundIndices[foundIndices.size() - 1].absoluteFilePath(),
                                         module));
                    indexToAdd = foundIndices[foundIndices.size() - 1].absoluteFilePath();
                } else if (foundIndices.size() == 1) {
                    indexToAdd = foundIndices[0].absoluteFilePath();
                }
                if (!indexToAdd.isEmpty()) {
                    if (!indexFiles.contains(indexToAdd))
                        indexFiles << indexToAdd;
                } else if (!asteriskUsed) {
                    Location().warning(
                            QString("\"%1\" Cannot locate index file for dependency \"%2\"")
                                    .arg(config.getString(CONFIG_PROJECT), module));
                }
            }
        } else {
            Location().warning(
                    QLatin1String("Dependent modules specified, but no index directories were set. "
                                  "There will probably be errors for missing links."));
        }
    }
    qdb->readIndexes(indexFiles);
}

/*!
    \internal
    Prints to stderr the name of the project that QDoc is running for,
    in which mode and which phase.

    If QDoc is running in debug mode, also logs the command line arguments.
 */
void logStartEndMessage(const QLatin1String &startStop, const Config &config)
{
    const QString runName = " qdoc for "
            + config.getString(CONFIG_PROJECT)
            + QLatin1String(" in ")
            + QLatin1String(config.singleExec() ? "single" : "dual")
            + QLatin1String(" process mode: ")
            + QLatin1String(config.preparing() ? "prepare" : "generate")
            + QLatin1String(" phase.");

    const QString msg = startStop + runName;
    qCInfo(lcQdoc) << msg.toUtf8().data();
}

/*!
    Processes the qdoc config file \a fileName. This is the controller for all
    of QDoc. The \a config instance represents the configuration data for QDoc.
    All other classes are initialized with the same config.
 */
static void processQdocconfFile(const QString &fileName)
{
    Config &config = Config::instance();
    config.setPreviousCurrentDir(QDir::currentPath());

    /*
      With the default configuration values in place, load
      the qdoc configuration file. Note that the configuration
      file may include other configuration files.

      The Location class keeps track of the current location
      in the file being processed, mainly for error reporting
      purposes.
     */
    Location::initialize();
    config.load(fileName);
    QString project = config.getString(CONFIG_PROJECT);
    if (project.isEmpty()) {
        qCCritical(lcQdoc) << QLatin1String("qdoc can't run; no project set in qdocconf file");
        exit(1);
    }
    Location::terminate();

    config.setCurrentDir(QFileInfo(fileName).path());
    if (!config.currentDir().isEmpty())
        QDir::setCurrent(config.currentDir());

    logStartEndMessage(QLatin1String("Start"), config);

    if (config.getDebug()) {
        Utilities::startDebugging(QString("command line"));
        qCDebug(lcQdoc).noquote() << "Arguments:" << QCoreApplication::arguments();
    }

    /*
      Initialize all the classes and data structures with the
      qdoc configuration. This is safe to do for each qdocconf
      file processed, because all the data structures created
      are either cleared after they have been used, or they
      are cleared in the terminate() functions below.
     */
    Location::initialize();
    Tokenizer::initialize();
    CodeMarker::initialize();
    CodeParser::initialize();
    Generator::initialize();
    Doc::initialize();

#ifndef QT_NO_TRANSLATION
    /*
      Load the language translators, if the configuration specifies any,
      but only if they haven't already been loaded. This works in both
      -prepare/-generate mode and -singleexec mode.
     */
    const QStringList fileNames = config.getStringList(CONFIG_TRANSLATORS);
    for (const auto &fileName : fileNames) {
        bool found = false;
        if (!translators.isEmpty()) {
            for (const auto &translator : translators) {
                if (translator.first == fileName) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            QTranslator *translator = new QTranslator(nullptr);
            if (!translator->load(fileName)) {
                config.lastLocation().error(
                        QCoreApplication::translate("QDoc", "Cannot load translator '%1'")
                                .arg(fileName));
            } else {
                QCoreApplication::instance()->installTranslator(translator);
                translators.append(Translator(fileName, translator));
            }
        }
    }
#endif

    /*
      Get the source language (Cpp) from the configuration
      and the location in the configuration file where the
      source language was set.
     */
    QString lang = config.getString(CONFIG_LANGUAGE);
    Location langLocation = config.lastLocation();

    /*
      Initialize the qdoc database, where all the parsed source files
      will be stored. The database includes a tree of nodes, which gets
      built as the source files are parsed. The documentation output is
      generated by traversing that tree.

      Note: qdocDB() allocates a new instance only if no instance exists.
      So it is safe to call qdocDB() any time.
     */
    QDocDatabase *qdb = QDocDatabase::qdocDB();
    qdb->setVersion(config.getString(CONFIG_VERSION));
    qdb->setShowInternal(config.getBool(CONFIG_SHOWINTERNAL));
    qdb->setSingleExec(config.getBool(CONFIG_SINGLEEXEC));
    /*
      By default, the only output format is HTML.
     */
    QSet<QString> outputFormats = config.getOutputFormats();
    Location outputFormatsLocation = config.lastLocation();

    qdb->clearSearchOrder();
    if (!config.singleExec()) {
        if (!config.preparing()) {
            qCDebug(lcQdoc, "  loading index files");
            loadIndexFiles(outputFormats);
            qCDebug(lcQdoc, "  done loading index files");
        }
        qdb->newPrimaryTree(project);
    } else if (config.preparing())
        qdb->newPrimaryTree(project);
    else
        qdb->setPrimaryTree(project);

    const QString moduleHeader = config.getString(CONFIG_MODULEHEADER);
    if (!moduleHeader.isNull())
        clangParser_->setModuleHeader(moduleHeader);
    else
        clangParser_->setModuleHeader(project);

    // Retrieve the dependencies if loadIndexFiles() was not called
    if (config.dependModules().isEmpty()) {
        config.dependModules() = config.getStringList(CONFIG_DEPENDS);
        config.dependModules().removeDuplicates();
    }
    qdb->setSearchOrder(config.dependModules());

    // Store the title of the index (landing) page
    NamespaceNode *root = qdb->primaryTreeRoot();
    if (root) {
        QString title = config.getString(CONFIG_NAVIGATION + Config::dot + CONFIG_LANDINGPAGE);
        root->tree()->setIndexTitle(
                config.getString(CONFIG_NAVIGATION + Config::dot + CONFIG_LANDINGTITLE, title));
    }

    const auto &excludedDirList = config.getCanonicalPathList(CONFIG_EXCLUDEDIRS);
    QSet<QString> excludedDirs = QSet<QString>(excludedDirList.cbegin(), excludedDirList.cend());
    const auto &excludedFilesList = config.getCanonicalPathList(CONFIG_EXCLUDEFILES);
    QSet<QString> excludedFiles =
            QSet<QString>(excludedFilesList.cbegin(), excludedFilesList.cend());

    qCDebug(lcQdoc, "Adding doc/image dirs found in exampledirs to imagedirs");
    QSet<QString> exampleImageDirs;
    QStringList exampleImageList = config.getExampleImageFiles(excludedDirs, excludedFiles);
    for (const auto &image : exampleImageList) {
        if (image.contains("doc/images")) {
            QString t = image.left(image.lastIndexOf("doc/images") + 10);
            if (!exampleImageDirs.contains(t))
                exampleImageDirs.insert(t);
        }
    }
    Generator::augmentImageDirs(exampleImageDirs);

    if (config.dualExec() || config.preparing()) {
        QStringList headerList;
        QStringList sourceList;

        qCDebug(lcQdoc, "Reading headerdirs");
        headerList =
                config.getAllFiles(CONFIG_HEADERS, CONFIG_HEADERDIRS, excludedDirs, excludedFiles);
        QMap<QString, QString> headers;
        QMultiMap<QString, QString> headerFileNames;
        for (const auto &header : headerList) {
            if (header.contains(QLatin1String("doc/snippets")))
                continue;
            if (headers.contains(header))
                continue;
            headers.insert(header, header);
            QString t = header.mid(header.lastIndexOf('/') + 1);
            headerFileNames.insert(t, t);
        }

        qCDebug(lcQdoc, "Reading sourcedirs");
        sourceList =
                config.getAllFiles(CONFIG_SOURCES, CONFIG_SOURCEDIRS, excludedDirs, excludedFiles);
        QMap<QString, QString> sources;
        QMultiMap<QString, QString> sourceFileNames;
        for (const auto &source : sourceList) {
            if (source.contains(QLatin1String("doc/snippets")))
                continue;
            if (sources.contains(source))
                continue;
            sources.insert(source, source);
            QString t = source.mid(source.lastIndexOf('/') + 1);
            sourceFileNames.insert(t, t);
        }
        /*
          Find all the qdoc files in the example dirs, and add
          them to the source files to be parsed.
        */
        qCDebug(lcQdoc, "Reading exampledirs");
        QStringList exampleQdocList = config.getExampleQdocFiles(excludedDirs, excludedFiles);
        for (const auto &example : exampleQdocList) {
            if (!sources.contains(example)) {
                sources.insert(example, example);
                QString t = example.mid(example.lastIndexOf('/') + 1);
                sourceFileNames.insert(t, t);
            }
        }
        /*
          Parse each header file in the set using the appropriate parser and add it
          to the big tree.
        */

        qCDebug(lcQdoc, "Parsing header files");
        int parsed = 0;
        for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
            CodeParser *codeParser = CodeParser::parserForHeaderFile(it.key());
            if (codeParser) {
                ++parsed;
                qCDebug(lcQdoc, "Parsing %s", qPrintable(it.key()));
                codeParser->parseHeaderFile(config.location(), it.key());
            }
        }

        clangParser_->precompileHeaders();

        /*
          Parse each source text file in the set using the appropriate parser and
          add it to the big tree.
        */
        parsed = 0;
        qCInfo(lcQdoc) << "Parse source files for" << project;
        for (const auto &key : sources.keys()) {
            auto *codeParser = CodeParser::parserForSourceFile(key);
            if (codeParser) {
                ++parsed;
                qCDebug(lcQdoc, "Parsing %s", qPrintable(key));
                codeParser->parseSourceFile(config.location(), key);
            }
        }
        qCInfo(lcQdoc) << "Source files parsed for" << project;
    }
    /*
      Now the primary tree has been built from all the header and
      source files. Resolve all the class names, function names,
      targets, URLs, links, and other stuff that needs resolving.
    */
    qCDebug(lcQdoc, "Resolving stuff prior to generating docs");
    qdb->resolveStuff();

    /*
      The primary tree is built and all the stuff that needed
      resolving has been resolved. Now traverse the tree and
      generate the documentation output. More than one output
      format can be requested. The tree is traversed for each
      one.
     */
    qCDebug(lcQdoc, "Generating docs");
    for (const auto &format : outputFormats) {
        auto *generator = Generator::generatorForFormat(format);
        if (generator == nullptr)
            outputFormatsLocation.fatal(
                    QCoreApplication::translate("QDoc", "Unknown output format '%1'").arg(format));
        generator->initializeFormat();
        generator->generateDocs();
    }
    qdb->clearLinkCounts();

    qCDebug(lcQdoc, "Terminating qdoc classes");
    if (Utilities::debugging())
        Utilities::stopDebugging(project);

    logStartEndMessage(QLatin1String("End"), config);
    QDocDatabase::qdocDB()->setVersion(QString());
    Generator::terminate();
    CodeParser::terminate();
    CodeMarker::terminate();
    Doc::terminate();
    Tokenizer::terminate();
    Location::terminate();
    QDir::setCurrent(config.previousCurrentDir());

    qCDebug(lcQdoc, "qdoc classes terminated");
}

QT_END_NAMESPACE

int main(int argc, char **argv)
{
    QT_USE_NAMESPACE

    // Initialize Qt:
#ifndef QT_BOOTSTRAPPED
    qSetGlobalQHashSeed(0); // set the hash seed to 0 if it wasn't set yet
#endif
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));

    // Instantiate various singletons (used via static methods):
    /*
      Create code parsers for the languages to be parsed,
      and create a tree for C++.
     */
    ClangCodeParser clangParser;
    clangParser_ = &clangParser;
    QmlCodeParser qmlParser;
    PureDocParser docParser;

    /*
      Create code markers for plain text, C++,
      javascript, and QML.

      The plain CodeMarker must be instantiated first because it is used as
      fallback when the other markers cannot be used.

      Each marker instance is prepended to the CodeMarker::markers list by the
      base class constructor.
     */
    CodeMarker fallbackMarker;
    CppCodeMarker cppMarker;
    JsCodeMarker jsMarker;
    QmlCodeMarker qmlMarker;

    HtmlGenerator htmlGenerator;
    WebXMLGenerator webXMLGenerator;
    DocBookGenerator docBookGenerator;

    Config::instance().init(QCoreApplication::translate("QDoc", "qdoc"), app.arguments());
    Config &config = Config::instance();

    // Get the list of files to act on:
    QStringList qdocFiles = config.qdocFiles();
    if (qdocFiles.isEmpty())
        config.showHelp();

    if (config.singleExec())
        qdocFiles = Config::loadMaster(qdocFiles.at(0));

    if (config.singleExec()) {
        // single qdoc process for prepare and generate phases
        config.setQDocPass(Config::Prepare);
        for (const auto &file : qAsConst(qdocFiles)) {
            config.dependModules().clear();
            processQdocconfFile(file);
        }
        config.setQDocPass(Config::Generate);
        QDocDatabase::qdocDB()->processForest();
        for (const auto &file : qAsConst(qdocFiles)) {
            config.dependModules().clear();
            processQdocconfFile(file);
        }
    } else {
        // separate qdoc processes for prepare and generate phases
        for (const auto &file : qAsConst(qdocFiles)) {
            config.dependModules().clear();
            processQdocconfFile(file);
        }
    }

    // Tidy everything away:
#ifndef QT_NO_TRANSLATION
    if (!translators.isEmpty()) {
        for (const auto &translator : translators)
            delete translator.second;
    }
    translators.clear();
#endif
    QmlTypeNode::terminate();

#ifdef DEBUG_SHUTDOWN_CRASH
    qDebug() << "main(): Delete qdoc database";
#endif
    QDocDatabase::destroyQdocDB();
#ifdef DEBUG_SHUTDOWN_CRASH
    qDebug() << "main(): qdoc database deleted";
#endif

    return Location::exitCode();
}
