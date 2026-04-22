// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TEXTUTILS_H
#define TEXTUTILS_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

namespace TextUtils {

// English-list punctuation. Templates that iterate a list of items should
// emit separator(i, count) after each one instead of a literal comma or
// period. For the last item, returns "." (full stop); otherwise delegates
// to comma() for the appropriate inter-item punctuation.
QString separator(qsizetype wordPosition, qsizetype numberOfWords);

// Inter-item punctuation for English lists of length numberOfWords at
// wordPosition. Empty for a single-item list, " and " between two items,
// ", " between earlier items of a longer list, ", and " before the last
// item of a longer list, and empty at the last position (its separator is
// a period, via separator()).
QString comma(qsizetype wordPosition, qsizetype numberOfWords);

// Returns an ASCII-printable lowercase form of a string, suitable for
// filenames and URL fragments. Alphanumerics are preserved, non-alnum
// subset characters become hyphens (collapsed and trimmed), and any
// non-ASCII content appends an MD5 hash suffix so the result stays unique.
QString asAsciiPrintable(const QString &name);

// HTML-escapes &, <, >, and " in the input string.
QString protect(const QString &string);

using namespace Qt::Literals::StringLiterals;

static constexpr QLatin1StringView samp = "&amp;"_L1;
static constexpr QLatin1StringView slt = "&lt;"_L1;
static constexpr QLatin1StringView sgt = "&gt;"_L1;
static constexpr QLatin1StringView squot = "&quot;"_L1;

} // namespace TextUtils

QT_END_NAMESPACE

#endif // TEXTUTILS_H
