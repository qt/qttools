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
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef CLANGCODEPARSER_H
#define CLANGCODEPARSER_H

#include "cppcodeparser.h"

#include <QtCore/qtemporarydir.h>

QT_BEGIN_NAMESPACE

class ClangCodeParser : public CppCodeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::ClangCodeParser)

public:
    ~ClangCodeParser() override;

    void initializeParser() override;
    void terminateParser() override;
    QString language() override;
    QStringList headerFileNameFilter() override;
    QStringList sourceFileNameFilter() override;
    void parseHeaderFile(const Location &location, const QString &filePath) override;
    void parseSourceFile(const Location &location, const QString &filePath) override;
    void precompileHeaders() override;
    Node *parseFnArg(const Location &location, const QString &fnArg) override;
    static const QByteArray &fn() { return fn_; }

private:
    void getDefaultArgs();
    bool getMoreArgs();
    void buildPCH();

private:
    int printParsingErrors_;
    QString version_;
    QHash<QString, QString> allHeaders_; // file name->path
    QVector<QByteArray> includePaths_;
    QScopedPointer<QTemporaryDir> pchFileDir_;
    QByteArray pchName_;
    QVector<QByteArray> defines_;
    std::vector<const char *> args_;
    QVector<QByteArray> moreArgs_;
    QStringList namespaceScope_;
    static QByteArray fn_;
};

QT_END_NAMESPACE

#endif
