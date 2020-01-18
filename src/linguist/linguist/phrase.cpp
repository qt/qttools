/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "phrase.h"
#include "translator.h"
#include "xmlparser.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QTextCodec>
#include <QTextStream>
#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE

static QString protect(const QString & str)
{
    QString p = str;
    p.replace(QLatin1Char('&'),  QLatin1String("&amp;"));
    p.replace(QLatin1Char('\"'), QLatin1String("&quot;"));
    p.replace(QLatin1Char('>'),  QLatin1String("&gt;"));
    p.replace(QLatin1Char('<'),  QLatin1String("&lt;"));
    p.replace(QLatin1Char('\''), QLatin1String("&apos;"));
    return p;
}

Phrase::Phrase()
    : shrtc(-1), m_phraseBook(0)
{
}

Phrase::Phrase(const QString &source, const QString &target, const QString &definition,
               const Candidate &candidate, int sc)
    : shrtc(sc), s(source), t(target), d(definition), cand(candidate), m_phraseBook(0)
{
}

Phrase::Phrase(const QString &source, const QString &target,
               const QString &definition, PhraseBook *phraseBook)
    : shrtc(-1), s(source), t(target), d(definition),
      m_phraseBook(phraseBook)
{
}

void Phrase::setSource(const QString &ns)
{
    if (s == ns)
        return;
    s = ns;
    if (m_phraseBook)
        m_phraseBook->phraseChanged(this);
}

void Phrase::setTarget(const QString &nt)
{
    if (t == nt)
        return;
    t = nt;
    if (m_phraseBook)
        m_phraseBook->phraseChanged(this);
}

void Phrase::setDefinition(const QString &nd)
{
    if (d == nd)
        return;
    d = nd;
    if (m_phraseBook)
        m_phraseBook->phraseChanged(this);
}

bool operator==(const Phrase &p, const Phrase &q)
{
    return p.source() == q.source() && p.target() == q.target() &&
        p.definition() == q.definition() && p.phraseBook() == q.phraseBook();
}

class QphHandler : public XmlParser
{
public:
    QphHandler(PhraseBook *phraseBook, QXmlStreamReader &reader)
        : XmlParser(reader), pb(phraseBook), ferrorCount(0)
    {
    }
    ~QphHandler() override = default;

    QString language() const { return m_language; }
    QString sourceLanguage() const { return m_sourceLanguage; }

private:
    bool startElement(const QStringRef &namespaceURI, const QStringRef &localName,
                      const QStringRef &qName, const QXmlStreamAttributes &atts) override;
    bool endElement(const QStringRef &namespaceURI, const QStringRef &localName,
                    const QStringRef &qName) override;
    bool characters(const QStringRef &ch) override;
    bool fatalError(qint64 line, qint64 column, const QString &message) override;

    PhraseBook *pb;
    QString source;
    QString target;
    QString definition;
    QString m_language;
    QString m_sourceLanguage;

    QString accum;
    int ferrorCount;
};

bool QphHandler::startElement(const QStringRef &namespaceURI, const QStringRef &localName,
                              const QStringRef &qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(localName)

    if (qName == QLatin1String("QPH")) {
        m_language = atts.value(QLatin1String("language")).toString();
        m_sourceLanguage = atts.value(QLatin1String("sourcelanguage")).toString();
    } else if (qName == QLatin1String("phrase")) {
        source.truncate(0);
        target.truncate(0);
        definition.truncate(0);
    }
    accum.truncate(0);
    return true;
}

bool QphHandler::endElement(const QStringRef &namespaceURI, const QStringRef &localName,
                            const QStringRef &qName)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(localName)

    if (qName == QLatin1String("source"))
        source = accum;
    else if (qName == QLatin1String("target"))
        target = accum;
    else if (qName == QLatin1String("definition"))
        definition = accum;
    else if (qName == QLatin1String("phrase"))
        pb->m_phrases.append(new Phrase(source, target, definition, pb));
    return true;
}

bool QphHandler::characters(const QStringRef &ch)
{
    accum += ch;
    return true;
}

bool QphHandler::fatalError(qint64 line, qint64 column, const QString &message)
{
    if (ferrorCount++ == 0) {
        QString msg = PhraseBook::tr("Parse error at line %1, column %2 (%3).")
                              .arg(line)
                              .arg(column)
                              .arg(message);
        QMessageBox::information(nullptr, QObject::tr("Qt Linguist"), msg);
    }
    return false;
}

PhraseBook::PhraseBook() :
    m_changed(false),
    m_language(QLocale::C),
    m_sourceLanguage(QLocale::C),
    m_country(QLocale::AnyCountry),
    m_sourceCountry(QLocale::AnyCountry)
{
}

PhraseBook::~PhraseBook()
{
    qDeleteAll(m_phrases);
}

void PhraseBook::setLanguageAndCountry(QLocale::Language lang, QLocale::Country country)
{
    if (m_language == lang && m_country == country)
        return;
    m_language = lang;
    m_country = country;
    setModified(true);
}

void PhraseBook::setSourceLanguageAndCountry(QLocale::Language lang, QLocale::Country country)
{
    if (m_sourceLanguage == lang && m_sourceCountry == country)
        return;
    m_sourceLanguage = lang;
    m_sourceCountry = country;
    setModified(true);
}

bool PhraseBook::load(const QString &fileName, bool *langGuessed)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    m_fileName = fileName;

    QXmlStreamReader reader(&f);
    QphHandler *hand = new QphHandler(this, reader);
    reader.setNamespaceProcessing(false);
    bool ok = hand->parse();

    Translator::languageAndCountry(hand->language(), &m_language, &m_country);
    *langGuessed = false;
    if (m_language == QLocale::C) {
        QLocale sys;
        m_language = sys.language();
        m_country = sys.country();
        *langGuessed = true;
    }

    QString lang = hand->sourceLanguage();
    if (lang.isEmpty()) {
        m_sourceLanguage = QLocale::C;
        m_sourceCountry = QLocale::AnyCountry;
    } else {
        Translator::languageAndCountry(lang, &m_sourceLanguage, &m_sourceCountry);
    }

    delete hand;
    f.close();
    if (!ok) {
        qDeleteAll(m_phrases);
        m_phrases.clear();
    } else {
        emit listChanged();
    }

    return ok;
}

bool PhraseBook::save(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return false;

    m_fileName = fileName;

    QTextStream t(&f);
    t.setCodec( QTextCodec::codecForName("UTF-8") );

    t << "<!DOCTYPE QPH>\n<QPH";
    if (sourceLanguage() != QLocale::C)
        t << " sourcelanguage=\""
          << Translator::makeLanguageCode(sourceLanguage(), sourceCountry()) << '"';
    if (language() != QLocale::C)
        t << " language=\"" << Translator::makeLanguageCode(language(), country()) << '"';
    t << ">\n";
    foreach (Phrase *p, m_phrases) {
        t << "<phrase>\n";
        t << "    <source>" << protect( p->source() ) << "</source>\n";
        t << "    <target>" << protect( p->target() ) << "</target>\n";
        if (!p->definition().isEmpty())
            t << "    <definition>" << protect( p->definition() )
              << "</definition>\n";
        t << "</phrase>\n";
    }
    t << "</QPH>\n";
    f.close();
    setModified(false);
    return true;
}

void PhraseBook::append(Phrase *phrase)
{
    m_phrases.append(phrase);
    phrase->setPhraseBook(this);
    setModified(true);
    emit listChanged();
}

void PhraseBook::remove(Phrase *phrase)
{
    m_phrases.removeOne(phrase);
    phrase->setPhraseBook(0);
    setModified(true);
    emit listChanged();
}

void PhraseBook::setModified(bool modified)
 {
     if (m_changed != modified) {
         emit modifiedChanged(modified);
         m_changed = modified;
     }
}

void PhraseBook::phraseChanged(Phrase *p)
{
    Q_UNUSED(p);

    setModified(true);
}

QString PhraseBook::friendlyPhraseBookName() const
{
    if (!m_fileName.isEmpty())
        return QFileInfo(m_fileName).fileName();
    return QString();
}

QT_END_NAMESPACE
