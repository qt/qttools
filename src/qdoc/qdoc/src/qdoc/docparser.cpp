// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "docparser.h"

#include "codemarker.h"
#include "doc.h"
#include "docprivate.h"
#include "editdistance.h"
#include "macro.h"
#include "openedlist.h"
#include "tokenizer.h"

#include <QtCore/qfile.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qtextstream.h>

#include <cctype>
#include <climits>
#include <functional>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

DocUtilities &DocParser::s_utilities = DocUtilities::instance();

enum {
    CMD_A,
    CMD_ANNOTATEDLIST,
    CMD_B,
    CMD_BADCODE,
    CMD_BOLD,
    CMD_BR,
    CMD_BRIEF,
    CMD_C,
    CMD_CAPTION,
    CMD_CODE,
    CMD_CODELINE,
    CMD_COMPARESWITH,
    CMD_DETAILS,
    CMD_DIV,
    CMD_DOTS,
    CMD_E,
    CMD_ELSE,
    CMD_ENDCODE,
    CMD_ENDCOMPARESWITH,
    CMD_ENDDETAILS,
    CMD_ENDDIV,
    CMD_ENDFOOTNOTE,
    CMD_ENDIF,
    CMD_ENDLEGALESE,
    CMD_ENDLINK,
    CMD_ENDLIST,
    CMD_ENDMAPREF,
    CMD_ENDOMIT,
    CMD_ENDQUOTATION,
    CMD_ENDRAW,
    CMD_ENDSECTION1,
    CMD_ENDSECTION2,
    CMD_ENDSECTION3,
    CMD_ENDSECTION4,
    CMD_ENDSIDEBAR,
    CMD_ENDTABLE,
    CMD_FOOTNOTE,
    CMD_GENERATELIST,
    CMD_HEADER,
    CMD_HR,
    CMD_I,
    CMD_IF,
    CMD_IMAGE,
    CMD_IMPORTANT,
    CMD_INCLUDE,
    CMD_INLINEIMAGE,
    CMD_INDEX,
    CMD_INPUT,
    CMD_KEYWORD,
    CMD_L,
    CMD_LEGALESE,
    CMD_LI,
    CMD_LINK,
    CMD_LIST,
    CMD_META,
    CMD_NOTE,
    CMD_NOTRANSLATE,
    CMD_O,
    CMD_OMIT,
    CMD_OMITVALUE,
    CMD_OVERLOAD,
    CMD_PRINTLINE,
    CMD_PRINTTO,
    CMD_PRINTUNTIL,
    CMD_QUOTATION,
    CMD_QUOTEFILE,
    CMD_QUOTEFROMFILE,
    CMD_RAW,
    CMD_ROW,
    CMD_SA,
    CMD_SECTION1,
    CMD_SECTION2,
    CMD_SECTION3,
    CMD_SECTION4,
    CMD_SIDEBAR,
    CMD_SINCELIST,
    CMD_SKIPLINE,
    CMD_SKIPTO,
    CMD_SKIPUNTIL,
    CMD_SNIPPET,
    CMD_SPAN,
    CMD_SUB,
    CMD_SUP,
    CMD_TABLE,
    CMD_TABLEOFCONTENTS,
    CMD_TARGET,
    CMD_TM,
    CMD_TT,
    CMD_UICONTROL,
    CMD_UNDERLINE,
    CMD_UNICODE,
    CMD_VALUE,
    CMD_WARNING,
    CMD_QML,
    CMD_ENDQML,
    CMD_CPP,
    CMD_ENDCPP,
    CMD_CPPTEXT,
    CMD_ENDCPPTEXT,
    NOT_A_CMD
};

static struct
{
    const char *name;
    int no;
    bool is_formatting_command { false };
} cmds[] = { { "a", CMD_A, true },
             { "annotatedlist", CMD_ANNOTATEDLIST },
             { "b", CMD_B, true },
             { "badcode", CMD_BADCODE },
             { "bold", CMD_BOLD, true },
             { "br", CMD_BR },
             { "brief", CMD_BRIEF },
             { "c", CMD_C, true },
             { "caption", CMD_CAPTION },
             { "code", CMD_CODE },
             { "codeline", CMD_CODELINE },
             { "compareswith", CMD_COMPARESWITH },
             { "details", CMD_DETAILS },
             { "div", CMD_DIV },
             { "dots", CMD_DOTS },
             { "e", CMD_E, true },
             { "else", CMD_ELSE },
             { "endcode", CMD_ENDCODE },
             { "endcompareswith", CMD_ENDCOMPARESWITH },
             { "enddetails", CMD_ENDDETAILS },
             { "enddiv", CMD_ENDDIV },
             { "endfootnote", CMD_ENDFOOTNOTE },
             { "endif", CMD_ENDIF },
             { "endlegalese", CMD_ENDLEGALESE },
             { "endlink", CMD_ENDLINK },
             { "endlist", CMD_ENDLIST },
             { "endmapref", CMD_ENDMAPREF },
             { "endomit", CMD_ENDOMIT },
             { "endquotation", CMD_ENDQUOTATION },
             { "endraw", CMD_ENDRAW },
             { "endsection1", CMD_ENDSECTION1 }, // ### don't document for now
             { "endsection2", CMD_ENDSECTION2 }, // ### don't document for now
             { "endsection3", CMD_ENDSECTION3 }, // ### don't document for now
             { "endsection4", CMD_ENDSECTION4 }, // ### don't document for now
             { "endsidebar", CMD_ENDSIDEBAR },
             { "endtable", CMD_ENDTABLE },
             { "footnote", CMD_FOOTNOTE },
             { "generatelist", CMD_GENERATELIST },
             { "header", CMD_HEADER },
             { "hr", CMD_HR },
             { "i", CMD_I, true },
             { "if", CMD_IF },
             { "image", CMD_IMAGE },
             { "important", CMD_IMPORTANT },
             { "include", CMD_INCLUDE },
             { "inlineimage", CMD_INLINEIMAGE },
             { "index", CMD_INDEX }, // ### don't document for now
             { "input", CMD_INPUT },
             { "keyword", CMD_KEYWORD },
             { "l", CMD_L },
             { "legalese", CMD_LEGALESE },
             { "li", CMD_LI },
             { "link", CMD_LINK },
             { "list", CMD_LIST },
             { "meta", CMD_META },
             { "note", CMD_NOTE },
             { "notranslate", CMD_NOTRANSLATE },
             { "o", CMD_O },
             { "omit", CMD_OMIT },
             { "omitvalue", CMD_OMITVALUE },
             { "overload", CMD_OVERLOAD },
             { "printline", CMD_PRINTLINE },
             { "printto", CMD_PRINTTO },
             { "printuntil", CMD_PRINTUNTIL },
             { "quotation", CMD_QUOTATION },
             { "quotefile", CMD_QUOTEFILE },
             { "quotefromfile", CMD_QUOTEFROMFILE },
             { "raw", CMD_RAW },
             { "row", CMD_ROW },
             { "sa", CMD_SA },
             { "section1", CMD_SECTION1 },
             { "section2", CMD_SECTION2 },
             { "section3", CMD_SECTION3 },
             { "section4", CMD_SECTION4 },
             { "sidebar", CMD_SIDEBAR },
             { "sincelist", CMD_SINCELIST },
             { "skipline", CMD_SKIPLINE },
             { "skipto", CMD_SKIPTO },
             { "skipuntil", CMD_SKIPUNTIL },
             { "snippet", CMD_SNIPPET },
             { "span", CMD_SPAN },
             { "sub", CMD_SUB, true },
             { "sup", CMD_SUP, true },
             { "table", CMD_TABLE },
             { "tableofcontents", CMD_TABLEOFCONTENTS },
             { "target", CMD_TARGET },
             { "tm", CMD_TM, true },
             { "tt", CMD_TT, true },
             { "uicontrol", CMD_UICONTROL, true },
             { "underline", CMD_UNDERLINE, true },
             { "unicode", CMD_UNICODE },
             { "value", CMD_VALUE },
             { "warning", CMD_WARNING },
             { "qml", CMD_QML },
             { "endqml", CMD_ENDQML },
             { "cpp", CMD_CPP },
             { "endcpp", CMD_ENDCPP },
             { "cpptext", CMD_CPPTEXT },
             { "endcpptext", CMD_ENDCPPTEXT },
             { nullptr, 0 } };

int DocParser::s_tabSize;
QStringList DocParser::s_ignoreWords;
bool DocParser::s_quoting = false;
FileResolver *DocParser::file_resolver{ nullptr };
static void processComparesWithCommand(DocPrivate *priv, const Location &location);

static QString cleanLink(const QString &link)
{
    qsizetype colonPos = link.indexOf(':');
    if ((colonPos == -1) || (!link.startsWith("file:") && !link.startsWith("mailto:")))
        return link;
    return link.mid(colonPos + 1).simplified();
}

void DocParser::initialize(const Config &config, FileResolver &file_resolver)
{
    s_tabSize = config.get(CONFIG_TABSIZE).asInt();
    s_ignoreWords = config.get(CONFIG_IGNOREWORDS).asStringList();

    int i = 0;
    while (cmds[i].name) {
        s_utilities.cmdHash.insert(cmds[i].name, cmds[i].no);

        if (cmds[i].no != i)
            Location::internalError(QStringLiteral("command %1 missing").arg(i));
        ++i;
    }

    // If any of the formats define quotinginformation, activate quoting
    DocParser::s_quoting = config.get(CONFIG_QUOTINGINFORMATION).asBool();
    const auto &outputFormats = config.getOutputFormats();
    for (const auto &format : outputFormats)
        DocParser::s_quoting = DocParser::s_quoting
                || config.get(format + Config::dot + CONFIG_QUOTINGINFORMATION).asBool();

    // KLUDGE: file_resolver is temporarily a pointer. See the
    // comment for file_resolver in the header file for more context.
    DocParser::file_resolver = &file_resolver;
}

/*!
  Parse the \a source string to build a Text data structure
  in \a docPrivate. The Text data structure is a linked list
  of Atoms.

  \a metaCommandSet is the set of metacommands that may be
  found in \a source. These metacommands are not markup text
  commands. They are topic commands and related metacommands.
 */
void DocParser::parse(const QString &source, DocPrivate *docPrivate,
                      const QSet<QString> &metaCommandSet, const QSet<QString> &possibleTopics)
{
    m_input = source;
    m_position = 0;
    m_inputLength = m_input.size();
    m_cachedLocation = docPrivate->m_start_loc;
    m_cachedPosition = 0;
    m_private = docPrivate;
    m_private->m_text << Atom::Nop;
    m_private->m_topics.clear();

    m_paragraphState = OutsideParagraph;
    m_inTableHeader = false;
    m_inTableRow = false;
    m_inTableItem = false;
    m_indexStartedParagraph = false;
    m_pendingParagraphLeftType = Atom::Nop;
    m_pendingParagraphRightType = Atom::Nop;

    m_braceDepth = 0;
    m_currentSection = Doc::NoSection;
    m_openedCommands.push(CMD_OMIT);
    m_quoter.reset();

    CodeMarker *marker = nullptr;
    Atom *currentLinkAtom = nullptr;
    QString p1, p2;
    QStack<bool> preprocessorSkipping;
    int numPreprocessorSkipping = 0;

    while (m_position < m_inputLength) {
        QChar ch = m_input.at(m_position);

        switch (ch.unicode()) {
        case '\\': {
            QString cmdStr;
            m_backslashPosition = m_position;
            ++m_position;
            while (m_position < m_inputLength) {
                ch = m_input.at(m_position);
                if (ch.isLetterOrNumber()) {
                    cmdStr += ch;
                    ++m_position;
                } else {
                    break;
                }
            }
            m_endPosition = m_position;
            if (cmdStr.isEmpty()) {
                if (m_position < m_inputLength) {
                    enterPara();
                    if (m_input.at(m_position).isSpace()) {
                        skipAllSpaces();
                        appendChar(QLatin1Char(' '));
                    } else {
                        appendChar(m_input.at(m_position++));
                    }
                }
            } else {
                // Ignore quoting atoms to make appendToCode()
                // append to the correct atom.
                if (!s_quoting || !isQuote(m_private->m_text.lastAtom()))
                    m_lastAtom = m_private->m_text.lastAtom();

                int cmd = s_utilities.cmdHash.value(cmdStr, NOT_A_CMD);
                switch (cmd) {
                case CMD_A:
                    enterPara();
                    p1 = getArgument();
                    appendAtom(Atom(Atom::FormattingLeft, ATOM_FORMATTING_PARAMETER));
                    appendAtom(Atom(Atom::String, p1));
                    appendAtom(Atom(Atom::FormattingRight, ATOM_FORMATTING_PARAMETER));
                    m_private->m_params.insert(p1);
                    break;
                case CMD_BADCODE:
                    leavePara();
                    appendAtom(Atom(Atom::CodeBad,
                           getCode(CMD_BADCODE, marker, getMetaCommandArgument(cmdStr))));
                    break;
                case CMD_BR:
                    enterPara();
                    appendAtom(Atom(Atom::BR));
                    break;
                case CMD_BOLD:
                    location().warning(QStringLiteral("'\\bold' is deprecated. Use '\\b'"));
                    Q_FALLTHROUGH();
                case CMD_B:
                    startFormat(ATOM_FORMATTING_BOLD, cmd);
                    break;
                case CMD_BRIEF:
                    leavePara();
                    enterPara(Atom::BriefLeft, Atom::BriefRight);
                    break;
                case CMD_C:
                    enterPara();
                    p1 = untabifyEtc(getArgument(ArgumentParsingOptions::Verbatim));
                    marker = CodeMarker::markerForCode(p1);
                    appendAtom(Atom(Atom::C, marker->markedUpCode(p1, nullptr, location())));
                    break;
                case CMD_CAPTION:
                    leavePara();
                    enterPara(Atom::CaptionLeft, Atom::CaptionRight);
                    break;
                case CMD_CODE:
                    leavePara();
                    appendAtom(Atom(Atom::Code, getCode(CMD_CODE, nullptr, getMetaCommandArgument(cmdStr))));
                    break;
                case CMD_QML:
                    leavePara();
                    appendAtom(Atom(Atom::Qml,
                           getCode(CMD_QML, CodeMarker::markerForLanguage(QLatin1String("QML")),
                                   getMetaCommandArgument(cmdStr))));
                    break;
                case CMD_DETAILS:
                    leavePara();
                    appendAtom(Atom(Atom::DetailsLeft, getArgument()));
                    m_openedCommands.push(cmd);
                    break;
                case CMD_ENDDETAILS:
                    leavePara();
                    appendAtom(Atom(Atom::DetailsRight));
                    closeCommand(cmd);
                    break;
                case CMD_DIV:
                    leavePara();
                    p1 = getArgument(ArgumentParsingOptions::Verbatim);
                    appendAtom(Atom(Atom::DivLeft, p1));
                    m_openedCommands.push(cmd);
                    break;
                case CMD_ENDDIV:
                    leavePara();
                    appendAtom(Atom(Atom::DivRight));
                    closeCommand(cmd);
                    break;
                case CMD_CODELINE:
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, " "));
                    }
                    if (isCode(m_lastAtom) && m_lastAtom->string().endsWith("\n\n"))
                        m_lastAtom->chopString();
                    appendToCode("\n");
                    break;
                case CMD_DOTS: {
                    QString arg = getOptionalArgument();
                    if (arg.isEmpty())
                        arg = "4";
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, arg));
                    }
                    if (isCode(m_lastAtom) && m_lastAtom->string().endsWith("\n\n"))
                        m_lastAtom->chopString();

                    int indent = arg.toInt();
                    for (int i = 0; i < indent; ++i)
                        appendToCode(" ");
                    appendToCode("...\n");
                    break;
                }
                case CMD_ELSE:
                    if (!preprocessorSkipping.empty()) {
                        if (preprocessorSkipping.top()) {
                            --numPreprocessorSkipping;
                        } else {
                            ++numPreprocessorSkipping;
                        }
                        preprocessorSkipping.top() = !preprocessorSkipping.top();
                        (void)getRestOfLine(); // ### should ensure that it's empty
                        if (numPreprocessorSkipping)
                            skipToNextPreprocessorCommand();
                    } else {
                        location().warning(
                                QStringLiteral("Unexpected '\\%1'").arg(cmdName(CMD_ELSE)));
                    }
                    break;
                case CMD_ENDCODE:
                    closeCommand(cmd);
                    break;
                case CMD_ENDQML:
                    closeCommand(cmd);
                    break;
                case CMD_ENDFOOTNOTE:
                    if (closeCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::FootnoteRight));
                    }
                    break;
                case CMD_ENDIF:
                    if (preprocessorSkipping.size() > 0) {
                        if (preprocessorSkipping.pop())
                            --numPreprocessorSkipping;
                        (void)getRestOfLine(); // ### should ensure that it's empty
                        if (numPreprocessorSkipping)
                            skipToNextPreprocessorCommand();
                    } else {
                        location().warning(
                                QStringLiteral("Unexpected '\\%1'").arg(cmdName(CMD_ENDIF)));
                    }
                    break;
                case CMD_ENDLEGALESE:
                    if (closeCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::LegaleseRight));
                    }
                    break;
                case CMD_ENDLINK:
                    if (closeCommand(cmd)) {
                        if (m_private->m_text.lastAtom()->type() == Atom::String
                            && m_private->m_text.lastAtom()->string().endsWith(QLatin1Char(' ')))
                            m_private->m_text.lastAtom()->chopString();
                        appendAtom(Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK));
                    }
                    break;
                case CMD_ENDLIST:
                    if (closeCommand(cmd)) {
                        leavePara();
                        if (m_openedLists.top().isStarted()) {
                            appendAtom(Atom(Atom::ListItemRight, m_openedLists.top().styleString()));
                            appendAtom(Atom(Atom::ListRight, m_openedLists.top().styleString()));
                        }
                        m_openedLists.pop();
                    }
                    break;
                case CMD_ENDOMIT:
                    closeCommand(cmd);
                    break;
                case CMD_ENDQUOTATION:
                    if (closeCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::QuotationRight));
                    }
                    break;
                case CMD_ENDRAW:
                    location().warning(
                            QStringLiteral("Unexpected '\\%1'").arg(cmdName(CMD_ENDRAW)));
                    break;
                case CMD_ENDSECTION1:
                    endSection(Doc::Section1, cmd);
                    break;
                case CMD_ENDSECTION2:
                    endSection(Doc::Section2, cmd);
                    break;
                case CMD_ENDSECTION3:
                    endSection(Doc::Section3, cmd);
                    break;
                case CMD_ENDSECTION4:
                    endSection(Doc::Section4, cmd);
                    break;
                case CMD_ENDSIDEBAR:
                    if (closeCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::SidebarRight));
                    }
                    break;
                case CMD_ENDTABLE:
                    if (closeCommand(cmd)) {
                        leaveTableRow();
                        appendAtom(Atom(Atom::TableRight));
                    }
                    break;
                case CMD_FOOTNOTE:
                    if (openCommand(cmd)) {
                        enterPara();
                        appendAtom(Atom(Atom::FootnoteLeft));
                    }
                    break;
                case CMD_ANNOTATEDLIST: {
                    // Optional sorting directive [ascending|descending]
                    if (isLeftBracketAhead())
                        p2 = getBracketedArgument();
                    else
                        p2.clear();
                    appendAtom(Atom(Atom::AnnotatedList, getArgument(), p2));
                } break;
                case CMD_SINCELIST:
                    leavePara();
                    appendAtom(Atom(Atom::SinceList, getRestOfLine().simplified()));
                    break;
                case CMD_GENERATELIST: {
                    // Optional sorting directive [ascending|descending]
                    if (isLeftBracketAhead())
                        p2 = getBracketedArgument();
                    else
                        p2.clear();
                    QString arg1 = getArgument();
                    QString arg2 = getOptionalArgument();
                    if (!arg2.isEmpty())
                        arg1 += " " + arg2;
                    appendAtom(Atom(Atom::GeneratedList, arg1, p2));
                } break;
                case CMD_HEADER:
                    if (m_openedCommands.top() == CMD_TABLE) {
                        leaveTableRow();
                        appendAtom(Atom(Atom::TableHeaderLeft));
                        m_inTableHeader = true;
                    } else {
                        if (m_openedCommands.contains(CMD_TABLE))
                            location().warning(QStringLiteral("Cannot use '\\%1' within '\\%2'")
                                                       .arg(cmdName(CMD_HEADER),
                                                            cmdName(m_openedCommands.top())));
                        else
                            location().warning(
                                    QStringLiteral("Cannot use '\\%1' outside of '\\%2'")
                                            .arg(cmdName(CMD_HEADER), cmdName(CMD_TABLE)));
                    }
                    break;
                case CMD_I:
                    location().warning(QStringLiteral(
                            "'\\i' is deprecated. Use '\\e' for italic or '\\li' for list item"));
                    Q_FALLTHROUGH();
                case CMD_E:
                    startFormat(ATOM_FORMATTING_ITALIC, cmd);
                    break;
                case CMD_HR:
                    leavePara();
                    appendAtom(Atom(Atom::HR));
                    break;
                case CMD_IF:
                    preprocessorSkipping.push(!Tokenizer::isTrue(getRestOfLine()));
                    if (preprocessorSkipping.top())
                        ++numPreprocessorSkipping;
                    if (numPreprocessorSkipping)
                        skipToNextPreprocessorCommand();
                    break;
                case CMD_IMAGE:
                    cmd_image(cmd);
                    break;
                case CMD_IMPORTANT:
                    leavePara();
                    enterPara(Atom::ImportantLeft, Atom::ImportantRight);
                    break;
                case CMD_INCLUDE:
                case CMD_INPUT: {
                    QString fileName = getArgument();
                    QStringList parameters;
                    QString identifier;
                    if (isLeftBraceAhead()) {
                        identifier = getArgument();
                        while (isLeftBraceAhead() && parameters.size() < 9)
                            parameters << getArgument();
                    } else {
                        identifier = getRestOfLine();
                    }
                    include(fileName, identifier, parameters);
                    break;
                }
                case CMD_INLINEIMAGE:
                    cmd_image(cmd);
                    break;
                case CMD_INDEX:
                    if (m_paragraphState == OutsideParagraph) {
                        enterPara();
                        m_indexStartedParagraph = true;
                    } else {
                        const Atom *last = m_private->m_text.lastAtom();
                        if (m_indexStartedParagraph
                            && (last->type() != Atom::FormattingRight
                                || last->string() != ATOM_FORMATTING_INDEX))
                            m_indexStartedParagraph = false;
                    }
                    startFormat(ATOM_FORMATTING_INDEX, cmd);
                    break;
                case CMD_KEYWORD:
                    leavePara();
                    insertKeyword(getRestOfLine());
                    break;
                case CMD_L:
                    enterPara();
                    if (isLeftBracketAhead())
                        p2 = getBracketedArgument();

                    p1 = getArgument();

                    appendAtom(LinkAtom(p1, p2, location()));

                    if (isLeftBraceAhead()) {
                        currentLinkAtom = m_private->m_text.lastAtom();
                        startFormat(ATOM_FORMATTING_LINK, cmd);
                    } else {
                        appendAtom(Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK));
                        appendAtom(Atom(Atom::String, cleanLink(p1)));
                        appendAtom(Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK));
                    }

                    p2.clear();

                    break;
                case CMD_LEGALESE:
                    leavePara();
                    if (openCommand(cmd))
                        appendAtom(Atom(Atom::LegaleseLeft));
                    docPrivate->m_hasLegalese = true;
                    break;
                case CMD_LINK:
                    if (openCommand(cmd)) {
                        enterPara();
                        p1 = getArgument();
                        appendAtom(Atom(Atom::Link, p1));
                        appendAtom(Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK));
                        skipSpacesOrOneEndl();
                    }
                    break;
                case CMD_LIST:
                    if (openCommand(cmd)) {
                        leavePara();
                        m_openedLists.push(OpenedList(location(), getOptionalArgument()));
                    }
                    break;
                case CMD_META:
                    m_private->constructExtra();
                    p1 = getArgument();
                    m_private->extra->m_metaMap.insert(p1, getArgument());
                    break;
                case CMD_NOTE:
                    leavePara();
                    enterPara(Atom::NoteLeft, Atom::NoteRight);
                    break;
                case CMD_NOTRANSLATE:
                    startFormat(ATOM_FORMATTING_NOTRANSLATE, cmd);
                    break;
                case CMD_O:
                    location().warning(QStringLiteral("'\\o' is deprecated. Use '\\li'"));
                    Q_FALLTHROUGH();
                case CMD_LI:
                    leavePara();
                    if (m_openedCommands.top() == CMD_LIST) {
                        if (m_openedLists.top().isStarted())
                            appendAtom(Atom(Atom::ListItemRight, m_openedLists.top().styleString()));
                        else
                            appendAtom(Atom(Atom::ListLeft, m_openedLists.top().styleString()));
                        m_openedLists.top().next();
                        appendAtom(Atom(Atom::ListItemNumber, m_openedLists.top().numberString()));
                        appendAtom(Atom(Atom::ListItemLeft, m_openedLists.top().styleString()));
                        enterPara();
                    } else if (m_openedCommands.top() == CMD_TABLE) {
                        p1 = "1,1";
                        p2.clear();
                        if (isLeftBraceAhead()) {
                            p1 = getArgument();
                            if (isLeftBraceAhead())
                                p2 = getArgument();
                        }

                        if (!m_inTableHeader && !m_inTableRow) {
                            location().warning(
                                    QStringLiteral("Missing '\\%1' or '\\%2' before '\\%3'")
                                            .arg(cmdName(CMD_HEADER), cmdName(CMD_ROW),
                                                 cmdName(CMD_LI)));
                            appendAtom(Atom(Atom::TableRowLeft));
                            m_inTableRow = true;
                        } else if (m_inTableItem) {
                            appendAtom(Atom(Atom::TableItemRight));
                            m_inTableItem = false;
                        }

                        appendAtom(Atom(Atom::TableItemLeft, p1, p2));
                        m_inTableItem = true;
                    } else
                        location().warning(
                                QStringLiteral("Command '\\%1' outside of '\\%2' and '\\%3'")
                                        .arg(cmdName(cmd), cmdName(CMD_LIST), cmdName(CMD_TABLE)));
                    break;
                case CMD_OMIT:
                    getUntilEnd(cmd);
                    break;
                case CMD_OMITVALUE: {
                    leavePara();
                    p1 = getArgument();
                    if (!m_private->m_enumItemList.contains(p1))
                        m_private->m_enumItemList.append(p1);
                    if (!m_private->m_omitEnumItemList.contains(p1))
                        m_private->m_omitEnumItemList.append(p1);
                    skipSpacesOrOneEndl();
                    // Skip potential description paragraph
                    while (m_position < m_inputLength && !isBlankLine()) {
                        skipAllSpaces();
                        if (qsizetype pos = m_position; pos < m_input.size()
                                && m_input.at(pos++).unicode() == '\\') {
                            QString nextCmdStr;
                            while (pos < m_input.size() && m_input[pos].isLetterOrNumber())
                                nextCmdStr += m_input[pos++];
                            int nextCmd = s_utilities.cmdHash.value(cmdStr, NOT_A_CMD);
                            if (nextCmd == cmd || nextCmd == CMD_VALUE)
                                break;
                        }
                        getRestOfLine();
                    }
                    break;
                }
                case CMD_COMPARESWITH:
                    leavePara();
                    p1 = getRestOfLine();
                    if (openCommand(cmd))
                        appendAtom(Atom(Atom::ComparesLeft, p1));
                    break;
                case CMD_ENDCOMPARESWITH:
                    if (closeCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::ComparesRight));
                        processComparesWithCommand(m_private, location());
                    }
                    break;
                case CMD_PRINTLINE: {
                    leavePara();
                    QString rest = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, rest));
                    }
                    appendToCode(m_quoter.quoteLine(location(), cmdStr, rest));
                    break;
                }
                case CMD_PRINTTO: {
                    leavePara();
                    QString rest = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, rest));
                    }
                    appendToCode(m_quoter.quoteTo(location(), cmdStr, rest));
                    break;
                }
                case CMD_PRINTUNTIL: {
                    leavePara();
                    QString rest = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, rest));
                    }
                    appendToCode(m_quoter.quoteUntil(location(), cmdStr, rest));
                    break;
                }
                case CMD_QUOTATION:
                    if (openCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::QuotationLeft));
                    }
                    break;
                case CMD_QUOTEFILE: {
                    leavePara();

                    QString fileName = getArgument();
                    quoteFromFile(fileName);
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, fileName));
                    }
                    appendAtom(Atom(Atom::Code, m_quoter.quoteTo(location(), cmdStr, QString())));
                    m_quoter.reset();
                    break;
                }
                case CMD_QUOTEFROMFILE: {
                    leavePara();
                    QString arg = getArgument();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, arg));
                    }
                    quoteFromFile(arg);
                    break;
                }
                case CMD_RAW:
                    leavePara();
                    p1 = getRestOfLine();
                    if (p1.isEmpty())
                        location().warning(QStringLiteral("Missing format name after '\\%1'")
                                                   .arg(cmdName(CMD_RAW)));
                    appendAtom(Atom(Atom::FormatIf, p1));
                    appendAtom(Atom(Atom::RawString, untabifyEtc(getUntilEnd(cmd))));
                    appendAtom(Atom(Atom::FormatElse));
                    appendAtom(Atom(Atom::FormatEndif));
                    break;
                case CMD_ROW:
                    if (m_openedCommands.top() == CMD_TABLE) {
                        p1.clear();
                        if (isLeftBraceAhead())
                            p1 = getArgument(ArgumentParsingOptions::Verbatim);
                        leaveTableRow();
                        appendAtom(Atom(Atom::TableRowLeft, p1));
                        m_inTableRow = true;
                    } else {
                        if (m_openedCommands.contains(CMD_TABLE))
                            location().warning(QStringLiteral("Cannot use '\\%1' within '\\%2'")
                                                       .arg(cmdName(CMD_ROW),
                                                            cmdName(m_openedCommands.top())));
                        else
                            location().warning(QStringLiteral("Cannot use '\\%1' outside of '\\%2'")
                                                       .arg(cmdName(CMD_ROW), cmdName(CMD_TABLE)));
                    }
                    break;
                case CMD_SA:
                    parseAlso();
                    break;
                case CMD_SECTION1:
                    startSection(Doc::Section1, cmd);
                    break;
                case CMD_SECTION2:
                    startSection(Doc::Section2, cmd);
                    break;
                case CMD_SECTION3:
                    startSection(Doc::Section3, cmd);
                    break;
                case CMD_SECTION4:
                    startSection(Doc::Section4, cmd);
                    break;
                case CMD_SIDEBAR:
                    if (openCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::SidebarLeft));
                    }
                    break;
                case CMD_SKIPLINE: {
                    leavePara();
                    QString rest = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, rest));
                    }
                    m_quoter.quoteLine(location(), cmdStr, rest);
                    break;
                }
                case CMD_SKIPTO: {
                    leavePara();
                    QString rest = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, rest));
                    }
                    m_quoter.quoteTo(location(), cmdStr, rest);
                    break;
                }
                case CMD_SKIPUNTIL: {
                    leavePara();
                    QString rest = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::CodeQuoteCommand, cmdStr));
                        appendAtom(Atom(Atom::CodeQuoteArgument, rest));
                    }
                    m_quoter.quoteUntil(location(), cmdStr, rest);
                    break;
                }
                case CMD_SPAN:
                    p1 = ATOM_FORMATTING_SPAN + getArgument(ArgumentParsingOptions::Verbatim);
                    startFormat(p1, cmd);
                    break;
                case CMD_SNIPPET: {
                    leavePara();
                    QString snippet = getArgument();
                    QString identifier = getRestOfLine();
                    if (s_quoting) {
                        appendAtom(Atom(Atom::SnippetCommand, cmdStr));
                        appendAtom(Atom(Atom::SnippetLocation, snippet));
                        appendAtom(Atom(Atom::SnippetIdentifier, identifier));
                    }
                    marker = CodeMarker::markerForFileName(snippet);
                    quoteFromFile(snippet);
                    appendToCode(m_quoter.quoteSnippet(location(), identifier), marker->atomType());
                    break;
                }
                case CMD_SUB:
                    startFormat(ATOM_FORMATTING_SUBSCRIPT, cmd);
                    break;
                case CMD_SUP:
                    startFormat(ATOM_FORMATTING_SUPERSCRIPT, cmd);
                    break;
                case CMD_TABLE:
                    leaveValueList();
                    p1 = getOptionalArgument();
                    p2 = getOptionalArgument();
                    if (openCommand(cmd)) {
                        leavePara();
                        appendAtom(Atom(Atom::TableLeft, p1, p2));
                        m_inTableHeader = false;
                        m_inTableRow = false;
                        m_inTableItem = false;
                    }
                    break;
                case CMD_TABLEOFCONTENTS:
                    p1 = "1";
                    if (isLeftBraceAhead())
                        p1 = getArgument();
                    p1 += QLatin1Char(',');
                    p1 += QString::number((int)getSectioningUnit());
                    appendAtom(Atom(Atom::TableOfContents, p1));
                    break;
                case CMD_TARGET:
                    if (m_openedCommands.top() == CMD_TABLE && !m_inTableItem) {
                        location().warning("Found a \\target command outside table item in a table.\n"
                                           "Move the \\target inside the \\li to resolve this warning.");
                    }
                    insertTarget(getRestOfLine());
                    break;
                case CMD_TM:
                     // Ignore command while parsing \section<N> argument
                    if (m_paragraphState != InSingleLineParagraph)
                        startFormat(ATOM_FORMATTING_TRADEMARK, cmd);
                    break;
                case CMD_TT:
                    startFormat(ATOM_FORMATTING_TELETYPE, cmd);
                    break;
                case CMD_UICONTROL:
                    startFormat(ATOM_FORMATTING_UICONTROL, cmd);
                    break;
                case CMD_UNDERLINE:
                    startFormat(ATOM_FORMATTING_UNDERLINE, cmd);
                    break;
                case CMD_UNICODE: {
                    enterPara();
                    p1 = getArgument();
                    bool ok;
                    uint unicodeChar = p1.toUInt(&ok, 0);
                    if (!ok || (unicodeChar == 0x0000) || (unicodeChar > 0xFFFE))
                        location().warning(
                                QStringLiteral("Invalid Unicode character '%1' specified with '%2'")
                                        .arg(p1, cmdName(CMD_UNICODE)));
                    else
                        appendAtom(Atom(Atom::String, QChar(unicodeChar)));
                    break;
                }
                case CMD_VALUE:
                    leaveValue();
                    if (m_openedLists.top().style() == OpenedList::Value) {
                        QString p2;
                        p1 = getArgument();
                        if (p1.startsWith(QLatin1String("[since "))
                            && p1.endsWith(QLatin1String("]"))) {
                            p2 = p1.mid(7, p1.size() - 8);
                            p1 = getArgument();
                        }
                        if (!m_private->m_enumItemList.contains(p1))
                            m_private->m_enumItemList.append(p1);

                        m_openedLists.top().next();
                        appendAtom(Atom(Atom::ListTagLeft, ATOM_LIST_VALUE));
                        appendAtom(Atom(Atom::String, p1));
                        appendAtom(Atom(Atom::ListTagRight, ATOM_LIST_VALUE));
                        if (!p2.isEmpty()) {
                            appendAtom(Atom(Atom::SinceTagLeft, ATOM_LIST_VALUE));
                            appendAtom(Atom(Atom::String, p2));
                            appendAtom(Atom(Atom::SinceTagRight, ATOM_LIST_VALUE));
                        }
                        appendAtom(Atom(Atom::ListItemLeft, ATOM_LIST_VALUE));

                        skipSpacesOrOneEndl();
                        if (isBlankLine())
                            appendAtom(Atom(Atom::Nop));
                    } else {
                        // ### unknown problems
                    }
                    break;
                case CMD_WARNING:
                    leavePara();
                    enterPara(Atom::WarningLeft, Atom::WarningRight);
                    break;
                case CMD_OVERLOAD:
                    cmd_overload();
                    break;
                case NOT_A_CMD:
                    if (metaCommandSet.contains(cmdStr)) {
                        QString arg;
                        QString bracketedArg;
                        m_private->m_metacommandsUsed.insert(cmdStr);
                        if (isLeftBracketAhead())
                            bracketedArg = getBracketedArgument();
                        // Force a linebreak after \obsolete or \deprecated
                        // to treat potential arguments as a new text paragraph.
                        if (m_position < m_inputLength
                            && (cmdStr == QLatin1String("obsolete")
                                || cmdStr == QLatin1String("deprecated")))
                            m_input[m_position] = '\n';
                        else
                            arg = getMetaCommandArgument(cmdStr);
                        m_private->m_metaCommandMap[cmdStr].append(ArgPair(arg, bracketedArg));
                        if (possibleTopics.contains(cmdStr)) {
                            if (!cmdStr.endsWith(QLatin1String("propertygroup")))
                                m_private->m_topics.append(Topic(cmdStr, std::move(arg)));
                        }
                    } else if (s_utilities.macroHash.contains(cmdStr)) {
                        const Macro &macro = s_utilities.macroHash.value(cmdStr);
                        QStringList macroArgs;
                        int numPendingFi = 0;
                        int numFormatDefs = 0;
                        for (auto it = macro.m_otherDefs.constBegin();
                             it != macro.m_otherDefs.constEnd(); ++it) {
                            if (it.key() != "match") {
                                if (numFormatDefs == 0)
                                    macroArgs = getMacroArguments(cmdStr, macro);
                                appendAtom(Atom(Atom::FormatIf, it.key()));
                                expandMacro(*it, macroArgs);
                                ++numFormatDefs;
                                if (it == macro.m_otherDefs.constEnd()) {
                                    appendAtom(Atom(Atom::FormatEndif));
                                } else {
                                    appendAtom(Atom(Atom::FormatElse));
                                    ++numPendingFi;
                                }
                            }
                        }
                        while (numPendingFi-- > 0)
                            appendAtom(Atom(Atom::FormatEndif));

                        if (!macro.m_defaultDef.isEmpty()) {
                            if (numFormatDefs > 0) {
                                macro.m_defaultDefLocation.warning(
                                        QStringLiteral("Macro cannot have both "
                                                       "format-specific and qdoc-"
                                                       "syntax definitions"));
                            } else {
                                QString expanded = expandMacroToString(cmdStr, macro);
                                m_input.replace(m_backslashPosition,
                                                m_endPosition - m_backslashPosition, expanded);
                                m_inputLength = m_input.size();
                                m_position = m_backslashPosition;
                            }
                        }
                    } else if (isAutoLinkString(cmdStr)) {
                        appendWord(cmdStr);
                    } else {
                        if (!cmdStr.endsWith("propertygroup")) {
                            // The QML property group commands are no longer required
                            // for grouping QML properties. They are allowed but ignored.
                            location().warning(QStringLiteral("Unknown command '\\%1'").arg(cmdStr),
                                               detailsUnknownCommand(metaCommandSet, cmdStr));
                        }
                        enterPara();
                        appendAtom(Atom(Atom::UnknownCommand, cmdStr));
                    }
                }
            } // case '\\' (qdoc markup command)
            break;
        }
        case '-': { // Catch en-dash (--) and em-dash (---) markup here.
            enterPara();
            qsizetype dashCount = 1;
            ++m_position;

            // Figure out how many hyphens in a row.
            while ((m_position < m_inputLength) && (m_input.at(m_position) == '-')) {
                ++dashCount;
                ++m_position;
            }

            if (dashCount == 3) {
                // 3 hyphens, append an em-dash character.
                const QChar emDash(8212);
                appendChar(emDash);
            } else if (dashCount == 2) {
                // 2 hyphens; append an en-dash character.
                const QChar enDash(8211);
                appendChar(enDash);
            } else {
                // dashCount is either one or more than three. Append a hyphen
                // the appropriate number of times. This ensures '----' doesn't
                // end up as an em-dash followed by a hyphen in the output.
                for (qsizetype i = 0; i < dashCount; ++i)
                    appendChar('-');
            }
            break;
        }
        case '{':
            enterPara();
            appendChar('{');
            ++m_braceDepth;
            ++m_position;
            break;
        case '}': {
            --m_braceDepth;
            ++m_position;

            auto format = m_pendingFormats.find(m_braceDepth);
            if (format == m_pendingFormats.end()) {
                enterPara();
                appendChar('}');
            } else {
                const auto &last{m_private->m_text.lastAtom()->string()};
                appendAtom(Atom(Atom::FormattingRight, *format));
                if (*format == ATOM_FORMATTING_INDEX) {
                    if (m_indexStartedParagraph)
                        skipAllSpaces();
                } else if (*format == ATOM_FORMATTING_LINK) {
                    // hack for C++ to support links like
                    // \l{QString::}{count()}
                    if (currentLinkAtom && currentLinkAtom->string().endsWith("::")) {
                        QString suffix =
                                Text::subText(currentLinkAtom, m_private->m_text.lastAtom())
                                        .toString();
                        currentLinkAtom->concatenateString(suffix);
                    }
                    currentLinkAtom = nullptr;
                } else if (*format == ATOM_FORMATTING_TRADEMARK) {
                    m_private->m_text.lastAtom()->append(last);
                }
                m_pendingFormats.erase(format);
            }
            break;
        }
            // Do not parse content after '//!' comments
        case '/': {
            if (m_position + 2 < m_inputLength)
                if (m_input.at(m_position + 1) == '/')
                    if (m_input.at(m_position + 2) == '!') {
                        m_position += 2;
                        getRestOfLine();
                        if (m_input.at(m_position - 1) == '\n')
                            --m_position;
                        break;
                    }
            Q_FALLTHROUGH(); // fall through
        }
        default: {
            bool newWord;
            switch (m_private->m_text.lastAtom()->type()) {
            case Atom::ParaLeft:
                newWord = true;
                break;
            default:
                newWord = false;
            }

            if (m_paragraphState == OutsideParagraph) {
                if (ch.isSpace()) {
                    ++m_position;
                    newWord = false;
                } else {
                    enterPara();
                    newWord = true;
                }
            } else {
                if (ch.isSpace()) {
                    ++m_position;
                    if ((ch == '\n')
                        && (m_paragraphState == InSingleLineParagraph || isBlankLine())) {
                        leavePara();
                        newWord = false;
                    } else {
                        appendChar(' ');
                        newWord = true;
                    }
                } else {
                    newWord = true;
                }
            }

            if (newWord) {
                qsizetype startPos = m_position;
                // No auto-linking inside links
                bool autolink = (!m_pendingFormats.isEmpty() &&
                        m_pendingFormats.last() == ATOM_FORMATTING_LINK) ?
                            false : isAutoLinkString(m_input, m_position);
                if (m_position == startPos) {
                    if (!ch.isSpace()) {
                        appendChar(ch);
                        ++m_position;
                    }
                } else {
                    QString word = m_input.mid(startPos, m_position - startPos);
                    if (autolink) {
                        if (s_ignoreWords.contains(word) || word.startsWith(QString("__")))
                            appendWord(word);
                        else
                            appendAtom(Atom(Atom::AutoLink, word));
                    } else {
                        appendWord(word);
                    }
                }
            }
        } // default:
        } // switch (ch.unicode())
    }
    leaveValueList();

    // for compatibility
    if (m_openedCommands.top() == CMD_LEGALESE) {
        appendAtom(Atom(Atom::LegaleseRight));
        m_openedCommands.pop();
    }

    if (m_openedCommands.top() != CMD_OMIT) {
        location().warning(
                QStringLiteral("Missing '\\%1'").arg(endCmdName(m_openedCommands.top())));
    } else if (preprocessorSkipping.size() > 0) {
        location().warning(QStringLiteral("Missing '\\%1'").arg(cmdName(CMD_ENDIF)));
    }

    if (m_currentSection > Doc::NoSection) {
        appendAtom(Atom(Atom::SectionRight, QString::number(m_currentSection)));
        m_currentSection = Doc::NoSection;
    }

    m_private->m_text.stripFirstAtom();
}

/*!
  Returns the current location.
 */
Location &DocParser::location()
{
    while (!m_openedInputs.isEmpty() && m_openedInputs.top() <= m_position) {
        m_cachedLocation.pop();
        m_cachedPosition = m_openedInputs.pop();
    }
    while (m_cachedPosition < m_position)
        m_cachedLocation.advance(m_input.at(m_cachedPosition++));
    return m_cachedLocation;
}

QString DocParser::detailsUnknownCommand(const QSet<QString> &metaCommandSet, const QString &str)
{
    QSet<QString> commandSet = metaCommandSet;
    int i = 0;
    while (cmds[i].name != nullptr) {
        commandSet.insert(cmds[i].name);
        ++i;
    }

    QString best = nearestName(str, commandSet);
    if (best.isEmpty())
        return QString();
    return QStringLiteral("Maybe you meant '\\%1'?").arg(best);
}

/*!
    \internal

    Issues a warning about an empty or duplicate definition of either a
    \\target or \\keyword command (determined by \a cmdString) at
    \a location. \a duplicateDefinition is the target being processed;
    the already registered definition is \a previousDefinition.
 */
static void warnAboutEmptyOrPreexistingTarget(const Location &location, const QString &duplicateDefinition,
                                              const QString &cmdString, const QString &previousDefinition)
{
    if (duplicateDefinition.isEmpty()) {
        location.warning("Expected an argument for \\%1"_L1.arg(cmdString));
    } else {
        location.warning(
                "Duplicate %3 name '%1'. The previous occurrence is here: %2"_L1
                        .arg(duplicateDefinition, previousDefinition, cmdString));
    }
}

/*!
    \internal

    \brief Registers \a target as a linkable entity.

    The main purpose of this method is to register a target as defined
    along with its location, so that becomes a valid link target from other
    parts of the documentation.

    If the \a target name is already registered, a warning is issued,
    as multiple definitions are problematic.

    \sa insertKeyword, target-command
 */
void DocParser::insertTarget(const QString &target)
{
    if (target.isEmpty() || m_targetMap.contains(target))
        return warnAboutEmptyOrPreexistingTarget(location(), target,
                s_utilities.cmdHash.key(CMD_TARGET), m_targetMap[target].toString());

    m_targetMap.insert(target, location());
    m_private->constructExtra();

    appendAtom(Atom(Atom::Target, target));
    m_private->extra->m_targets.append(m_private->m_text.lastAtom());
}

/*!
    \internal

    \brief Registers \a keyword as a linkable entity.

    The main purpose of this method is to register a keyword as defined
    along with its location, so that becomes a valid link target from other
    parts of the documentation.

    If the \a keyword name is already registered, a warning is issued,
    as multiple definitions are problematic.

    \sa insertTarget, keyword-command
 */
void DocParser::insertKeyword(const QString &keyword)
{
    if (keyword.isEmpty() || m_targetMap.contains(keyword))
        return warnAboutEmptyOrPreexistingTarget(location(), keyword,
                s_utilities.cmdHash.key(CMD_KEYWORD), m_targetMap[keyword].toString());

    m_targetMap.insert(keyword, location());
    m_private->constructExtra();

    appendAtom(Atom(Atom::Keyword, keyword));
    m_private->extra->m_keywords.append(m_private->m_text.lastAtom());
}

void DocParser::include(const QString &fileName, const QString &identifier, const QStringList &parameters)
{
    if (location().depth() > 16)
        location().fatal(QStringLiteral("Too many nested '\\%1's").arg(cmdName(CMD_INCLUDE)));
    QString filePath = Config::instance().getIncludeFilePath(fileName);
    if (filePath.isEmpty()) {
        location().warning(QStringLiteral("Cannot find qdoc include file '%1'").arg(fileName));
    } else {
        QFile inFile(filePath);
        if (!inFile.open(QFile::ReadOnly)) {
            location().warning(
                    QStringLiteral("Cannot open qdoc include file '%1'").arg(filePath));
        } else {
            location().push(fileName);
            QTextStream inStream(&inFile);
            QString includedContent = inStream.readAll();
            inFile.close();

            if (identifier.isEmpty()) {
                expandArgumentsInString(includedContent, parameters);
                m_input.insert(m_position, includedContent);
                m_inputLength = m_input.size();
                m_openedInputs.push(m_position + includedContent.size());
            } else {
                QStringList lineBuffer = includedContent.split(QLatin1Char('\n'));
                qsizetype bufLen{lineBuffer.size()};
                qsizetype i;
                QStringView trimmedLine;
                for (i = 0; i < bufLen; ++i) {
                    trimmedLine = QStringView{lineBuffer[i]}.trimmed();
                    if (trimmedLine.startsWith(QLatin1String("//!")) &&
                        trimmedLine.contains(identifier))
                        break;
                }
                if (i < bufLen - 1) {
                    ++i;
                } else {
                    location().warning(
                            QStringLiteral("Cannot find '%1' in '%2'").arg(identifier, filePath));
                    return;
                }
                QString result;
                do {
                    trimmedLine = QStringView{lineBuffer[i]}.trimmed();
                    if (trimmedLine.startsWith(QLatin1String("//!")) &&
                        trimmedLine.contains(identifier))
                        break;
                    else
                        result += lineBuffer[i] + QLatin1Char('\n');
                    ++i;
                } while (i < bufLen);

                expandArgumentsInString(result, parameters);
                if (result.isEmpty()) {
                    location().warning(QStringLiteral("Empty qdoc snippet '%1' in '%2'")
                                               .arg(identifier, filePath));
                } else {
                    m_input.insert(m_position, result);
                    m_inputLength = m_input.size();
                    m_openedInputs.push(m_position + result.size());
                }
            }
        }
    }
}

void DocParser::startFormat(const QString &format, int cmd)
{
    enterPara();

    for (const auto &item : std::as_const(m_pendingFormats)) {
        if (item == format) {
            location().warning(QStringLiteral("Cannot nest '\\%1' commands").arg(cmdName(cmd)));
            return;
        }
    }

    appendAtom(Atom(Atom::FormattingLeft, format));

    if (isLeftBraceAhead()) {
        skipSpacesOrOneEndl();
        m_pendingFormats.insert(m_braceDepth, format);
        ++m_braceDepth;
        ++m_position;
    } else {
        const auto &arg{getArgument()};
        appendAtom(Atom(Atom::String, arg));
        appendAtom(Atom(Atom::FormattingRight, format));
        if (format == ATOM_FORMATTING_INDEX && m_indexStartedParagraph) {
            skipAllSpaces();
            m_indexStartedParagraph = false;
        } else if (format == ATOM_FORMATTING_TRADEMARK) {
            m_private->m_text.lastAtom()->append(arg);
        }
    }
}

bool DocParser::openCommand(int cmd)
{
    int outer = m_openedCommands.top();
    bool ok = true;

    if (cmd == CMD_COMPARESWITH && m_openedCommands.contains(cmd)) {
        location().warning(u"Cannot nest '\\%1' commands"_s.arg(cmdName(cmd)));
        return false;
    } else if (cmd != CMD_LINK) {
        if (outer == CMD_LIST) {
            ok = (cmd == CMD_FOOTNOTE || cmd == CMD_LIST);
        } else if (outer == CMD_SIDEBAR) {
            ok = (cmd == CMD_LIST || cmd == CMD_QUOTATION || cmd == CMD_SIDEBAR);
        } else if (outer == CMD_QUOTATION) {
            ok = (cmd == CMD_LIST);
        } else if (outer == CMD_TABLE) {
            ok = (cmd == CMD_LIST || cmd == CMD_FOOTNOTE || cmd == CMD_QUOTATION);
        } else if (outer == CMD_FOOTNOTE || outer == CMD_LINK) {
            ok = false;
        }
    }

    if (ok) {
        m_openedCommands.push(cmd);
    } else {
        location().warning(
                QStringLiteral("Can't use '\\%1' in '\\%2'").arg(cmdName(cmd), cmdName(outer)));
    }
    return ok;
}

/*!
    Returns \c true if \a word qualifies for auto-linking.

    A word qualifies for auto-linking if either:

    \list
        \li It is composed of only upper and lowercase characters
        \li AND It contains at least one uppercase character that is not
        the first character of word
        \li AND it contains at least two lowercase characters
    \endlist

    Or

    \list
        \li It is composed only of uppercase characters, lowercase
        characters, characters in [_@] and the \c {"::"} sequence.
        \li It contains at least one uppercase character that is not
        the first character of word or it contains at least one
        lowercase character
        \li AND it contains at least one character in [_@] or it
        contains at least one \c {"::"} sequence.
    \endlist

    Inserting or suffixing, but not prefixing, any sequence in [0-9]+
    in a word that qualifies for auto-linking by the above rules
    preserves the auto-linkability of the word.

    Suffixing the sequence \c {"()"} to a word that qualifies for
    auto-linking by the above rules preserves the auto-linkability of
    a word.

    FInally, a word qualifies for auto-linking if:

    \list
        \li It is composed of only uppercase characters, lowercase
        characters and the sequence \c {"()"}
        \li AND it contains one lowercase character and a sequence of zero, one
        or two upper or lowercase characters
        \li AND it contains exactly one sequence \c {"()"}
        \li AND it contains one sequence \c {"()"} as the last two
        characters of word
    \endlist

    For example, \c {"fOo"}, \c {"FooBar"} and \c {"foobaR"} qualify
    for auto-linking by the first rule.

    \c {"QT_DEBUG"}, \c {"::Qt"} and \c {"std::move"} qualifies for
    auto-linking by the second rule.

    \c {"SIMDVector256"} qualifies by suffixing \c {"SIMDVector"},
    which qualifies by the first rule, with the sequence \c {"256"}

    \c {"FooBar::Bar()"} qualifies by suffixing \c {"FooBar::Bar"},
    which qualifies by the first and second rule, with the sequence \c
    {"()"}.

    \c {"Foo()"} and \c {"a()"} qualifies by the last rule.

    Instead, \c {"Q"}, \c {"flower"}, \c {"_"} and \c {"()"} do not
    qualify for auto-linking.

    The rules are intended as a heuristic to catch common cases in the
    Qt documentation where a word might represent an important
    documented element such as a class or a method that could be
    linked to while at the same time avoiding catching common words
    such as \c {"A"} or \c {"Nonetheless"}.

    The heuristic assumes that Qt's codebase respects a style where
    camelCasing is the standard for most of the elements, a function
    call is identified by the use of parenthesis and certain elements,
    such as macros, might be fully uppercase.

    Furthemore, it assumes that the Qt codebase is written in a
    language that has an identifier grammar similar to the one for
    C++.
*/
inline bool DocParser::isAutoLinkString(const QString &word)
{
    qsizetype start = 0;
    return isAutoLinkString(word, start) && (start == word.size());
}

/*!
    Returns \c true if a prefix of a substring of \a word qualifies
    for auto-linking.

    Respects the same parsing rules as the unary overload.

    \a curPos defines the offset, from the first character of \ word,
    at which the parsed substring starts.

    When the call completes, \a curPos represents the offset, from the
    first character of word, that is the successor of the offset of
    the last parsed character.

    If the return value of the call is \c true, it is guaranteed that
    the prefix of the substring of \word that contains the characters
    from the initial value of \a curPos and up to but not including \a
    curPos qualifies for auto-linking.

    If \a curPos is initially zero, the considered substring is the
    entirety of \a word.
*/
bool DocParser::isAutoLinkString(const QString &word, qsizetype &curPos)
{
    qsizetype len = word.size();
    qsizetype startPos = curPos;
    int numUppercase = 0;
    int numLowercase = 0;
    int numStrangeSymbols = 0;

    while (curPos < len) {
        unsigned char latin1Ch = word.at(curPos).toLatin1();
        if (islower(latin1Ch)) {
            ++numLowercase;
            ++curPos;
        } else if (isupper(latin1Ch)) {
            if (curPos > startPos)
                ++numUppercase;
            ++curPos;
        } else if (isdigit(latin1Ch)) {
            if (curPos > startPos)
                ++curPos;
            else
                break;
        } else if (latin1Ch == '_' || latin1Ch == '@') {
            ++numStrangeSymbols;
            ++curPos;
        } else if ((latin1Ch == ':') && (curPos < len - 1)
                   && (word.at(curPos + 1) == QLatin1Char(':'))) {
            ++numStrangeSymbols;
            curPos += 2;
        } else if (latin1Ch == '(') {
            if ((curPos < len - 1) && (word.at(curPos + 1) == QLatin1Char(')'))) {
                ++numStrangeSymbols;
                curPos += 2;
            }

            break;
       } else {
            break;
        }
    }

    return ((numUppercase >= 1 && numLowercase >= 2) || (numStrangeSymbols > 0 && (numUppercase + numLowercase >= 1)));
}

bool DocParser::closeCommand(int endCmd)
{
    if (endCmdFor(m_openedCommands.top()) == endCmd && m_openedCommands.size() > 1) {
        m_openedCommands.pop();
        return true;
    } else {
        bool contains = false;
        QStack<int> opened2 = m_openedCommands;
        while (opened2.size() > 1) {
            if (endCmdFor(opened2.top()) == endCmd) {
                contains = true;
                break;
            }
            opened2.pop();
        }

        if (contains) {
            while (endCmdFor(m_openedCommands.top()) != endCmd && m_openedCommands.size() > 1) {
                location().warning(
                        QStringLiteral("Missing '\\%1' before '\\%2'")
                                .arg(endCmdName(m_openedCommands.top()), cmdName(endCmd)));
                m_openedCommands.pop();
            }
        } else {
            location().warning(QStringLiteral("Unexpected '\\%1'").arg(cmdName(endCmd)));
        }
        return false;
    }
}

void DocParser::startSection(Doc::Sections unit, int cmd)
{
    leaveValueList();

    if (m_currentSection == Doc::NoSection) {
        m_currentSection = static_cast<Doc::Sections>(unit);
        m_private->constructExtra();
    } else {
        endSection(unit, cmd);
    }

    appendAtom(Atom(Atom::SectionLeft, QString::number(unit)));
    m_private->constructExtra();
    m_private->extra->m_tableOfContents.append(m_private->m_text.lastAtom());
    m_private->extra->m_tableOfContentsLevels.append(unit);
    enterPara(Atom::SectionHeadingLeft, Atom::SectionHeadingRight, QString::number(unit));
    m_currentSection = unit;
}

void DocParser::endSection(int, int) // (int unit, int endCmd)
{
    leavePara();
    appendAtom(Atom(Atom::SectionRight, QString::number(m_currentSection)));
    m_currentSection = (Doc::NoSection);
}

/*!
    \internal

    \brief Processes CMD_IMAGE and CMD_INLINEIMAGE, as specified by \a cmd.

    The first argument to the command is the image file name. The rest of the
    line is an optional string that's used as the text description of the image
    (e.g. the HTML <img> alt attribute). The optional argument can be wrapped in
    curly braces, in which case it can span multiple lines.

    This function may modify the optional argument by removing one pair of
    double quotes, if they wrap the string.
 */
void DocParser::cmd_image(int cmd) {
    Atom::AtomType imageAtom{};
    switch (cmd) {
    case CMD_IMAGE: {
        leaveValueList();
        imageAtom = Atom::AtomType::Image;
        break;
    }
    case CMD_INLINEIMAGE: {
        enterPara();
        imageAtom = Atom::AtomType::InlineImage;
        break;
    }
    default:
        break;
    }

    const QString imageFileName = getArgument();
    QString imageText;
    bool hasAltTextArgument{false};
    if (isLeftBraceAhead()) {
        hasAltTextArgument = true;
        imageText = getArgument();
    } else if (cmd == CMD_IMAGE) {
        imageText = getRestOfLine();
    }

    if (imageText.length() > 1) {
        if (imageText.front() == '"' && imageText.back() == '"') {
            imageText.removeFirst();
            imageText.removeLast();
        }
    }

    if (!hasAltTextArgument && imageText.isEmpty() && Config::instance().reportMissingAltTextForImages())
        location().report(QStringLiteral("\\%1 %2 is without a textual description, "
                                         "QDoc will not generate an alt text for the image.")
                                  .arg(cmdName(cmd))
                                  .arg(imageFileName));
    appendAtom(Atom(imageAtom, imageFileName));
    appendAtom(Atom(Atom::ImageText, imageText));
}

/*!
    \brief Processes the \\overload command in documentation comments.

    This function registers metadata when the \\overload command is used in a
    documentation comment. It records the use of the command and stores any
    arguments to the command. Arguments are optional and can be passed with or
    without curly braces. This allows the user to use either of:

    \badcode
        \overload someFunction
        \overload {someFunction}
    \endcode
 */
void DocParser::cmd_overload()
{
    const QString cmd{"overload"};

    leavePara();
    m_private->m_metacommandsUsed.insert(cmd);
    const QString overloadArgument = isBlankLine() ? getMetaCommandArgument(cmd) : getRestOfLine();

    m_private->m_metaCommandMap[cmd].append(ArgPair(overloadArgument, QString()));
}

/*!
    \internal
    \brief Parses arguments to QDoc's see also command.

    Parses space or comma separated arguments passed to the \\sa command.
    Multi-line input requires that the arguments are comma separated. Wrap
    arguments in curly braces for multi-word targets, and for scope resolution
    (for example, {QString::}{count()}).

    This method updates the list of links for the See also section.

    \sa {DocPrivate::}{addAlso()}, getArgument()
 */
void DocParser::parseAlso()
{
    auto line_comment = [this]() -> bool {
        skipSpacesOnLine();
        if (m_position + 2 > m_inputLength)
            return false;
        if (m_input[m_position].unicode() == '/') {
            if (m_input[m_position + 1].unicode() == '/') {
                if (m_input[m_position + 2].unicode() == '!') {
                    return true;
                }
            }
        }
        return false;
    };

    auto skip_everything_until_newline = [this]() -> void {
        while (m_position < m_inputLength && m_input[m_position] != '\n')
            ++m_position;
    };

    leavePara();
    skipSpacesOnLine();
    while (m_position < m_inputLength && m_input[m_position] != '\n') {
        QString target;
        QString str;
        bool skipMe = false;

        if (m_input[m_position] == '{') {
            target = getArgument();
            skipSpacesOnLine();
            if (m_position < m_inputLength && m_input[m_position] == '{') {
                str = getArgument();

                // hack for C++ to support links like \l{QString::}{count()}
                if (target.endsWith("::"))
                    target += str;
            } else {
                str = target;
            }
        } else {
            target = getArgument();
            str = cleanLink(target);
            if (target == QLatin1String("and") || target == QLatin1String("."))
                skipMe = true;
        }

        if (!skipMe) {
            Text also;
            also << Atom(Atom::Link, target) << Atom(Atom::FormattingLeft, ATOM_FORMATTING_LINK)
                 << str << Atom(Atom::FormattingRight, ATOM_FORMATTING_LINK);
            m_private->addAlso(also);
        }

        skipSpacesOnLine();

        if (line_comment())
            skip_everything_until_newline();

        if (m_position < m_inputLength && m_input[m_position] == ',') {
            m_position++;
            if (line_comment())
                skip_everything_until_newline();
            skipSpacesOrOneEndl();
        } else if (m_position >= m_inputLength || m_input[m_position] != '\n') {
            location().warning(QStringLiteral("Missing comma in '\\%1'").arg(cmdName(CMD_SA)));
        }
    }
}

void DocParser::appendAtom(const Atom& atom) {
    m_private->m_text << atom;
}

void DocParser::appendAtom(const LinkAtom& atom) {
    m_private->m_text << atom;
}

void DocParser::appendChar(QChar ch)
{
    if (m_private->m_text.lastAtom()->type() != Atom::String)
        appendAtom(Atom(Atom::String));
    Atom *atom = m_private->m_text.lastAtom();
    if (ch == QLatin1Char(' ')) {
        if (!atom->string().endsWith(QLatin1Char(' ')))
            atom->appendChar(QLatin1Char(' '));
    } else
        atom->appendChar(ch);
}

void DocParser::appendWord(const QString &word)
{
    if (m_private->m_text.lastAtom()->type() != Atom::String) {
        appendAtom(Atom(Atom::String, word));
    } else
        m_private->m_text.lastAtom()->concatenateString(word);
}

void DocParser::appendToCode(const QString &markedCode)
{
    if (!isCode(m_lastAtom)) {
        appendAtom(Atom(Atom::Code));
        m_lastAtom = m_private->m_text.lastAtom();
    }
    m_lastAtom->concatenateString(markedCode);
}

void DocParser::appendToCode(const QString &markedCode, Atom::AtomType defaultType)
{
    if (!isCode(m_lastAtom)) {
        appendAtom(Atom(defaultType, markedCode));
        m_lastAtom = m_private->m_text.lastAtom();
    } else {
        m_lastAtom->concatenateString(markedCode);
    }
}

void DocParser::enterPara(Atom::AtomType leftType, Atom::AtomType rightType, const QString &string)
{
    if (m_paragraphState != OutsideParagraph)
        return;

    if ((m_private->m_text.lastAtom()->type() != Atom::ListItemLeft)
        && (m_private->m_text.lastAtom()->type() != Atom::DivLeft)
        && (m_private->m_text.lastAtom()->type() != Atom::DetailsLeft)) {
        leaveValueList();
    }

    appendAtom(Atom(leftType, string));
    m_indexStartedParagraph = false;
    m_pendingParagraphLeftType = leftType;
    m_pendingParagraphRightType = rightType;
    m_pendingParagraphString = string;
    if (leftType == Atom::SectionHeadingLeft) {
        m_paragraphState = InSingleLineParagraph;
    } else {
        m_paragraphState = InMultiLineParagraph;
    }
    skipSpacesOrOneEndl();
}

void DocParser::leavePara()
{
    if (m_paragraphState == OutsideParagraph)
        return;

    if (!m_pendingFormats.isEmpty()) {
        location().warning(QStringLiteral("Missing '}'"));
        m_pendingFormats.clear();
    }

    if (m_private->m_text.lastAtom()->type() == m_pendingParagraphLeftType) {
        m_private->m_text.stripLastAtom();
    } else {
        if (m_private->m_text.lastAtom()->type() == Atom::String
            && m_private->m_text.lastAtom()->string().endsWith(QLatin1Char(' '))) {
            m_private->m_text.lastAtom()->chopString();
        }
        appendAtom(Atom(m_pendingParagraphRightType, m_pendingParagraphString));
    }
    m_paragraphState = OutsideParagraph;
    m_indexStartedParagraph = false;
    m_pendingParagraphRightType = Atom::Nop;
    m_pendingParagraphString.clear();
}

void DocParser::leaveValue()
{
    leavePara();
    if (m_openedLists.isEmpty()) {
        m_openedLists.push(OpenedList(OpenedList::Value));
        appendAtom(Atom(Atom::ListLeft, ATOM_LIST_VALUE));
    } else {
        if (m_private->m_text.lastAtom()->type() == Atom::Nop)
            m_private->m_text.stripLastAtom();
        appendAtom(Atom(Atom::ListItemRight, ATOM_LIST_VALUE));
    }
}

void DocParser::leaveValueList()
{
    leavePara();
    if (!m_openedLists.isEmpty() && (m_openedLists.top().style() == OpenedList::Value)) {
        if (m_private->m_text.lastAtom()->type() == Atom::Nop)
            m_private->m_text.stripLastAtom();
        appendAtom(Atom(Atom::ListItemRight, ATOM_LIST_VALUE));
        appendAtom(Atom(Atom::ListRight, ATOM_LIST_VALUE));
        m_openedLists.pop();
    }
}

void DocParser::leaveTableRow()
{
    if (m_inTableItem) {
        leavePara();
        appendAtom(Atom(Atom::TableItemRight));
        m_inTableItem = false;
    }
    if (m_inTableHeader) {
        appendAtom(Atom(Atom::TableHeaderRight));
        m_inTableHeader = false;
    }
    if (m_inTableRow) {
        appendAtom(Atom(Atom::TableRowRight));
        m_inTableRow = false;
    }
}

void DocParser::quoteFromFile(const QString &filename)
{
    // KLUDGE: We dereference file_resolver as it is temporarily a pointer.
    // See the comment for file_resolver in the header files for more context.
    //
    // We spefically dereference it, instead of using the arrow
    // operator, to better represent that we do not consider this as
    // an actual pointer, as it should not be.
    //
    // Do note that we are considering it informally safe to
    // dereference the pointer, as we expect it to always hold a value
    // at this point, but actual enforcement of this appears nowhere
    // in the codebase.
    auto maybe_resolved_file{(*file_resolver).resolve(filename)};
    if (!maybe_resolved_file) {
        // TODO: [uncentralized-admonition][failed-resolve-file]
        // This warning is required in multiple places.
        // To ensure the consistency of the warning and avoid
        // duplicating code everywhere, provide a centralized effort
        // where the warning message can be generated (but not
        // issued).
        // The current format is based on what was used before, review
        // it when it is moved out.
        QString details = std::transform_reduce(
            (*file_resolver).get_search_directories().cbegin(),
            (*file_resolver).get_search_directories().cend(),
            u"Searched directories:"_s,
            std::plus(),
            [](const DirectoryPath &directory_path) -> QString { return u' ' + directory_path.value(); }
        );

        location().warning(u"Cannot find file to quote from: %1"_s.arg(filename), details);

        // REMARK: The following is duplicated from
        // Doc::quoteFromFile. If, for some reason (such as a file
        // that is inaccessible), the quoting fails but, previously,
        // the logic duplicated here was still run.
        // This is not true anymore as quoteFromFile does require a
        // resolved file to be run now.
        // It is not entirely clear if this is required for the
        // semantics of DocParser to be preserved, but for the sake of
        // avoiding premature breakages this was retained.
        // Do note that this should be considered temporary as the
        // quoter state, if any will be preserved, should not be
        // managed in such a spread and unlocal way.
        m_quoter.reset();

        CodeMarker *marker = CodeMarker::markerForFileName(QString{});
        m_quoter.quoteFromFile(filename, QString{}, marker->markedUpCode(QString{}, nullptr, location()));
    } else Doc::quoteFromFile(location(), m_quoter, *maybe_resolved_file);
}

/*!
  Expands a macro in-place in input.

  Expects the current \e pos in the input to point to a backslash, and the macro to have a
  default definition. Format-specific macros are not expanded.

  Behavior depends on \a options:

  \value ArgumentParsingOptions::Default
         Default macro expansion; the string following the backslash
         must be a macro with a default definition.
  \value ArgumentParsingOptions::Verbatim
         The string following the backslash is rendered verbatim;
         No macro expansion is performed.
  \value ArgumentParsingOptions::MacroArguments
         Used for parsing argument(s) for a macro. Allows expanding
         macros, and also preserves a subset of commands (formatting
         commands) within the macro argument.

  \note In addition to macros, a valid use for a backslash in an argument include
  escaping non-alnum characters, and splitting a single argument across multiple
  lines by escaping newlines. Escaping is also handled here.

  Returns \c true on successful macro expansion.
 */
bool DocParser::expandMacro(ArgumentParsingOptions options)
{
    Q_ASSERT(m_input[m_position].unicode() == '\\');

    if (options == ArgumentParsingOptions::Verbatim)
        return false;

    QString cmdStr;
    qsizetype backslashPos = m_position++;
    while (m_position < m_input.size() && m_input[m_position].isLetterOrNumber())
        cmdStr += m_input[m_position++];

    m_endPosition = m_position;
    if (!cmdStr.isEmpty()) {
        if (s_utilities.macroHash.contains(cmdStr)) {
            const Macro &macro = s_utilities.macroHash.value(cmdStr);
            if (!macro.m_defaultDef.isEmpty()) {
                QString expanded = expandMacroToString(cmdStr, macro);
                m_input.replace(backslashPos, m_position - backslashPos, expanded);
                m_inputLength = m_input.size();
                m_position = backslashPos;
                return true;
            } else {
                location().warning("Macro '%1' does not have a default definition"_L1.arg(cmdStr));
            }
        } else {
            int cmd = s_utilities.cmdHash.value(cmdStr, NOT_A_CMD);
            m_position = backslashPos;
            if (options != ArgumentParsingOptions::MacroArguments
                    || cmd == NOT_A_CMD || !cmds[cmd].is_formatting_command) {
                location().warning("Unknown macro '%1'"_L1.arg(cmdStr));
                ++m_position;
            }
        }
    } else if (m_input[m_position].isSpace()) {
        skipAllSpaces();
    } else if (m_input[m_position].unicode() == '\\') {
        // allow escaping a backslash
        m_input.remove(m_position--, 1);
        --m_inputLength;
    }
    return false;
}

void DocParser::expandMacro(const QString &def, const QStringList &args)
{
    if (args.isEmpty()) {
        appendAtom(Atom(Atom::RawString, def));
    } else {
        QString rawString;

        for (int j = 0; j < def.size(); ++j) {
            if (int paramNo = def[j].unicode(); paramNo >= 1 && paramNo <= args.length()) {
                if (!rawString.isEmpty()) {
                    appendAtom(Atom(Atom::RawString, rawString));
                    rawString.clear();
                }
                appendAtom(Atom(Atom::String, args[paramNo - 1]));
            } else {
                rawString += def[j];
            }
        }
        if (!rawString.isEmpty())
            appendAtom(Atom(Atom::RawString, rawString));
    }
}

QString DocParser::expandMacroToString(const QString &name, const Macro &macro)
{
    const QString &def{macro.m_defaultDef};
    QString rawString;

    if (macro.numParams == 0) {
        rawString = macro.m_defaultDef;
    } else {
        QStringList args{getMacroArguments(name, macro)};

        for (int j = 0; j < def.size(); ++j) {
            int paramNo = def[j].unicode();
            rawString += (paramNo >= 1 && paramNo <= args.length()) ? args[paramNo - 1] : def[j];
        }
    }
    QString matchExpr{macro.m_otherDefs.value("match")};
    if (matchExpr.isEmpty())
        return rawString;

    QString result;
    QRegularExpression re(matchExpr);
    int capStart = (re.captureCount() > 0) ? 1 : 0;
    qsizetype i = 0;
    QRegularExpressionMatch match;
    while ((match = re.match(rawString, i)).hasMatch()) {
        for (int c = capStart; c <= re.captureCount(); ++c)
            result += match.captured(c);
        i = match.capturedEnd();
    }

    return result;
}

Doc::Sections DocParser::getSectioningUnit()
{
    QString name = getOptionalArgument();

    if (name == "section1") {
        return Doc::Section1;
    } else if (name == "section2") {
        return Doc::Section2;
    } else if (name == "section3") {
        return Doc::Section3;
    } else if (name == "section4") {
        return Doc::Section4;
    } else if (name.isEmpty()) {
        return Doc::NoSection;
    } else {
        location().warning(QStringLiteral("Invalid section '%1'").arg(name));
        return Doc::NoSection;
    }
}

/*!
  Gets an argument that is enclosed in braces and returns it
  without the enclosing braces. On entry, the current character
  is the left brace. On exit, the current character is the one
  that comes after the right brace.

  If \a options is ArgumentParsingOptions::Verbatim, no macro
  expansion is performed, nor is the returned string stripped
  of extra whitespace.
 */
QString DocParser::getBracedArgument(ArgumentParsingOptions options)
{
    QString arg;
    int delimDepth = 0;
    if (m_position < m_input.size() && m_input[m_position] == '{') {
        ++m_position;
        while (m_position < m_input.size() && delimDepth >= 0) {
            switch (m_input[m_position].unicode()) {
            case '{':
                ++delimDepth;
                arg += QLatin1Char('{');
                ++m_position;
                break;
            case '}':
                --delimDepth;
                if (delimDepth >= 0)
                    arg += QLatin1Char('}');
                ++m_position;
                break;
            case '\\':
                if (!expandMacro(options))
                    arg += m_input[m_position++];
                break;
            default:
                if (m_input[m_position].isSpace() && options != ArgumentParsingOptions::Verbatim)
                    arg += QChar(' ');
                else
                    arg += m_input[m_position];
                ++m_position;
            }
        }
        if (delimDepth > 0)
            location().warning(QStringLiteral("Missing '}'"));
    }
    m_endPosition = m_position;
    return arg;
}

/*!
  Parses and returns an argument for a command, using
  specific parsing \a options.

  Typically, an argument ends at the next white-space. However,
  braces can be used to group words:

  {a few words}

  Also, opening and closing parentheses have to match. Thus,

  printf("%d\n", x)

  is an argument too, although it contains spaces. Finally,
  trailing punctuation is not included in an argument, nor is 's.
*/
QString DocParser::getArgument(ArgumentParsingOptions options)
{
    skipSpacesOrOneEndl();

    int delimDepth = 0;
    qsizetype startPos = m_position;
    QString arg = getBracedArgument(options);
    if (arg.isEmpty()) {
        while ((m_position < m_input.size())
               && ((delimDepth > 0) || ((delimDepth == 0) && !m_input[m_position].isSpace()))) {
            switch (m_input[m_position].unicode()) {
            case '(':
            case '[':
            case '{':
                ++delimDepth;
                arg += m_input[m_position];
                ++m_position;
                break;
            case ')':
            case ']':
            case '}':
                --delimDepth;
                if (m_position == startPos || delimDepth >= 0) {
                    arg += m_input[m_position];
                    ++m_position;
                }
                break;
            case '\\':
                if (!expandMacro(options))
                    arg += m_input[m_position++];
                break;
            default:
                arg += m_input[m_position];
                ++m_position;
            }
        }
        m_endPosition = m_position;
        if ((arg.size() > 1) && (QString(".,:;!?").indexOf(m_input[m_position - 1]) != -1)
            && !arg.endsWith("...")) {
            arg.truncate(arg.size() - 1);
            --m_position;
        }
        if (arg.size() > 2 && m_input.mid(m_position - 2, 2) == "'s") {
            arg.truncate(arg.size() - 2);
            m_position -= 2;
        }
    }
    return arg.simplified();
}

/*!
  Gets an argument that is enclosed in brackets and returns it
  without the enclosing brackets. On entry, the current character
  is the left bracket. On exit, the current character is the one
  that comes after the right bracket.
 */
QString DocParser::getBracketedArgument()
{
    QString arg;
    int delimDepth = 0;
    skipSpacesOrOneEndl();
    if (m_position < m_input.size() && m_input[m_position] == '[') {
        ++m_position;
        while (m_position < m_input.size() && delimDepth >= 0) {
            switch (m_input[m_position].unicode()) {
            case '[':
                ++delimDepth;
                arg += QLatin1Char('[');
                ++m_position;
                break;
            case ']':
                --delimDepth;
                if (delimDepth >= 0)
                    arg += QLatin1Char(']');
                ++m_position;
                break;
            case '\\':
                arg += m_input[m_position];
                ++m_position;
                break;
            default:
                arg += m_input[m_position];
                ++m_position;
            }
        }
        if (delimDepth > 0)
            location().warning(QStringLiteral("Missing ']'"));
    }
    return arg;
}


/*!
    Returns the list of arguments passed to a \a macro with name \a name.

    If a macro takes more than a single argument, they are expected to be
    wrapped in braces.
*/
QStringList DocParser::getMacroArguments(const QString &name, const Macro &macro)
{
    QStringList args;
    for (int i = 0; i < macro.numParams; ++i) {
        if (macro.numParams == 1 || isLeftBraceAhead()) {
            args << getArgument(ArgumentParsingOptions::MacroArguments);
        } else {
            location().warning(QStringLiteral("Macro '\\%1' invoked with too few"
                    " arguments (expected %2, got %3)")
                            .arg(name)
                            .arg(macro.numParams)
                            .arg(i));
            break;
        }
    }
    return args;
}

/*!
    Returns the next token as an optional argument unless the token is
    another command. The next token can be preceded by spaces and an optional
    newline.
*/
QString DocParser::getOptionalArgument()
{
    skipSpacesOrOneEndl();
    if (m_position + 1 < m_input.size() && m_input[m_position] == '\\'
        && m_input[m_position + 1].isLetterOrNumber()) {
        return QString();
    } else {
        return getArgument();
    }
}

/*!
    \brief Create a string that may optionally span multiple lines as one line.

    Process a block of text that may span multiple lines using trailing
    backslashes (`\`) as line continuation character. Trailing backslashes and
    any newline character that follow them are removed.

    Returns a string as if it was one continuous line of text. If trailing
    backslashes are removed, the method returns a "simplified" QString, which
    means any sequence of internal whitespace is replaced with a single space.

    Whitespace at the start and end is always removed from the returned string.

    \sa QString::simplified(), QString::trimmed().
 */
QString DocParser::getRestOfLine()
{
    auto lineHasTrailingBackslash = [this](bool trailingBackslash) -> bool {
        while (m_position < m_inputLength && m_input[m_position] != '\n') {
            if (m_input[m_position] == '\\' && !trailingBackslash) {
                trailingBackslash = true;
                ++m_position;
                skipSpacesOnLine();
            } else {
                trailingBackslash = false;
                ++m_position;
            }
        }
        return trailingBackslash;
    };

    QString rest_of_line;
    skipSpacesOnLine();
    bool trailing_backslash{ false };
    bool return_simplified_string{ false };

    for (qsizetype start_position = m_position; m_position < m_inputLength; ++m_position) {
        trailing_backslash = lineHasTrailingBackslash(trailing_backslash);

        if (!rest_of_line.isEmpty())
            rest_of_line += QLatin1Char(' ');
        rest_of_line += m_input.sliced(start_position, m_position - start_position);

        if (trailing_backslash) {
            rest_of_line.truncate(rest_of_line.lastIndexOf('\\'));
            return_simplified_string = true;
        }

        if (m_position < m_inputLength)
            ++m_position;

        if (!trailing_backslash)
            break;
        start_position = m_position;
    }

    if (return_simplified_string)
        return rest_of_line.simplified();

    return rest_of_line.trimmed();
}

/*!
  The metacommand argument is normally the remaining text to
  the right of the metacommand itself. The extra blanks are
  stripped and the argument string is returned.
 */
QString DocParser::getMetaCommandArgument(const QString &cmdStr)
{
    skipSpacesOnLine();

    qsizetype begin = m_position;
    int parenDepth = 0;

    while (m_position < m_input.size() && (m_input[m_position] != '\n' || parenDepth > 0)) {
        if (m_input.at(m_position) == '(')
            ++parenDepth;
        else if (m_input.at(m_position) == ')')
            --parenDepth;
        else if (m_input.at(m_position) == '\\' && expandMacro(ArgumentParsingOptions::Default))
            continue;
        ++m_position;
    }
    if (m_position == m_input.size() && parenDepth > 0) {
        m_position = begin;
        location().warning(QStringLiteral("Unbalanced parentheses in '%1'").arg(cmdStr));
    }

    QString t = m_input.mid(begin, m_position - begin).simplified();
    skipSpacesOnLine();
    return t;
}

QString DocParser::getUntilEnd(int cmd)
{
    int endCmd = endCmdFor(cmd);
    QRegularExpression rx("\\\\" + cmdName(endCmd) + "\\b");
    QString t;
    auto match = rx.match(m_input, m_position);

    if (!match.hasMatch()) {
        location().warning(QStringLiteral("Missing '\\%1'").arg(cmdName(endCmd)));
        m_position = m_input.size();
    } else {
        qsizetype end = match.capturedStart();
        t = m_input.mid(m_position, end - m_position);
        m_position = match.capturedEnd();
    }
    return t;
}

void DocParser::expandArgumentsInString(QString &str, const QStringList &args)
{
    if (args.isEmpty())
        return;

    qsizetype paramNo;
    qsizetype j = 0;
    while (j < str.size()) {
        if (str[j] == '\\' && j < str.size() - 1 && (paramNo = str[j + 1].digitValue()) >= 1
            && paramNo <= args.size()) {
            const QString &r = args[paramNo - 1];
            str.replace(j, 2, r);
            j += qMin(1, r.size());
        } else {
            ++j;
        }
    }
}

/*!
    Returns the marked-up code following the code-quoting command \a cmd, expanding
    any arguments passed in \a argStr.

    Uses the \a marker to mark up the code. If it's \c nullptr, resolve the marker
    based on the topic and the quoted code itself.
*/
QString DocParser::getCode(int cmd, CodeMarker *marker, const QString &argStr)
{
    QString code = untabifyEtc(getUntilEnd(cmd));
    expandArgumentsInString(code, argStr.split(" ", Qt::SkipEmptyParts));

    int indent = indentLevel(code);
    code = dedent(indent, code);

    // If we're in a QML topic, check if the QML marker recognizes the code
    if (!marker && !m_private->m_topics.isEmpty()
            && m_private->m_topics[0].m_topic.startsWith("qml")) {
        auto qmlMarker = CodeMarker::markerForLanguage("QML");
        marker = (qmlMarker && qmlMarker->recognizeCode(code)) ? qmlMarker : nullptr;
    }
    if (marker == nullptr)
        marker = CodeMarker::markerForCode(code);
    return marker->markedUpCode(code, nullptr, location());
}

bool DocParser::isBlankLine()
{
    qsizetype i = m_position;

    while (i < m_inputLength && m_input[i].isSpace()) {
        if (m_input[i] == '\n')
            return true;
        ++i;
    }
    return false;
}

bool DocParser::isLeftBraceAhead()
{
    int numEndl = 0;
    qsizetype i = m_position;

    while (i < m_inputLength && m_input[i].isSpace() && numEndl < 2) {
        // ### bug with '\\'
        if (m_input[i] == '\n')
            numEndl++;
        ++i;
    }
    return numEndl < 2 && i < m_inputLength && m_input[i] == '{';
}

bool DocParser::isLeftBracketAhead()
{
    int numEndl = 0;
    qsizetype i = m_position;

    while (i < m_inputLength && m_input[i].isSpace() && numEndl < 2) {
        // ### bug with '\\'
        if (m_input[i] == '\n')
            numEndl++;
        ++i;
    }
    return numEndl < 2 && i < m_inputLength && m_input[i] == '[';
}

/*!
  Skips to the next non-space character or EOL.
 */
void DocParser::skipSpacesOnLine()
{
    while ((m_position < m_input.size()) && m_input[m_position].isSpace()
           && (m_input[m_position].unicode() != '\n'))
        ++m_position;
}

/*!
  Skips spaces and one EOL.
 */
void DocParser::skipSpacesOrOneEndl()
{
    qsizetype firstEndl = -1;
    while (m_position < m_input.size() && m_input[m_position].isSpace()) {
        QChar ch = m_input[m_position];
        if (ch == '\n') {
            if (firstEndl == -1) {
                firstEndl = m_position;
            } else {
                m_position = firstEndl;
                break;
            }
        }
        ++m_position;
    }
}

void DocParser::skipAllSpaces()
{
    while (m_position < m_inputLength && m_input[m_position].isSpace())
        ++m_position;
}

void DocParser::skipToNextPreprocessorCommand()
{
    QRegularExpression rx("\\\\(?:" + cmdName(CMD_IF) + QLatin1Char('|') + cmdName(CMD_ELSE)
                          + QLatin1Char('|') + cmdName(CMD_ENDIF) + ")\\b");
    auto match = rx.match(m_input, m_position + 1); // ### + 1 necessary?

    if (!match.hasMatch())
        m_position = m_input.size();
    else
        m_position = match.capturedStart();
}

int DocParser::endCmdFor(int cmd)
{
    switch (cmd) {
    case CMD_BADCODE:
        return CMD_ENDCODE;
    case CMD_CODE:
        return CMD_ENDCODE;
    case CMD_COMPARESWITH:
        return CMD_ENDCOMPARESWITH;
    case CMD_DETAILS:
        return CMD_ENDDETAILS;
    case CMD_DIV:
        return CMD_ENDDIV;
    case CMD_QML:
        return CMD_ENDQML;
    case CMD_FOOTNOTE:
        return CMD_ENDFOOTNOTE;
    case CMD_LEGALESE:
        return CMD_ENDLEGALESE;
    case CMD_LINK:
        return CMD_ENDLINK;
    case CMD_LIST:
        return CMD_ENDLIST;
    case CMD_OMIT:
        return CMD_ENDOMIT;
    case CMD_QUOTATION:
        return CMD_ENDQUOTATION;
    case CMD_RAW:
        return CMD_ENDRAW;
    case CMD_SECTION1:
        return CMD_ENDSECTION1;
    case CMD_SECTION2:
        return CMD_ENDSECTION2;
    case CMD_SECTION3:
        return CMD_ENDSECTION3;
    case CMD_SECTION4:
        return CMD_ENDSECTION4;
    case CMD_SIDEBAR:
        return CMD_ENDSIDEBAR;
    case CMD_TABLE:
        return CMD_ENDTABLE;
    default:
        return cmd;
    }
}

QString DocParser::cmdName(int cmd)
{
    return cmds[cmd].name;
}

QString DocParser::endCmdName(int cmd)
{
    return cmdName(endCmdFor(cmd));
}

QString DocParser::untabifyEtc(const QString &str)
{
    QString result;
    result.reserve(str.size());
    int column = 0;

    for (const auto &character : str) {
        if (character == QLatin1Char('\r'))
            continue;
        if (character == QLatin1Char('\t')) {
            result += &"        "[column % s_tabSize];
            column = ((column / s_tabSize) + 1) * s_tabSize;
            continue;
        }
        if (character == QLatin1Char('\n')) {
            while (result.endsWith(QLatin1Char(' ')))
                result.chop(1);
            result += character;
            column = 0;
            continue;
        }
        result += character;
        ++column;
    }

    while (result.endsWith("\n\n"))
        result.truncate(result.size() - 1);
    while (result.startsWith(QLatin1Char('\n')))
        result = result.mid(1);

    return result;
}

int DocParser::indentLevel(const QString &str)
{
    int minIndent = INT_MAX;
    int column = 0;

    for (const auto &character : str) {
        if (character == '\n') {
            column = 0;
        } else {
            if (character != ' ' && column < minIndent)
                minIndent = column;
            ++column;
        }
    }
    return minIndent;
}

QString DocParser::dedent(int level, const QString &str)
{
    if (level == 0)
        return str;

    QString result;
    int column = 0;

    for (const auto &character : str) {
        if (character == QLatin1Char('\n')) {
            result += '\n';
            column = 0;
        } else {
            if (column >= level)
                result += character;
            ++column;
        }
    }
    return result;
}

/*!
 Returns \c true if \a atom represents a code snippet.
 */
bool DocParser::isCode(const Atom *atom)
{
    Atom::AtomType type = atom->type();
    return (type == Atom::Code || type == Atom::Qml);
}

/*!
 Returns \c true if \a atom represents quoting information.
 */
bool DocParser::isQuote(const Atom *atom)
{
    Atom::AtomType type = atom->type();
    return (type == Atom::CodeQuoteArgument || type == Atom::CodeQuoteCommand
            || type == Atom::SnippetCommand || type == Atom::SnippetIdentifier
            || type == Atom::SnippetLocation);
}

/*!
    \internal
    Processes the arguments passed to the \\compareswith block command.
    The arguments are stored as text within the first atom of the block
    (Atom::ComparesLeft).

    Extracts the comparison category and the list of types, and stores
    the information into a map accessed via \a priv.
*/
static void processComparesWithCommand(DocPrivate *priv, const Location &location)
{
    static auto take_while = [](QStringView input, auto predicate) {
        QStringView::size_type end{0};

        while (end < input.size() && std::invoke(predicate, input[end]))
            ++end;

        return std::make_tuple(input.sliced(0, end), input.sliced(end));
    };

    static auto peek = [](QStringView input, QChar c) {
        return !input.empty() && input.first() == c;
    };

    static auto skip_one = [](QStringView input) {
        if (input.empty()) return std::make_tuple(QStringView{}, input);
        else return std::make_tuple(input.sliced(0, 1), input.sliced(1));
    };

    static auto enclosed = [](QStringView input, QChar open, QChar close) {
        if (!peek(input, open)) return std::make_tuple(QStringView{}, input);

        auto [opened, without_open] = skip_one(input);
        auto [parsed, remaining] = take_while(without_open, [close](QChar c){ return c != close; });

        if (remaining.empty()) return std::make_tuple(QStringView{}, input);

        auto [closed, without_close] = skip_one(remaining);

        return std::make_tuple(parsed.trimmed(), without_close);
    };

    static auto one_of = [](auto first, auto second) {
        return [first, second](QStringView input) {
            auto [parsed, remaining] = std::invoke(first, input);

            if (parsed.empty()) return std::invoke(second, input);
            else return std::make_tuple(parsed, remaining);
        };
    };

    static auto collect = [](QStringView input, auto parser) {
        QStringList collected{};

        while (true) {
          auto [parsed, remaining] = std::invoke(parser, input);

          if (parsed.empty()) break;
          collected.append(parsed.toString());

          input = remaining;
        };

        return collected;
    };

    static auto spaces = [](QStringView input) {
        return take_while(input, [](QChar c){ return c.isSpace(); });
    };

    static auto word = [](QStringView input) {
        return take_while(input, [](QChar c){ return !c.isSpace(); });
    };

    static auto parse_argument = [](QStringView input) {
        auto [_, without_spaces] = spaces(input);

        return one_of(
            [](QStringView input){ return enclosed(input, '{', '}'); },
            word
        )(without_spaces);
    };

    const QString cmd{DocParser::cmdName(CMD_COMPARESWITH)};
    Text text = priv->m_text.splitAtFirst(Atom::ComparesLeft);

    auto *atom = text.firstAtom();
    QStringList segments = collect(atom->string(), parse_argument);

    QString categoryString;
    if (!segments.isEmpty())
        categoryString = segments.takeFirst();
    auto category = comparisonCategoryFromString(categoryString.toStdString());

    if (category == ComparisonCategory::None) {
        location.warning(u"Invalid argument to \\%1 command: `%2`"_s.arg(cmd, categoryString),
                         u"Valid arguments are `strong`, `weak`, `partial`, or `equality`."_s);
        return;
    }

    if (segments.isEmpty()) {
        location.warning(u"Missing argument to \\%1 command."_s.arg(cmd),
                         u"Provide at least one type name, or a list of types separated by spaces."_s);
        return;
    }

    // Store cleaned-up type names back into the atom
    segments.removeDuplicates();
    atom->setString(segments.join(QLatin1Char(';')));

    // Add an entry to meta-command map for error handling in CppCodeParser
    priv->m_metaCommandMap[cmd].append(ArgPair(categoryString, atom->string()));
    priv->m_metacommandsUsed.insert(cmd);

    priv->constructExtra();
    const auto end{priv->extra->m_comparesWithMap.cend()};
    priv->extra->m_comparesWithMap.insert(end, category, text);
}

QT_END_NAMESPACE
