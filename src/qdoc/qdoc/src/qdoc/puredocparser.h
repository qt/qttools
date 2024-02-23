// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PUREDOCPARSER_H
#define PUREDOCPARSER_H

#include "cppcodeparser.h"

#include <QtCore/QFile>

QT_BEGIN_NAMESPACE

class Location;

class PureDocParser : public CodeParser
{
public:
    PureDocParser() = default;
    ~PureDocParser() override = default;

    void initializeParser() override {}
    QString language() override { return "QDoc"; }

    QStringList sourceFileNameFilter() override;
    void parseSourceFile(const Location &, const QString &, CppCodeParser&) override {}
    std::vector<UntiedDocumentation> parse_qdoc_file(const Location& location, const QString& filePath);

private:
    std::vector<UntiedDocumentation> processQdocComments(QFile& input_file);
};

QT_END_NAMESPACE

#endif
