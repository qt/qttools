// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "textutils.h"

#include <QtCore/qcryptographichash.h>

QT_BEGIN_NAMESPACE

/*!
    \namespace TextUtils
    \internal
    \brief Pure string helpers with no dependencies on QDoc driver types.

    TextUtils groups text-manipulation helpers that the IR builders, the
    template generator, and the legacy generators all need, but that do
    not touch Node, Tree, Config, or Generator. Keeping them here lets
    QDocLib components call them without dragging in the rest of QDoc.
 */
namespace TextUtils {

/*!
    \internal
    Returns the punctuation character for the word at \a wordPosition in a
    list of \a numberOfWords length. For the last position, returns "."
    (full stop). For any other word, delegates to comma().

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
    Returns the inter-item punctuation for a list of \a numberOfWords words
    at \a wordPosition.

    For a list of length one, returns an empty QString. For a list of length
    two, returns " and ". For longer lists, returns ", " for early items and
    ", and " for the item before the last. The last position returns an
    empty QString; its punctuation is the period returned by separator().

    \sa separator()
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
    \brief Returns an ASCII-printable representation of \a str.

    Replaces non-ASCII-printable characters in \a str from a subset of such
    characters. The subset includes alphanumeric (alnum) characters
    ([a-zA-Z0-9]), space, punctuation characters, and common symbols. Non-alnum
    characters in this subset are replaced by a single hyphen. Leading,
    trailing, and consecutive hyphens are removed, such that the resulting
    string does not start or end with a hyphen. All characters are converted to
    lowercase.

    If any character in \a str is non-latin, or latin and not found in the
    aforementioned subset (e.g. 'ß', 'å', or 'ö'), a hash of \a str is appended
    to the final string.

    Returns a string that is normalized for use where ASCII-printable strings
    are required, such as file names or fragment identifiers in URLs.
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
    HTML-escapes the ampersand, less-than, greater-than, and double-quote
    characters in \a str, leaving other characters untouched.
 */
QString protect(const QString &str)
{
    qsizetype n = str.size();
    QString marked;
    marked.reserve(n * 2 + 30);
    const QChar *data = str.constData();
    for (int i = 0; i != n; ++i) {
        switch (data[i].unicode()) {
        case '&':
            marked += samp;
            break;
        case '<':
            marked += slt;
            break;
        case '>':
            marked += sgt;
            break;
        case '"':
            marked += squot;
            break;
        default:
            marked += data[i];
        }
    }
    return marked;
}

} // namespace TextUtils

QT_END_NAMESPACE
