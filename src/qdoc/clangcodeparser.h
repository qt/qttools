// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
