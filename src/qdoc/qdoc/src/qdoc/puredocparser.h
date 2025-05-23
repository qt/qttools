// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef PUREDOCPARSER_H
#define PUREDOCPARSER_H

#include "cppcodeparser.h"

#include <QtCore/QFile>

QT_BEGIN_NAMESPACE

class Location;

class PureDocParser
{
public:
    PureDocParser(const Location& location) : location{location} {}

    std::vector<UntiedDocumentation> parse_qdoc_file(const QString& filePath);

private:
    std::vector<UntiedDocumentation> processQdocComments(QFile& input_file);

private:
    const Location& location;
};

QT_END_NAMESPACE

#endif
