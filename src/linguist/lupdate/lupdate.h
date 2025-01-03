// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef LUPDATE_H
#define LUPDATE_H

#include <QtCore/qtcore-config.h>
#include <QtTools/private/qttools-config_p.h>

#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>

QT_BEGIN_NAMESPACE

class ConversionData;
class Translator;
class TranslatorMessage;

enum UpdateOption {
    Verbose = 1,
    NoObsolete = 2,
    PluralOnly = 4,
    NoSort = 8,
    HeuristicSameText = 16,
    HeuristicSimilarText = 32,
    AbsoluteLocations = 256,
    RelativeLocations = 512,
    NoLocations = 1024,
    NoUiLines = 2048,
    SourceIsUtf16 = 4096
};

Q_DECLARE_FLAGS(UpdateOptions, UpdateOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(UpdateOptions)

Translator merge(
    const Translator &tor, const Translator &virginTor, const QList<Translator> &aliens,
    UpdateOptions options, QString &err);

void loadCPP(Translator &translator, const QStringList &filenames, ConversionData &cd);
bool loadJava(Translator &translator, const QString &filename, ConversionData &cd);
bool loadPython(Translator &translator, const QString &fileName, ConversionData &cd);
bool loadUI(Translator &translator, const QString &filename, ConversionData &cd);

#ifndef QT_NO_QML
bool loadQScript(Translator &translator, const QString &filename, ConversionData &cd);
bool loadQml(Translator &translator, const QString &filename, ConversionData &cd);
#endif

#define LUPDATE_FOR_EACH_TR_FUNCTION(UNARY_MACRO) \
    /* from cpp.cpp */ \
    UNARY_MACRO(Q_DECLARE_TR_FUNCTIONS) \
    UNARY_MACRO(QT_TR_N_NOOP) \
    UNARY_MACRO(QT_TRID_N_NOOP) \
    UNARY_MACRO(QT_TRANSLATE_N_NOOP) \
    UNARY_MACRO(QT_TRANSLATE_N_NOOP3) \
    UNARY_MACRO(QT_TR_NOOP) \
    UNARY_MACRO(QT_TRID_NOOP) \
    UNARY_MACRO(QT_TRANSLATE_NOOP) \
    UNARY_MACRO(QT_TRANSLATE_NOOP3) \
    UNARY_MACRO(QT_TR_NOOP_UTF8) \
    UNARY_MACRO(QT_TRANSLATE_NOOP_UTF8) \
    UNARY_MACRO(QT_TRANSLATE_NOOP3_UTF8) \
    UNARY_MACRO(findMessage) /* QTranslator::findMessage() has the same parameters as QApplication::translate() */ \
    UNARY_MACRO(qtTrId) \
    UNARY_MACRO(tr) \
    UNARY_MACRO(trUtf8) \
    UNARY_MACRO(translate) \
    /* from qdeclarative.cpp: */ \
    UNARY_MACRO(qsTr) \
    UNARY_MACRO(qsTrId) \
    UNARY_MACRO(qsTranslate) \
    /*end*/

class ParserTool
{
public:
    static QString transcode(const QString &str);
};

class TrFunctionAliasManager {
public:
    TrFunctionAliasManager();
    ~TrFunctionAliasManager();

    enum TrFunction {
        // need to uglify names b/c most of the names are themselves macros:
#define MAKE_ENTRY(F) Function_##F,
        LUPDATE_FOR_EACH_TR_FUNCTION(MAKE_ENTRY)
#undef MAKE_ENTRY
        NumTrFunctions
    };

    using NameToTrFunctionMap = QHash<QString, TrFunction>;

    enum Operation { AddAlias, SetAlias };

    int trFunctionByName(const QString &trFunctionName) const;

    void modifyAlias(int trFunction, const QString &alias, Operation op);

    bool isAliasFor(const QString &identifier, TrFunction trFunction) const
    { return m_trFunctionAliases[trFunction].contains(identifier); }

    QStringList availableFunctionsWithAliases() const;
    QStringList listAliases() const;

    const NameToTrFunctionMap &nameToTrFunctionMap() const;

private:
    void ensureTrFunctionHashUpdated() const;

private:
    QStringList m_trFunctionAliases[NumTrFunctions];
    mutable NameToTrFunctionMap m_nameToTrFunctionMap;
};

QT_END_NAMESPACE

extern QT_PREPEND_NAMESPACE(TrFunctionAliasManager) trFunctionAliasManager;

#endif
