// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qprocess.h>
#include "utilities.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQdoc, "qt.qdoc")
Q_LOGGING_CATEGORY(lcQdocClang, "qt.qdoc.clang")

/*!
    \namespace Utilities
    \internal
    \brief This namespace holds QDoc-internal utility methods.
 */
namespace Utilities {
static inline void setDebugEnabled(bool value)
{
    const_cast<QLoggingCategory &>(lcQdoc()).setEnabled(QtDebugMsg, value);
    const_cast<QLoggingCategory &>(lcQdocClang()).setEnabled(QtDebugMsg, value);
}

void startDebugging(const QString &message)
{
    setDebugEnabled(true);
    qCDebug(lcQdoc, "START DEBUGGING: %ls", qUtf16Printable(message));
}

void stopDebugging(const QString &message)
{
    qCDebug(lcQdoc, "STOP DEBUGGING: %ls", qUtf16Printable(message));
    setDebugEnabled(false);
}

bool debugging()
{
    return lcQdoc().isEnabled(QtDebugMsg);
}

/*!
    \internal
    Convenience method that's used to get the correct punctuation character for
    the words at \a wordPosition in a list of \a numberOfWords length.
    For the last position in the list, returns "." (full stop). For any other
    word, this method calls comma().

    \sa comma()
 */
QString separator(qsizetype wordPosition, qsizetype numberOfWords)
{
    static QString terminator = QStringLiteral(".");
    if (wordPosition == numberOfWords - 1)
        return terminator;
    else
        return comma(wordPosition, numberOfWords);
}

/*!
    \internal
    Convenience method that's used to get the correct punctuation character for
    the words at \a wordPosition in a list of \a numberOfWords length.

    For a list of length one, returns an empty QString. For a list of length
    two, returns the string " and ". For any length beyond two, returns the
    string ", " until the last element, which returns ", and ".

    \sa comma()
 */
QString comma(qsizetype wordPosition, qsizetype numberOfWords)
{
    if (wordPosition == numberOfWords - 1)
        return QString();
    if (numberOfWords == 2)
        return QStringLiteral(" and ");
    if (wordPosition == 0 || wordPosition < numberOfWords - 2)
        return QStringLiteral(", ");
    return QStringLiteral(", and ");
}

/*!
    \internal
*/
static bool runProcess(const QString &program, const QStringList &arguments,
                       QByteArray *stdOutIn, QByteArray *stdErrIn)
{
    QProcess process;
    process.start(program, arguments, QProcess::ReadWrite);
    if (!process.waitForStarted()) {
        qCDebug(lcQdoc).nospace() << "Unable to start " << process.program()
                                  << ": " << process.errorString();
        return false;
    }
    process.closeWriteChannel();
    const bool finished = process.waitForFinished();
    const QByteArray stdErr = process.readAllStandardError();
    if (stdErrIn)
        *stdErrIn = stdErr;
    if (stdOutIn)
        *stdOutIn = process.readAllStandardOutput();

    if (!finished) {
        qCDebug(lcQdoc).nospace() << process.program() << " timed out: " << stdErr;
        process.kill();
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit) {
        qCDebug(lcQdoc).nospace() << process.program() << " crashed: " << stdErr;
        return false;
    }

    if (process.exitCode() != 0) {
        qCDebug(lcQdoc).nospace() <<  process.program() << " exited with "
            << process.exitCode() << ": " << stdErr;
        return false;
    }

    return true;
}

/*!
    \internal
*/
static QByteArray frameworkSuffix() {
    return QByteArrayLiteral(" (framework directory)");
}

/*!
    \internal
    Determine the compiler's internal include paths from the output of

    \badcode
    [clang++|g++] -E -x c++ - -v </dev/null
    \endcode

    Output looks like:

    \badcode
    #include <...> search starts here:
    /usr/local/include
    /System/Library/Frameworks (framework directory)
    End of search list.
    \endcode
*/
QStringList getInternalIncludePaths(const QString &compiler)
{
    QStringList result;
    QStringList arguments;
    arguments << QStringLiteral("-E") << QStringLiteral("-x") << QStringLiteral("c++")
              << QStringLiteral("-") << QStringLiteral("-v");
    QByteArray stdOut;
    QByteArray stdErr;
    if (!runProcess(compiler, arguments, &stdOut, &stdErr))
        return result;
    const QByteArrayList stdErrLines = stdErr.split('\n');
    bool isIncludeDir = false;
    for (const QByteArray &line : stdErrLines) {
        if (isIncludeDir) {
            if (line.startsWith(QByteArrayLiteral("End of search list"))) {
                isIncludeDir = false;
            } else {
                QByteArray prefix("-I");
                QByteArray headerPath{line.trimmed()};
                if (headerPath.endsWith(frameworkSuffix())) {
                    headerPath.truncate(headerPath.size() - frameworkSuffix().size());
                    prefix = QByteArrayLiteral("-F");
                }
                result.append(QString::fromLocal8Bit(prefix + headerPath));
            }
        } else if (line.startsWith(QByteArrayLiteral("#include <...> search starts here"))) {
            isIncludeDir = true;
        }
    }

    return result;
}

} // namespace Utilities

QT_END_NAMESPACE
