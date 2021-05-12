/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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
#ifndef DOCPARSER_H
#define DOCPARSER_H

#include "atom.h"
#include "config.h"
#include "docutilities.h"
#include "location.h"
#include "openedlist.h"
#include "quoter.h"

#include <QtCore/QCoreApplication>
#include <QtCore/qglobalstatic.h>
#include <QtCore/qhash.h>
#include <QtCore/qstack.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class Doc;
class DocPrivate;
class CodeMarker;

class DocParser
{
public:
    void parse(const QString &source, DocPrivate *docPrivate, const QSet<QString> &metaCommandSet,
               const QSet<QString> &possibleTopics);

    static void initialize(const Config &config);
    static void terminate();
    static int endCmdFor(int cmd);
    static QString cmdName(int cmd);
    static QString endCmdName(int cmd);
    static QString untabifyEtc(const QString &str);
    static int indentLevel(const QString &str);
    static QString dedent(int level, const QString &str);
    static QString slashed(const QString &str);

    static int s_tabSize;
    static QStringList s_exampleFiles;
    static QStringList s_exampleDirs;
    static QStringList s_sourceFiles;
    static QStringList s_sourceDirs;
    static QStringList s_ignoreWords;
    static bool s_quoting;

private:
    Location &location();
    QString detailsUnknownCommand(const QSet<QString> &metaCommandSet, const QString &str);
    void insertTarget(const QString &target, bool keyword);
    void include(const QString &fileName, const QString &identifier);
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
    CodeMarker *quoteFromFile();
    bool expandMacro();
    void expandMacro(const QString &name, const QString &def, int numParams);
    QString expandMacroToString(const QString &name, const QString &def, int numParams,
                                const QString &matchExpr);
    Doc::Sections getSectioningUnit();
    QString getArgument(bool verbatim = false);
    QString getBracedArgument(bool verbatim);
    QString getBracketedArgument();
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
};
QT_END_NAMESPACE

#endif // DOCPARSER_H
