// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "location.h"

#include "config.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qtextstream.h>

#include <cstdio>
#include <cstdlib>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

int Location::s_tabSize;
int Location::s_warningCount = 0;
int Location::s_warningLimit = -1;
QString Location::s_programName;
QString Location::s_project;
QString Location::s_projectRoot;
QSet<QString> Location::s_reports;
QRegularExpression *Location::s_spuriousRegExp = nullptr;
std::unique_ptr<QFile> Location::s_warningLogFile;
std::unique_ptr<QTextStream> Location::s_warningLogStream;

/*!
  \class Location

  \brief The Location class provides a way to mark a location in a file.

  It maintains a stack of file positions. A file position
  consists of the file path, line number, and column number.
  The location is used for printing error messages that are
  tied to a location in a file.
 */

/*!
  Constructs an empty location.
 */
Location::Location() : m_stk(nullptr), m_stkTop(&m_stkBottom), m_stkDepth(0), m_etc(false)
{
    // nothing.
}

/*!
  Constructs a location with (fileName, 1, 1) on its file
  position stack.
 */
Location::Location(const QString &fileName)
    : m_stk(nullptr), m_stkTop(&m_stkBottom), m_stkDepth(0), m_etc(false)
{
    push(fileName);
}

/*!
  The copy constructor copies the contents of \a other into
  this Location using the assignment operator.
 */
Location::Location(const Location &other)
    : m_stk(nullptr), m_stkTop(&m_stkBottom), m_stkDepth(0), m_etc(false)
{
    *this = other;
}

/*!
  The assignment operator does a deep copy of the entire
  state of \a other into this Location.
 */
Location &Location::operator=(const Location &other)
{
    if (this == &other)
        return *this;

    QStack<StackEntry> *oldStk = m_stk;

    m_stkBottom = other.m_stkBottom;
    if (other.m_stk == nullptr) {
        m_stk = nullptr;
        m_stkTop = &m_stkBottom;
    } else {
        m_stk = new QStack<StackEntry>(*other.m_stk);
        m_stkTop = &m_stk->top();
    }
    m_stkDepth = other.m_stkDepth;
    m_etc = other.m_etc;
    delete oldStk;
    return *this;
}

/*!
  If the file position on top of the stack has a line number
  less than 1, set its line number to 1 and its column number
  to 1. Otherwise, do nothing.
  */
void Location::start()
{
    if (m_stkTop->m_lineNo < 1) {
        m_stkTop->m_lineNo = 1;
        m_stkTop->m_columnNo = 1;
    }
}

/*!
  Advance the current file position, using \a ch to decide how to do
  that. If \a ch is a \c{'\\n'}, increment the current line number and
  set the column number to 1. If \ch is a \c{'\\t'}, increment to the
  next tab column. Otherwise, increment the column number by 1.

  The current file position is the one on top of the position stack.
 */
void Location::advance(QChar ch)
{
    if (ch == QLatin1Char('\n')) {
        m_stkTop->m_lineNo++;
        m_stkTop->m_columnNo = 1;
    } else if (ch == QLatin1Char('\t')) {
        m_stkTop->m_columnNo = 1 + s_tabSize * (m_stkTop->m_columnNo + s_tabSize - 1) / s_tabSize;
    } else {
        m_stkTop->m_columnNo++;
    }
}

/*!
  Pushes \a filePath onto the file position stack. The current
  file position becomes (\a filePath, 1, 1).

  \sa pop()
*/
void Location::push(const QString &filePath)
{
    if (m_stkDepth++ >= 1) {
        if (m_stk == nullptr)
            m_stk = new QStack<StackEntry>;
        m_stk->push(StackEntry());
        m_stkTop = &m_stk->top();
    }

    m_stkTop->m_filePath = filePath;
    m_stkTop->m_lineNo = INT_MIN;
    m_stkTop->m_columnNo = 1;
}

/*!
  Pops the top of the internal stack. The current file position
  becomes the next one in the new top of stack.

  \sa push()
*/
void Location::pop()
{
    if (--m_stkDepth == 0) {
        m_stkBottom = StackEntry();
    } else {
        if (!m_stk)
            return;
        m_stk->pop();
        if (m_stk->isEmpty()) {
            delete m_stk;
            m_stk = nullptr;
            m_stkTop = &m_stkBottom;
        } else {
            m_stkTop = &m_stk->top();
        }
    }
}

/*! \fn bool Location::isEmpty() const

  Returns \c true if there is no file name set yet; returns \c false
  otherwise. The functions filePath(), lineNo() and columnNo()
  must not be called on an empty Location object.
 */

/*! \fn const QString &Location::filePath() const
  Returns the current path and file name. If the Location is
  empty, the returned string is null.

  \sa lineNo(), columnNo()
 */

/*!
  Returns the file name part of the file path, ie the current
  file. Returns an empty string if the file path is empty.
 */
QString Location::fileName() const
{
    QFileInfo fi(filePath());
    return fi.fileName();
}

/*!
  Returns the suffix of the file name. Returns an empty string
  if the file path is empty.
 */
QString Location::fileSuffix() const
{
    QString fp = filePath();
    return (fp.isEmpty() ? fp : fp.mid(fp.lastIndexOf('.') + 1));
}

/*! \fn int Location::lineNo() const
  Returns the current line number.
  Must not be called on an empty Location object.

  \sa filePath(), columnNo()
*/

/*! \fn int Location::columnNo() const
  Returns the current column number.
  Must not be called on an empty Location object.

  \sa filePath(), lineNo()
*/

/*!
  Writes \a message and \a details to stderr as a formatted
  warning message. Does not write the message if qdoc is in
  the Prepare phase.
 */
void Location::warning(const QString &message, const QString &details) const
{
    const auto &config = Config::instance();
    if (!config.preparing() || config.singleExec())
        emitMessage(Warning, message, details);
}

/*!
  Writes \a message and \a details to stderr as a formatted
  error message. Does not write the message if qdoc is in
  the Prepare phase.
 */
void Location::error(const QString &message, const QString &details) const
{
    const auto &config = Config::instance();
    if (!config.preparing() || config.singleExec())
        emitMessage(Error, message, details);
}

/*!
  Returns the error code QDoc should exit with; EXIT_SUCCESS
  or the number of documentation warnings if they exceeded
  the limit set by warninglimit configuration variable.
 */
int Location::exitCode()
{
    if (s_warningLimit < 0 || s_warningCount <= s_warningLimit)
        return EXIT_SUCCESS;

    Location().emitMessage(
            Error,
            QStringLiteral("Documentation warnings (%1) exceeded the limit (%2) for '%3'.")
                    .arg(QString::number(s_warningCount), QString::number(s_warningLimit),
                         s_project),
            QString());
    return s_warningCount;
}

/*!
  Writes \a message and \a details to stderr as a formatted
  error message and then exits the program. qdoc prints fatal
  errors in either phase (Prepare or Generate).
 */
void Location::fatal(const QString &message, const QString &details) const
{
    emitMessage(Error, message, details);
    information(message);
    information(details);
    information("Aborting");
    exit(EXIT_FAILURE);
}

/*!
  Writes \a message and \a details to stderr as a formatted
  report message.

  A report does not include any filename/line number information.
  Recurring reports with an identical \a message are ignored.

  A report is generated only in \e generate or \e {single-exec}
  phase. In \e {prepare} phase, this function does nothing.
 */
void Location::report(const QString &message, const QString &details) const
{
    const auto &config = Config::instance();
    if ((!config.preparing() || config.singleExec()) && !s_reports.contains(message)) {
        emitMessage(Report, message, details);
        s_reports << message;
    }
}

/*!
  Gets several parameters from the config, including
  tab size, program name, and a regular expression that
  appears to be used for matching certain error messages
  so that emitMessage() can avoid printing them.
 */
void Location::initialize()
{
    Config &config = Config::instance();
    s_tabSize = config.get(CONFIG_TABSIZE).asInt();
    s_programName = config.programName();
    s_project = config.get(CONFIG_PROJECT).asString();

    // Initialize project root for relative path calculation with priority:
    // 1. QDOC_PROJECT_ROOT environment variable (highest priority)
    // 2. projectroot configuration variable (fallback)
    // 3. Leave empty if neither available (absolute paths)
    QString qdocProjectRoot = qEnvironmentVariable("QDOC_PROJECT_ROOT");
    if (qdocProjectRoot.isNull())
        qdocProjectRoot = config.get(CONFIG_PROJECTROOT).asString();

    if (!qdocProjectRoot.isEmpty() && QDir(qdocProjectRoot).exists())
        s_projectRoot = QDir::cleanPath(qdocProjectRoot);

    if (!config.singleExec())
        s_warningCount = 0;
    if (qEnvironmentVariableIsSet("QDOC_ENABLE_WARNINGLIMIT")
        || config.get(CONFIG_WARNINGLIMIT + Config::dot + "enabled").asBool())
        s_warningLimit = config.get(CONFIG_WARNINGLIMIT).asInt();

    QRegularExpression regExp = config.getRegExp(CONFIG_SPURIOUS);
    if (regExp.isValid()) {
        s_spuriousRegExp = new QRegularExpression(regExp);
    } else {
        config.get(CONFIG_SPURIOUS).location()
                .warning(QStringLiteral("Invalid regular expression '%1'")
                        .arg(regExp.pattern()));
    }

    if (config.get(CONFIG_LOGWARNINGS).asBool())
        initializeWarningLog(config);
}

/*!
    Creates the header for the warning log file.
*/
QString Location::warningLogHeader()
{
    const auto &config = Config::instance();
    QStringList lines;

    lines << "# QDoc Warning Log"_L1;
    lines << "# Project: "_L1 + s_project;

    // Add command line arguments unless disabled
    if (!config.get(CONFIG_LOGWARNINGSDISABLECLIARGS).asBool()) {
        const QStringList args = QCoreApplication::arguments();
        if (!args.isEmpty()) {
            QStringList quotedArgs;
            for (const QString &arg : args) {
                // Quote arguments containing spaces
                if (arg.contains(QLatin1Char(' '))) {
                    quotedArgs << '"'_L1 + arg + '"'_L1;
                } else {
                    quotedArgs << arg;
                }
            }
            lines << "# Command: "_L1 + quotedArgs.join(QLatin1Char(' '));
        }
    }

    if (!s_projectRoot.isEmpty()) {
        // Indicate which method was used to determine project root
        if (!qEnvironmentVariable("QDOC_PROJECT_ROOT").isNull()) {
            lines << "# Root: QDOC_PROJECT_ROOT"_L1;
        } else {
            lines << "# Root: projectroot config"_L1;
        }
        lines << "# Path-Format: relative"_L1;
    } else {
        lines << "# Path-Format: absolute"_L1;
    }

    lines << "#"_L1;
    return lines.join('\n'_L1);
}

/*!
  Formats a file path for the warning log, converting to relative path
  if a project root is configured.
 */
QString Location::formatPathForWarningLog(const QString &path)
{
    if (s_projectRoot.isEmpty())
        return path;

    QDir projectDir(s_projectRoot);
    QString relativePath = projectDir.relativeFilePath(path);

    // Only use relative path if it doesn't go outside the project root
    if (!relativePath.startsWith("../"_L1))
        return relativePath;

    return path;
}

/*!
  Initializes the warning log file, creates the output directory if needed, and
  adds the command-line arguments to QDoc to the top of the log file.

  This function assumes that log warnings are enabled and should only be
  called when \c {CONFIG_LOGWARNINGS} is \c {true}.
 */
void Location::initializeWarningLog(const Config &config)
{
    const QString logFileName = s_project + "-qdoc-warnings.log";
    const QString &outputDir = config.getOutputDir();

    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    const QString &logFilePath = dir.filePath(logFileName);

    s_warningLogFile = std::make_unique<QFile>(logFilePath);
    if (s_warningLogFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        s_warningLogStream = std::make_unique<QTextStream>(s_warningLogFile.get());

        *s_warningLogStream << warningLogHeader() << Qt::endl;
        *s_warningLogStream << Qt::endl;
    } else {
        Location().warning(QStringLiteral("Failed to open warning log file: %1").arg(logFilePath));
        s_warningLogFile.reset();
    }
}

/*!
  Writes the message \a formattedMessage to the warning log file if it is
  enabled and the message type \a type is a warning.
 */
void Location::writeToWarningLog(MessageType type, const QString &formattedMessage)
{
    if (type != Warning || !s_warningLogStream)
        return;

    *s_warningLogStream << formattedMessage << Qt::endl;
}

/*!
  Apparently, all this does is delete the regular expression
  used for intercepting certain error messages that should
  not be emitted by emitMessage().
 */
void Location::terminate()
{
    delete s_spuriousRegExp;
    s_spuriousRegExp = nullptr;

    s_warningLogStream.reset();
    s_warningLogFile.reset();
}

/*!
  Prints \a message to \c stdout followed by a \c{'\n'}.
 */
void Location::information(const QString &message)
{
    printf("%s\n", message.toLatin1().data());
    fflush(stdout);
}

/*!
  Report a program bug, including the \a hint.
 */
void Location::internalError(const QString &hint)
{
    Location().fatal(QStringLiteral("Internal error (%1)").arg(hint),
                     QStringLiteral("There is a bug in %1. Seek advice from your local"
                                    " %2 guru.")
                             .arg(s_programName, s_programName));
}

/*!
  Formats \a message and \a details into a single string
  and outputs that string to \c stderr. \a type specifies
  whether the \a message is an error or a warning.
 */
void Location::emitMessage(MessageType type, const QString &message, const QString &details) const
{
    if (type == Warning && s_spuriousRegExp != nullptr) {
        auto match = s_spuriousRegExp->match(message, 0, QRegularExpression::NormalMatch,
                                             QRegularExpression::AnchorAtOffsetMatchOption);
        if (match.hasMatch() && match.capturedLength() == message.size())
            return;
    }

    QString result = message;
    if (!details.isEmpty())
        result += "\n[" + details + QLatin1Char(']');
    result.replace("\n", "\n    ");
    if (isEmpty()) {
        if (type == Error)
            result.prepend(QStringLiteral(": error: "));
        else if (type == Warning) {
            result.prepend(QStringLiteral(": warning: "));
            ++s_warningCount;
        }
    } else {
        if (type == Error)
            result.prepend(QStringLiteral(": (qdoc) error: "));
        else if (type == Warning) {
            result.prepend(QStringLiteral(": (qdoc) warning: "));
            ++s_warningCount;
        }
    }
    if (type != Report)
        result.prepend(toString());
    else
        result.prepend("qdoc: '%1': "_L1.arg(s_project));
    fprintf(stderr, "%s\n", result.toLatin1().data());
    fflush(stderr);

    // Create a version with relative paths for the warning log
    QString logMessage = result;
    if (type == Warning && !s_projectRoot.isEmpty()) {
        // Replace the absolute path portion with a relative path for log file
        QString locationString = toString();
        QString formattedLocationString = formatPathForWarningLog(locationString);
        logMessage.replace(locationString, formattedLocationString);
    }
    writeToWarningLog(type, logMessage);
}

/*!
  Converts the location to a string to be prepended to error
  messages.
 */
QString Location::toString() const
{
    QString str;

    if (isEmpty()) {
        str = s_programName;
    } else {
        Location loc2 = *this;
        loc2.setEtc(false);
        loc2.pop();
        if (!loc2.isEmpty()) {
            QString blah = QStringLiteral("In file included from ");
            for (;;) {
                str += blah;
                str += loc2.top();
                loc2.pop();
                if (loc2.isEmpty())
                    break;
                str += QStringLiteral(",\n");
                blah.fill(' ');
            }
            str += QStringLiteral(":\n");
        }
        str += top();
    }
    return str;
}

QString Location::top() const
{
    QDir path(filePath());
    QString str = path.absolutePath();
    if (lineNo() >= 1) {
        str += QLatin1Char(':');
        str += QString::number(lineNo());
    }
    if (etc())
        str += QLatin1String(" (etc.)");
    return str;
}

QT_END_NAMESPACE
