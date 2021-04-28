/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

} // namespace Utilities

QT_END_NAMESPACE
