// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qprocess.h>
#include <QCryptographicHash>
#include "location.h"
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
    \brief Returns an ascii-printable representation of \a str.

    Replace non-ascii-printable characters in \a str from a subset of such
    characters. The subset includes alphanumeric (alnum) characters
    ([a-zA-Z0-9]), space, punctuation characters, and common symbols. Non-alnum
    characters in this subset are replaced by a single hyphen. Leading,
    trailing, and consecutive hyphens are removed, such that the resulting
    string does not start or end with a hyphen. All characters are converted to
    lowercase.

    If any character in \a str is non-latin, or latin and not found in the
    aforementioned subset (e.g. 'ß', 'å', or 'ö'), a hash of \a str is appended
    to the final string.

    Returns a string that is normalized for use where ascii-printable strings
    are required, such as file names or fragment identifiers in URLs.

    The implementation is equivalent to:

    \code
      name.replace(QRegularExpression("[^A-Za-z0-9]+"), " ");
      name = name.simplified();
      name.replace(QLatin1Char(' '), QLatin1Char('-'));
      name = name.toLower();
    \endcode

    However, it has been measured to be approximately four times faster.
*/
QString asAsciiPrintable(const QString &str)
{
    auto legal_ascii = [](const uint value) {
        const uint start_ascii_subset{ 32 };
        const uint end_ascii_subset{ 126 };

        return value >= start_ascii_subset && value <= end_ascii_subset;
    };

    QString result;
    bool begun = false;
    bool has_non_alnum_content{ false };

    for (const auto &c : str) {
        char16_t u = c.unicode();
        if (!legal_ascii(u))
            has_non_alnum_content = true;
        if (u >= 'A' && u <= 'Z')
            u += 'a' - 'A';
        if ((u >= 'a' && u <= 'z') || (u >= '0' && u <= '9')) {
            result += QLatin1Char(u);
            begun = true;
        } else if (begun) {
            result += QLatin1Char('-');
            begun = false;
        }
    }
    if (result.endsWith(QLatin1Char('-')))
        result.chop(1);

    if (has_non_alnum_content) {
        auto title_hash = QString::fromLocal8Bit(
                QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
        title_hash.truncate(8);
        if (!result.isEmpty())
            result.append(QLatin1Char('-'));
        result.append(title_hash);
    }

    return result;
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
