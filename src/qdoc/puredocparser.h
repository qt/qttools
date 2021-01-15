/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
****************************************************************************/

#ifndef PUREDOCPARSER_H
#define PUREDOCPARSER_H

#include "cppcodeparser.h"

QT_BEGIN_NAMESPACE

class Location;

class PureDocParser : public CppCodeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::PureDocParser)

public:
    PureDocParser() : tokenizer_(nullptr), tok_(0) { pureParser_ = this; }
    ~PureDocParser() override { pureParser_ = nullptr; }

    QStringList sourceFileNameFilter() override;
    void parseSourceFile(const Location &location, const QString &filePath) override;

    static PureDocParser *pureDocParser() { return pureParser_; }

private:
    bool processQdocComments();
    static PureDocParser *pureParser_;
    Tokenizer *tokenizer_;
    int tok_;
};

QT_END_NAMESPACE

#endif
