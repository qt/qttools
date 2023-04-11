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
  Terminating a code parser is trivial.
 */
void CodeParser::terminateParser()
{
    // nothing.
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
    return (Config::instance().showInternal()
            || !doc.metaCommandsUsed().contains(QStringLiteral("internal")));
}

QT_END_NAMESPACE
