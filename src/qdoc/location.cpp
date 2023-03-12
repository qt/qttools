// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "location.h"

#include "config.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qregularexpression.h>

#include <climits>
#include <cstdio>
#include <cstdlib>

QT_BEGIN_NAMESPACE

int Location::s_tabSize;
int Location::s_warningCount = 0;
int Location::s_warningLimit = -1;
QString Location::s_programName;
QString Location::s_project;
QRegularExpression *Location::s_spuriousRegExp = nullptr;

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
 */
void Location::report(const QString &message, const QString &details) const
{
    emitMessage(Report, message, details);
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
    fprintf(stderr, "%s\n", result.toLatin1().data());
    fflush(stderr);
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
