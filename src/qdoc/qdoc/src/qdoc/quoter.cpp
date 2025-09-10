// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "quoter.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

QHash<QString, QString> Quoter::s_commentHash;

static void replaceMultipleNewlines(QString &s)
{
    const qsizetype n = s.size();
    bool slurping = false;
    int j = -1;
    const QChar newLine = QLatin1Char('\n');
    QChar *d = s.data();
    for (int i = 0; i != n; ++i) {
        const QChar c = d[i];
        bool hit = (c == newLine);
        if (slurping && hit)
            continue;
        d[++j] = c;
        slurping = hit;
    }
    s.resize(++j);
}

// This is equivalent to  line.split( QRegularExpression("\n(?!\n|$)") ) but much faster
QStringList Quoter::splitLines(const QString &line)
{
    QStringList result;
    qsizetype i = line.size();
    while (true) {
        qsizetype j = i - 1;
        while (j >= 0 && line.at(j) == QLatin1Char('\n'))
            --j;
        while (j >= 0 && line.at(j) != QLatin1Char('\n'))
            --j;
        result.prepend(line.mid(j + 1, i - j - 1));
        if (j < 0)
            break;
        i = j;
    }
    return result;
}

/*
  Transforms 'int x = 3 + 4' into 'int x=3+4'. A white space is kept
  between 'int' and 'x' because it is meaningful in C++.
*/
static void trimWhiteSpace(QString &str)
{
    enum { Normal, MetAlnum, MetSpace } state = Normal;
    const qsizetype n = str.size();

    int j = -1;
    QChar *d = str.data();
    for (int i = 0; i != n; ++i) {
        const QChar c = d[i];
        if (c.isLetterOrNumber()) {
            if (state == Normal) {
                state = MetAlnum;
            } else {
                if (state == MetSpace)
                    str[++j] = c;
                state = Normal;
            }
            str[++j] = c;
        } else if (c.isSpace()) {
            if (state == MetAlnum)
                state = MetSpace;
        } else {
            state = Normal;
            str[++j] = c;
        }
    }
    str.resize(++j);
}

Quoter::Quoter() : m_silent(false)
{
    /* We're going to hard code these delimiters:
        * C++, Qt, Qt Script, Java:
          //! [<id>]
        * .pro, .py, CMake files:
          #! [<id>]
        * .html, .qrc, .ui, .xq, .xml files:
          <!-- [<id>] -->
    */
    if (s_commentHash.empty()) {
        s_commentHash["pro"] = "#!";
        s_commentHash["py"] = "#!";
        s_commentHash["cmake"] = "#!";
        s_commentHash["html"] = "<!--";
        s_commentHash["qrc"] = "<!--";
        s_commentHash["ui"] = "<!--";
        s_commentHash["xml"] = "<!--";
        s_commentHash["xq"] = "<!--";
    }
}

void Quoter::reset()
{
    m_silent = false;
    m_plainLines.clear();
    m_markedLines.clear();
    m_codeLocation = Location();
}

void Quoter::quoteFromFile(const QString &userFriendlyFilePath, const QString &plainCode,
                           const QString &markedCode)
{
    m_silent = false;

    /*
      Split the source code into logical lines. Empty lines are
      treated specially. Before:

   p->alpha();
   p->beta();

   p->gamma();


   p->delta();

      After:

   p->alpha();
   p->beta();\n
   p->gamma();\n\n
   p->delta();

      Newlines are preserved because they affect codeLocation.
    */
    m_codeLocation = Location(userFriendlyFilePath);

    m_plainLines = splitLines(plainCode);
    m_markedLines = splitLines(markedCode);
    if (m_markedLines.size() != m_plainLines.size()) {
        m_codeLocation.warning(
                QStringLiteral("Something is wrong with qdoc's handling of marked code"));
        m_markedLines = m_plainLines;
    }

    /*
      Squeeze blanks (cat -s).
    */
    for (auto &line : m_markedLines)
        replaceMultipleNewlines(line);
    m_codeLocation.start();
}

QString Quoter::quoteLine(const Location &docLocation, const QString &command,
                          const QString &pattern)
{
    if (m_plainLines.isEmpty()) {
        failedAtEnd(docLocation, command);
        return QString();
    }

    if (pattern.isEmpty()) {
        docLocation.warning(QStringLiteral("Missing pattern after '\\%1'").arg(command));
        return QString();
    }

    if (match(docLocation, pattern, m_plainLines.first()))
        return getLine();

    if (!m_silent) {
        docLocation.warning(QStringLiteral("Command '\\%1' failed").arg(command));
        m_codeLocation.warning(QStringLiteral("Pattern '%1' didn't match here").arg(pattern));
        m_silent = true;
    }
    return QString();
}

QString Quoter::quoteSnippet(const Location &docLocation, const QString &identifier)
{
    QString comment = commentForCode();
    QString delimiter = comment + QString(" [%1]").arg(identifier);
    QString t;
    int indent = 0;

    while (!m_plainLines.isEmpty()) {
        if (match(docLocation, delimiter, m_plainLines.first())) {
            QString startLine = getLine();
            while (indent < startLine.size() && startLine[indent] == QLatin1Char(' '))
                indent++;
            break;
        }
        getLine();
    }
    while (!m_plainLines.isEmpty()) {
        QString line = m_plainLines.first();
        if (match(docLocation, delimiter, line)) {
            QString lastLine = getLine(indent);
            qsizetype dIndex = lastLine.indexOf(delimiter);
            if (dIndex > 0) {
                // The delimiter might be preceded on the line by other
                // delimeters, so look for the first comment on the line.
                QString leading = lastLine.left(dIndex);
                dIndex = leading.indexOf(comment);
                if (dIndex != -1)
                    leading = leading.left(dIndex);
                if (leading.endsWith(QLatin1String("<@comment>")))
                    leading.chop(10);
                if (!leading.trimmed().isEmpty())
                    t += leading;
            }
            return t;
        }

        t += removeSpecialLines(line, comment, indent);
    }
    failedAtEnd(docLocation, QString("snippet (%1)").arg(delimiter));
    return t;
}

QString Quoter::quoteTo(const Location &docLocation, const QString &command, const QString &pattern)
{
    QString t;
    QString comment = commentForCode();

    if (pattern.isEmpty()) {
        while (!m_plainLines.isEmpty()) {
            QString line = m_plainLines.first();
            t += removeSpecialLines(line, comment);
        }
    } else {
        while (!m_plainLines.isEmpty()) {
            if (match(docLocation, pattern, m_plainLines.first())) {
                return t;
            }
            t += getLine();
        }
        failedAtEnd(docLocation, command);
    }
    return t;
}

QString Quoter::quoteUntil(const Location &docLocation, const QString &command,
                           const QString &pattern)
{
    QString t = quoteTo(docLocation, command, pattern);
    t += getLine();
    return t;
}

QString Quoter::getLine(int unindent)
{
    if (m_plainLines.isEmpty())
        return QString();

    m_plainLines.removeFirst();

    QString t = m_markedLines.takeFirst();
    int i = 0;
    while (i < unindent && i < t.size() && t[i] == QLatin1Char(' '))
        i++;

    t = t.mid(i);
    t += QLatin1Char('\n');
    m_codeLocation.advanceLines(t.count(QLatin1Char('\n')));
    return t;
}

bool Quoter::match(const Location &docLocation, const QString &pattern0, const QString &line)
{
    QString str = line;
    while (str.endsWith(QLatin1Char('\n')))
        str.truncate(str.size() - 1);

    QString pattern = pattern0;
    if (pattern.startsWith(QLatin1Char('/')) && pattern.endsWith(QLatin1Char('/'))
        && pattern.size() > 2) {
        QRegularExpression rx(pattern.mid(1, pattern.size() - 2));
        if (!m_silent && !rx.isValid()) {
            docLocation.warning(
                    QStringLiteral("Invalid regular expression '%1'").arg(rx.pattern()));
            m_silent = true;
        }
        return str.indexOf(rx) != -1;
    }
    trimWhiteSpace(str);
    trimWhiteSpace(pattern);
    return str.indexOf(pattern) != -1;
}

void Quoter::failedAtEnd(const Location &docLocation, const QString &command)
{
    if (!m_silent && !command.isEmpty()) {
        if (m_codeLocation.filePath().isEmpty()) {
            docLocation.warning(QStringLiteral("Unexpected '\\%1'").arg(command));
        } else {
            docLocation.warning(QStringLiteral("Command '\\%1' failed at end of file '%2'")
                                        .arg(command, m_codeLocation.filePath()));
        }
        m_silent = true;
    }
}

QString Quoter::commentForCode() const
{
    QFileInfo fi = QFileInfo(m_codeLocation.fileName());
    if (fi.fileName() == "CMakeLists.txt")
        return "#!";
    return s_commentHash.value(fi.suffix(), "//!");
}

QString Quoter::removeSpecialLines(const QString &line, const QString &comment, int unindent)
{
    QString t;

    // Remove special macros to support Qt namespacing.
    QString trimmed = line.trimmed();
    if (trimmed.startsWith("QT_BEGIN_NAMESPACE")) {
        getLine();
    } else if (trimmed.startsWith("QT_END_NAMESPACE")) {
        getLine();
        t += QLatin1Char('\n');
    } else if (!trimmed.startsWith(comment)) {
        // Ordinary code
        t += getLine(unindent);
    } else {
        // Comments
        if (line.contains(QLatin1Char('\n')))
            t += QLatin1Char('\n');
        getLine();
    }
    return t;
}

QT_END_NAMESPACE
