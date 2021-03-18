/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
  config.h
*/

#ifndef CONFIG_H
#define CONFIG_H

#include "location.h"
#include "qdoccommandlineparser.h"

#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qset.h>
#include <QtCore/qstack.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

template<typename T>
class Singleton
{
public:
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
    static T &instance()
    {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
};

/*
  This struct contains all the information for
  one config variable found in a qdocconf file.
 */
struct ConfigVar
{
    bool m_plus {};
    QString m_name {};
    QStringList m_values {};
    QString m_currentPath {};
    Location m_location {};

    ConfigVar() : m_plus(false) {}

    ConfigVar(const QString &name, const QStringList &values, const QString &dir)
        : m_plus(true), m_name(name), m_values(values), m_currentPath(dir)
    {
    }

    ConfigVar(const QString &name, const QStringList &values, const QString &dir,
              const Location &loc)
        : m_plus(false), m_name(name), m_values(values), m_currentPath(dir), m_location(loc)
    {
    }
};

/*
  In this multimap, the key is a config variable name.
 */
typedef QMultiMap<QString, ConfigVar> ConfigVarMultimap;

class Config : public Singleton<Config>
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::Config)

public:
    ~Config();

    enum QDocPass { Neither, Prepare, Generate };

    void init(const QString &programName, const QStringList &args);
    bool getDebug() const { return m_debug; }

    void clear();
    void reset();
    void load(const QString &fileName);
    void setStringList(const QString &var, const QStringList &values);
    void insertStringList(const QString &var, const QStringList &values);

    void showHelp(int exitCode = 0) { m_parser.showHelp(exitCode); }
    QStringList qdocFiles() const { return m_parser.positionalArguments(); }
    const QString &programName() const { return m_prog; }
    const Location &location() const { return m_location; }
    const Location &lastLocation() const { return m_lastLocation; }
    bool getBool(const QString &var) const;
    int getInt(const QString &var) const;

    QString getOutputDir(const QString &format = QString("HTML")) const;
    QSet<QString> getOutputFormats() const;
    QString getString(const QString &var, const QString &defaultString = QString()) const;
    QSet<QString> getStringSet(const QString &var) const;
    QStringList getStringList(const QString &var) const;
    QStringList getCanonicalPathList(const QString &var, bool validate = false) const;
    QRegExp getRegExp(const QString &var) const;
    QVector<QRegExp> getRegExpList(const QString &var) const;
    QSet<QString> subVars(const QString &var) const;
    void subVarsAndValues(const QString &var, ConfigVarMultimap &t) const;
    QStringList getAllFiles(const QString &filesVar, const QString &dirsVar,
                            const QSet<QString> &excludedDirs = QSet<QString>(),
                            const QSet<QString> &excludedFiles = QSet<QString>());
    QString getIncludeFilePath(const QString &fileName) const;
    QStringList getExampleQdocFiles(const QSet<QString> &excludedDirs,
                                    const QSet<QString> &excludedFiles);
    QStringList getExampleImageFiles(const QSet<QString> &excludedDirs,
                                     const QSet<QString> &excludedFiles);
    QString getExampleProjectFile(const QString &examplePath);

    static QStringList loadMaster(const QString &fileName);
    static bool isFileExcluded(const QString &fileName, const QSet<QString> &excludedFiles);
    static QStringList getFilesHere(const QString &dir, const QString &nameFilter,
                                    const Location &location = Location(),
                                    const QSet<QString> &excludedDirs = QSet<QString>(),
                                    const QSet<QString> &excludedFiles = QSet<QString>());
    static QString findFile(const Location &location, const QStringList &files,
                            const QStringList &dirs, const QString &fileName,
                            QString *userFriendlyFilePath = nullptr);
    static QString findFile(const Location &location, const QStringList &files,
                            const QStringList &dirs, const QString &fileBase,
                            const QStringList &fileExtensions,
                            QString *userFriendlyFilePath = nullptr);
    static QString copyFile(const Location &location, const QString &sourceFilePath,
                            const QString &userFriendlySourceFilePath,
                            const QString &targetDirPath);
    static int numParams(const QString &value);
    static bool removeDirContents(const QString &dir);
    static void pushWorkingDir(const QString &dir);
    static QString popWorkingDir();

    static const QString dot;

    static bool generateExamples;
    static QString installDir;
    static QString overrideOutputDir;
    static QSet<QString> overrideOutputFormats;

    inline bool singleExec() const;
    inline bool dualExec() const;
    QStringList &defines() { return m_defines; }
    QStringList &dependModules() { return m_dependModules; }
    QStringList &includePaths() { return m_includePaths; }
    QStringList &indexDirs() { return m_indexDirs; }
    QString currentDir() const { return m_currentDir; }
    void setCurrentDir(const QString &path) { m_currentDir = path; }
    QString previousCurrentDir() const { return m_previousCurrentDir; }
    void setPreviousCurrentDir(const QString &path) { m_previousCurrentDir = path; }

    QDocPass qdocPass() const { return m_qdocPass; }
    void setQDocPass(const QDocPass &pass) { m_qdocPass = pass; };
    bool preparing() const { return (m_qdocPass == Prepare); }
    bool generating() const { return (m_qdocPass == Generate); }

private:
    void processCommandLineOptions(const QStringList &args);
    void setIncludePaths();
    void setIndexDirs();

    QStringList m_dependModules {};
    QStringList m_defines {};
    QStringList m_includePaths {};
    QStringList m_indexDirs {};
    QStringList m_exampleFiles {};
    QStringList m_exampleDirs {};
    QString m_currentDir {};
    QString m_previousCurrentDir {};

    static bool m_debug;
    static bool isMetaKeyChar(QChar ch);
    void load(Location location, const QString &fileName);

    QString m_prog {};
    Location m_location {};
    Location m_lastLocation {};
    ConfigVarMultimap m_configVars {};

    static QMap<QString, QString> m_uncompressedFiles;
    static QMap<QString, QString> m_extractedDirs;
    static QStack<QString> m_workingDirs;
    static QMap<QString, QStringList> m_includeFilesMap;
    QDocCommandLineParser m_parser {};

    QDocPass m_qdocPass { Neither };
};

struct ConfigStrings
{
    static QString ALIAS;
    static QString AUTOLINKERRORS;
    static QString BUILDVERSION;
    static QString CLANGDEFINES;
    static QString CODEINDENT;
    static QString CODEPREFIX;
    static QString CODESUFFIX;
    static QString CPPCLASSESPAGE;
    static QString CPPCLASSESTITLE;
    static QString DEFINES;
    static QString DEPENDS;
    static QString DESCRIPTION;
    static QString DOCBOOKEXTENSIONS;
    static QString ENDHEADER;
    static QString EXAMPLEDIRS;
    static QString EXAMPLES;
    static QString EXAMPLESINSTALLPATH;
    static QString EXCLUDEDIRS;
    static QString EXCLUDEFILES;
    static QString EXTRAIMAGES;
    static QString FALSEHOODS;
    static QString FORMATTING;
    static QString HEADERDIRS;
    static QString HEADERS;
    static QString HEADERSCRIPTS;
    static QString HEADERSTYLES;
    static QString HOMEPAGE;
    static QString HOMETITLE;
    static QString IGNOREDIRECTIVES;
    static QString IGNORETOKENS;
    static QString IGNORESINCE;
    static QString IGNOREWORDS;
    static QString IMAGEDIRS;
    static QString IMAGES;
    static QString INCLUDEPATHS;
    static QString INDEXES;
    static QString LANDINGPAGE;
    static QString LANDINGTITLE;
    static QString LANGUAGE;
    static QString LOCATIONINFO;
    static QString LOGPROGRESS;
    static QString MACRO;
    static QString MANIFESTMETA;
    static QString MODULEHEADER;
    static QString NATURALLANGUAGE;
    static QString NAVIGATION;
    static QString NOLINKERRORS;
    static QString OBSOLETELINKS;
    static QString OUTPUTDIR;
    static QString OUTPUTENCODING;
    static QString OUTPUTFORMATS;
    static QString OUTPUTPREFIXES;
    static QString OUTPUTSUFFIXES;
    static QString PROJECT;
    static QString REDIRECTDOCUMENTATIONTODEVNULL;
    static QString QHP;
    static QString QUOTINGINFORMATION;
    static QString SCRIPTDIRS;
    static QString SCRIPTS;
    static QString SHOWINTERNAL;
    static QString SINGLEEXEC;
    static QString SOURCEDIRS;
    static QString SOURCEENCODING;
    static QString SOURCES;
    static QString SPURIOUS;
    static QString STYLEDIRS;
    static QString STYLE;
    static QString STYLES;
    static QString STYLESHEETS;
    static QString SYNTAXHIGHLIGHTING;
    static QString TABSIZE;
    static QString TAGFILE;
    static QString TIMESTAMPS;
    static QString TRANSLATORS;
    static QString URL;
    static QString VERSION;
    static QString VERSIONSYM;
    static QString FILEEXTENSIONS;
    static QString IMAGEEXTENSIONS;
    static QString QMLONLY;
    static QString QMLTYPESPAGE;
    static QString QMLTYPESTITLE;
    static QString WARNINGLIMIT;
    static QString WRITEQAPAGES;
};

#define CONFIG_ALIAS ConfigStrings::ALIAS
#define CONFIG_AUTOLINKERRORS ConfigStrings::AUTOLINKERRORS
#define CONFIG_BUILDVERSION ConfigStrings::BUILDVERSION
#define CONFIG_CLANGDEFINES ConfigStrings::CLANGDEFINES
#define CONFIG_CODEINDENT ConfigStrings::CODEINDENT
#define CONFIG_CODEPREFIX ConfigStrings::CODEPREFIX
#define CONFIG_CODESUFFIX ConfigStrings::CODESUFFIX
#define CONFIG_CPPCLASSESPAGE ConfigStrings::CPPCLASSESPAGE
#define CONFIG_CPPCLASSESTITLE ConfigStrings::CPPCLASSESTITLE
#define CONFIG_DEFINES ConfigStrings::DEFINES
#define CONFIG_DEPENDS ConfigStrings::DEPENDS
#define CONFIG_DESCRIPTION ConfigStrings::DESCRIPTION
#define CONFIG_DOCBOOKEXTENSIONS ConfigStrings::DOCBOOKEXTENSIONS
#define CONFIG_ENDHEADER ConfigStrings::ENDHEADER
#define CONFIG_EXAMPLEDIRS ConfigStrings::EXAMPLEDIRS
#define CONFIG_EXAMPLES ConfigStrings::EXAMPLES
#define CONFIG_EXAMPLESINSTALLPATH ConfigStrings::EXAMPLESINSTALLPATH
#define CONFIG_EXCLUDEDIRS ConfigStrings::EXCLUDEDIRS
#define CONFIG_EXCLUDEFILES ConfigStrings::EXCLUDEFILES
#define CONFIG_EXTRAIMAGES ConfigStrings::EXTRAIMAGES
#define CONFIG_FALSEHOODS ConfigStrings::FALSEHOODS
#define CONFIG_FORMATTING ConfigStrings::FORMATTING
#define CONFIG_HEADERDIRS ConfigStrings::HEADERDIRS
#define CONFIG_HEADERS ConfigStrings::HEADERS
#define CONFIG_HEADERSCRIPTS ConfigStrings::HEADERSCRIPTS
#define CONFIG_HEADERSTYLES ConfigStrings::HEADERSTYLES
#define CONFIG_HOMEPAGE ConfigStrings::HOMEPAGE
#define CONFIG_HOMETITLE ConfigStrings::HOMETITLE
#define CONFIG_IGNOREDIRECTIVES ConfigStrings::IGNOREDIRECTIVES
#define CONFIG_IGNORESINCE ConfigStrings::IGNORESINCE
#define CONFIG_IGNORETOKENS ConfigStrings::IGNORETOKENS
#define CONFIG_IGNOREWORDS ConfigStrings::IGNOREWORDS
#define CONFIG_IMAGEDIRS ConfigStrings::IMAGEDIRS
#define CONFIG_IMAGES ConfigStrings::IMAGES
#define CONFIG_INCLUDEPATHS ConfigStrings::INCLUDEPATHS
#define CONFIG_INDEXES ConfigStrings::INDEXES
#define CONFIG_LANDINGPAGE ConfigStrings::LANDINGPAGE
#define CONFIG_LANDINGTITLE ConfigStrings::LANDINGTITLE
#define CONFIG_LANGUAGE ConfigStrings::LANGUAGE
#define CONFIG_LOCATIONINFO ConfigStrings::LOCATIONINFO
#define CONFIG_LOGPROGRESS ConfigStrings::LOGPROGRESS
#define CONFIG_MACRO ConfigStrings::MACRO
#define CONFIG_MANIFESTMETA ConfigStrings::MANIFESTMETA
#define CONFIG_MODULEHEADER ConfigStrings::MODULEHEADER
#define CONFIG_NATURALLANGUAGE ConfigStrings::NATURALLANGUAGE
#define CONFIG_NAVIGATION ConfigStrings::NAVIGATION
#define CONFIG_NOLINKERRORS ConfigStrings::NOLINKERRORS
#define CONFIG_OBSOLETELINKS ConfigStrings::OBSOLETELINKS
#define CONFIG_OUTPUTDIR ConfigStrings::OUTPUTDIR
#define CONFIG_OUTPUTENCODING ConfigStrings::OUTPUTENCODING
#define CONFIG_OUTPUTFORMATS ConfigStrings::OUTPUTFORMATS
#define CONFIG_OUTPUTPREFIXES ConfigStrings::OUTPUTPREFIXES
#define CONFIG_OUTPUTSUFFIXES ConfigStrings::OUTPUTSUFFIXES
#define CONFIG_PROJECT ConfigStrings::PROJECT
#define CONFIG_REDIRECTDOCUMENTATIONTODEVNULL ConfigStrings::REDIRECTDOCUMENTATIONTODEVNULL
#define CONFIG_QHP ConfigStrings::QHP
#define CONFIG_QUOTINGINFORMATION ConfigStrings::QUOTINGINFORMATION
#define CONFIG_SCRIPTDIRS ConfigStrings::SCRIPTDIRS
#define CONFIG_SCRIPTS ConfigStrings::SCRIPTS
#define CONFIG_SHOWINTERNAL ConfigStrings::SHOWINTERNAL
#define CONFIG_SINGLEEXEC ConfigStrings::SINGLEEXEC
#define CONFIG_SOURCEDIRS ConfigStrings::SOURCEDIRS
#define CONFIG_SOURCEENCODING ConfigStrings::SOURCEENCODING
#define CONFIG_SOURCES ConfigStrings::SOURCES
#define CONFIG_SPURIOUS ConfigStrings::SPURIOUS
#define CONFIG_STYLEDIRS ConfigStrings::STYLEDIRS
#define CONFIG_STYLE ConfigStrings::STYLE
#define CONFIG_STYLES ConfigStrings::STYLES
#define CONFIG_STYLESHEETS ConfigStrings::STYLESHEETS
#define CONFIG_SYNTAXHIGHLIGHTING ConfigStrings::SYNTAXHIGHLIGHTING
#define CONFIG_TABSIZE ConfigStrings::TABSIZE
#define CONFIG_TAGFILE ConfigStrings::TAGFILE
#define CONFIG_TIMESTAMPS ConfigStrings::TIMESTAMPS
#define CONFIG_TRANSLATORS ConfigStrings::TRANSLATORS
#define CONFIG_URL ConfigStrings::URL
#define CONFIG_VERSION ConfigStrings::VERSION
#define CONFIG_VERSIONSYM ConfigStrings::VERSIONSYM
#define CONFIG_FILEEXTENSIONS ConfigStrings::FILEEXTENSIONS
#define CONFIG_IMAGEEXTENSIONS ConfigStrings::IMAGEEXTENSIONS
#define CONFIG_QMLONLY ConfigStrings::QMLONLY
#define CONFIG_QMLTYPESPAGE ConfigStrings::QMLTYPESPAGE
#define CONFIG_QMLTYPESTITLE ConfigStrings::QMLTYPESTITLE
#define CONFIG_WARNINGLIMIT ConfigStrings::WARNINGLIMIT
#define CONFIG_WRITEQAPAGES ConfigStrings::WRITEQAPAGES

inline bool Config::singleExec() const
{
    return getBool(CONFIG_SINGLEEXEC);
}

inline bool Config::dualExec() const
{
    return !getBool(CONFIG_SINGLEEXEC);
}

QT_END_NAMESPACE

#endif
