// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "config.h"
#include "utilities.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qvariant.h>
#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

QString ConfigStrings::AUTOLINKERRORS = QStringLiteral("autolinkerrors");
QString ConfigStrings::BUILDVERSION = QStringLiteral("buildversion");
QString ConfigStrings::CLANGDEFINES = QStringLiteral("clangdefines");
QString ConfigStrings::CODEINDENT = QStringLiteral("codeindent");
QString ConfigStrings::CODEPREFIX = QStringLiteral("codeprefix");
QString ConfigStrings::CODESUFFIX = QStringLiteral("codesuffix");
QString ConfigStrings::CPPCLASSESPAGE = QStringLiteral("cppclassespage");
QString ConfigStrings::CPPCLASSESTITLE = QStringLiteral("cppclassestitle");
QString ConfigStrings::DEFINES = QStringLiteral("defines");
QString ConfigStrings::DEPENDS = QStringLiteral("depends");
QString ConfigStrings::DESCRIPTION = QStringLiteral("description");
QString ConfigStrings::DOCBOOKEXTENSIONS = QStringLiteral("usedocbookextensions");
QString ConfigStrings::ENDHEADER = QStringLiteral("endheader");
QString ConfigStrings::EXAMPLEDIRS = QStringLiteral("exampledirs");
QString ConfigStrings::EXAMPLES = QStringLiteral("examples");
QString ConfigStrings::EXAMPLESINSTALLPATH = QStringLiteral("examplesinstallpath");
QString ConfigStrings::EXCLUDEDIRS = QStringLiteral("excludedirs");
QString ConfigStrings::EXCLUDEFILES = QStringLiteral("excludefiles");
QString ConfigStrings::EXTRAIMAGES = QStringLiteral("extraimages");
QString ConfigStrings::FALSEHOODS = QStringLiteral("falsehoods");
QString ConfigStrings::FORMATTING = QStringLiteral("formatting");
QString ConfigStrings::HEADERDIRS = QStringLiteral("headerdirs");
QString ConfigStrings::HEADERS = QStringLiteral("headers");
QString ConfigStrings::HEADERSCRIPTS = QStringLiteral("headerscripts");
QString ConfigStrings::HEADERSTYLES = QStringLiteral("headerstyles");
QString ConfigStrings::HOMEPAGE = QStringLiteral("homepage");
QString ConfigStrings::HOMETITLE = QStringLiteral("hometitle");
QString ConfigStrings::IGNOREDIRECTIVES = QStringLiteral("ignoredirectives");
QString ConfigStrings::IGNORESINCE = QStringLiteral("ignoresince");
QString ConfigStrings::IGNORETOKENS = QStringLiteral("ignoretokens");
QString ConfigStrings::IGNOREWORDS = QStringLiteral("ignorewords");
QString ConfigStrings::IMAGEDIRS = QStringLiteral("imagedirs");
QString ConfigStrings::IMAGES = QStringLiteral("images");
QString ConfigStrings::INCLUDEPATHS = QStringLiteral("includepaths");
QString ConfigStrings::INCLUSIVE = QStringLiteral("inclusive");
QString ConfigStrings::INDEXES = QStringLiteral("indexes");
QString ConfigStrings::LANDINGPAGE = QStringLiteral("landingpage");
QString ConfigStrings::LANDINGTITLE = QStringLiteral("landingtitle");
QString ConfigStrings::LANGUAGE = QStringLiteral("language");
QString ConfigStrings::LOCATIONINFO = QStringLiteral("locationinfo");
QString ConfigStrings::LOGPROGRESS = QStringLiteral("logprogress");
QString ConfigStrings::MACRO = QStringLiteral("macro");
QString ConfigStrings::MANIFESTMETA = QStringLiteral("manifestmeta");
QString ConfigStrings::MODULEHEADER = QStringLiteral("moduleheader");
QString ConfigStrings::NATURALLANGUAGE = QStringLiteral("naturallanguage");
QString ConfigStrings::NAVIGATION = QStringLiteral("navigation");
QString ConfigStrings::NOLINKERRORS = QStringLiteral("nolinkerrors");
QString ConfigStrings::OUTPUTDIR = QStringLiteral("outputdir");
QString ConfigStrings::OUTPUTFORMATS = QStringLiteral("outputformats");
QString ConfigStrings::OUTPUTPREFIXES = QStringLiteral("outputprefixes");
QString ConfigStrings::OUTPUTSUFFIXES = QStringLiteral("outputsuffixes");
QString ConfigStrings::PROJECT = QStringLiteral("project");
QString ConfigStrings::REDIRECTDOCUMENTATIONTODEVNULL =
        QStringLiteral("redirectdocumentationtodevnull");
QString ConfigStrings::QHP = QStringLiteral("qhp");
QString ConfigStrings::QUOTINGINFORMATION = QStringLiteral("quotinginformation");
QString ConfigStrings::SCRIPTS = QStringLiteral("scripts");
QString ConfigStrings::SHOWINTERNAL = QStringLiteral("showinternal");
QString ConfigStrings::SINGLEEXEC = QStringLiteral("singleexec");
QString ConfigStrings::SOURCEDIRS = QStringLiteral("sourcedirs");
QString ConfigStrings::SOURCEENCODING = QStringLiteral("sourceencoding");
QString ConfigStrings::SOURCES = QStringLiteral("sources");
QString ConfigStrings::SPURIOUS = QStringLiteral("spurious");
QString ConfigStrings::STYLESHEETS = QStringLiteral("stylesheets");
QString ConfigStrings::SYNTAXHIGHLIGHTING = QStringLiteral("syntaxhighlighting");
QString ConfigStrings::TABSIZE = QStringLiteral("tabsize");
QString ConfigStrings::TAGFILE = QStringLiteral("tagfile");
QString ConfigStrings::TIMESTAMPS = QStringLiteral("timestamps");
QString ConfigStrings::TOCTITLES = QStringLiteral("toctitles");
QString ConfigStrings::URL = QStringLiteral("url");
QString ConfigStrings::VERSION = QStringLiteral("version");
QString ConfigStrings::VERSIONSYM = QStringLiteral("versionsym");
QString ConfigStrings::FILEEXTENSIONS = QStringLiteral("fileextensions");
QString ConfigStrings::IMAGEEXTENSIONS = QStringLiteral("imageextensions");
QString ConfigStrings::QMLTYPESPAGE = QStringLiteral("qmltypespage");
QString ConfigStrings::QMLTYPESTITLE = QStringLiteral("qmltypestitle");
QString ConfigStrings::WARNINGLIMIT = QStringLiteral("warninglimit");

/*!
  An entry in a stack, where each entry is a list
  of string values.
 */
class MetaStackEntry
{
public:
    void open();
    void close();

    QStringList accum;
    QStringList next;
};
Q_DECLARE_TYPEINFO(MetaStackEntry, Q_RELOCATABLE_TYPE);

/*!
  Start accumulating values in a list by appending an empty
  string to the list.
 */
void MetaStackEntry::open()
{
    next.append(QString());
}

/*!
  Stop accumulating values and append the list of accumulated
  values to the complete list of accumulated values.

 */
void MetaStackEntry::close()
{
    accum += next;
    next.clear();
}

/*!
  \class MetaStack

  This class maintains a stack of values of config file variables.
*/
class MetaStack : private QStack<MetaStackEntry>
{
public:
    MetaStack();

    void process(QChar ch, const Location &location);
    QStringList getExpanded(const Location &location);
};

/*!
  The default constructor pushes a new stack entry and
  opens it.
 */
MetaStack::MetaStack()
{
    push(MetaStackEntry());
    top().open();
}

/*!
  Processes the character \a ch using the \a location.
  It really just builds up a name by appending \a ch to
  it.
 */
void MetaStack::process(QChar ch, const Location &location)
{
    if (ch == QLatin1Char('{')) {
        push(MetaStackEntry());
        top().open();
    } else if (ch == QLatin1Char('}')) {
        if (size() == 1)
            location.fatal(QStringLiteral("Unexpected '}'"));

        top().close();
        const QStringList suffixes = pop().accum;
        const QStringList prefixes = top().next;

        top().next.clear();
        for (const auto &prefix : prefixes) {
            for (const auto &suffix : suffixes)
                top().next << prefix + suffix;
        }
    } else if (ch == QLatin1Char(',') && size() > 1) {
        top().close();
        top().open();
    } else {
        for (QString &topNext : top().next)
            topNext += ch;
    }
}

/*!
  Returns the accumulated string values.
 */
QStringList MetaStack::getExpanded(const Location &location)
{
    if (size() > 1)
        location.fatal(QStringLiteral("Missing '}'"));

    top().close();
    return top().accum;
}

const QString Config::dot = QLatin1String(".");
bool Config::m_debug = false;
bool Config::m_atomsDump = false;
bool Config::generateExamples = true;
QString Config::overrideOutputDir;
QString Config::installDir;
QSet<QString> Config::overrideOutputFormats;
QMap<QString, QString> Config::m_extractedDirs;
QStack<QString> Config::m_workingDirs;
QMap<QString, QStringList> Config::m_includeFilesMap;

/*!
  \class ConfigVar
  \brief contains all the information for a single config variable in a
         .qdocconf file.
*/

/*!
  Returns this configuration variable as a string.

  If the variable is not defined, returns \a defaultString.

  \note By default, \a defaultString is a null string.
  This allows determining whether a configuration variable is
  undefined (returns a null string) or defined as empty
  (returns a non-null, empty string).
*/
QString ConfigVar::asString(const QString defaultString) const
{
    if (m_name.isEmpty())
        return defaultString;

    QString result(""); // an empty but non-null string
    for (const auto &value : std::as_const(m_values)) {
        if (!result.isEmpty() && !result.endsWith(QChar('\n')))
            result.append(QChar(' '));
        result.append(value.m_value);
    }
    return result;
}

/*!
  Returns this config variable as a string list.
*/
QStringList ConfigVar::asStringList() const
{
    QStringList result;
    for (const auto &value : std::as_const(m_values))
        result << value.m_value;
    return result;
}

/*!
  Returns this config variable as a string set.
*/
QSet<QString> ConfigVar::asStringSet() const
{
    const auto &stringList = asStringList();
    return QSet<QString>(stringList.cbegin(), stringList.cend());
}

/*!
  Returns this config variable as a boolean.
*/
bool ConfigVar::asBool() const
{
    return QVariant(asString()).toBool();
}

/*!
  Returns this configuration variable as an integer; iterates
  through the string list, interpreting each
  string in the list as an integer and adding it to a total sum.

  Returns 0 if this variable is defined as empty, and
  -1 if it's is not defined.
 */
int ConfigVar::asInt() const
{
    const QStringList strs = asStringList();
    if (strs.isEmpty())
        return -1;

    int sum = 0;
    for (const auto &str : strs)
        sum += str.toInt();
    return sum;
}

/*!
  Appends values to this ConfigVar, and adjusts the ExpandVar
  parameters so that they continue to refer to the correct values.
*/
void ConfigVar::append(const ConfigVar &other)
{
    m_expandVars << other.m_expandVars;
    QList<ExpandVar>::Iterator it = m_expandVars.end();
    it -= other.m_expandVars.size();
    std::for_each(it, m_expandVars.end(), [this](ExpandVar &v) {
        v.m_valueIndex += m_values.size();
    });
    m_values << other.m_values;
    m_location = other.m_location;
}

/*!
  \class Config
  \brief The Config class contains the configuration variables
  for controlling how qdoc produces documentation.

  Its load() function reads, parses, and processes a qdocconf file.
 */

/*!
  \enum Config::PathFlags

  Flags used for retrieving canonicalized paths from Config.

  \value Validate
         Issue a warning for paths that do not exist and
         remove them from the returned list.

  \value IncludePaths
         Assume the variable contains include paths with
         prefixes such as \c{-I} that are to be removed
         before canonicalizing and then re-inserted.

  \omitvalue None

  \sa getCanonicalPathList()
*/

/*!
  Initializes the Config with \a programName and sets all
  internal state variables to either default values or to ones
  defined in command line arguments \a args.
 */
void Config::init(const QString &programName, const QStringList &args)
{
    m_prog = programName;
    processCommandLineOptions(args);
    reset();
}

Config::~Config()
{
    clear();
}

/*!
  Clears the location and internal maps for config variables.
 */
void Config::clear()
{
    m_location = Location();
    m_configVars.clear();
    m_includeFilesMap.clear();
}

/*!
  Resets the Config instance - used by load()
 */
void Config::reset()
{
    clear();

    // Default values
    setStringList(CONFIG_CODEINDENT, QStringList("0"));
    setStringList(CONFIG_FALSEHOODS, QStringList("0"));
    setStringList(CONFIG_HEADERS + dot + CONFIG_FILEEXTENSIONS, QStringList("*.ch *.h *.h++ *.hh *.hpp *.hxx"));
    setStringList(CONFIG_SOURCES + dot + CONFIG_FILEEXTENSIONS, QStringList("*.c++ *.cc *.cpp *.cxx *.mm *.qml *.qdoc"));
    setStringList(CONFIG_LANGUAGE, QStringList("Cpp")); // i.e. C++
    setStringList(CONFIG_OUTPUTFORMATS, QStringList("HTML"));
    setStringList(CONFIG_TABSIZE, QStringList("8"));
    setStringList(CONFIG_LOCATIONINFO, QStringList("true"));

    // Publish options from the command line as config variables
    const auto setListFlag = [this](const QString &key, bool test) {
        setStringList(key, QStringList(test ? QStringLiteral("true") : QStringLiteral("false")));
    };
#define SET(opt, test) setListFlag(opt, m_parser.isSet(m_parser.test))
    SET(CONFIG_SYNTAXHIGHLIGHTING, highlightingOption);
    SET(CONFIG_SHOWINTERNAL, showInternalOption);
    SET(CONFIG_SINGLEEXEC, singleExecOption);
    SET(CONFIG_REDIRECTDOCUMENTATIONTODEVNULL, redirectDocumentationToDevNullOption);
    SET(CONFIG_AUTOLINKERRORS, autoLinkErrorsOption);
#undef SET
    m_showInternal = m_configVars.value(CONFIG_SHOWINTERNAL).asBool();
    setListFlag(CONFIG_NOLINKERRORS,
                m_parser.isSet(m_parser.noLinkErrorsOption)
                        || qEnvironmentVariableIsSet("QDOC_NOLINKERRORS"));

    // CONFIG_DEFINES and CONFIG_INCLUDEPATHS are set in load()
}

/*!
  Loads and parses the qdoc configuration file \a fileName.
  If a previous project was loaded, this function first resets the
  Config instance. Then it calls the other load() function, which
  does the loading, parsing, and processing of the configuration file.
 */
void Config::load(const QString &fileName)
{
    // Reset if a previous project was loaded
    if (m_configVars.contains(CONFIG_PROJECT))
        reset();

    load(Location(), fileName);
    if (m_location.isEmpty())
        m_location = Location(fileName);
    else
        m_location.setEtc(true);

    expandVariables();

    // Add defines and includepaths from command line to their
    // respective configuration variables. Values set here are
    // always added to what's defined in configuration file.
    insertStringList(CONFIG_DEFINES, m_defines);
    insertStringList(CONFIG_INCLUDEPATHS, m_includePaths);

    // Prefetch values that are used internally
    m_exampleFiles = getCanonicalPathList(CONFIG_EXAMPLES);
    m_exampleDirs = getCanonicalPathList(CONFIG_EXAMPLEDIRS);
}

/*!
    Expands other config variables referred to in all stored ConfigVars.
*/
void Config::expandVariables()
{
     for (auto &configVar : m_configVars) {
        for (auto it = configVar.m_expandVars.crbegin(); it != configVar.m_expandVars.crend(); ++it) {
            Q_ASSERT(it->m_valueIndex < configVar.m_values.size());
            const QString &key = it->m_var;
            const auto &refVar = m_configVars.value(key);
            if (refVar.m_name.isEmpty()) {
                configVar.m_location.fatal(
                        QStringLiteral("Environment or configuration variable '%1' undefined")
                                .arg(it->m_var));
            } else if (!refVar.m_expandVars.empty()) {
                configVar.m_location.fatal(
                        QStringLiteral("Nested variable expansion not allowed"),
                        QStringLiteral("When expanding '%1' at %2:%3")
                                .arg(refVar.m_name, refVar.m_location.filePath(),
                                     QString::number(refVar.m_location.lineNo())));
            }
            QString expanded;
            if (it->m_delim.isNull())
                expanded = m_configVars.value(key).asStringList().join(QString());
            else
                expanded = m_configVars.value(key).asStringList().join(it->m_delim);
            configVar.m_values[it->m_valueIndex].m_value.insert(it->m_index, expanded);
        }
        configVar.m_expandVars.clear();
     }
}

/*!
  Sets the \a values of a configuration variable \a var from a string list.
 */
void Config::setStringList(const QString &var, const QStringList &values)
{
    m_configVars.insert(var, ConfigVar(var, values, QDir::currentPath()));
}

/*!
  Adds the \a values from a string list to the configuration variable \a var.
  Existing value(s) are kept.
*/
void Config::insertStringList(const QString &var, const QStringList &values)
{
    m_configVars[var].append(ConfigVar(var, values, QDir::currentPath()));
}

/*!
  Process and store variables from the command line.
 */
void Config::processCommandLineOptions(const QStringList &args)
{
    m_parser.process(args);

    m_defines = m_parser.values(m_parser.defineOption);
    m_dependModules = m_parser.values(m_parser.dependsOption);
    setIndexDirs();
    setIncludePaths();

    generateExamples = !m_parser.isSet(m_parser.noExamplesOption);
    if (m_parser.isSet(m_parser.installDirOption))
        installDir = m_parser.value(m_parser.installDirOption);
    if (m_parser.isSet(m_parser.outputDirOption))
        overrideOutputDir = m_parser.value(m_parser.outputDirOption);

    const auto outputFormats = m_parser.values(m_parser.outputFormatOption);
    for (const auto &format : outputFormats)
        overrideOutputFormats.insert(format);
    m_debug = m_parser.isSet(m_parser.debugOption) || qEnvironmentVariableIsSet("QDOC_DEBUG");
    m_atomsDump = m_parser.isSet(m_parser.atomsDumpOption);
    m_showInternal = m_parser.isSet(m_parser.showInternalOption)
            || qEnvironmentVariableIsSet("QDOC_SHOW_INTERNAL");

    if (m_parser.isSet(m_parser.prepareOption))
        m_qdocPass = Prepare;
    if (m_parser.isSet(m_parser.generateOption))
        m_qdocPass = Generate;
    if (m_debug || m_parser.isSet(m_parser.logProgressOption))
        setStringList(CONFIG_LOGPROGRESS, QStringList("true"));
    if (m_parser.isSet(m_parser.timestampsOption))
        setStringList(CONFIG_TIMESTAMPS, QStringList("true"));
    if (m_parser.isSet(m_parser.useDocBookExtensions))
        setStringList(CONFIG_DOCBOOKEXTENSIONS, QStringList("true"));
}

void Config::setIncludePaths()
{
    QDir currentDir = QDir::current();
    const auto addIncludePaths = [this, currentDir](const char *flag, const QStringList &paths) {
        for (const auto &path : paths)
            m_includePaths << currentDir.absoluteFilePath(path).insert(0, flag);
    };

    addIncludePaths("-I", m_parser.values(m_parser.includePathOption));
#ifdef QDOC_PASS_ISYSTEM
    addIncludePaths("-isystem", m_parser.values(m_parser.includePathSystemOption));
#endif
    addIncludePaths("-F", m_parser.values(m_parser.frameworkOption));
}

/*!
  Stores paths from -indexdir command line option(s).
 */
void Config::setIndexDirs()
{
    m_indexDirs = m_parser.values(m_parser.indexDirOption);
    auto it = std::remove_if(m_indexDirs.begin(), m_indexDirs.end(),
                             [](const QString &s) { return !QFile::exists(s); });

    std::for_each(it, m_indexDirs.end(), [](const QString &s) {
        qCWarning(lcQdoc) << "Cannot find index directory: " << s;
    });
    m_indexDirs.erase(it, m_indexDirs.end());
}

/*!
  Function to return the correct outputdir for the output \a format.
  If \a format is not specified, defaults to 'HTML'.
  outputdir can be set using the qdocconf or the command-line
  variable -outputdir.
  */
QString Config::getOutputDir(const QString &format) const
{
    QString t;
    if (overrideOutputDir.isNull())
        t = m_configVars.value(CONFIG_OUTPUTDIR).asString();
    else
        t = overrideOutputDir;
    if (m_configVars.value(CONFIG_SINGLEEXEC).asBool()) {
        QString project = m_configVars.value(CONFIG_PROJECT).asString();
        t += QLatin1Char('/') + project.toLower();
    }
    if (m_configVars.value(format + Config::dot + "nosubdirs").asBool()) {
        t = t.left(t.lastIndexOf('/'));
        QString singleOutputSubdir = m_configVars.value(format + Config::dot + "outputsubdir").asString();
        if (singleOutputSubdir.isEmpty())
            singleOutputSubdir = "html";
        t += QLatin1Char('/') + singleOutputSubdir;
    }
    return t;
}

/*!
  Function to return the correct outputformats.
  outputformats can be set using the qdocconf or the command-line
  variable -outputformat.
  */
QSet<QString> Config::getOutputFormats() const
{
    if (overrideOutputFormats.isEmpty())
        return m_configVars.value(CONFIG_OUTPUTFORMATS).asStringSet();
    else
        return overrideOutputFormats;
}

// TODO: [late-canonicalization][pod-configuration]
// The canonicalization for paths is done at the time where they are
// required, and done each time they are requested.
// Instead, config should be parsed to an intermediate format that is
// a POD type that already contains canonicalized representations for
// each element.
// Those representations should provide specific guarantees about
// their format and be representable at the API boundaries.
//
// This would ensure that the correct canonicalization is always
// applied, is applied only once and that dependent sub-logics can be
// written in a way that doesn't require branching or futher
// canonicalization.

/*!
   Returns a path list where all paths from the config variable \a var
   are canonicalized. If \a flags contains \c Validate, outputs a warning
   for invalid paths. The \c IncludePaths flag is used as a hint to strip
   away potential prefixes found in include paths before attempting to
   canonicalize.
 */
QStringList Config::getCanonicalPathList(const QString &var, PathFlags flags) const
{
    QStringList result;
    const auto &configVar = m_configVars.value(var);

    for (const auto &value : configVar.m_values) {
        const QString &currentPath = value.m_path;
        QString rawValue = value.m_value.simplified();
        QString prefix;

        if (flags & IncludePaths) {
            const QStringList prefixes = QStringList()
                    << QLatin1String("-I")
                    << QLatin1String("-F")
                    << QLatin1String("-isystem");
            const auto end = std::end(prefixes);
            const auto it =
                std::find_if(std::begin(prefixes), end,
                     [&rawValue](const QString &p) {
                        return rawValue.startsWith(p);
                });
            if (it != end) {
                prefix = *it;
                rawValue.remove(0, it->size());
                if (rawValue.isEmpty())
                    continue;
            } else {
                prefix = prefixes[0]; // -I as default
            }
        }

        QDir dir(rawValue.trimmed());
        const QString path = dir.path();

        if (dir.isRelative())
            dir.setPath(currentPath + QLatin1Char('/') + path);
        if ((flags & Validate) && !QFileInfo::exists(dir.path()))
            configVar.m_location.warning(QStringLiteral("Cannot find file or directory: %1").arg(path));
        else {
            const QString canonicalPath = dir.canonicalPath();
            if (!canonicalPath.isEmpty())
                result.append(prefix + canonicalPath);
            else if (path.contains(QLatin1Char('*')) || path.contains(QLatin1Char('?')))
                result.append(path);
            else
                qCDebug(lcQdoc) <<
                        qUtf8Printable(QStringLiteral("%1: Ignored nonexistent path \'%2\'")
                                .arg(configVar.m_location.toString(), rawValue));
        }
    }
    return result;
}

/*!
  Calls getRegExpList() with the control variable \a var and
  iterates through the resulting list of regular expressions,
  concatenating them with extra characters to form a single
  QRegularExpression, which is then returned.

  \sa getRegExpList()
 */
QRegularExpression Config::getRegExp(const QString &var) const
{
    QString pattern;
    const auto subRegExps = getRegExpList(var);

    for (const auto &regExp : subRegExps) {
        if (!regExp.isValid())
            return regExp;
        if (!pattern.isEmpty())
            pattern += QLatin1Char('|');
        pattern += QLatin1String("(?:") + regExp.pattern() + QLatin1Char(')');
    }
    if (pattern.isEmpty())
        pattern = QLatin1String("$x"); // cannot match
    return QRegularExpression(pattern);
}

/*!
  Looks up the configuration variable \a var in the string list
  map, converts the string list to a list of regular expressions,
  and returns it.
 */
QList<QRegularExpression> Config::getRegExpList(const QString &var) const
{
    const QStringList strs = m_configVars.value(var).asStringList();
    QList<QRegularExpression> regExps;
    for (const auto &str : strs)
        regExps += QRegularExpression(str);
    return regExps;
}

/*!
  This function is slower than it could be. What it does is
  find all the keys that begin with \a var + dot and return
  the matching keys in a set, stripped of the matching prefix
  and dot.
 */
QSet<QString> Config::subVars(const QString &var) const
{
    QSet<QString> result;
    QString varDot = var + QLatin1Char('.');
    for (auto it = m_configVars.constBegin(); it != m_configVars.constEnd(); ++it) {
        if (it.key().startsWith(varDot)) {
            QString subVar = it.key().mid(varDot.size());
            int dot = subVar.indexOf(QLatin1Char('.'));
            if (dot != -1)
                subVar.truncate(dot);
            result.insert(subVar);
        }
    }
    return result;
}

/*!
  Searches for a path to \a fileName in 'sources', 'sourcedirs', and
  'exampledirs' config variables and returns a full path to the first
  match found. If the file is not found, returns an empty string.
 */
QString Config::getIncludeFilePath(const QString &fileName) const
{
    QString ext = QFileInfo(fileName).suffix();

    if (!m_includeFilesMap.contains(ext)) {
        QStringList result = getCanonicalPathList(CONFIG_SOURCES);
        result.erase(std::remove_if(result.begin(), result.end(),
                     [&](const QString &s) { return !s.endsWith(ext); }),
                    result.end());
        const QStringList dirs =
            getCanonicalPathList(CONFIG_SOURCEDIRS) +
            getCanonicalPathList(CONFIG_EXAMPLEDIRS);

        for (const auto &dir : dirs)
            result += getFilesHere(dir, "*." + ext, location());
        result.removeDuplicates();
        m_includeFilesMap.insert(ext, result);
    }
    const QStringList &paths = (*m_includeFilesMap.find(ext));
    QString match = fileName;
    if (!match.startsWith('/'))
        match.prepend('/');
    for (const auto &path : paths) {
        if (path.endsWith(match))
            return path;
    }
    return QString();
}

/*!
  Builds and returns a list of file pathnames for the file
  type specified by \a filesVar (e.g. "headers" or "sources").
  The files are found in the directories specified by
  \a dirsVar, and they are filtered by \a defaultNameFilter
  if a better filter can't be constructed from \a filesVar.
  The directories in \a excludedDirs are avoided. The files
  in \a excludedFiles are not included in the return list.
 */
QStringList Config::getAllFiles(const QString &filesVar, const QString &dirsVar,
                                const QSet<QString> &excludedDirs,
                                const QSet<QString> &excludedFiles)
{
    QStringList result = getCanonicalPathList(filesVar, Validate);
    const QStringList dirs = getCanonicalPathList(dirsVar, Validate);

    const QString nameFilter = m_configVars.value(filesVar + dot + CONFIG_FILEEXTENSIONS).asString();

    for (const auto &dir : dirs)
        result += getFilesHere(dir, nameFilter, location(), excludedDirs, excludedFiles);
    return result;
}

QStringList Config::getExampleQdocFiles(const QSet<QString> &excludedDirs,
                                        const QSet<QString> &excludedFiles)
{
    QStringList result;
    const QStringList dirs = getCanonicalPathList("exampledirs");
    const QString nameFilter = " *.qdoc";

    for (const auto &dir : dirs)
        result += getFilesHere(dir, nameFilter, location(), excludedDirs, excludedFiles);
    return result;
}

QStringList Config::getExampleImageFiles(const QSet<QString> &excludedDirs,
                                         const QSet<QString> &excludedFiles)
{
    QStringList result;
    const QStringList dirs = getCanonicalPathList("exampledirs");
    const QString nameFilter = m_configVars.value(CONFIG_EXAMPLES + dot + CONFIG_IMAGEEXTENSIONS).asString();

    for (const auto &dir : dirs)
        result += getFilesHere(dir, nameFilter, location(), excludedDirs, excludedFiles);
    return result;
}

// TODO: [misplaced-logic][examples][pod-configuration]
// The definition of how an example is structured and how to find its
// components should not be part of Config or, for that matter,
// CppCodeParser, which is the actual caller of this method.
// Move this method to a more appropriate place as soon as a suitable
// place is available for it.

/*!
    Returns the path to the project file for \a examplePath, or an empty string
    if no project file was found.
 */
QString Config::getExampleProjectFile(const QString &examplePath)
{
    QFileInfo fileInfo(examplePath);
    QStringList validNames;
    validNames << QLatin1String("CMakeLists.txt")
               << fileInfo.fileName() + QLatin1String(".pro")
               << fileInfo.fileName() + QLatin1String(".qmlproject")
               << fileInfo.fileName() + QLatin1String(".pyproject")
               << QLatin1String("qbuild.pro"); // legacy

    QString projectFile;

    for (const auto &name : std::as_const(validNames)) {
        projectFile = Config::findFile(Location(), m_exampleFiles, m_exampleDirs,
                                       examplePath + QLatin1Char('/') + name);
        if (!projectFile.isEmpty())
            return projectFile;
    }

    return projectFile;
}

// TODO: [pod-configuration]
// Remove findFile completely from the configuration.
// External usages of findFile were already removed but a last caller
// of this method exists internally to Config in
// `getExampleProjectFile`.
// That method has to be removed at some point and this method should
// go with it.
// Do notice that FileResolver is the replacement for findFile but it
// is designed, for now, with a scope that does only care about the
// usages of findFile that are outside the Config class.
// More specifically, it was designed to replace only the uses of
// findFile that deal with user provided queries or queries related to
// that.
// The logic that is used internally in Config is the same, but has a
// different conceptual meaning.
// When findFile is permanently removed, it must be considered whether
// FileResolver itself should be used for the same logic or not.

/*!
  \a fileName is the path of the file to find.

  \a files and \a dirs are the lists where we must find the
  components of \a fileName.

  \a location is used for obtaining the file and line numbers
  for report qdoc errors.
 */
QString Config::findFile(const Location &location, const QStringList &files,
                         const QStringList &dirs, const QString &fileName,
                         QString *userFriendlyFilePath)
{
    if (fileName.isEmpty() || fileName.startsWith(QLatin1Char('/'))) {
        if (userFriendlyFilePath)
            *userFriendlyFilePath = fileName;
        return fileName;
    }

    QFileInfo fileInfo;
    QStringList components = fileName.split(QLatin1Char('?'));
    QString firstComponent = components.first();

    for (const auto &file : files) {
        if (file == firstComponent || file.endsWith(QLatin1Char('/') + firstComponent)) {
            fileInfo.setFile(file);
            if (!fileInfo.exists())
                location.fatal(QStringLiteral("File '%1' does not exist").arg(file));
            break;
        }
    }

    if (fileInfo.fileName().isEmpty()) {
        for (const auto &dir : dirs) {
            fileInfo.setFile(QDir(dir), firstComponent);
            if (fileInfo.exists())
                break;
        }
    }

    if (userFriendlyFilePath)
        userFriendlyFilePath->clear();
    if (!fileInfo.exists())
        return QString();

    // <<REMARK: This is actually dead code. It is unclear what it tries
    // to do and why but its usage is unnecessary in the current
    // codebase.
    // Indeed, the whole concept of the "userFriendlyFilePath" is
    // removed for file searching.
    // It will be removed directly with the whole of findFile, but it
    // should not be considered anymore until then.
    if (userFriendlyFilePath) {
        for (auto c = components.constBegin();;) {
            bool isArchive = (c != components.constEnd() - 1);
            userFriendlyFilePath->append(*c);

            if (isArchive) {
                QString extracted = m_extractedDirs[fileInfo.filePath()];

                ++c;
                fileInfo.setFile(QDir(extracted), *c);
            } else {
                break;
            }

            userFriendlyFilePath->append(QLatin1Char('?'));
        }
    }
    // REMARK>>

    return fileInfo.filePath();
}

// TODO: [pod-configuration]
// An intermediate representation for the configuration should only
// contain data that will later be destructured into subsystem that
// care about specific subsets of the configuration and can carry that
// information with them, uniquely.
// Remove copyFile, moving it into whatever will have the unique
// resposability of knowing how to build an output directory for a
// QDoc execution.
// Should copy file being used for not only copying file to the build
// output directory, split its responsabilities into smaller elements
// instead of forcing the logic together.

/*!
  Copies the \a sourceFilePath to the file name constructed by
  concatenating \a targetDirPath and the file name from the
  \a userFriendlySourceFilePath. \a location is for identifying
  the file and line number where a qdoc error occurred. The
  constructed output file name is returned.
 */
QString Config::copyFile(const Location &location, const QString &sourceFilePath,
                         const QString &userFriendlySourceFilePath, const QString &targetDirPath)
{
    // TODO: A copying operation should only be performed on files
    // that we assume to be available. Ensure that this is true at the
    // API boundary and bubble up the error checking and reporting to
    // call-site users. Possibly this will be as simple as
    // ResolvedFile, but could not be done at the time of the introduction of
    // that type as we first need to encapsulate the logic for
    // copying files into an appropriate subsystem and have a better
    // understanding of call-site usages.

    QFile inFile(sourceFilePath);
    if (!inFile.open(QFile::ReadOnly)) {
        location.warning(QStringLiteral("Cannot open input file for copy: '%1': %2")
                                 .arg(sourceFilePath, inFile.errorString()));
        return QString();
    }

    // TODO: [non-canonical-representation]
    // Similar to other part of QDoc, we do a series of non-intuitive
    // checks to canonicalize some multi-format parameter into
    // something we can use.
    // Understand which of those formats are actually in use and
    // provide a canonicalized version that can be requested at the
    // API boundary to ensure that correct formatting is used.
    // If possible, gradually bubble up the canonicalization until a
    // single entry-point in the program exists where the
    // canonicalization can be processed to avoid complicating
    // intermediate steps.
    // ADDENDUM 1: At least one usage of this seems to depend on the
    // processing done for files coming from
    // Generator::copyTemplateFile, which are expressed as absolute
    // paths. This seems to be the only usage that is currently
    // needed, hence a temporary new implementation is provided that
    // only takes this case into account.
    // Do notice that we assume that in this case we always want a
    // flat structure, that is, we are copying the file as a direct
    // child of the target directory.
    // Nonetheless, it is possible that this case will not be needed,
    // such that it can be removed later on, or that it will be nedeed
    // in multiple places such that an higher level interface for it
    // should be provided.
    // Furthermoe, it might be possible that there is an edge case
    // that is now not considered, as it is unknown, that was
    // considered before.
    // As it is now unclear what kind of paths are used here, what
    // format they have, why they are used and why they have some
    // specific format, further processing is avoided but a more
    // torough overview of what should is needed must be done when
    // more information are gathered and this function is extracted
    // away from config.

    QString outFileName{userFriendlySourceFilePath};
    QFileInfo outFileNameInfo{userFriendlySourceFilePath};
    if (outFileNameInfo.isAbsolute())
        outFileName = outFileNameInfo.fileName();

    outFileName = targetDirPath + "/" + outFileName;
    QDir targetDir(targetDirPath);
    if (!targetDir.exists())
        targetDir.mkpath(".");

    QFile outFile(outFileName);
    if (!outFile.open(QFile::WriteOnly)) {
        // TODO: [uncrentralized-warning]
        location.warning(QStringLiteral("Cannot open output file for copy: '%1': %2")
                                 .arg(outFileName, outFile.errorString()));
        return QString();
    }

    // TODO: There shouldn't be any particular advantage to copying
    // the file by readying its content and writing it compared to
    // asking the underlying system to do the copy for us.
    // Consider simplifying this part by avoiding doing the manual
    // work ourselves.

    char buffer[1024];
    qsizetype len;
    while ((len = inFile.read(buffer, sizeof(buffer))) > 0)
        outFile.write(buffer, len);
    return outFileName;
}

/*!
  Finds the largest unicode digit in \a value in the range
  1..7 and returns it.
 */
int Config::numParams(const QString &value)
{
    int max = 0;
    for (int i = 0; i != value.size(); ++i) {
        uint c = value[i].unicode();
        if (c > 0 && c < 8)
            max = qMax(max, static_cast<int>(c));
    }
    return max;
}

/*!
  Returns \c true if \a ch is a letter, number, '_', '.',
  '{', '}', or ','.
 */
bool Config::isMetaKeyChar(QChar ch)
{
    return ch.isLetterOrNumber() || ch == QLatin1Char('_') || ch == QLatin1Char('.')
            || ch == QLatin1Char('{') || ch == QLatin1Char('}') || ch == QLatin1Char(',');
}

/*!
  \a fileName is a master qdocconf file. It contains a list of
  qdocconf files and nothing else. Read the list and return it.
 */
QStringList Config::loadMaster(const QString &fileName)
{
    Location location;
    QFile fin(fileName);
    if (!fin.open(QFile::ReadOnly | QFile::Text)) {
        if (!Config::installDir.isEmpty()) {
            qsizetype prefix = location.filePath().size() - location.fileName().size();
            fin.setFileName(Config::installDir + QLatin1Char('/')
                            + fileName.right(fileName.size() - prefix));
        }
        if (!fin.open(QFile::ReadOnly | QFile::Text))
            location.fatal(QStringLiteral("Cannot open master qdocconf file '%1': %2")
                                   .arg(fileName, fin.errorString()));
    }
    QTextStream stream(&fin);
    QStringList qdocFiles;
    QDir configDir(QFileInfo(fileName).canonicalPath());
    QString line = stream.readLine();
    while (!line.isNull()) {
        if (!line.isEmpty())
            qdocFiles.append(QFileInfo(configDir, line).filePath());
        line = stream.readLine();
    }
    fin.close();
    return qdocFiles;
}

/*!
  Load, parse, and process a qdoc configuration file. This
  function is only called by the other load() function, but
  this one is recursive, i.e., it calls itself when it sees
  an \c{include} statement in the qdoc configuration file.
 */
void Config::load(Location location, const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    pushWorkingDir(fileInfo.canonicalPath());
    static const QRegularExpression keySyntax(QRegularExpression::anchoredPattern(QLatin1String("\\w+(?:\\.\\w+)*")));

#define SKIP_CHAR()                                                                                \
    do {                                                                                           \
        location.advance(c);                                                                       \
        ++i;                                                                                       \
        c = text.at(i);                                                                            \
        cc = c.unicode();                                                                          \
    } while (0)

#define SKIP_SPACES()                                                                              \
    while (c.isSpace() && cc != '\n')                                                              \
    SKIP_CHAR()

#define PUT_CHAR()                                                                                 \
    word += c;                                                                                     \
    SKIP_CHAR();

    if (location.depth() > 16)
        location.fatal(QStringLiteral("Too many nested includes"));

    QFile fin(fileInfo.fileName());
    if (!fin.open(QFile::ReadOnly | QFile::Text)) {
        if (!Config::installDir.isEmpty()) {
            qsizetype prefix = location.filePath().size() - location.fileName().size();
            fin.setFileName(Config::installDir + QLatin1Char('/')
                            + fileName.right(fileName.size() - prefix));
        }
        if (!fin.open(QFile::ReadOnly | QFile::Text))
            location.fatal(
                    QStringLiteral("Cannot open file '%1': %2").arg(fileName, fin.errorString()));
    }

    QTextStream stream(&fin);
    QString text = stream.readAll();
    text += QLatin1String("\n\n");
    text += QLatin1Char('\0');
    fin.close();

    location.push(fileName);
    location.start();

    int i = 0;
    QChar c = text.at(0);
    uint cc = c.unicode();
    while (i < text.size()) {
        if (cc == 0) {
            ++i;
        } else if (c.isSpace()) {
            SKIP_CHAR();
        } else if (cc == '#') {
            do {
                SKIP_CHAR();
            } while (cc != '\n');
        } else if (isMetaKeyChar(c)) {
            Location keyLoc = location;
            bool plus = false;
            QStringList rhsValues;
            QList<ExpandVar> expandVars;
            QString word;
            bool inQuote = false;
            bool needsExpansion = false;

            MetaStack stack;
            do {
                stack.process(c, location);
                SKIP_CHAR();
            } while (isMetaKeyChar(c));

            const QStringList keys = stack.getExpanded(location);
            SKIP_SPACES();

            if (keys.size() == 1 && keys.first() == QLatin1String("include")) {
                QString includeFile;

                if (cc != '(')
                    location.fatal(QStringLiteral("Bad include syntax"));
                SKIP_CHAR();
                SKIP_SPACES();

                while (!c.isSpace() && cc != '#' && cc != ')') {

                    if (cc == '$') {
                        QString var;
                        SKIP_CHAR();
                        while (c.isLetterOrNumber() || cc == '_') {
                            var += c;
                            SKIP_CHAR();
                        }
                        if (!var.isEmpty()) {
                            const QByteArray val = qgetenv(var.toLatin1().data());
                            if (val.isNull()) {
                                location.fatal(QStringLiteral("Environment variable '%1' undefined")
                                                       .arg(var));
                            } else {
                                includeFile += QString::fromLatin1(val);
                            }
                        }
                    } else {
                        includeFile += c;
                        SKIP_CHAR();
                    }
                }
                SKIP_SPACES();
                if (cc != ')')
                    location.fatal(QStringLiteral("Bad include syntax"));
                SKIP_CHAR();
                SKIP_SPACES();
                if (cc != '#' && cc != '\n')
                    location.fatal(QStringLiteral("Trailing garbage"));

                /*
                  Here is the recursive call.
                 */
                load(location, QFileInfo(QDir(m_workingDirs.top()), includeFile).filePath());
            } else {
                /*
                  It wasn't an include statement, so it's something else.
                  We must see either '=' or '+=' next. If not, fatal error.
                 */
                if (cc == '+') {
                    plus = true;
                    SKIP_CHAR();
                }
                if (cc != '=')
                    location.fatal(QStringLiteral("Expected '=' or '+=' after key"));
                SKIP_CHAR();
                SKIP_SPACES();

                for (;;) {
                    if (cc == '\\') {
                        qsizetype metaCharPos;

                        SKIP_CHAR();
                        if (cc == '\n') {
                            SKIP_CHAR();
                        } else if (cc > '0' && cc < '8') {
                            word += QChar(c.digitValue());
                            SKIP_CHAR();
                        } else if ((metaCharPos = QString::fromLatin1("abfnrtv").indexOf(c))
                                   != -1) {
                            word += QLatin1Char("\a\b\f\n\r\t\v"[metaCharPos]);
                            SKIP_CHAR();
                        } else {
                            PUT_CHAR();
                        }
                    } else if (c.isSpace() || cc == '#') {
                        if (inQuote) {
                            if (cc == '\n')
                                location.fatal(QStringLiteral("Unterminated string"));
                            PUT_CHAR();
                        } else {
                            if (!word.isEmpty() || needsExpansion) {
                                rhsValues << word;
                                word.clear();
                                needsExpansion = false;
                            }
                            if (cc == '\n' || cc == '#')
                                break;
                            SKIP_SPACES();
                        }
                    } else if (cc == '"') {
                        if (inQuote) {
                            if (!word.isEmpty() || needsExpansion)
                                rhsValues << word;
                            word.clear();
                            needsExpansion = false;
                        }
                        inQuote = !inQuote;
                        SKIP_CHAR();
                    } else if (cc == '$') {
                        QString var;
                        QChar delim(' ');
                        bool braces = false;
                        SKIP_CHAR();
                        if (cc == '{') {
                            SKIP_CHAR();
                            braces = true;
                        }
                        while (c.isLetterOrNumber() || cc == '_') {
                            var += c;
                            SKIP_CHAR();
                        }
                        if (braces) {
                            if (cc == ',') {
                                SKIP_CHAR();
                                delim = c;
                                SKIP_CHAR();
                            }
                            if (cc == '}')
                                SKIP_CHAR();
                            else if (delim == '}')
                                delim = QChar(); // null delimiter
                            else
                                location.fatal(QStringLiteral("Missing '}'"));
                        }
                        if (!var.isEmpty()) {
                            const QByteArray val = qgetenv(var.toLatin1().constData());
                            if (val.isNull()) {
                                expandVars << ExpandVar(rhsValues.size(), word.size(), var, delim);
                                needsExpansion = true;
                            } else if (braces) { // ${VAR} inserts content from an env. variable for processing
                                text.insert(i, QString::fromLatin1(val));
                                c = text.at(i);
                                cc = c.unicode();
                            } else { // while $VAR simply reads the value and stores it to a config variable.
                                word += QString::fromLatin1(val);
                            }
                        }
                    } else {
                        if (!inQuote && cc == '=')
                            location.fatal(QStringLiteral("Unexpected '='"));
                        PUT_CHAR();
                    }
                }
                for (const auto &key : keys) {
                    if (!keySyntax.match(key).hasMatch())
                        keyLoc.fatal(QStringLiteral("Invalid key '%1'").arg(key));

                    ConfigVar configVar(key, rhsValues, QDir::currentPath(), keyLoc, expandVars);
                    if (plus && m_configVars.contains(key)) {
                        m_configVars[key].append(configVar);
                    } else {
                        m_configVars.insert(key, configVar);
                    }
                }
            }
        } else {
            location.fatal(QStringLiteral("Unexpected character '%1' at beginning of line").arg(c));
        }
    }
    popWorkingDir();

#undef SKIP_CHAR
#undef SKIP_SPACES
#undef PUT_CHAR
}

bool Config::isFileExcluded(const QString &fileName, const QSet<QString> &excludedFiles)
{
    for (const QString &entry : excludedFiles) {
        if (entry.contains(QLatin1Char('*')) || entry.contains(QLatin1Char('?'))) {
            QRegularExpression re(QRegularExpression::wildcardToRegularExpression(entry));
            if (re.match(fileName).hasMatch())
                return true;
        }
    }
    return excludedFiles.contains(fileName);
}

QStringList Config::getFilesHere(const QString &uncleanDir, const QString &nameFilter,
                                 const Location &location, const QSet<QString> &excludedDirs,
                                 const QSet<QString> &excludedFiles)
{
    // TODO: Understand why location is used to branch the
    // canonicalization and why the two different methods are used.
    QString dir =
            location.isEmpty() ? QDir::cleanPath(uncleanDir) : QDir(uncleanDir).canonicalPath();
    QStringList result;
    if (excludedDirs.contains(dir))
        return result;

    QDir dirInfo(dir);

    dirInfo.setNameFilters(nameFilter.split(QLatin1Char(' ')));
    dirInfo.setSorting(QDir::Name);
    dirInfo.setFilter(QDir::Files);
    QStringList fileNames = dirInfo.entryList();
    for (const auto &file : std::as_const(fileNames)) {
        // TODO: Understand if this is needed and, should it be, if it
        // is indeed the only case that should be considered.
        if (!file.startsWith(QLatin1Char('~'))) {
            QString s = dirInfo.filePath(file);
            QString c = QDir::cleanPath(s);
            if (!isFileExcluded(c, excludedFiles))
                result.append(c);
        }
    }

    dirInfo.setNameFilters(QStringList(QLatin1String("*")));
    dirInfo.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    fileNames = dirInfo.entryList();
    for (const auto &file : fileNames)
        result += getFilesHere(dirInfo.filePath(file), nameFilter, location, excludedDirs,
                               excludedFiles);
    return result;
}

/*!
  Set \a dir as the working directory and push it onto the
  stack of working directories.
 */
void Config::pushWorkingDir(const QString &dir)
{
    m_workingDirs.push(dir);
    QDir::setCurrent(dir);
}

/*!
  Pop the top entry from the stack of working directories.
  Set the working directory to the next one on the stack,
  if one exists.
 */
void Config::popWorkingDir()
{
    Q_ASSERT(!m_workingDirs.isEmpty());
    m_workingDirs.pop();
    if (!m_workingDirs.isEmpty())
        QDir::setCurrent(m_workingDirs.top());
}

QT_END_NAMESPACE
