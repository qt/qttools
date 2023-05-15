// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
#include "utilities.h"

#include <qcryptographichash.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

DocUtilities &Doc::m_utilities = DocUtilities::instance();

/*!
    \typedef ArgList
    \relates Doc

    A list of metacommand arguments that appear in a Doc. Each entry
    in the list is a <QString, QString> pair (ArgPair):

    \list
        \li \c {ArgPair.first} - arguments passed to the command.
        \li \c {ArgPair.second} - optional argument string passed
            within brackets immediately following the command.
    \endlist
*/

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
    m_priv = new DocPrivate(start_loc, end_loc, source);
    DocParser parser;
    parser.parse(source, m_priv, metaCommandSet, topics);

    if (Config::instance().getAtomsDump()) {
        start_loc.information(u"==== Atoms Structure for block comment starting at %1 ===="_s.arg(
                start_loc.toString()));
        body().dump();
        end_loc.information(
                u"==== Ending atoms Structure for block comment ending at %1 ===="_s.arg(
                        end_loc.toString()));
    }
}

Doc::Doc(const Doc &doc) : m_priv(nullptr)
{
    operator=(doc);
}

Doc::~Doc()
{
    if (m_priv && m_priv->deref())
        delete m_priv;
}

Doc &Doc::operator=(const Doc &doc)
{
    if (&doc == this)
        return *this;
    if (doc.m_priv)
        doc.m_priv->ref();
    if (m_priv && m_priv->deref())
        delete m_priv;
    m_priv = doc.m_priv;
    return *this;
}

/*!
  Returns the starting location of a qdoc comment.
 */
const Location &Doc::location() const
{
    static const Location dummy;
    return m_priv == nullptr ? dummy : m_priv->m_start_loc;
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
    return m_priv == nullptr ? null : m_priv->m_src;
}

bool Doc::isEmpty() const
{
    return m_priv == nullptr || m_priv->m_src.isEmpty();
}

const Text &Doc::body() const
{
    static const Text dummy;
    return m_priv == nullptr ? dummy : m_priv->m_text;
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
            whats.truncate(whats.size() - 1);

        if (!whats.isEmpty())
            whats[0] = whats[0].toUpper();

        // ### move this once \brief is abolished for properties
        resultText << whats;
    }
    return resultText;
}

Text Doc::legaleseText() const
{
    if (m_priv == nullptr || !m_priv->m_hasLegalese)
        return Text();
    else
        return body().subText(Atom::LegaleseLeft, Atom::LegaleseRight);
}

QSet<QString> Doc::parameterNames() const
{
    return m_priv == nullptr ? QSet<QString>() : m_priv->m_params;
}

QStringList Doc::enumItemNames() const
{
    return m_priv == nullptr ? QStringList() : m_priv->m_enumItemList;
}

QStringList Doc::omitEnumItemNames() const
{
    return m_priv == nullptr ? QStringList() : m_priv->m_omitEnumItemList;
}

QSet<QString> Doc::metaCommandsUsed() const
{
    return m_priv == nullptr ? QSet<QString>() : m_priv->m_metacommandsUsed;
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
    return m_priv == nullptr ? TopicList() : m_priv->m_topics;
}

ArgList Doc::metaCommandArgs(const QString &metacommand) const
{
    return m_priv == nullptr ? ArgList() : m_priv->m_metaCommandMap.value(metacommand);
}

QList<Text> Doc::alsoList() const
{
    return m_priv == nullptr ? QList<Text>() : m_priv->m_alsoList;
}

bool Doc::hasTableOfContents() const
{
    return m_priv && m_priv->extra && !m_priv->extra->m_tableOfContents.isEmpty();
}

bool Doc::hasKeywords() const
{
    return m_priv && m_priv->extra && !m_priv->extra->m_keywords.isEmpty();
}

bool Doc::hasTargets() const
{
    return m_priv && m_priv->extra && !m_priv->extra->m_targets.isEmpty();
}

const QList<Atom *> &Doc::tableOfContents() const
{
    m_priv->constructExtra();
    return m_priv->extra->m_tableOfContents;
}

const QList<int> &Doc::tableOfContentsLevels() const
{
    m_priv->constructExtra();
    return m_priv->extra->m_tableOfContentsLevels;
}

const QList<Atom *> &Doc::keywords() const
{
    m_priv->constructExtra();
    return m_priv->extra->m_keywords;
}

const QList<Atom *> &Doc::targets() const
{
    m_priv->constructExtra();
    return m_priv->extra->m_targets;
}

QStringMultiMap *Doc::metaTagMap() const
{
    return m_priv && m_priv->extra ? &m_priv->extra->m_metaMap : nullptr;
}

void Doc::initialize(FileResolver& file_resolver)
{
    Config &config = Config::instance();
    DocParser::initialize(config, file_resolver);

    const auto &configMacros = config.subVars(CONFIG_MACRO);
    for (const auto &macroName : configMacros) {
        QString macroDotName = CONFIG_MACRO + Config::dot + macroName;
        Macro macro;
        macro.numParams = -1;
        const auto &macroConfigVar = config.get(macroDotName);
        macro.m_defaultDef = macroConfigVar.asString();
        if (!macro.m_defaultDef.isEmpty()) {
            macro.m_defaultDefLocation = macroConfigVar.location();
            macro.numParams = Config::numParams(macro.m_defaultDef);
        }
        bool silent = false;

        const auto &macroDotNames = config.subVars(macroDotName);
        for (const auto &f : macroDotNames) {
            const auto &macroSubVar = config.get(macroDotName + Config::dot + f);
            QString def{macroSubVar.asString()};
            if (!def.isEmpty()) {
                macro.m_otherDefs.insert(f, def);
                int m = Config::numParams(def);
                if (macro.numParams == -1)
                    macro.numParams = m;
                // .match definition is a regular expression that contains no params
                else if (macro.numParams != m && f != QLatin1String("match")) {
                    if (!silent) {
                        QString other = QStringLiteral("default");
                        if (macro.m_defaultDef.isEmpty())
                            other = macro.m_otherDefs.constBegin().key();
                        macroSubVar.location().warning(
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
    m_utilities.cmdHash.clear();
    m_utilities.macroHash.clear();
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

    for (i = 0; i < str.size(); ++i) {
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
    if (cleaned.size() == str.size())
        str = cleaned;

    for (int i = 0; i < 3; ++i)
        location.advance(str[i]);
    str = str.mid(3, str.size() - 5);
}

CodeMarker *Doc::quoteFromFile(const Location &location, Quoter &quoter, ResolvedFile resolved_file)
{
    // TODO: quoteFromFile should not care about modifying a stateful
    // quoter from the outside, instead, it should produce a quoter
    // that allows the caller to retrieve the required information
    // about the quoted file.
    //
    // When changing the way in which quoting works, this kind of
    // spread resposability should be removed, together with quoteFromFile.
    quoter.reset();

    QString code;
    {
        QFile input_file{resolved_file.get_path()};
        input_file.open(QFile::ReadOnly);
        code = DocParser::untabifyEtc(QTextStream{&input_file}.readAll());
    }

    CodeMarker *marker = CodeMarker::markerForFileName(resolved_file.get_path());
    quoter.quoteFromFile(resolved_file.get_path(), code, marker->markedUpCode(code, nullptr, location));
    return marker;
}

void Doc::detach()
{
    if (m_priv == nullptr) {
        m_priv = new DocPrivate;
        return;
    }
    if (m_priv->count == 1)
        return;

    --m_priv->count;

    auto *newPriv = new DocPrivate(*m_priv);
    newPriv->count = 1;
    if (m_priv->extra)
        newPriv->extra = new DocPrivateExtra(*m_priv->extra);

    m_priv = newPriv;
}

QT_END_NAMESPACE
