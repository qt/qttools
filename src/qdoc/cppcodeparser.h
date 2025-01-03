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

class CppCodeParser : public CodeParser
{
    struct ExtraFuncData
    {
        Aggregate *root; // Used as the parent.
        Node::NodeType type; // The node type: Function, etc.
        bool isAttached; // If true, the method is attached.
        bool isMacro; // If true, we are parsing a macro signature.
        ExtraFuncData() : root(nullptr), type(Node::Function), isAttached(false), isMacro(false) {}
    };

public:
    CppCodeParser();

    void initializeParser() override;
    void terminateParser() override;
    QString language() override { return QStringLiteral("Cpp"); }
    QStringList headerFileNameFilter() override;
    QStringList sourceFileNameFilter() override;
    FunctionNode *parseMacroArg(const Location &location, const QString &macroArg);
    FunctionNode *parseOtherFuncArg(const QString &topic, const Location &location,
                                    const QString &funcArg);
    static bool isQMLMethodTopic(const QString &t);
    static bool isQMLPropertyTopic(const QString &t);

protected:
    static const QSet<QString> &topicCommands();
    static const QSet<QString> &metaCommands();
    virtual Node *processTopicCommand(const Doc &doc, const QString &command,
                                      const ArgPair &arg);
    void processQmlProperties(const Doc &doc, NodeList &nodes, DocList &docs);
    bool splitQmlPropertyArg(const QString &arg, QString &type, QString &module, QString &element,
                             QString &name, const Location &location);
    void processMetaCommand(const Doc &doc, const QString &command, const ArgPair &argLocPair,
                            Node *node);
    void processMetaCommands(const Doc &doc, Node *node);
    void processMetaCommands(NodeList &nodes, DocList &docs);
    void processTopicArgs(const Doc &doc, const QString &topic, NodeList &nodes, DocList &docs);
    [[nodiscard]] bool hasTooManyTopics(const Doc &doc) const;

private:
    void setExampleFileLists(ExampleNode *en);

protected:
    typedef bool (Node::*NodeTypeTestFunc)() const;
    QMap<QString, NodeTypeTestFunc> m_nodeTypeTestFuncMap;
    QMap<QString, Node::NodeType> m_nodeTypeMap;

private:
    static QSet<QString> m_excludeDirs;
    static QSet<QString> m_excludeFiles;
    QString m_exampleNameFilter;
    QString m_exampleImageFilter;
    bool m_showLinkErrors { false };
};

QT_END_NAMESPACE

#endif
