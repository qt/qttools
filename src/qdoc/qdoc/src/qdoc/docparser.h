// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#ifndef DOCPARSER_H
#define DOCPARSER_H

#include "atom.h"
#include "config.h"
#include "docutilities.h"
#include "location.h"
#include "openedlist.h"
#include "quoter.h"

#include "filesystem/fileresolver.h"

#include <QtCore/QCoreApplication>
#include <QtCore/qglobalstatic.h>
#include <QtCore/qhash.h>
#include <QtCore/qstack.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Doc;
class DocPrivate;
class CodeMarker;
struct Macro;

class DocParser
{
public:
    void parse(const QString &source, DocPrivate *docPrivate, const QSet<QString> &metaCommandSet,
               const QSet<QString> &possibleTopics);

    static void initialize(const Config &config, FileResolver& file_resolver);
    static int endCmdFor(int cmd);
    static QString cmdName(int cmd);
    static QString endCmdName(int cmd);
    static QString untabifyEtc(const QString &str);
    static int indentLevel(const QString &str);
    static QString dedent(int level, const QString &str);

    static int s_tabSize;
    static QStringList s_ignoreWords;
    static bool s_quoting;

private:
    Location &location();
    QString detailsUnknownCommand(const QSet<QString> &metaCommandSet, const QString &str);
    void insertTarget(const QString &target);
    void insertKeyword(const QString &keyword);
    void include(const QString &fileName, const QString &identifier, const QStringList &parameters);
    void startFormat(const QString &format, int cmd);
    bool openCommand(int cmd);
    bool closeCommand(int endCmd);
    void startSection(Doc::Sections unit, int cmd);
    void endSection(int unit, int endCmd);
    void parseAlso();
    void append(const QString &string);
    void append(Atom::AtomType type, const QString &string = QString());
    void append(Atom::AtomType type, const QString &p1, const QString &p2);
    void append(const QString &p1, const QString &p2);
    void appendChar(QChar ch);
    void appendWord(const QString &word);
    void appendToCode(const QString &code);
    void appendToCode(const QString &code, Atom::AtomType defaultType);
    void enterPara(Atom::AtomType leftType = Atom::ParaLeft,
                   Atom::AtomType rightType = Atom::ParaRight, const QString &string = QString());
    void leavePara();
    void leaveValue();
    void leaveValueList();
    void leaveTableRow();
    void quoteFromFile(const QString& filename);
    bool expandMacro();
    void expandMacro(const QString &def, const QStringList &args);
    QString expandMacroToString(const QString &name, const Macro &macro);
    Doc::Sections getSectioningUnit();
    QString getArgument(bool verbatim = false);
    QString getBracedArgument(bool verbatim);
    QString getBracketedArgument();
    QStringList getMacroArguments(const QString &name, const Macro &macro);
    QString getOptionalArgument();
    QString getRestOfLine();
    QString getMetaCommandArgument(const QString &cmdStr);
    QString getUntilEnd(int cmd);
    QString getCode(int cmd, CodeMarker *marker, const QString &argStr = QString());

    inline bool isAutoLinkString(const QString &word);
    bool isAutoLinkString(const QString &word, qsizetype &curPos);
    bool isBlankLine();
    bool isLeftBraceAhead();
    bool isLeftBracketAhead();
    void skipSpacesOnLine();
    void skipSpacesOrOneEndl();
    void skipAllSpaces();
    void skipToNextPreprocessorCommand();
    static bool isCode(const Atom *atom);
    static bool isQuote(const Atom *atom);
    static void expandArgumentsInString(QString &str, const QStringList &args);

    QStack<qsizetype> m_openedInputs {};

    QString m_input {};
    qsizetype m_position {};
    qsizetype m_backslashPosition {};
    qsizetype m_endPosition {};
    qsizetype m_inputLength {};
    Location m_cachedLocation {};
    qsizetype m_cachedPosition {};

    DocPrivate *m_private { nullptr };
    enum ParagraphState { OutsideParagraph, InSingleLineParagraph, InMultiLineParagraph };
    ParagraphState m_paragraphState {};
    bool m_inTableHeader {};
    bool m_inTableRow {};
    bool m_inTableItem {};
    bool m_indexStartedParagraph {}; // ### rename
    Atom::AtomType m_pendingParagraphLeftType {};
    Atom::AtomType m_pendingParagraphRightType {};
    QString m_pendingParagraphString {};

    int m_braceDepth {};
    Doc::Sections m_currentSection {};
    QMap<QString, Location> m_targetMap {};
    QMap<int, QString> m_pendingFormats {};
    QStack<int> m_openedCommands {};
    QStack<OpenedList> m_openedLists {};
    Quoter m_quoter {};
    Atom *m_lastAtom { nullptr };

    static DocUtilities &s_utilities;

    // KLUDGE: When parsing documentation, there is a need to find
    // files to resolve quoting commands. Ideally, the system that
    // takes care of this would be a non-static member that is a
    // reference that is passed at
    // construction time.
    // Nonetheless, with how the current codebase is constructed, this
    // has proven to be extremely difficult until more changes are
    // done. In particular, the construction of a DocParser happens in
    // multiple places at multiple depths and, in particular, happens
    // in one of Doc's constructor.
    // Doc itself is built, again, in multiple places at multiple
    // depths, making it clumsy and sometimes infeasible to pass the
    // dependency around so that it is available at the required
    // places. In particular, this stems from the fact that Doc is
    // holding many responsabilities and is spread troughtout much of
    // the codebase in different ways. DocParser mostly depends on Doc
    // and Doc currently depends on DocParser, making the two
    // difficult to separate.
    //
    // In the future, we expect Doc to mostly be removed, such as to
    // remove this dependencies and the parsing of documentation to
    // happen near main and atomically from other endevours, producing
    // an intermediate representation that is consumed by later
    // phases.
    // At that point, it should be possible to not have this kind of
    // indirection while, for now, the only accessible way to pass
    // this dependency is trough the initialize method which passes
    // for Doc::initialize.
    //
    // Furthemore, as we cannot late-bind a reference, and having a
    // desire to avoid an unnecessary copy, we are thus forced to use
    // a different storage method, in this case a pointer.
    // This too should be removed later on, using reference or move
    // semantic depending on the required data-flow.
    static FileResolver* file_resolver;
};
QT_END_NAMESPACE

#endif // DOCPARSER_H
