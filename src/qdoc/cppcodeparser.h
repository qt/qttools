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
    static bool isJSMethodTopic(const QString &t);
    static bool isQMLMethodTopic(const QString &t);
    static bool isJSPropertyTopic(const QString &t);
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
};

QT_END_NAMESPACE

#endif
