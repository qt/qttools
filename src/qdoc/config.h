// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CONFIG_H
#define CONFIG_H

#include "location.h"
#include "qdoccommandlineparser.h"
#include "singleton.h"

#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <QtCore/qstack.h>
#include <QtCore/qstringlist.h>

#include <utility>

QT_BEGIN_NAMESPACE

class Config;

/*
 Contains information about a location
 where a ConfigVar string needs to be expanded
 from another config variable.
*/
struct ExpandVar
{
    int m_valueIndex {};
    int m_index {};
    QString m_var {};
    QChar m_delim {};

    ExpandVar(int valueIndex, int index, QString var, const QChar &delim)
        : m_valueIndex(valueIndex), m_index(index), m_var(std::move(var)), m_delim(delim)
    {
    }
};

class ConfigVar
{
public:
    struct ConfigValue {
        QString m_value;
        QString m_path;
    };

    [[nodiscard]] QString asString(const QString defaultString = QString()) const;
    [[nodiscard]] QStringList asStringList() const;
    [[nodiscard]] QSet<QString> asStringSet() const;
    [[nodiscard]] bool asBool() const;
    [[nodiscard]] int asInt() const;
    [[nodiscard]] const Location &location() const { return m_location; }

    ConfigVar() = default;
    ConfigVar(QString name, const QStringList &values, const QString &dir,
              const Location &loc = Location(),
              const QList<ExpandVar> &expandVars = QList<ExpandVar>())
        : m_name(std::move(name)), m_location(loc), m_expandVars(expandVars)
    {
        for (const auto &v : values)
            m_values << ConfigValue {v, dir};
    }

private:
    void append(const ConfigVar &other);

private:
    QString m_name {};
    QList<ConfigValue> m_values {};
    Location m_location {};
    QList<ExpandVar> m_expandVars {};

    friend class Config;
};

/*
  In this multimap, the key is a config variable name.
 */
typedef QMap<QString, ConfigVar> ConfigVarMap;

class Config : public Singleton<Config>
{
public:
    ~Config();

    enum QDocPass { Neither, Prepare, Generate };

    enum PathFlags : unsigned char {
        None = 0x0,
        // TODO: [unenforced-unclear-validation]
        // The Validate flag is used, for example, during the retrival
        // of paths in getCanonicalPathList.
        // It is unclear what kind of validation it performs, if any,
        // and when this validation is required.
        // Instead, remove this kind of flag and ensure that any
        // amount of required validation is performed during the
        // parsing step, if possilbe, and only once.
        // Furthemore, ensure any such validation removes some
        // uncertainty on dependent subsystems, moving constraints to
        // preconditions and expressing them at the API boundaries.
        Validate = 0x1,
        IncludePaths = 0x2
    };

    void init(const QString &programName, const QStringList &args);
    [[nodiscard]] bool getDebug() const { return m_debug; }
    [[nodiscard]] bool getAtomsDump() const { return m_atomsDump; }
    [[nodiscard]] bool showInternal() const { return m_showInternal; }

    void clear();
    void reset();
    void load(const QString &fileName);
    void setStringList(const QString &var, const QStringList &values);
    void insertStringList(const QString &var, const QStringList &values);

    void showHelp(int exitCode = 0) { m_parser.showHelp(exitCode); }
    [[nodiscard]] QStringList qdocFiles() const { return m_parser.positionalArguments(); }
    [[nodiscard]] const QString &programName() const { return m_prog; }
    [[nodiscard]] const Location &location() const { return m_location; }
    [[nodiscard]] const ConfigVar &get(const QString &var) const
    {
        // Avoid injecting default-constructed values to map if var doesn't exist
        static ConfigVar empty;
        auto it = m_configVars.constFind(var);
        return (it != m_configVars.constEnd()) ? *it : empty;
    }
    [[nodiscard]] QString getOutputDir(const QString &format = QString("HTML")) const;
    [[nodiscard]] QSet<QString> getOutputFormats() const;
    [[nodiscard]] QStringList getCanonicalPathList(const QString &var,
                                                   PathFlags flags = None) const;
    [[nodiscard]] QRegularExpression getRegExp(const QString &var) const;
    [[nodiscard]] QList<QRegularExpression> getRegExpList(const QString &var) const;
    [[nodiscard]] QSet<QString> subVars(const QString &var) const;
    QStringList getAllFiles(const QString &filesVar, const QString &dirsVar,
                            const QSet<QString> &excludedDirs = QSet<QString>(),
                            const QSet<QString> &excludedFiles = QSet<QString>());
    [[nodiscard]] QString getIncludeFilePath(const QString &fileName) const;
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
    static QString copyFile(const Location &location, const QString &sourceFilePath,
                            const QString &userFriendlySourceFilePath,
                            const QString &targetDirPath);
    static int numParams(const QString &value);
    static void pushWorkingDir(const QString &dir);
    static void popWorkingDir();

    static const QString dot;

    static bool generateExamples;
    static QString installDir;
    static QString overrideOutputDir;
    static QSet<QString> overrideOutputFormats;

    [[nodiscard]] inline bool singleExec() const;
    [[nodiscard]] inline bool dualExec() const;
    QStringList &defines() { return m_defines; }
    QStringList &dependModules() { return m_dependModules; }
    QStringList &includePaths() { return m_includePaths; }
    QStringList &indexDirs() { return m_indexDirs; }
    [[nodiscard]] QString currentDir() const { return m_currentDir; }
    void setCurrentDir(const QString &path) { m_currentDir = path; }
    [[nodiscard]] QString previousCurrentDir() const { return m_previousCurrentDir; }
    void setPreviousCurrentDir(const QString &path) { m_previousCurrentDir = path; }

    void setQDocPass(const QDocPass &pass) { m_qdocPass = pass; };
    [[nodiscard]] bool preparing() const { return (m_qdocPass == Prepare); }
    [[nodiscard]] bool generating() const { return (m_qdocPass == Generate); }

private:
    void processCommandLineOptions(const QStringList &args);
    void setIncludePaths();
    void setIndexDirs();
    void expandVariables();

    QStringList m_dependModules {};
    QStringList m_defines {};
    QStringList m_includePaths {};
    QStringList m_indexDirs {};
    QStringList m_exampleFiles {};
    QStringList m_exampleDirs {};
    QString m_currentDir {};
    QString m_previousCurrentDir {};

    bool m_showInternal { false };
    static bool m_debug;

    // An option that can be set trough a similarly named command-line option.
    // When this is set, every time QDoc parses a block-comment, a
    // human-readable presentation of the `Atom`s structure for that
    // block will shown to the user.
    static bool m_atomsDump;

    static bool isMetaKeyChar(QChar ch);
    void load(Location location, const QString &fileName);

    QString m_prog {};
    Location m_location {};
    ConfigVarMap m_configVars {};

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
    static QString INCLUSIVE;
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
    static QString OUTPUTDIR;
    static QString OUTPUTFORMATS;
    static QString OUTPUTPREFIXES;
    static QString OUTPUTSUFFIXES;
    static QString PROJECT;
    static QString REDIRECTDOCUMENTATIONTODEVNULL;
    static QString QHP;
    static QString QUOTINGINFORMATION;
    static QString SCRIPTS;
    static QString SHOWINTERNAL;
    static QString SINGLEEXEC;
    static QString SOURCEDIRS;
    static QString SOURCEENCODING;
    static QString SOURCES;
    static QString SPURIOUS;
    static QString STYLESHEETS;
    static QString SYNTAXHIGHLIGHTING;
    static QString TABSIZE;
    static QString TAGFILE;
    static QString TIMESTAMPS;
    static QString TOCTITLES;
    static QString URL;
    static QString VERSION;
    static QString VERSIONSYM;
    static QString FILEEXTENSIONS;
    static QString IMAGEEXTENSIONS;
    static QString QMLTYPESPAGE;
    static QString QMLTYPESTITLE;
    static QString WARNINGLIMIT;
};

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
#define CONFIG_INCLUSIVE ConfigStrings::INCLUSIVE
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
#define CONFIG_OUTPUTDIR ConfigStrings::OUTPUTDIR
#define CONFIG_OUTPUTFORMATS ConfigStrings::OUTPUTFORMATS
#define CONFIG_OUTPUTPREFIXES ConfigStrings::OUTPUTPREFIXES
#define CONFIG_OUTPUTSUFFIXES ConfigStrings::OUTPUTSUFFIXES
#define CONFIG_PROJECT ConfigStrings::PROJECT
#define CONFIG_REDIRECTDOCUMENTATIONTODEVNULL ConfigStrings::REDIRECTDOCUMENTATIONTODEVNULL
#define CONFIG_QHP ConfigStrings::QHP
#define CONFIG_QUOTINGINFORMATION ConfigStrings::QUOTINGINFORMATION
#define CONFIG_SCRIPTS ConfigStrings::SCRIPTS
#define CONFIG_SHOWINTERNAL ConfigStrings::SHOWINTERNAL
#define CONFIG_SINGLEEXEC ConfigStrings::SINGLEEXEC
#define CONFIG_SOURCEDIRS ConfigStrings::SOURCEDIRS
#define CONFIG_SOURCEENCODING ConfigStrings::SOURCEENCODING
#define CONFIG_SOURCES ConfigStrings::SOURCES
#define CONFIG_SPURIOUS ConfigStrings::SPURIOUS
#define CONFIG_STYLESHEETS ConfigStrings::STYLESHEETS
#define CONFIG_SYNTAXHIGHLIGHTING ConfigStrings::SYNTAXHIGHLIGHTING
#define CONFIG_TABSIZE ConfigStrings::TABSIZE
#define CONFIG_TAGFILE ConfigStrings::TAGFILE
#define CONFIG_TIMESTAMPS ConfigStrings::TIMESTAMPS
#define CONFIG_TOCTITLES ConfigStrings::TOCTITLES
#define CONFIG_URL ConfigStrings::URL
#define CONFIG_VERSION ConfigStrings::VERSION
#define CONFIG_VERSIONSYM ConfigStrings::VERSIONSYM
#define CONFIG_FILEEXTENSIONS ConfigStrings::FILEEXTENSIONS
#define CONFIG_IMAGEEXTENSIONS ConfigStrings::IMAGEEXTENSIONS
#define CONFIG_QMLTYPESPAGE ConfigStrings::QMLTYPESPAGE
#define CONFIG_QMLTYPESTITLE ConfigStrings::QMLTYPESTITLE
#define CONFIG_WARNINGLIMIT ConfigStrings::WARNINGLIMIT

inline bool Config::singleExec() const
{
    return m_configVars.value(CONFIG_SINGLEEXEC).asBool();
}

inline bool Config::dualExec() const
{
    return !m_configVars.value(CONFIG_SINGLEEXEC).asBool();
}

QT_END_NAMESPACE

#endif
