/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
