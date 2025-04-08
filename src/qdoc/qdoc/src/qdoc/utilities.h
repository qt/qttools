// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore/qstring.h>
#include <QtCore/qloggingcategory.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQdoc)
Q_DECLARE_LOGGING_CATEGORY(lcQdocClang)

class INode;
class Location;

namespace Utilities {
void startDebugging(const QString &message);
void stopDebugging(const QString &message);
bool debugging();

const INode *nodeForString(const QString &string);
QString stringForNode(const INode *node);

QString uniqueIdentifier(const Location &loc, const QString &prefix);
QString separator(qsizetype wordPosition, qsizetype numberOfWords);
QString comma(qsizetype wordPosition, qsizetype numberOfWords);
QString asAsciiPrintable(const QString &name);
QString protect(const QString &string);
QStringList getInternalIncludePaths(const QString &compiler);
bool isGeneratedFile(const QString &path);

static constexpr QLatin1StringView samp = "&amp;"_L1;
static constexpr QLatin1StringView slt = "&lt;"_L1;
static constexpr QLatin1StringView sgt = "&gt;"_L1;
static constexpr QLatin1StringView squot = "&quot;"_L1;
}

QT_END_NAMESPACE

#endif // UTILITIES_H
