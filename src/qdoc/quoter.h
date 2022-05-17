// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QUOTER_H
#define QUOTER_H

#include "location.h"

#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

class Quoter
{
public:
    Quoter();

    void reset();
    void quoteFromFile(const QString &userFriendlyFileName, const QString &plainCode,
                       const QString &markedCode);
    QString quoteLine(const Location &docLocation, const QString &command, const QString &pattern);
    QString quoteTo(const Location &docLocation, const QString &command, const QString &pattern);
    QString quoteUntil(const Location &docLocation, const QString &command, const QString &pattern);
    QString quoteSnippet(const Location &docLocation, const QString &identifier);

    static QStringList splitLines(const QString &line);

private:
    QString getLine(int unindent = 0);
    void failedAtEnd(const Location &docLocation, const QString &command);
    bool match(const Location &docLocation, const QString &pattern, const QString &line);
    [[nodiscard]] QString commentForCode() const;
    QString removeSpecialLines(const QString &line, const QString &comment, int unindent = 0);

    bool m_silent {};
    QStringList m_plainLines {};
    QStringList m_markedLines {};
    Location m_codeLocation {};
    static QHash<QString, QString> s_commentHash;
};

QT_END_NAMESPACE

#endif
