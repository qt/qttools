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

#include "doc.h"

#include "atom.h"
#include "config.h"
#include "codemarker.h"
#include "docparser.h"
#include "docprivate.h"
#include "generator.h"
#include "qmltypenode.h"
#include "quoter.h"
#include "text.h"

QT_BEGIN_NAMESPACE

DocUtilities &Doc::m_utilities = DocUtilities::instance();

/*!
  Parse the qdoc comment \a source. Build up a list of all the topic
  commands found including their arguments.  This constructor is used
  when there can be more than one topic command in theqdoc comment.
  Normally, there is only one topic command in a qdoc comment, but in
  QML documentation, there is the case where the qdoc \e{qmlproperty}
  command can appear multiple times in a qdoc comment.
 */
Doc::Doc(const Location &start_loc, const Location &end_loc, const QString &source,
         const QSet<QString> &metaCommandSet, const QSet<QString> &topics)
{
    priv = new DocPrivate(start_loc, end_loc, source);
    DocParser parser;
    parser.parse(source, priv, metaCommandSet, topics);
}

Doc::Doc(const Doc &doc) : priv(nullptr)
{
    operator=(doc);
}

Doc::~Doc()
{
    if (priv && priv->deref())
        delete priv;
}

Doc &Doc::operator=(const Doc &doc)
{
    if (doc.priv)
        doc.priv->ref();
    if (priv && priv->deref())
        delete priv;
    priv = doc.priv;
    return *this;
}

/*!
  Returns the starting location of a qdoc comment.
 */
const Location &Doc::location() const
{
    static const Location dummy;
    return priv == nullptr ? dummy : priv->start_loc;
}

/*!
  Returns the starting location of a qdoc comment.
 */
const Location &Doc::startLocation() const
{
    return location();
}

const QString &Doc::source() const
{
    static QString null;
    return priv == nullptr ? null : priv->src;
}

bool Doc::isEmpty() const
{
    return priv == nullptr || priv->src.isEmpty();
}

const Text &Doc::body() const
{
    static const Text dummy;
    return priv == nullptr ? dummy : priv->text;
}

Text Doc::briefText(bool inclusive) const
{
    return body().subText(Atom::BriefLeft, Atom::BriefRight, nullptr, inclusive);
}

Text Doc::trimmedBriefText(const QString &className) const
{
    QString classNameOnly = className;
    if (className.contains("::"))
        classNameOnly = className.split("::").last();

    Text originalText = briefText();
    Text resultText;
    const Atom *atom = originalText.firstAtom();
    if (atom) {
        QString briefStr;
        QString whats;
        /*
          This code is really ugly. The entire \brief business
          should be rethought.
        */
        while (atom) {
            if (atom->type() == Atom::AutoLink || atom->type() == Atom::String) {
                briefStr += atom->string();
            } else if (atom->type() == Atom::C) {
                briefStr += Generator::plainCode(atom->string());
            }
            atom = atom->next();
        }

        QStringList w = briefStr.split(QLatin1Char(' '));
        if (!w.isEmpty() && w.first() == "Returns") {
        } else {
            if (!w.isEmpty() && w.first() == "The")
                w.removeFirst();

            if (!w.isEmpty() && (w.first() == className || w.first() == classNameOnly))
                w.removeFirst();

            if (!w.isEmpty()
                && ((w.first() == "class") || (w.first() == "function") || (w.first() == "macro")
                    || (w.first() == "widget") || (w.first() == "namespace")
                    || (w.first() == "header")))
                w.removeFirst();

            if (!w.isEmpty() && (w.first() == "is" || w.first() == "provides"))
                w.removeFirst();

            if (!w.isEmpty() && (w.first() == "a" || w.first() == "an"))
                w.removeFirst();
        }

        whats = w.join(' ');

        if (whats.endsWith(QLatin1Char('.')))
            whats.truncate(whats.length() - 1);

        if (!whats.isEmpty())
            whats[0] = whats[0].toUpper();

        // ### move this once \brief is abolished for properties
        resultText << whats;
    }
    return resultText;
}

Text Doc::legaleseText() const
{
    if (priv == nullptr || !priv->hasLegalese)
        return Text();
    else
        return body().subText(Atom::LegaleseLeft, Atom::LegaleseRight);
}

QSet<QString> Doc::parameterNames() const
{
    return priv == nullptr ? QSet<QString>() : priv->params;
}

QStringList Doc::enumItemNames() const
{
    return priv == nullptr ? QStringList() : priv->enumItemList;
}

QStringList Doc::omitEnumItemNames() const
{
    return priv == nullptr ? QStringList() : priv->omitEnumItemList;
}

QSet<QString> Doc::metaCommandsUsed() const
{
    return priv == nullptr ? QSet<QString>() : priv->metacommandsUsed;
}

/*!
  Returns true if the set of metacommands used in the doc
  comment contains \e {internal}.
 */
bool Doc::isInternal() const
{
    return metaCommandsUsed().contains(QLatin1String("internal"));
}

/*!
  Returns true if the set of metacommands used in the doc
  comment contains \e {reimp}.
 */
bool Doc::isMarkedReimp() const
{
    return metaCommandsUsed().contains(QLatin1String("reimp"));
}

/*!
  Returns a reference to the list of topic commands used in the
  current qdoc comment. Normally there is only one, but there
  can be multiple \e{qmlproperty} commands, for example.
 */
TopicList Doc::topicsUsed() const
{
    return priv == nullptr ? TopicList() : priv->topics_;
}

ArgList Doc::metaCommandArgs(const QString &metacommand) const
{
    return priv == nullptr ? ArgList() : priv->metaCommandMap.value(metacommand);
}

QList<Text> Doc::alsoList() const
{
    return priv == nullptr ? QList<Text>() : priv->alsoList;
}

bool Doc::hasTableOfContents() const
{
    return priv && priv->extra && !priv->extra->tableOfContents_.isEmpty();
}

bool Doc::hasKeywords() const
{
    return priv && priv->extra && !priv->extra->keywords_.isEmpty();
}

bool Doc::hasTargets() const
{
    return priv && priv->extra && !priv->extra->targets_.isEmpty();
}

const QList<Atom *> &Doc::tableOfContents() const
{
    priv->constructExtra();
    return priv->extra->tableOfContents_;
}

const QList<int> &Doc::tableOfContentsLevels() const
{
    priv->constructExtra();
    return priv->extra->tableOfContentsLevels_;
}

const QList<Atom *> &Doc::keywords() const
{
    priv->constructExtra();
    return priv->extra->keywords_;
}

const QList<Atom *> &Doc::targets() const
{
    priv->constructExtra();
    return priv->extra->targets_;
}

QStringMultiMap *Doc::metaTagMap() const
{
    return priv && priv->extra ? &priv->extra->metaMap_ : nullptr;
}

void Doc::initialize()
{
    Config &config = Config::instance();
    DocParser::initialize(config);

    QStringMap reverseAliasMap;

    for (const auto &a : config.subVars(CONFIG_ALIAS)) {
        QString alias = config.getString(CONFIG_ALIAS + Config::dot + a);
        if (reverseAliasMap.contains(alias)) {
            config.lastLocation().warning(QStringLiteral("Command name '\\%1' cannot stand"
                                                         " for both '\\%2' and '\\%3'")
                                                  .arg(alias, reverseAliasMap[alias], a));
        } else {
            reverseAliasMap.insert(alias, a);
        }
        m_utilities.aliasMap.insert(a, alias);
    }

    for (const auto &macroName : config.subVars(CONFIG_MACRO)) {
        QString macroDotName = CONFIG_MACRO + Config::dot + macroName;
        Macro macro;
        macro.numParams = -1;
        macro.defaultDef = config.getString(macroDotName);
        if (!macro.defaultDef.isEmpty()) {
            macro.defaultDefLocation = config.lastLocation();
            macro.numParams = Config::numParams(macro.defaultDef);
        }
        bool silent = false;

        for (const auto &f : config.subVars(macroDotName)) {
            QString def = config.getString(macroDotName + Config::dot + f);
            if (!def.isEmpty()) {
                macro.otherDefs.insert(f, def);
                int m = Config::numParams(def);
                if (macro.numParams == -1)
                    macro.numParams = m;
                else if (macro.numParams != m) {
                    if (!silent) {
                        QString other = QStringLiteral("default");
                        if (macro.defaultDef.isEmpty())
                            other = macro.otherDefs.constBegin().key();
                        config.lastLocation().warning(
                                QStringLiteral("Macro '\\%1' takes inconsistent number of "
                                               "arguments (%2 %3, %4 %5)")
                                        .arg(macroName, f, QString::number(m), other,
                                             QString::number(macro.numParams)));
                        silent = true;
                    }
                    if (macro.numParams < m)
                        macro.numParams = m;
                }
            }
        }
        if (macro.numParams != -1)
            m_utilities.macroHash.insert(macroName, macro);
    }
}

/*!
  All the heap allocated variables are deleted.
 */
void Doc::terminate()
{
    m_utilities.aliasMap.clear();
    m_utilities.cmdHash.clear();
    m_utilities.macroHash.clear();
    DocParser::terminate();
}

QString Doc::alias(const QString &english)
{
    return m_utilities.aliasMap.value(english, english);
}

/*!
  Trims the deadwood out of \a str. i.e., this function
  cleans up \a str.
 */
void Doc::trimCStyleComment(Location &location, QString &str)
{
    QString cleaned;
    Location m = location;
    bool metAsterColumn = true;
    int asterColumn = location.columnNo() + 1;
    int i;

    for (i = 0; i < str.length(); ++i) {
        if (m.columnNo() == asterColumn) {
            if (str[i] != '*')
                break;
            cleaned += ' ';
            metAsterColumn = true;
        } else {
            if (str[i] == '\n') {
                if (!metAsterColumn)
                    break;
                metAsterColumn = false;
            }
            cleaned += str[i];
        }
        m.advance(str[i]);
    }
    if (cleaned.length() == str.length())
        str = cleaned;

    for (int i = 0; i < 3; ++i)
        location.advance(str[i]);
    str = str.mid(3, str.length() - 5);
}

QString Doc::resolveFile(const Location &location, const QString &fileName,
                         QString *userFriendlyFilePath)
{
    const QString result = Config::findFile(location, DocParser::exampleFiles,
                                            DocParser::exampleDirs, fileName, userFriendlyFilePath);
    qCDebug(lcQdoc).noquote().nospace()
            << __FUNCTION__ << "(location=" << location.fileName() << ':' << location.lineNo()
            << ", fileName=\"" << fileName << "\"), resolved to \"" << result;
    return result;
}

CodeMarker *Doc::quoteFromFile(const Location &location, Quoter &quoter, const QString &fileName)
{
    quoter.reset();

    QString code;

    QString userFriendlyFilePath;
    const QString filePath = resolveFile(location, fileName, &userFriendlyFilePath);
    if (filePath.isEmpty()) {
        QString details = QLatin1String("Example directories: ")
                + DocParser::exampleDirs.join(QLatin1Char(' '));
        if (!DocParser::exampleFiles.isEmpty())
            details += QLatin1String(", example files: ")
                    + DocParser::exampleFiles.join(QLatin1Char(' '));
        location.warning(QStringLiteral("Cannot find file to quote from: '%1'").arg(fileName),
                         details);
    } else {
        QFile inFile(filePath);
        if (!inFile.open(QFile::ReadOnly)) {
            location.warning(QStringLiteral("Cannot open file to quote from: '%1'")
                                     .arg(userFriendlyFilePath));
        } else {
            QTextStream inStream(&inFile);
            code = DocParser::untabifyEtc(inStream.readAll());
        }
    }

    CodeMarker *marker = CodeMarker::markerForFileName(fileName);
    quoter.quoteFromFile(userFriendlyFilePath, code, marker->markedUpCode(code, nullptr, location));
    return marker;
}

QString Doc::canonicalTitle(const QString &title)
{
    // The code below is equivalent to the following chunk, but _much_
    // faster (accounts for ~10% of total running time)
    //
    //  QRegularExpression attributeExpr("[^A-Za-z0-9]+");
    //  QString result = title.toLower();
    //  result.replace(attributeExpr, " ");
    //  result = result.simplified();
    //  result.replace(QLatin1Char(' '), QLatin1Char('-'));

    QString result;
    result.reserve(title.size());

    bool dashAppended = false;
    bool begun = false;
    qsizetype lastAlnum = 0;
    for (int i = 0; i != title.size(); ++i) {
        uint c = title.at(i).unicode();
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        bool alnum = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        if (alnum) {
            result += QLatin1Char(c);
            begun = true;
            dashAppended = false;
            lastAlnum = result.size();
        } else if (!dashAppended) {
            if (begun)
                result += QLatin1Char('-');
            dashAppended = true;
        }
    }
    result.truncate(lastAlnum);
    return result;
}

void Doc::detach()
{
    if (priv == nullptr) {
        priv = new DocPrivate;
        return;
    }
    if (priv->count == 1)
        return;

    --priv->count;

    auto *newPriv = new DocPrivate(*priv);
    newPriv->count = 1;
    if (priv->extra)
        newPriv->extra = new DocPrivateExtra(*priv->extra);

    priv = newPriv;
}

/*!
    Returns the argument passed in square brackets to the command \a command,
    if it exists.
 */
QString Doc::bracketedArgs(const QString &command) const
{
    if (priv && priv->extra && !priv->extra->bracketedArgs_.isEmpty())
        return priv->extra->bracketedArgs_[command];
    return QString();
}

QT_END_NAMESPACE
