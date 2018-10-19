/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include <qregexp.h>

#include "codeparser.h"

QT_BEGIN_NAMESPACE

class ClassNode;
class CodeChunk;
class CppCodeParserPrivate;
class FunctionNode;
class Aggregate;
class Tokenizer;

class CppCodeParser : public CodeParser
{
    Q_DECLARE_TR_FUNCTIONS(QDoc::CppCodeParser)

    struct ExtraFuncData {
        Aggregate* root; // Used as the parent.
        Node::NodeType type; // The node type: Function, etc.
        bool isAttached; // If true, the method is attached.
        bool isMacro;    // If true, we are parsing a macro signature.
        ExtraFuncData() : root(0), type(Node::Function), isAttached(false), isMacro(false) { }
        ExtraFuncData(Aggregate* r, Node::NodeType t, bool a)
          : root(r), type(t), isAttached(a), isMacro(false) { }
    };

public:
    CppCodeParser();
    ~CppCodeParser();

    void initializeParser(const Config& config) override;
    void terminateParser() override;
    QString language() override;
    QStringList headerFileNameFilter() override;
    QStringList sourceFileNameFilter() override;
    bool parseParameters(const QString& parameters, QVector<Parameter>& pvect, bool& isQPrivateSignal);
    Node *parseMacroArg(const Location &location, const QString &macroArg) override;
    Node *parseOtherFuncArg(const QString &topic,
                            const Location &location,
                            const QString &funcArg) override;
    const Location& declLoc() const { return declLoc_; }
    void setDeclLoc() { declLoc_ = location(); }
    static bool isJSMethodTopic(const QString &t);
    static bool isQMLMethodTopic(const QString &t);
    static bool isJSPropertyTopic(const QString &t);
    static bool isQMLPropertyTopic(const QString &t);

protected:
    static const QSet<QString>& topicCommands();
    static const QSet<QString>& otherMetaCommands();
    virtual Node* processTopicCommand(const Doc& doc,
                                      const QString& command,
                                      const ArgLocPair& arg);
    void processQmlProperties(const Doc& doc, NodeList& nodes, DocList& docs, bool jsProps);
    bool splitQmlPropertyGroupArg(const QString& arg,
                                  QString& module,
                                  QString& element,
                                  QString& name,
                                  const Location& location);
    bool splitQmlPropertyArg(const QString& arg,
                             QString& type,
                             QString& module,
                             QString& element,
                             QString& name,
                             const Location& location);
    virtual void processOtherMetaCommand(const Doc& doc,
                                         const QString& command,
                                         const ArgLocPair& argLocPair,
                                         Node *node);
    void processOtherMetaCommands(const Doc& doc, Node *node);
    void processOtherMetaCommands(NodeList &nodes, DocList& docs);
    void processTopicArgs(const Doc &doc, const QString &topic, NodeList &nodes, DocList &docs);
    bool hasTooManyTopics(const Doc &doc) const;

 protected:
    void reset();
    void readToken();
    const Location& location();
    QString previousLexeme();
    QString lexeme();

 private:
    bool match(int target);
    bool skipTo(int target);
    bool matchModuleQualifier(QString& name);
    bool matchTemplateAngles(CodeChunk *type = 0);
    bool matchDataType(CodeChunk *type, QString *var = 0, bool qProp = false);
    bool matchParameter(QVector<Parameter>& pvect, bool& isQPrivateSignal);
    bool matchUsingDecl(Aggregate* parent);
    void setExampleFileLists(PageNode *pn);

 protected:
    QMap<QString, Node::NodeType> nodeTypeMap;
    Tokenizer *tokenizer;
    int tok;
    Node::Access access;
    FunctionNode::Metaness metaness_;
    QString physicalModuleName;
    QStringList lastPath_;
    QRegExp varComment;
    QRegExp sep;
    Location declLoc_;

 private:

    static QStringList exampleFiles;
    static QStringList exampleDirs;
    static QSet<QString> excludeDirs;
    static QSet<QString> excludeFiles;
    QString exampleNameFilter;
    QString exampleImageFilter;
};

QT_END_NAMESPACE

#endif
