// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include "codeparser.h"

QT_BEGIN_NAMESPACE

class ClassNode;
class ExampleNode;
class FunctionNode;
class Aggregate;

class CppCodeParser
{
public:
    static inline const QSet<QString> topic_commands{
        COMMAND_CLASS, COMMAND_DONTDOCUMENT, COMMAND_ENUM, COMMAND_EXAMPLE,
        COMMAND_EXTERNALPAGE, COMMAND_FN, COMMAND_GROUP, COMMAND_HEADERFILE,
        COMMAND_MACRO, COMMAND_MODULE, COMMAND_NAMESPACE, COMMAND_PAGE,
        COMMAND_PROPERTY, COMMAND_TYPEALIAS, COMMAND_TYPEDEF, COMMAND_VARIABLE,
        COMMAND_QMLTYPE, COMMAND_QMLPROPERTY, COMMAND_QMLPROPERTYGROUP,
        COMMAND_QMLATTACHEDPROPERTY, COMMAND_QMLSIGNAL, COMMAND_QMLATTACHEDSIGNAL,
        COMMAND_QMLMETHOD, COMMAND_QMLATTACHEDMETHOD, COMMAND_QMLVALUETYPE, COMMAND_QMLBASICTYPE,
        COMMAND_QMLMODULE, COMMAND_STRUCT, COMMAND_UNION,
    };

    static inline const QSet<QString> meta_commands = QSet<QString>(CodeParser::common_meta_commands)
        << COMMAND_INHEADERFILE << COMMAND_NEXTPAGE
        << COMMAND_OVERLOAD << COMMAND_PREVIOUSPAGE
        << COMMAND_QMLINSTANTIATES << COMMAND_REIMP
        << COMMAND_RELATES;

public:
    CppCodeParser();
    ~CppCodeParser();

    FunctionNode *parseMacroArg(const Location &location, const QString &macroArg);
    FunctionNode *parseOtherFuncArg(const QString &topic, const Location &location,
                                    const QString &funcArg);
    static bool isQMLMethodTopic(const QString &t);
    static bool isQMLPropertyTopic(const QString &t);

    [[nodiscard]] bool hasTooManyTopics(const Doc &doc) const;

    void processTopicArgs(const Doc &doc, const QString &topic, NodeList &nodes, DocList &docs);

    void processMetaCommand(const Doc &doc, const QString &command, const ArgPair &argLocPair,
                            Node *node);
    void processMetaCommands(const Doc &doc, Node *node);
    void processMetaCommands(NodeList &nodes, DocList &docs);

protected:
    virtual Node *processTopicCommand(const Doc &doc, const QString &command,
                                      const ArgPair &arg);
    void processQmlProperties(const Doc &doc, NodeList &nodes, DocList &docs);
    bool splitQmlPropertyArg(const QString &arg, QString &type, QString &module, QString &element,
                             QString &name, const Location &location);

private:
    void setExampleFileLists(ExampleNode *en);

private:
    static QSet<QString> m_excludeDirs;
    static QSet<QString> m_excludeFiles;
    QString m_exampleNameFilter;
    QString m_exampleImageFilter;
    bool m_showLinkErrors { false };
};

QT_END_NAMESPACE

#endif
