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
#  include <QtCore/qcoreapplication.h>
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
static QList<Translator> translators;
#endif

static ClangCodeParser* clangParser_ = nullptr;

/*!
  Read some XML indexes containing definitions from other
  documentation sets. \a config contains a variable that
  lists directories where index files can be found. It also
  contains the \c depends variable, which lists the modules
  that the current module depends on. \a formats contains
  a list of output formats; each format may have a different
  output subdirectory where index files are located.
*/
static void loadIndexFiles(Config &config, const QSet<QString> &formats)
{
    QDocDatabase *qdb = QDocDatabase::qdocDB();
    QStringList indexFiles;
    const QStringList configIndexes = config.getStringList(CONFIG_INDEXES);
    for (const auto &index : configIndexes) {
        QFileInfo fi(index);
        if (fi.exists() && fi.isFile())
            indexFiles << index;
        else
            Location::null.warning(QString("Index file not found: %1").arg(index));
    }

    config.dependModules() += config.getStringList(CONFIG_DEPENDS);
    config.dependModules().removeDuplicates();
    bool useNoSubDirs = false;
    QSet<QString> subDirs;

    for (const auto &format : formats) {
        if (config.getBool(format + Config::dot + "nosubdirs")) {
            useNoSubDirs = true;
            QString singleOutputSubdir = config.getString(format
                                                          + Config::dot
                                                          + "outputsubdir");
            if (singleOutputSubdir.isEmpty())
                singleOutputSubdir = "html";
            subDirs << singleOutputSubdir;
        }
    }

    if (config.dependModules().size() > 0) {
        if (config.indexDirs().size() > 0) {
            for (int i = 0; i < config.indexDirs().size(); i++) {
                if (config.indexDirs()[i].startsWith("..")) {
                    const QString prefix(QDir(config.currentDir()).relativeFilePath(config.previousCurrentDir()));
                    if (!prefix.isEmpty())
                        config.indexDirs()[i].prepend(prefix + QLatin1Char('/'));
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
                    std::for_each(formats.begin(), formats.end(),
                        [&](const QString &format) {
                            QDir scanDir(config.getOutputDir(format));
                            QStringList foundModules = scanDir.entryList(QStringList("*.index"), QDir::Files);
                            std::transform(foundModules.begin(), foundModules.end(), foundModules.begin(),
                                [](const QString &index) {
                                    return QFileInfo(index).baseName();
                            });
                            config.dependModules() << foundModules;
                        });
                } else {
                    for (int i = 0; i < config.indexDirs().size(); i++) {
                        QDir scanDir = QDir(config.indexDirs()[i]);
                        scanDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
                        QFileInfoList dirList = scanDir.entryInfoList();
                        for (int j = 0; j < dirList.size(); j++) {
                            config.dependModules().append(dirList[j].fileName());
                        }
                    }
                }
                // Remove self-dependencies and possible duplicates
                config.dependModules().removeAll(config.getString(CONFIG_PROJECT).toLower());
                config.dependModules().removeDuplicates();
                Location::logToStdErrAlways(QString("qdocconf file has depends = *;"
                                                    " loading all %1 index files found").arg(config.dependModules().count()));
            }
            for (int i = 0; i < config.dependModules().size(); i++) {
                QString indexToAdd;
                QString dependModule = config.dependModules()[i];
                QList<QFileInfo> foundIndices;
                // Always look in module-specific subdir, even with *.nosubdirs config
                bool useModuleSubDir = !subDirs.contains(dependModule);
                subDirs << dependModule;

                for (int j = 0; j < config.indexDirs().size(); j++) {
                    for (const auto &subDir : subDirs) {
                        QString fileToLookFor = config.indexDirs()[j]
                                + QLatin1Char('/') + subDir
                                + QLatin1Char('/') + dependModule + ".index";
                        if (QFile::exists(fileToLookFor)) {
                            QFileInfo tempFileInfo(fileToLookFor);
                            if (!foundIndices.contains(tempFileInfo))
                                foundIndices.append(tempFileInfo);
                        }
                    }
                }
                // Clear the temporary module-specific subdir
                if (useModuleSubDir)
                    subDirs.remove(dependModule);
                std::sort(foundIndices.begin(), foundIndices.end(), creationTimeBefore);
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
                    Location::null.warning(QString("Multiple index files found for dependency \"%1\":\n%2").arg(
                                               dependModule, indexPaths.join('\n')));
                    Location::null.warning(QString("Using %1 as index file for dependency \"%2\"").arg(
                                               foundIndices[foundIndices.size() - 1].absoluteFilePath(),
                                               dependModule));
                    indexToAdd = foundIndices[foundIndices.size() - 1].absoluteFilePath();
                }
                else if (foundIndices.size() == 1) {
                    indexToAdd = foundIndices[0].absoluteFilePath();
                }
                if (!indexToAdd.isEmpty()) {
                    if (!indexFiles.contains(indexToAdd))
                        indexFiles << indexToAdd;
                }
                else if (!asteriskUsed) {
                    Location::null.warning(QString("\"%1\" Cannot locate index file for dependency \"%2\"").arg(
                                               config.getString(CONFIG_PROJECT), config.dependModules()[i]));
                }
            }
        }
        else {
            Location::null.warning(QLatin1String("Dependent modules specified, but no index directories were set. There will probably be errors for missing links."));
        }
    }
    qdb->readIndexes(indexFiles);
}

/*!
    Processes the qdoc config file \a fileName. This is the controller for all
    of QDoc. The \a config instance represents the configuration data for QDoc.
    All other classes are initialized with the same config.
 */
static void processQdocconfFile(const QString &fileName, Config &config)
{
    config.setPreviousCurrentDir(QDir::currentPath());

    /*
      With the default configuration values in place, load
      the qdoc configuration file. Note that the configuration
      file may include other configuration files.

      The Location class keeps track of the current location
      in the file being processed, mainly for error reporting
      purposes.
     */
    Location::initialize(config);
    config.load(fileName);
    QString project = config.getString(CONFIG_PROJECT);
    if (project.isEmpty()) {
        Location::logToStdErrAlways(QLatin1String("qdoc can't run; no project set in qdocconf file"));
        exit(1);
    }
    Location::terminate();

    config.setCurrentDir(QFileInfo(fileName).path());
    if (!config.currentDir().isEmpty())
        QDir::setCurrent(config.currentDir());

    QString phase = " in ";
    if (Generator::singleExec())
        phase += "single process mode, ";
    else
        phase += "dual process mode, ";
    if (Generator::preparing())
        phase += "(prepare phase)";
    else if (Generator::generating())
        phase += "(generate phase)";

    QString msg = "Start qdoc for " + config.getString(CONFIG_PROJECT) + phase;
    Location::logToStdErrAlways(msg);
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
    Location::initialize(config);
    Tokenizer::initialize(config);
    CodeMarker::initialize(config);
    CodeParser::initialize(config);
    Generator::initialize(config);
    Doc::initialize(config);

#ifndef QT_NO_TRANSLATION
    /*
      Load the language translators, if the configuration specifies any,
      but only if they haven't already been loaded. This works in both
      -prepare/-generate mode and -singleexec mode.
     */
    QStringList fileNames = config.getStringList(CONFIG_TRANSLATORS);
    QStringList::ConstIterator fn = fileNames.constBegin();
    while (fn != fileNames.constEnd()) {
        bool found = false;
        if (!translators.isEmpty()) {
            for (int i=0; i<translators.size(); ++i) {
                if (translators.at(i).first == *fn) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            QTranslator *translator = new QTranslator(nullptr);
            if (!translator->load(*fn)) {
                config.lastLocation().error(QCoreApplication::translate("QDoc", "Cannot load translator '%1'").arg(*fn));
            }
            else {
                QCoreApplication::instance()->installTranslator(translator);
                translators.append(Translator(*fn, translator));
            }
        }
        ++fn;
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
    if (!Generator::singleExec()) {
        if (!Generator::preparing()) {
            qCDebug(lcQdoc, "  loading index files");
            loadIndexFiles(config, outputFormats);
            qCDebug(lcQdoc, "  done loading index files");
        }
        qdb->newPrimaryTree(project);
    }
    else if (Generator::preparing())
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
        QString title = config.getString(CONFIG_NAVIGATION
                                        + Config::dot
                                        + CONFIG_LANDINGPAGE);
        root->tree()->setIndexTitle(config.getString(CONFIG_NAVIGATION
                                        + Config::dot
                                        + CONFIG_LANDINGTITLE, title));
    }

    const auto &excludedDirList = config.getCanonicalPathList(CONFIG_EXCLUDEDIRS);
    QSet<QString> excludedDirs = QSet<QString>(excludedDirList.cbegin(), excludedDirList.cend());
    const auto &excludedFilesList = config.getCanonicalPathList(CONFIG_EXCLUDEFILES);
    QSet<QString> excludedFiles = QSet<QString>(excludedFilesList.cbegin(), excludedFilesList.cend());

    qCDebug(lcQdoc, "Adding doc/image dirs found in exampledirs to imagedirs");
    QSet<QString> exampleImageDirs;
    QStringList exampleImageList = config.getExampleImageFiles(excludedDirs, excludedFiles);
    for (int i = 0; i < exampleImageList.size(); ++i) {
        if (exampleImageList[i].contains("doc/images")) {
            QString t = exampleImageList[i].left(exampleImageList[i].lastIndexOf("doc/images") + 10);
            if (!exampleImageDirs.contains(t)) {
                exampleImageDirs.insert(t);
            }
        }
    }
    Generator::augmentImageDirs(exampleImageDirs);

    if (Generator::dualExec() || Generator::preparing()) {
        QStringList headerList;
        QStringList sourceList;

        qCDebug(lcQdoc, "Reading headerdirs");
        headerList = config.getAllFiles(CONFIG_HEADERS,CONFIG_HEADERDIRS,excludedDirs,excludedFiles);
        QMap<QString,QString> headers;
        QMultiMap<QString,QString> headerFileNames;
        for (int i=0; i<headerList.size(); ++i) {
            if (headerList[i].contains(QString("doc/snippets")))
                continue;
            if (headers.contains(headerList[i]))
                continue;
            headers.insert(headerList[i],headerList[i]);
            QString t = headerList[i].mid(headerList[i].lastIndexOf('/')+1);
            headerFileNames.insert(t,t);
        }

        qCDebug(lcQdoc, "Reading sourcedirs");
        sourceList = config.getAllFiles(CONFIG_SOURCES,CONFIG_SOURCEDIRS,excludedDirs,excludedFiles);
        QMap<QString,QString> sources;
        QMultiMap<QString,QString> sourceFileNames;
        for (int i=0; i<sourceList.size(); ++i) {
            if (sourceList[i].contains(QString("doc/snippets")))
                continue;
            if (sources.contains(sourceList[i]))
                continue;
            sources.insert(sourceList[i],sourceList[i]);
            QString t = sourceList[i].mid(sourceList[i].lastIndexOf('/')+1);
            sourceFileNames.insert(t,t);
        }
        /*
          Find all the qdoc files in the example dirs, and add
          them to the source files to be parsed.
        */
        qCDebug(lcQdoc, "Reading exampledirs");
        QStringList exampleQdocList = config.getExampleQdocFiles(excludedDirs, excludedFiles);
        for (int i=0; i<exampleQdocList.size(); ++i) {
            if (!sources.contains(exampleQdocList[i])) {
                sources.insert(exampleQdocList[i],exampleQdocList[i]);
                QString t = exampleQdocList[i].mid(exampleQdocList[i].lastIndexOf('/')+1);
                sourceFileNames.insert(t,t);
            }
        }
        /*
          Parse each header file in the set using the appropriate parser and add it
          to the big tree.
        */

        qCDebug(lcQdoc, "Parsing header files");
        int parsed = 0;
        QMap<QString,QString>::ConstIterator h = headers.constBegin();
        while (h != headers.constEnd()) {
            CodeParser *codeParser = CodeParser::parserForHeaderFile(h.key());
            if (codeParser) {
                ++parsed;
                qCDebug(lcQdoc, "Parsing %s", qPrintable(h.key()));
                codeParser->parseHeaderFile(config.location(), h.key());
            }
            ++h;
        }

        clangParser_->precompileHeaders();

        /*
          Parse each source text file in the set using the appropriate parser and
          add it to the big tree.
        */
        parsed = 0;
        Location::logToStdErrAlways("Parse source files for " + project);
        QMap<QString,QString>::ConstIterator s = sources.constBegin();
        while (s != sources.constEnd()) {
            CodeParser *codeParser = CodeParser::parserForSourceFile(s.key());
            if (codeParser) {
                ++parsed;
                qCDebug(lcQdoc, "Parsing %s", qPrintable(s.key()));
                codeParser->parseSourceFile(config.location(), s.key());
            }
            ++s;
        }
        Location::logToStdErrAlways("Source files parsed for " + project);
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
    QSet<QString>::ConstIterator of = outputFormats.constBegin();
    while (of != outputFormats.constEnd()) {
        Generator *generator = Generator::generatorForFormat(*of);
        if (generator == nullptr)
            outputFormatsLocation.fatal(QCoreApplication::translate("QDoc",
                                               "Unknown output format '%1'").arg(*of));
        generator->initializeFormat(config);
        generator->generateDocs();
        ++of;
    }
    qdb->clearLinkCounts();

    qCDebug(lcQdoc, "Terminating qdoc classes");
    if (Utilities::debugging())
        Utilities::stopDebugging(project);

    msg = "End qdoc for " + config.getString(CONFIG_PROJECT) + phase;
    Location::logToStdErrAlways(msg);
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

    Config config(QCoreApplication::translate("QDoc", "qdoc"), app.arguments());

    // Get the list of files to act on:
    QStringList qdocFiles = config.qdocFiles();
    if (qdocFiles.isEmpty())
        config.showHelp();

    if (config.singleExec())
        qdocFiles = Config::loadMaster(qdocFiles.at(0));

    if (Generator::singleExec()) {
        // single qdoc process for prepare and generate phases
        Generator::setQDocPass(Generator::Prepare);
        for (const auto &file : qAsConst(qdocFiles)) {
            config.dependModules().clear();
            processQdocconfFile(file, config);
        }
        Generator::setQDocPass(Generator::Generate);
        QDocDatabase::qdocDB()->processForest();
        for (const auto &file : qAsConst(qdocFiles)) {
            config.dependModules().clear();
            processQdocconfFile(file, config);
        }
    } else {
        // separate qdoc processes for prepare and generate phases
        for (const auto &file : qAsConst(qdocFiles)) {
            config.dependModules().clear();
            processQdocconfFile(file, config);
        }
    }

    // Tidy everything away:
#ifndef QT_NO_TRANSLATION
    if (!translators.isEmpty()) {
        for (int i=0; i<translators.size(); ++i) {
            delete translators.at(i).second;
        }
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
