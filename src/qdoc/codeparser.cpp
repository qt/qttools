// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "codeparser.h"

#include "config.h"
#include "generator.h"
#include "node.h"
#include "qdocdatabase.h"

#include <QtCore/qregularexpression.h>

QT_BEGIN_NAMESPACE

QList<CodeParser *> CodeParser::s_parsers;
bool CodeParser::s_showInternal = false;

/*!
  The constructor adds this code parser to the static
  list of code parsers.
 */
CodeParser::CodeParser()
{
    m_qdb = QDocDatabase::qdocDB();
    s_parsers.prepend(this);
}

/*!
  The destructor removes this code parser from the static
  list of code parsers.
 */
CodeParser::~CodeParser()
{
    s_parsers.removeAll(this);
}

/*!
  Initialize the code parser base class.
 */
void CodeParser::initializeParser()
{
    s_showInternal = Config::instance().showInternal();
}

/*!
  Terminating a code parser is trivial.
 */
void CodeParser::terminateParser()
{
    // nothing.
}

QStringList CodeParser::headerFileNameFilter()
{
    return sourceFileNameFilter();
}

void CodeParser::parseHeaderFile(const Location &location, const QString &filePath)
{
    parseSourceFile(location, filePath);
}

/*!
  All the code parsers in the static list are initialized here,
  after the qdoc configuration variables have been set.
 */
void CodeParser::initialize()
{
    for (const auto &parser : std::as_const(s_parsers))
        parser->initializeParser();
}

/*!
  All the code parsers in the static list are terminated here.
 */
void CodeParser::terminate()
{
    for (const auto parser : s_parsers)
        parser->terminateParser();
}

CodeParser *CodeParser::parserForLanguage(const QString &language)
{
    for (const auto parser : std::as_const(s_parsers)) {
        if (parser->language() == language)
            return parser;
    }
    return nullptr;
}

CodeParser *CodeParser::parserForHeaderFile(const QString &filePath)
{
    QString fileName = QFileInfo(filePath).fileName();

    for (const auto &parser : std::as_const(s_parsers)) {
        const QStringList headerPatterns = parser->headerFileNameFilter();
        for (const auto &pattern : headerPatterns) {
            auto re = QRegularExpression::fromWildcard(pattern, Qt::CaseInsensitive);
            if (re.match(fileName).hasMatch())
                return parser;
        }
    }
    return nullptr;
}

CodeParser *CodeParser::parserForSourceFile(const QString &filePath)
{
    QString fileName = QFileInfo(filePath).fileName();

    for (const auto &parser : s_parsers) {
        const QStringList sourcePatterns = parser->sourceFileNameFilter();
        for (const QString &pattern : sourcePatterns) {
            auto re = QRegularExpression::fromWildcard(pattern, Qt::CaseInsensitive);
            if (re.match(fileName).hasMatch())
                return parser;
        }
    }
    return nullptr;
}

static QSet<QString> commonMetaCommands_;
/*!
  Returns the set of strings representing the common metacommands.
 */
const QSet<QString> &CodeParser::commonMetaCommands()
{
    if (commonMetaCommands_.isEmpty()) {
        commonMetaCommands_ << COMMAND_ABSTRACT << COMMAND_DEFAULT << COMMAND_DEPRECATED
                            << COMMAND_INGROUP << COMMAND_INMODULE << COMMAND_INPUBLICGROUP
                            << COMMAND_INQMLMODULE << COMMAND_INTERNAL << COMMAND_MODULESTATE
                            << COMMAND_NOAUTOLIST << COMMAND_NONREENTRANT << COMMAND_OBSOLETE
                            << COMMAND_PRELIMINARY << COMMAND_QMLABSTRACT << COMMAND_QMLDEFAULT
                            << COMMAND_QMLINHERITS << COMMAND_QMLREADONLY << COMMAND_QMLREQUIRED
                            << COMMAND_QTCMAKEPACKAGE << COMMAND_QTVARIABLE << COMMAND_REENTRANT
                            << COMMAND_SINCE << COMMAND_STARTPAGE  << COMMAND_SUBTITLE
                            << COMMAND_THREADSAFE << COMMAND_TITLE << COMMAND_WRAPPER
                            << COMMAND_ATTRIBUTION;
    }
    return commonMetaCommands_;
}

/*!
  \internal
 */
void CodeParser::extractPageLinkAndDesc(QStringView arg, QString *link, QString *desc)
{
    static const QRegularExpression bracedRegExp(
            QRegularExpression::anchoredPattern(QLatin1String(R"(\{([^{}]*)\}(?:\{([^{}]*)\})?)")));
    auto match = bracedRegExp.matchView(arg);
    if (match.hasMatch()) {
        *link = match.captured(1);
        *desc = match.captured(2);
        if (desc->isEmpty())
            *desc = *link;
    } else {
        qsizetype spaceAt = arg.indexOf(QLatin1Char(' '));
        if (arg.contains(QLatin1String(".html")) && spaceAt != -1) {
            *link = arg.left(spaceAt).trimmed().toString();
            *desc = arg.mid(spaceAt).trimmed().toString();
        } else {
            *link = arg.toString();
            *desc = *link;
        }
    }
}

/*!
  \internal
 */
void CodeParser::setLink(Node *node, Node::LinkType linkType, const QString &arg)
{
    QString link;
    QString desc;
    extractPageLinkAndDesc(arg, &link, &desc);
    node->setLink(linkType, link, desc);
}

/*!
  \brief Test for whether a doc comment warrants warnings.

  Returns true if qdoc should report that it has found something
  wrong with the qdoc comment in \a doc. Sometimes, qdoc should
  not report the warning, for example, when the comment contains
  the \c internal command, which normally means qdoc will not use
  the comment in the documentation anyway, so there is no point
  in reporting warnings about it.
 */
bool CodeParser::isWorthWarningAbout(const Doc &doc)
{
    return (s_showInternal || !doc.metaCommandsUsed().contains(QStringLiteral("internal")));
}

/*!
  For each node that is part of C++ API and produces a documentation
  page, this function ensures that the node belongs to a module.
 */
void CodeParser::checkModuleInclusion(Node *n)
{
    if (n->physicalModuleName().isEmpty()) {
        if (n->isInAPI() && !n->name().isEmpty()) {
            switch (n->nodeType()) {
            case Node::Class:
            case Node::Struct:
            case Node::Union:
            case Node::Namespace:
            case Node::HeaderFile:
                break;
            default:
                return;
            }
            n->setPhysicalModuleName(Generator::defaultModuleName());
            m_qdb->addToModule(Generator::defaultModuleName(), n);
            n->doc().location().warning(
                    QStringLiteral("Documentation for %1 '%2' has no \\inmodule command; "
                                   "using project name by default: %3")
                            .arg(Node::nodeTypeString(n->nodeType()), n->name(),
                                    n->physicalModuleName()));
        }
    }
}

QT_END_NAMESPACE
