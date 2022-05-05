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
******************************************************************************/

#ifndef CLANGCODEPARSER_H
#define CLANGCODEPARSER_H

#include "cppcodeparser.h"

#include <QtCore/qtemporarydir.h>

typedef struct CXTranslationUnitImpl *CXTranslationUnit;

QT_BEGIN_NAMESPACE

class ClangCodeParser : public CppCodeParser
{
public:
    ~ClangCodeParser() override = default;

    void initializeParser() override;
    void terminateParser() override;
    QString language() override;
    QStringList headerFileNameFilter() override;
    QStringList sourceFileNameFilter() override;
    void parseHeaderFile(const Location &location, const QString &filePath) override;
    void parseSourceFile(const Location &location, const QString &filePath) override;
    void precompileHeaders() override;
    Node *parseFnArg(const Location &location, const QString &fnSignature, const QString &idTag) override;
    static const QByteArray &fn() { return s_fn; }

private:
    void getDefaultArgs(); // FIXME: Clean up API
    void getMoreArgs(); // FIXME: Clean up API

    void buildPCH();

    void printDiagnostics(const CXTranslationUnit &translationUnit) const;

    QString m_version {};
    QMultiHash<QString, QString> m_allHeaders {}; // file name->path
    QList<QByteArray> m_includePaths {};
    QScopedPointer<QTemporaryDir> m_pchFileDir {};
    QByteArray m_pchName {};
    QList<QByteArray> m_defines {};
    std::vector<const char *> m_args {};
    QList<QByteArray> m_moreArgs {};
    QStringList m_namespaceScope {};
    static QByteArray s_fn;
};

QT_END_NAMESPACE

#endif
