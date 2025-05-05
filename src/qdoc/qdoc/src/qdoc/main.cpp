// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "clangcodeparser.h"
#include "codemarker.h"
#include "codeparser.h"
#include "config.h"
#include "cppcodemarker.h"
#include "doc.h"
#include "docbookgenerator.h"
#include "htmlgenerator.h"
#include "location.h"
#include "puredocparser.h"
#include "qdocdatabase.h"
#include "qmlcodemarker.h"
#include "qmlcodeparser.h"
#include "sourcefileparser.h"
#include "utilities.h"
#include "tokenizer.h"
#include "tree.h"
#include "webxmlgenerator.h"

#include "filesystem/fileresolver.h"
#include "boundaries/filesystem/directorypath.h"

#include <QtCore/qcompilerdetection.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qglobal.h>
#include <QtCore/qhashfunctions.h>

#include <set>

#ifndef QT_BOOTSTRAPPED
#    include <QtCore/qcoreapplication.h>
#endif

#include <algorithm>
#include <cstdlib>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

bool creationTimeBefore(const QFileInfo &fi1, const QFileInfo &fi2)
{
    return fi1.lastModified() < fi2.lastModified();
}

/*!
    \internal
    Inspects each file path in \a sources. File paths with a known
    source file type are parsed to extract user-provided
    documentation and information about source code level elements.

    \note Unknown source file types are silently ignored.

    The validity or availability of the file paths may or may not cause QDoc
    to generate warnings; this depends on the implementation of
    parseSourceFile() for the relevant parser.

    \sa CodeParser::parserForSourceFile, CodeParser::sourceFileNameFilter
*/
static void parseSourceFiles(
    std::vector<QString>&& sources,
    SourceFileParser& source_file_parser,
    CppCodeParser& cpp_code_parser
) {
    ParserErrorHandler error_handler{};
    std::stable_sort(sources.begin(), sources.end());

    sources.erase (
        std::unique(sources.begin(), sources.end()),
        sources.end()
    );

    sources.erase(std::remove_if(sources.begin(), sources.end(),
        [](const QString &source) {
            return Utilities::isGeneratedFile(source);
        }),
        sources.end());

    auto qml_sources =
        std::stable_partition(sources.begin(), sources.end(), [](const QString& source){
            return CodeParser::parserForSourceFile(source) == CodeParser::parserForLanguage("QML");
        });


    std::for_each(qml_sources, sources.end(),
            [&source_file_parser, &cpp_code_parser, &error_handler](const QString& source){
        qCDebug(lcQdoc, "Parsing %s", qPrintable(source));

        auto [untied_documentation, tied_documentation] = source_file_parser(tag_source_file(source));
        std::vector<FnMatchError> errors{};

        for (const auto &untied : std::as_const(untied_documentation)) {
            auto result = cpp_code_parser.processTopicArgs(untied);
            tied_documentation.insert(tied_documentation.end(), result.first.begin(), result.first.end());
            errors.insert(errors.end(), result.second.begin(), result.second.end());
        };

        cpp_code_parser.processMetaCommands(tied_documentation);

        // Process errors that occurred during parsing
        for (const auto &e : errors)
            error_handler(e);
    });

    std::for_each(sources.begin(), qml_sources, [&cpp_code_parser](const QString& source){
        auto *codeParser = CodeParser::parserForSourceFile(source);
        if (!codeParser) return;

        qCDebug(lcQdoc, "Parsing %s", qPrintable(source));
        codeParser->parseSourceFile(Config::instance().location(), source, cpp_code_parser);
    });

}

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
    const QStringList configIndexes{config.get(CONFIG_INDEXES).asStringList()};
    bool warn = !config.get(CONFIG_NOLINKERRORS).asBool();

    for (const auto &index : configIndexes) {
        QFileInfo fi(index);
        if (fi.exists() && fi.isFile())
            indexFiles << index;
        else if (warn)
            Location().warning(QString("Index file not found: %1").arg(index));
    }

    config.dependModules() += config.get(CONFIG_DEPENDS).asStringList();
    config.dependModules().removeDuplicates();
    bool useNoSubDirs = false;
    QSet<QString> subDirs;

    // Add format-specific output subdirectories to the set of
    // subdirectories where we look for index files
    for (const auto &format : formats) {
        if (config.get(format + Config::dot + "nosubdirs").asBool()) {
            useNoSubDirs = true;
            const auto singleOutputSubdir{QDir(config.getOutputDir(format)).dirName()};
            if (!singleOutputSubdir.isEmpty())
                subDirs << singleOutputSubdir;
        }
    }

    if (!config.dependModules().empty()) {
        if (!config.indexDirs().empty()) {
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
                QString project{config.get(CONFIG_PROJECT).asString()};
                config.dependModules().removeAll(project.toLower());
                config.dependModules().removeDuplicates();
                qCCritical(lcQdoc) << "Configuration file for"
                               << project << "has depends = *; loading all"
                               << config.dependModules().size()
                               << "index files found";
            }
            for (const auto &module : config.dependModules()) {
                QList<QFileInfo> foundIndices;
                // Always look in module-specific subdir, even with *.nosubdirs config
                bool useModuleSubDir = !subDirs.contains(module);
                subDirs << module;

                for (const auto &dir : config.indexDirs()) {
                    for (const auto &subDir : std::as_const(subDirs)) {
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
                    for (const auto &found : std::as_const(foundIndices))
                        indexPaths << found.absoluteFilePath();
                    if (warn) {
                        Location().warning(
                                QString("Multiple index files found for dependency \"%1\":\n%2")
                                        .arg(module, indexPaths.join('\n')));
                        Location().warning(
                                QString("Using %1 as index file for dependency \"%2\"")
                                        .arg(foundIndices[foundIndices.size() - 1].absoluteFilePath(),
                                             module));
                    }
                    indexToAdd = foundIndices[foundIndices.size() - 1].absoluteFilePath();
                } else if (foundIndices.size() == 1) {
                    indexToAdd = foundIndices[0].absoluteFilePath();
                }
                if (!indexToAdd.isEmpty()) {
                    if (!indexFiles.contains(indexToAdd))
                        indexFiles << indexToAdd;
                } else if (!asteriskUsed && warn) {
                    Location().warning(
                            QString(R"("%1" Cannot locate index file for dependency "%2")")
                                    .arg(config.get(CONFIG_PROJECT).asString(), module));
                }
            }
        } else if (warn) {
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

    If QDoc is not running in debug mode or --log-progress command line
    option is not set, do nothing.
 */
void logStartEndMessage(const QLatin1String &startStop, Config &config)
{
    if (!config.get(CONFIG_LOGPROGRESS).asBool())
        return;

    const QString runName = " qdoc for "
            + config.get(CONFIG_PROJECT).asString()
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
    QString project{config.get(CONFIG_PROJECT).asString()};
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

    // <<TODO: [cleanup-temporary-kludges]
    // The underlying validation should be performed at the
    // configuration level during parsing.
    // This cannot be done straightforwardly with how the Config class
    // is implemented.
    // When the Config class will be deprived of logic and
    // restructured, the compiler will notify us of this kludge, but
    // remember to reevaluate the code itself considering the new
    // data-flow and the possibility for optimizations as this is not
    // done for temporary code. Indeed some of the code is visibly wasteful.
    // Similarly, ensure that the loose definition that we use here is
    // not preserved.

    QStringList search_directories{config.getCanonicalPathList(CONFIG_EXAMPLEDIRS)};
    QStringList image_search_directories{config.getCanonicalPathList(CONFIG_IMAGEDIRS)};

    const auto& [excludedDirs, excludedFiles] = config.getExcludedPaths();

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

    // REMARK: The previous system discerned between search directories based on the kind of file that was searched for.
    // For example, an image search was bounded to some directories
    // that may or may not be the same as the ones where examples are
    // searched for.
    // The current Qt documentation does not use this feature. That
    // is, the output of QDoc when a unified search list is used is
    // the same as the output for that of separated lists.
    // For this reason, we currently simplify the process, albeit this
    // may at some point change, by joining the various lists into a
    // single search list and a unified interface.
    // Do note that the configuration still allows for those
    // parameters to be user defined in a split-way as this will not
    // be able to be changed until Config itself is looked upon.
    // Hence, we join the various directory sources into one list for the time being.
    // Do note that this means that the amount of searched directories for a file is now increased.
    // This shouldn't matter as the amount of directories is expected
    // to be generally small and the search routine complexity is
    // linear in the amount of directories.
    // There are some complications that may arise in very specific
    // cases by this choice (some of which where there before under
    // possibly different circumstances), making some files
    // unreachable.
    // See the remarks in FileResolver for more infomration.
    std::copy(image_search_directories.begin(), image_search_directories.end(), std::back_inserter(search_directories));
    std::copy(exampleImageDirs.begin(), exampleImageDirs.end(), std::back_inserter(search_directories));

    std::vector<DirectoryPath> validated_search_directories{};
    for (const QString& path : search_directories) {
        auto maybe_validated_path{DirectoryPath::refine(path)};
        if (!maybe_validated_path)
            // TODO: [uncentralized-admonition]
            qCDebug(lcQdoc).noquote() << u"%1 is not a valid path, it will be ignored when resolving a file"_s.arg(path);
        else validated_search_directories.push_back(*maybe_validated_path);
    }

    // TODO>>

    FileResolver file_resolver{std::move(validated_search_directories)};

    // REMARK: The constructor for generators doesn't actually perform
    // initialization of their content.
    // Indeed, Generators use the general antipattern of the static
    // initialize-terminate non-scoped mutable state that we see in
    // many parts of QDoc.
    // In their constructor, Generators mainly register themselves into a static list.
    // Previously, this was done at the start of main.
    // To be able to pass a correct FileResolver or other systems, we
    // need to construct them after the configuration has been read
    // and has been destructured.
    // For this reason, their construction was moved here.
    // This function may be called more than once for some of QDoc's
    // call, albeit this should not actually happen in Qt's
    // documentation.
    // Then, constructing the generators here might provide for some
    // unexpected behavior as new generators are appended to the list
    // and never used, considering that the list is searched in a
    // linearly fashion and each generator of some type T, in the
    // current codebase, will always be found if another instance of
    // that same type would have been found.
    // Furthermore, those instances would be destroyed by then, such
    // that accessing them would be erroneous.
    // To avoid this, the static list was made to be cleared in
    // Generator::terminate, which, in theory, will be called before
    // the generators will be constructed again.
    // We could have used the initialize method for this, but this
    // would force us into a limited and more complex semantic, see an
    // example of this in DocParser, and would restrain us further to
    // the initialize-terminate idiom which is expect to be purged in
    // the future.
    HtmlGenerator htmlGenerator{file_resolver};
    WebXMLGenerator webXMLGenerator{file_resolver};
    DocBookGenerator docBookGenerator{file_resolver};

    Generator::initialize();

    /*
      Initialize the qdoc database, where all the parsed source files
      will be stored. The database includes a tree of nodes, which gets
      built as the source files are parsed. The documentation output is
      generated by traversing that tree.

      Note: qdocDB() allocates a new instance only if no instance exists.
      So it is safe to call qdocDB() any time.
     */
    QDocDatabase *qdb = QDocDatabase::qdocDB();
    qdb->setVersion(config.get(CONFIG_VERSION).asString());
    /*
      By default, the only output format is HTML.
     */
    const QSet<QString> outputFormats = config.getOutputFormats();

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

    // Retrieve the dependencies if loadIndexFiles() was not called
    if (config.dependModules().isEmpty()) {
        config.dependModules() = config.get(CONFIG_DEPENDS).asStringList();
        config.dependModules().removeDuplicates();
    }
    qdb->setSearchOrder(config.dependModules());

    // Store the title of the index (landing) page
    NamespaceNode *root = qdb->primaryTreeRoot();
    if (root) {
        QString title{config.get(CONFIG_NAVIGATION + Config::dot + CONFIG_LANDINGPAGE).asString()};
        root->tree()->setIndexTitle(
                config.get(CONFIG_NAVIGATION + Config::dot + CONFIG_LANDINGTITLE).asString(std::move(title)));
    }


    std::vector<QByteArray> include_paths{};
    {
        auto args = config.getCanonicalPathList(CONFIG_INCLUDEPATHS,
                                                Config::IncludePaths);
#ifdef Q_OS_MACOS
        args.append(Utilities::getInternalIncludePaths(QStringLiteral("clang++")));
#elif defined(Q_OS_LINUX)
        args.append(Utilities::getInternalIncludePaths(QStringLiteral("g++")));
#endif

        for (const auto &path : std::as_const(args)) {
            if (!path.isEmpty())
                include_paths.push_back(path.toUtf8());
        }

        include_paths.erase(std::unique(include_paths.begin(), include_paths.end()),
                            include_paths.end());
    }

    QList<QByteArray> clang_defines{};
    {
        const QStringList config_defines{config.get(CONFIG_DEFINES).asStringList()};
        for (const QString &def : config_defines) {
            if (!def.contains(QChar('*'))) {
                QByteArray tmp("-D");
                tmp.append(def.toUtf8());
                clang_defines.append(tmp.constData());
            }
        }
    }

    std::optional<PCHFile> pch = std::nullopt;
    if (config.dualExec() || config.preparing()) {
        const QString moduleHeader = config.get(CONFIG_MODULEHEADER).asString();
        pch = buildPCH(
            QDocDatabase::qdocDB(),
            moduleHeader.isNull() ? project : moduleHeader,
            Config::instance().getHeaderFiles(),
            include_paths,
            clang_defines
        );
    }

    ClangCodeParser clangParser(QDocDatabase::qdocDB(), Config::instance(), include_paths, clang_defines, pch);
    PureDocParser docParser{config.location()};

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
    Doc::initialize(file_resolver);

    if (config.dualExec() || config.preparing()) {
        QStringList sourceList;

        qCDebug(lcQdoc, "Reading sourcedirs");

        if (config.get(CONFIG_DOCUMENTATIONINHEADERS).asBool()) {
            sourceList += config.getAllFiles(CONFIG_HEADERS, CONFIG_HEADERDIRS, excludedDirs,
                                             excludedFiles);
        }
        sourceList +=
                config.getAllFiles(CONFIG_SOURCES, CONFIG_SOURCEDIRS, excludedDirs, excludedFiles);

        std::vector<QString> sources{};
        for (const auto &source : sourceList) {
            if (!source.contains(QLatin1String("doc/snippets")))
                sources.emplace_back(source);
        }
        /*
          Find all the qdoc files in the example dirs, and add
          them to the source files to be parsed.
        */
        qCDebug(lcQdoc, "Reading exampledirs");
        QStringList exampleQdocList = config.getExampleQdocFiles(excludedDirs, excludedFiles);
        for (const auto &example : exampleQdocList) {
            if (!example.contains(QLatin1String("doc/snippets")))
                sources.emplace_back(example);
        }

        /*
          Parse each source text file in the set using the appropriate parser and
          add it to the big tree.
        */
        if (config.get(CONFIG_LOGPROGRESS).asBool())
            qCInfo(lcQdoc) << "Parse source files for" << project;


        auto headers = config.getHeaderFiles();
        CppCodeParser cpp_code_parser(FnCommandParser(qdb, headers, clang_defines, pch));

        SourceFileParser source_file_parser{clangParser, docParser};
        parseSourceFiles(std::move(sources), source_file_parser, cpp_code_parser);

        if (config.get(CONFIG_LOGPROGRESS).asBool())
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
        if (generator) {
            generator->initializeFormat();
            generator->generateDocs();
        } else {
            config.get(CONFIG_OUTPUTFORMATS)
                    .location()
                    .fatal(QStringLiteral("QDoc: Unknown output format '%1'").arg(format));
        }
    }

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

/*!
    \internal
    For each file in \a qdocFiles, first clear the configured module
    dependencies and then pass the file to processQdocconfFile().

    \sa processQdocconfFile(), singleExecutionMode(), dualExecutionMode()
*/
static void clearModuleDependenciesAndProcessQdocconfFile(const QStringList &qdocFiles)
{
    for (const auto &file : std::as_const(qdocFiles)) {
        Config::instance().dependModules().clear();
        processQdocconfFile(file);
    }
}

/*!
    \internal

    A single QDoc process for prepare and generate phases.
    The purpose is to first generate all index files for all documentation
    projects that combined make out the documentation set being generated.
    This allows QDoc to link to all content contained in all projects, e.g.
    user-defined types or overview documentation, regardless of the project
    that content belongs to when generating the final output.
*/
static void singleExecutionMode()
{
    const QStringList qdocFiles = Config::loadMaster(Config::instance().qdocFiles().at(0));

    Config::instance().setQDocPass(Config::Prepare);
    clearModuleDependenciesAndProcessQdocconfFile(qdocFiles);

    Config::instance().setQDocPass(Config::Generate);
    QDocDatabase::qdocDB()->processForest();
    clearModuleDependenciesAndProcessQdocconfFile(qdocFiles);
}

/*!
    \internal

    Process each .qdocconf-file passed as command line argument(s).
*/
static void dualExecutionMode()
{
    const QStringList qdocFiles = Config::instance().qdocFiles();
    clearModuleDependenciesAndProcessQdocconfFile(qdocFiles);
}

QT_END_NAMESPACE

int main(int argc, char **argv)
{
    QT_USE_NAMESPACE

    // Initialize Qt:
#ifndef QT_BOOTSTRAPPED
    // use deterministic hash seed
    QHashSeed::setDeterministicGlobalSeed();
#endif
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));

    // Instantiate various singletons (used via static methods):
    /*
      Create code parsers for the languages to be parsed,
      and create a tree for C++.
     */
    QmlCodeParser qmlParser;

    /*
      Create code markers for plain text, C++,
      and QML.

      The plain CodeMarker must be instantiated first because it is used as
      fallback when the other markers cannot be used.

      Each marker instance is prepended to the CodeMarker::markers list by the
      base class constructor.
     */
    CodeMarker fallbackMarker;
    CppCodeMarker cppMarker;
    QmlCodeMarker qmlMarker;

    Config::instance().init("QDoc", app.arguments());

    if (Config::instance().qdocFiles().isEmpty())
        Config::instance().showHelp();

    if (Config::instance().singleExec()) {
        singleExecutionMode();
    } else {
        dualExecutionMode();
    }

    // Tidy everything away:
    QmlTypeNode::terminate();
    QDocDatabase::destroyQdocDB();
    return Location::exitCode();
}
