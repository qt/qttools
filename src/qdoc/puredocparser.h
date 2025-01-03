// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PUREDOCPARSER_H
#define PUREDOCPARSER_H

#include "cppcodeparser.h"

QT_BEGIN_NAMESPACE

class Location;

class PureDocParser : public CppCodeParser
{
public:
    PureDocParser() = default;
    ~PureDocParser() override = default;

    QStringList sourceFileNameFilter() override;
    void parseSourceFile(const Location &location, const QString &filePath) override;

private:
    bool processQdocComments();
    Tokenizer *m_tokenizer { nullptr };
    int m_token { 0 };
};

QT_END_NAMESPACE

#endif
