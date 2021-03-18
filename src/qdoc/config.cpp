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

/*
  config.cpp
*/

#include "config.h"
#include "loggingcategory.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qvariant.h>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

QString ConfigStrings::ALIAS = QStringLiteral("alias");
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
QString ConfigStrings::OBSOLETELINKS = QStringLiteral("obsoletelinks");
QString ConfigStrings::OUTPUTDIR = QStringLiteral("outputdir");
QString ConfigStrings::OUTPUTENCODING = QStringLiteral("outputencoding");
QString ConfigStrings::OUTPUTFORMATS = QStringLiteral("outputformats");
QString ConfigStrings::OUTPUTPREFIXES = QStringLiteral("outputprefixes");
QString ConfigStrings::OUTPUTSUFFIXES = QStringLiteral("outputsuffixes");
QString ConfigStrings::PROJECT = QStringLiteral("project");
QString ConfigStrings::REDIRECTDOCUMENTATIONTODEVNULL =
        QStringLiteral("redirectdocumentationtodevnull");
QString ConfigStrings::QHP = QStringLiteral("qhp");
QString ConfigStrings::QUOTINGINFORMATION = QStringLiteral("quotinginformation");
QString ConfigStrings::SCRIPTDIRS = QStringLiteral("scriptdirs");
QString ConfigStrings::SCRIPTS = QStringLiteral("scripts");
QString ConfigStrings::SHOWINTERNAL = QStringLiteral("showinternal");
QString ConfigStrings::SINGLEEXEC = QStringLiteral("singleexec");
QString ConfigStrings::SOURCEDIRS = QStringLiteral("sourcedirs");
QString ConfigStrings::SOURCEENCODING = QStringLiteral("sourceencoding");
QString ConfigStrings::SOURCES = QStringLiteral("sources");
QString ConfigStrings::SPURIOUS = QStringLiteral("spurious");
QString ConfigStrings::STYLEDIRS = QStringLiteral("styledirs");
QString ConfigStrings::STYLE = QStringLiteral("style");
QString ConfigStrings::STYLES = QStringLiteral("styles");
QString ConfigStrings::STYLESHEETS = QStringLiteral("stylesheets");
QString ConfigStrings::SYNTAXHIGHLIGHTING = QStringLiteral("syntaxhighlighting");
QString ConfigStrings::TABSIZE = QStringLiteral("tabsize");
QString ConfigStrings::TAGFILE = QStringLiteral("tagfile");
QString ConfigStrings::TIMESTAMPS = QStringLiteral("timestamps");
QString ConfigStrings::TRANSLATORS = QStringLiteral("translators");
QString ConfigStrings::URL = QStringLiteral("url");
QString ConfigStrings::VERSION = QStringLiteral("version");
QString ConfigStrings::VERSIONSYM = QStringLiteral("versionsym");
QString ConfigStrings::FILEEXTENSIONS = QStringLiteral("fileextensions");
QString ConfigStrings::IMAGEEXTENSIONS = QStringLiteral("imageextensions");
QString ConfigStrings::QMLONLY = QStringLiteral("qmlonly");
QString ConfigStrings::QMLTYPESPAGE = QStringLiteral("qmltypespage");
QString ConfigStrings::QMLTYPESTITLE = QStringLiteral("qmltypestitle");
QString ConfigStrings::WARNINGLIMIT = QStringLiteral("warninglimit");
QString ConfigStrings::WRITEQAPAGES = QStringLiteral("writeqapages");

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
Q_DECLARE_TYPEINFO(MetaStackEntry, Q_MOVABLE_TYPE);

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
    Q_DECLARE_TR_FUNCTIONS(QDoc::MetaStack)

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
        if (count() == 1)
            location.fatal(tr("Unexpected '}'"));

        top().close();
        const QStringList suffixes = pop().accum;
        const QStringList prefixes = top().next;

        top().next.clear();
        for (const auto &prefix : prefixes) {
            for (const auto &suffix : suffixes)
                top().next << prefix + suffix;
        }
    } else if (ch == QLatin1Char(',') && count() > 1) {
        top().close();
        top().open();
    } else {
        /*
          This is where all the processing is done.
         */
        for (auto it = top().next.begin(); it != top().next.end(); ++it)
            *it += ch;
    }
}

/*!
  Returns the accumulated string values.
 */
QStringList MetaStack::getExpanded(const Location &location)
{
    if (count() > 1)
        location.fatal(tr("Missing '}'"));

    top().close();
    return top().accum;
}

const QString Config::dot = QLatin1String(".");
bool Config::m_debug = false;
bool Config::generateExamples = true;
QString Config::overrideOutputDir;
QString Config::installDir;
QSet<QString> Config::overrideOutputFormats;
QMap<QString, QString> Config::m_extractedDirs;
QStack<QString> Config::m_workingDirs;
QMap<QString, QStringList> Config::m_includeFilesMap;

/*!
  \class Config
  \brief The Config class contains the configuration variables
  for controlling how qdoc produces documentation.

  Its load() function, reads, parses, and processes a qdocconf file.
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
    m_location = m_lastLocation = Location();
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
    setStringList(CONFIG_FILEEXTENSIONS, QStringList("*.cpp *.h *.qdoc *.qml"));
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
    SET(CONFIG_WRITEQAPAGES, writeQaPagesOption);
    SET(CONFIG_REDIRECTDOCUMENTATIONTODEVNULL, redirectDocumentationToDevNullOption);
    SET(CONFIG_AUTOLINKERRORS, autoLinkErrorsOption);
    SET(CONFIG_OBSOLETELINKS, obsoleteLinksOption);
#undef SET
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
    m_lastLocation = Location();

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
  Sets the \a values of a configuration variable \a var from a string list.
 */
void Config::setStringList(const QString &var, const QStringList &values)
{
    m_configVars.replace(var, ConfigVar(var, values, QDir::currentPath()));
}

/*!
  Adds the \a values from a string list to the configuration variable \a var.
  Existing value(s) are kept.
*/
void Config::insertStringList(const QString &var, const QStringList &values)
{
    m_configVars.insert(var, ConfigVar(var, values, QDir::currentPath()));
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

    m_debug = m_parser.isSet(m_parser.debugOption);

    if (m_parser.isSet(m_parser.prepareOption))
        m_qdocPass = Prepare;
    if (m_parser.isSet(m_parser.generateOption))
        m_qdocPass = Generate;
    if (m_parser.isSet(m_parser.writeQaPagesOption)) {
        qCWarning(lcQdoc,
                "The QA pages option for QDoc is deprecated and will be removed in Qt 6.");
    }
    if (m_parser.isSet(m_parser.logProgressOption))
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
  Looks up the configuration variable \a var in the string
  map and returns the boolean value.
 */
bool Config::getBool(const QString &var) const
{
    return QVariant(getString(var)).toBool();
}

/*!
  Looks up the configuration variable \a var in the string list
  map. Iterates through the string list found, interpreting each
  string in the list as an integer and adding it to a total sum.
  Returns the sum or \c -1 if \a var is not set.
 */
int Config::getInt(const QString &var) const
{
    const QStringList strs = getStringList(var);
    if (strs.isEmpty())
        return -1;

    int sum = 0;
    for (const auto &str : strs)
        sum += str.toInt();
    return sum;
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
        t = getString(CONFIG_OUTPUTDIR);
    else
        t = overrideOutputDir;
    if (getBool(CONFIG_SINGLEEXEC)) {
        QString project = getString(CONFIG_PROJECT);
        t += QLatin1Char('/') + project.toLower();
    }
    if (getBool(format + Config::dot + "nosubdirs")) {
        t = t.left(t.lastIndexOf('/'));
        QString singleOutputSubdir = getString(format + Config::dot + "outputsubdir");
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
        return getStringSet(CONFIG_OUTPUTFORMATS);
    else
        return overrideOutputFormats;
}

/*!
  First, this function looks up the configuration variable \a var
  in the location map and, if found, sets the internal variable
  \c{lastLocation_} to the Location that \a var maps to.

  Then it looks up the configuration variable \a var in the string
  map and returns the string that \a var maps to.

  If \a var is not contained in the location map it returns
  \a defaultString.

  \note By default, \a defaultString is a null string. If \a var
  is found but contains an empty string, that is returned instead.
  This allows determining whether a configuration variable is
  undefined (null string) or defined as empty (empty string).
 */
QString Config::getString(const QString &var, const QString &defaultString) const
{
    QList<ConfigVar> configVars = m_configVars.values(var);
    if (!configVars.empty()) {
        QString value("");
        int i = configVars.size() - 1;
        while (i >= 0) {
            const ConfigVar &cv = configVars[i];
            if (!cv.m_location.isEmpty())
                const_cast<Config *>(this)->m_lastLocation = cv.m_location;
            if (!cv.m_values.isEmpty()) {
                if (!cv.m_plus)
                    value.clear();
                for (int j = 0; j < cv.m_values.size(); ++j) {
                    if (!value.isEmpty() && !value.endsWith(QChar('\n')))
                        value.append(QChar(' '));
                    value.append(cv.m_values[j]);
                }
            }
            --i;
        }
        return value;
    }
    return defaultString;
}

/*!
  Looks up the configuration variable \a var in the string
  list map, converts the string list it maps to into a set
  of strings, and returns the set.
 */
QSet<QString> Config::getStringSet(const QString &var) const
{
    const auto &stringList = getStringList(var);
    return QSet<QString>(stringList.cbegin(), stringList.cend());
}

/*!
  First, this function looks up the configuration variable \a var
  in the location map. If found, it sets the internal variable
  \c{lastLocation_} to the Location that \a var maps to.

  Then it looks up the configuration variable \a var in the map of
  configuration variable records. If found, it gets a list of all
  the records for \a var. Then it appends all the values for \a var
  to a list and returns the list. As it appends the values from each
  record, if the \a var used '=' instead of '+=' the list is cleared
  before the values are appended. \note '+=' should always be used.
  The final list is returned.
 */
QStringList Config::getStringList(const QString &var) const
{
    QList<ConfigVar> configVars = m_configVars.values(var);
    QStringList values;
    if (!configVars.empty()) {
        int i = configVars.size() - 1;
        while (i >= 0) {
            if (!configVars[i].m_location.isEmpty())
                const_cast<Config *>(this)->m_lastLocation = configVars[i].m_location;
            if (configVars[i].m_plus)
                values.append(configVars[i].m_values);
            else
                values = configVars[i].m_values;
            --i;
        }
    }
    return values;
}

/*!
   Returns the a path list where all paths from the config variable \a var
   are canonicalized. If \a validate is true, a warning for invalid paths is
   generated.

   First, this function looks up the configuration variable \a var
   in the location map and, if found, sets the internal variable
   \c{lastLocation_} the Location that \a var maps to.

   Then it looks up the configuration variable \a var in the string
   list map, which maps to one or more records that each contains a
   list of file paths.

   \sa Location::canonicalRelativePath()
 */
QStringList Config::getCanonicalPathList(const QString &var, bool validate) const
{
    QStringList t;
    QList<ConfigVar> configVars = m_configVars.values(var);
    if (!configVars.empty()) {
        int i = configVars.size() - 1;
        while (i >= 0) {
            const ConfigVar &cv = configVars[i];
            if (!cv.m_location.isEmpty())
                const_cast<Config *>(this)->m_lastLocation = cv.m_location;
            if (!cv.m_plus)
                t.clear();
            const QString d = cv.m_currentPath;
            const QStringList &sl = cv.m_values;
            if (!sl.isEmpty()) {
                t.reserve(t.size() + sl.size());
                for (int i = 0; i < sl.size(); ++i) {
                    QDir dir(sl[i].simplified());
                    QString path = dir.path();
                    if (dir.isRelative())
                        dir.setPath(d + QLatin1Char('/') + path);
                    if (validate && !QFileInfo::exists(dir.path()))
                        m_lastLocation.warning(tr("Cannot find file or directory: %1").arg(path));
                    else {
                        QString canonicalPath = dir.canonicalPath();
                        if (!canonicalPath.isEmpty())
                            t.append(canonicalPath);
                        else if (path.contains(QLatin1Char('*')) || path.contains(QLatin1Char('?')))
                            t.append(path);
                    }
                }
            }
            --i;
        }
    }
    return t;
}

/*!
  Calls getRegExpList() with the control variable \a var and
  iterates through the resulting list of regular expressions,
  concatening them with some extras characters to form a single
  QRegExp, which is returned/

  \sa getRegExpList()
 */
QRegExp Config::getRegExp(const QString &var) const
{
    QString pattern;
    const QVector<QRegExp> subRegExps = getRegExpList(var);

    for (const auto &regExp : subRegExps) {
        if (!regExp.isValid())
            return regExp;
        if (!pattern.isEmpty())
            pattern += QLatin1Char('|');
        pattern += QLatin1String("(?:") + regExp.pattern() + QLatin1Char(')');
    }
    if (pattern.isEmpty())
        pattern = QLatin1String("$x"); // cannot match
    return QRegExp(pattern);
}

/*!
  Looks up the configuration variable \a var in the string list
  map, converts the string list to a list of regular expressions,
  and returns it.
 */
QVector<QRegExp> Config::getRegExpList(const QString &var) const
{
    const QStringList strs = getStringList(var);
    QVector<QRegExp> regExps;
    for (const auto &str : strs)
        regExps += QRegExp(str);
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
            QString subVar = it.key().mid(varDot.length());
            int dot = subVar.indexOf(QLatin1Char('.'));
            if (dot != -1)
                subVar.truncate(dot);
            if (!result.contains(subVar))
                result.insert(subVar);
        }
    }
    return result;
}

/*!
  Same as subVars(), but in this case we return a config var
  multimap with the matching keys (stripped of the prefix \a var
  and mapped to their values. The pairs are inserted into \a t
 */
void Config::subVarsAndValues(const QString &var, ConfigVarMultimap &t) const
{
    QString varDot = var + QLatin1Char('.');
    for (auto it = m_configVars.constBegin(); it != m_configVars.constEnd(); ++it) {
        if (it.key().startsWith(varDot)) {
            QString subVar = it.key().mid(varDot.length());
            int dot = subVar.indexOf(QLatin1Char('.'));
            if (dot != -1)
                subVar.truncate(dot);
            t.insert(subVar, it.value());
        }
    }
}

/*!
  Get all .qdocinc files.
 */
QString Config::getIncludeFilePath(const QString &fileName) const
{
    QString ext = fileName.mid(fileName.lastIndexOf('.'));
    ext.prepend('*');

    if (!m_includeFilesMap.contains(ext)) {
        QSet<QString> t;
        QStringList result;
        const auto sourceDirs = getCanonicalPathList(CONFIG_SOURCEDIRS);
        for (const auto &dir : sourceDirs)
            result += getFilesHere(dir, ext, location(), t, t);
        // Append the include files from the exampledirs as well
        const auto exampleDirs = getCanonicalPathList(CONFIG_EXAMPLEDIRS);
        for (const auto &dir : exampleDirs)
            result += getFilesHere(dir, ext, location(), t, t);
        m_includeFilesMap.insert(ext, result);
    }
    const QStringList &paths = (*m_includeFilesMap.find(ext));
    for (const auto &path : paths) {
        if (path.endsWith(fileName))
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
    QStringList result = getCanonicalPathList(filesVar, true);
    const QStringList dirs = getCanonicalPathList(dirsVar, true);

    const QString nameFilter = getString(filesVar + dot + CONFIG_FILEEXTENSIONS);

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
    const QString nameFilter = getString(CONFIG_EXAMPLES + dot + CONFIG_IMAGEEXTENSIONS);

    for (const auto &dir : dirs)
        result += getFilesHere(dir, nameFilter, location(), excludedDirs, excludedFiles);
    return result;
}

/*!
    Returns the path to the project file for \a examplePath, or an empty string
    if no project file was found.
 */
QString Config::getExampleProjectFile(const QString &examplePath)
{
    QFileInfo fileInfo(examplePath);
    QStringList validNames;
    validNames << fileInfo.fileName() + QLatin1String(".pro")
               << fileInfo.fileName() + QLatin1String(".qmlproject")
               << fileInfo.fileName() + QLatin1String(".pyproject")
               << QLatin1String("CMakeLists.txt")
               << QLatin1String("qbuild.pro"); // legacy

    QString projectFile;

    for (const auto &name : qAsConst(validNames)) {
        projectFile = Config::findFile(Location(), m_exampleFiles, m_exampleDirs,
                                       examplePath + QLatin1Char('/') + name);
        if (!projectFile.isEmpty())
            return projectFile;
    }

    return projectFile;
}

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
                location.fatal(tr("File '%1' does not exist").arg(file));
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
    return fileInfo.filePath();
}

/*!
 */
QString Config::findFile(const Location &location, const QStringList &files,
                         const QStringList &dirs, const QString &fileBase,
                         const QStringList &fileExtensions, QString *userFriendlyFilePath)
{
    for (const auto &extension : fileExtensions) {
        QString filePath = findFile(location, files, dirs, fileBase + QLatin1Char('.') + extension,
                                    userFriendlyFilePath);
        if (!filePath.isEmpty())
            return filePath;
    }
    return findFile(location, files, dirs, fileBase, userFriendlyFilePath);
}

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
    QFile inFile(sourceFilePath);
    if (!inFile.open(QFile::ReadOnly)) {
        location.warning(tr("Cannot open input file for copy: '%1': %2")
                                 .arg(sourceFilePath)
                                 .arg(inFile.errorString()));
        return QString();
    }

    QString outFileName = userFriendlySourceFilePath;
    int slash = outFileName.lastIndexOf(QLatin1Char('/'));
    if (slash != -1)
        outFileName = outFileName.mid(slash);
    if ((outFileName.size()) > 0 && (outFileName[0] != '/'))
        outFileName = targetDirPath + QLatin1Char('/') + outFileName;
    else
        outFileName = targetDirPath + outFileName;
    QFile outFile(outFileName);
    if (!outFile.open(QFile::WriteOnly)) {
        location.warning(tr("Cannot open output file for copy: '%1': %2")
                                 .arg(outFileName)
                                 .arg(outFile.errorString()));
        return QString();
    }

    char buffer[1024];
    int len;
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
    for (int i = 0; i != value.length(); ++i) {
        uint c = value[i].unicode();
        if (c > 0 && c < 8)
            max = qMax(max, static_cast<int>(c));
    }
    return max;
}

/*!
  Removes everything from \a dir. This function is recursive.
  It doesn't remove \a dir itself, but if it was called
  recursively, then the caller will remove \a dir.
 */
bool Config::removeDirContents(const QString &dir)
{
    QDir dirInfo(dir);
    const QFileInfoList entries = dirInfo.entryInfoList();

    bool ok = true;

    for (const auto &entry : entries) {
        if (entry.isFile()) {
            if (!dirInfo.remove(entry.fileName()))
                ok = false;
        } else if (entry.isDir()) {
            if (entry.fileName() != QLatin1String(".") && entry.fileName() != QLatin1String("..")) {
                if (removeDirContents(entry.absoluteFilePath())) {
                    if (!dirInfo.rmdir(entry.fileName()))
                        ok = false;
                } else {
                    ok = false;
                }
            }
        }
    }
    return ok;
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
            int prefix = location.filePath().length() - location.fileName().length();
            fin.setFileName(Config::installDir + QLatin1Char('/')
                            + fileName.right(fileName.length() - prefix));
        }
        if (!fin.open(QFile::ReadOnly | QFile::Text))
            location.fatal(tr("Cannot open master qdocconf file '%1': %2")
                                   .arg(fileName)
                                   .arg(fin.errorString()));
    }
    QTextStream stream(&fin);
#ifndef QT_NO_TEXTCODEC
    stream.setCodec("UTF-8");
#endif
    QStringList qdocFiles;
    QDir configDir(QFileInfo(fileName).canonicalPath());
    QString line = stream.readLine();
    while (!line.isNull()) {
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
    QString path = fileInfo.canonicalPath();
    pushWorkingDir(path);
    QDir::setCurrent(path);
    QRegExp keySyntax(QLatin1String("\\w+(?:\\.\\w+)*"));

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
        location.fatal(tr("Too many nested includes"));

    QFile fin(fileInfo.fileName());
    if (!fin.open(QFile::ReadOnly | QFile::Text)) {
        if (!Config::installDir.isEmpty()) {
            int prefix = location.filePath().length() - location.fileName().length();
            fin.setFileName(Config::installDir + QLatin1Char('/')
                            + fileName.right(fileName.length() - prefix));
        }
        if (!fin.open(QFile::ReadOnly | QFile::Text))
            location.fatal(tr("Cannot open file '%1': %2").arg(fileName).arg(fin.errorString()));
    }

    QTextStream stream(&fin);
#ifndef QT_NO_TEXTCODEC
    stream.setCodec("UTF-8");
#endif
    QString text = stream.readAll();
    text += QLatin1String("\n\n");
    text += QLatin1Char('\0');
    fin.close();

    location.push(fileName);
    location.start();

    int i = 0;
    QChar c = text.at(0);
    uint cc = c.unicode();
    while (i < text.length()) {
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
            QString stringValue;
            QStringList rhsValues;
            QString word;
            bool inQuote = false;
            bool prevWordQuoted = true;
            bool metWord = false;

            MetaStack stack;
            do {
                stack.process(c, location);
                SKIP_CHAR();
            } while (isMetaKeyChar(c));

            const QStringList keys = stack.getExpanded(location);
            SKIP_SPACES();

            if (keys.count() == 1 && keys.first() == QLatin1String("include")) {
                QString includeFile;

                if (cc != '(')
                    location.fatal(tr("Bad include syntax"));
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
                                location.fatal(tr("Environment variable '%1' undefined").arg(var));
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
                    location.fatal(tr("Bad include syntax"));
                SKIP_CHAR();
                SKIP_SPACES();
                if (cc != '#' && cc != '\n')
                    location.fatal(tr("Trailing garbage"));

                /*
                  Here is the recursive call.
                 */
                load(location, QFileInfo(QDir(path), includeFile).filePath());
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
                    location.fatal(tr("Expected '=' or '+=' after key"));
                SKIP_CHAR();
                SKIP_SPACES();

                for (;;) {
                    if (cc == '\\') {
                        int metaCharPos;

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
                                location.fatal(tr("Unterminated string"));
                            PUT_CHAR();
                        } else {
                            if (!word.isEmpty()) {
                                if (metWord)
                                    stringValue += QLatin1Char(' ');
                                stringValue += word;
                                rhsValues << word;
                                metWord = true;
                                word.clear();
                                prevWordQuoted = false;
                            }
                            if (cc == '\n' || cc == '#')
                                break;
                            SKIP_SPACES();
                        }
                    } else if (cc == '"') {
                        if (inQuote) {
                            if (!prevWordQuoted)
                                stringValue += QLatin1Char(' ');
                            stringValue += word;
                            if (!word.isEmpty())
                                rhsValues << word;
                            metWord = true;
                            word.clear();
                            prevWordQuoted = true;
                        }
                        inQuote = !inQuote;
                        SKIP_CHAR();
                    } else if (cc == '$') {
                        QString var;
                        SKIP_CHAR();
                        while (c.isLetterOrNumber() || cc == '_') {
                            var += c;
                            SKIP_CHAR();
                        }
                        if (!var.isEmpty()) {
                            const QByteArray val = qgetenv(var.toLatin1().constData());
                            if (val.isNull()) {
                                location.fatal(tr("Environment variable '%1' undefined").arg(var));
                            } else {
                                word += QString::fromLatin1(val);
                            }
                        }
                    } else {
                        if (!inQuote && cc == '=')
                            location.fatal(tr("Unexpected '='"));
                        PUT_CHAR();
                    }
                }
                for (const auto &key : keys) {
                    if (!keySyntax.exactMatch(key))
                        keyLoc.fatal(tr("Invalid key '%1'").arg(key));

                    ConfigVarMultimap::Iterator i;
                    i = m_configVars.insert(key,
                                           ConfigVar(key, rhsValues, QDir::currentPath(), keyLoc));
                    i.value().m_plus = plus;
                }
            }
        } else {
            location.fatal(tr("Unexpected character '%1' at beginning of line").arg(c));
        }
    }
    popWorkingDir();
    if (!m_workingDirs.isEmpty())
        QDir::setCurrent(m_workingDirs.top());
}

bool Config::isFileExcluded(const QString &fileName, const QSet<QString> &excludedFiles)
{
    for (const QString &entry : excludedFiles) {
        if (entry.contains(QLatin1Char('*')) || entry.contains(QLatin1Char('?'))) {
            QRegExp re(entry, Qt::CaseSensitive, QRegExp::Wildcard);
            if (re.exactMatch(fileName))
                return true;
        }
    }
    return excludedFiles.contains(fileName);
}

QStringList Config::getFilesHere(const QString &uncleanDir, const QString &nameFilter,
                                 const Location &location, const QSet<QString> &excludedDirs,
                                 const QSet<QString> &excludedFiles)
{
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
    for (const auto &file : qAsConst(fileNames)) {
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
  Push \a dir onto the stack of working directories.
 */
void Config::pushWorkingDir(const QString &dir)
{
    m_workingDirs.push(dir);
}

/*!
  If the stack of working directories is not empty, pop the
  top entry and return it. Otherwise return an empty string.
 */
QString Config::popWorkingDir()
{
    if (!m_workingDirs.isEmpty())
        return m_workingDirs.pop();

    qDebug() << "RETURNED EMPTY WORKING DIR";
    return QString();
}

QT_END_NAMESPACE
