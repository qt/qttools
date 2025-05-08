// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include "clangcodeparser.h"
#include "codeparser.h"
#include "parsererror.h"
#include "utilities.h"

QT_BEGIN_NAMESPACE

class ClassNode;
class EnumNode;
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
        COMMAND_QMLATTACHEDPROPERTY, COMMAND_QMLENUM, COMMAND_QMLSIGNAL, COMMAND_QMLATTACHEDSIGNAL,
        COMMAND_QMLMETHOD, COMMAND_QMLATTACHEDMETHOD, COMMAND_QMLVALUETYPE, COMMAND_QMLBASICTYPE,
        COMMAND_QMLMODULE, COMMAND_STRUCT, COMMAND_UNION,
    };

    static inline const QSet<QString> meta_commands = QSet<QString>(CodeParser::common_meta_commands)
        << COMMAND_COMPARES << COMMAND_COMPARESWITH << COMMAND_INHEADERFILE
        << COMMAND_NEXTPAGE << COMMAND_OVERLOAD << COMMAND_PREVIOUSPAGE
        << COMMAND_QMLINSTANTIATES << COMMAND_QMLNATIVETYPE << COMMAND_REIMP << COMMAND_RELATES;

public:
    explicit CppCodeParser(FnCommandParser&& parser);

    FunctionNode *parseMacroArg(const Location &location, const QString &macroArg);
    FunctionNode *parseOtherFuncArg(const QString &topic, const Location &location,
                                    const QString &funcArg);
    static bool isQMLMethodTopic(const QString &t);
    static bool isQMLPropertyTopic(const QString &t);

    std::pair<std::vector<TiedDocumentation>, std::vector<FnMatchError>>
    processTopicArgs(const UntiedDocumentation &untied);

    void processMetaCommand(const Doc &doc, const QString &command, const ArgPair &argLocPair,
                            Node *node);
    void processMetaCommands(const Doc &doc, Node *node);
    void processMetaCommands(const std::vector<TiedDocumentation> &tied);

protected:
    virtual Node *processTopicCommand(const Doc &doc, const QString &command,
                                      const ArgPair &arg);
    std::vector<TiedDocumentation> processQmlProperties(const UntiedDocumentation& untied);

private:
    void setExampleFileLists(ExampleNode *en);
    static void processComparesCommand(Node *node, const QString &arg, const Location &loc);
    void processQmlNativeTypeCommand(Node *node, const QString &cmd,
                                     const QString &arg, const Location &loc);
    EnumNode *processQmlEnumTopic(const QStringList &enumItemNames, const Location &location,
                                  const QString &arg);

private:
    FnCommandParser fn_parser;
    QString m_exampleNameFilter;
    QString m_exampleImageFilter;
    bool m_showLinkErrors { false };
};

/*!
  * \internal
  * \brief Checks if there are too many topic commands in \a doc.
  *
  * This method compares the commands used in \a doc with the set of topic
  * commands. If zero or one topic command is found, or if all found topic
  * commands are {\\qml*}-commands, the method returns \c false.
  *
  * If more than one topic command is found, QDoc issues a warning and the list
  * of topic commands used in \a doc, and the method returns \c true.
  */
[[nodiscard]] inline bool hasTooManyTopics(const Doc &doc)
{
    const QSet<QString> topicCommandsUsed = CppCodeParser::topic_commands & doc.metaCommandsUsed();

    if (topicCommandsUsed.empty() || topicCommandsUsed.size() == 1)
        return false;
    if (std::all_of(topicCommandsUsed.cbegin(), topicCommandsUsed.cend(),
                    [](const auto &cmd) { return cmd.startsWith(QLatin1String("qml")); }))
        return false;

    const QStringList commands = topicCommandsUsed.values();
    const QString topicCommands{ std::accumulate(
            commands.cbegin(), commands.cend(), QString{},
            [index = qsizetype{ 0 }, numberOfCommands = commands.size()](
                    const QString &accumulator, const QString &topic) mutable -> QString {
                return accumulator + QLatin1String("\\") + topic
                        + Utilities::separator(index++, numberOfCommands);
            }) };

    doc.location().warning(
            QStringLiteral("Multiple topic commands found in comment: %1").arg(topicCommands));
    return true;
}

QT_END_NAMESPACE

#endif
