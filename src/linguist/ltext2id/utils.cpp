// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "utils.h"

#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <iostream>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace Utils {

void printErr(const QString &out)
{
    std::cerr << qPrintable(out) << std::endl;
}

void printOut(const QString &out)
{
    std::cout << qPrintable(out) << std::endl;
}

QMultiHash<QString, QString> getIncludeOptions(const QFileInfo &root, const QStringList &paths)
{
    QMultiHash<QString, QString> res;
    const qsizetype scanRootLen = root.absolutePath().size();
    for (const QString &fn : paths) {
        int offset = 0;
        int depth = 0;
        do {
            offset = fn.lastIndexOf(u'/', offset - 1);
            QString ffn = fn.mid(offset + 1);
            res.insert(ffn, fn);
        } while (++depth < 3 && offset > scanRootLen);
    }
    return res;
}

QString getIndentation(const QString &line)
{
    QString indentation;
    for (const QChar &c : line)
        if (c.isSpace())
            indentation += c;
        else
            return indentation;
    return indentation;
}

QStringList readLines(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printErr("ltext2id error: failed to open file %1 for reading.\n"_L1.arg(filename));
        return {};
    }
    QTextStream in(&file);
    return in.readAll().split('\n');
}

void writeLines(const QString &filename, const QStringList &lines)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        printErr("ltext2id error: failed to open file %1 for writing.\n"_L1.arg(filename));
        return;
    }
    QTextStream out(&file);
    out << lines.join('\n');
}

} // namespace Utils

QT_END_NAMESPACE
