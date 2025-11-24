// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "phrase.h"
#include "translator.h"
#include "xmlparser.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

static QString xmlProtect(const QString & str)
{
    QString p = str;
    p.replace(u'&', "&amp;"_L1);
    p.replace(u'\"', "&quot;"_L1);
    p.replace(u'>', "&gt;"_L1);
    p.replace(u'<', "&lt;"_L1);
    p.replace(QLatin1Char('\''), "&apos;"_L1);
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
    bool startElement(QStringView namespaceURI, QStringView localName,
                      QStringView qName, const QXmlStreamAttributes &atts) override;
    bool endElement(QStringView namespaceURI, QStringView localName,
                    QStringView qName) override;
    bool characters(QStringView ch) override;
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

bool QphHandler::startElement(QStringView namespaceURI, QStringView localName,
                              QStringView qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (qName == "QPH"_L1) {
        m_language = atts.value("language"_L1).toString();
        m_sourceLanguage = atts.value("sourcelanguage"_L1).toString();
    } else if (qName == "phrase"_L1) {
        source.truncate(0);
        target.truncate(0);
        definition.truncate(0);
    }
    accum.truncate(0);
    return true;
}

bool QphHandler::endElement(QStringView namespaceURI, QStringView localName,
                            QStringView qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);

    if (qName == "source"_L1)
        source = accum;
    else if (qName == "target"_L1)
        target = accum;
    else if (qName == "definition"_L1)
        definition = accum;
    else if (qName == "phrase"_L1)
        pb->m_phrases.append(new Phrase(source, target, definition, pb));
    return true;
}

bool QphHandler::characters(QStringView ch)
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
    m_territory(QLocale::AnyTerritory),
    m_sourceTerritory(QLocale::AnyTerritory)
{
}

PhraseBook::~PhraseBook()
{
    qDeleteAll(m_phrases);
}

void PhraseBook::setLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory)
{
    if (m_language == lang && m_territory == territory)
        return;
    m_language = lang;
    m_territory = territory;
    setModified(true);
}

void PhraseBook::setSourceLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory)
{
    if (m_sourceLanguage == lang && m_sourceTerritory == territory)
        return;
    m_sourceLanguage = lang;
    m_sourceTerritory = territory;
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

    Translator::languageAndTerritory(hand->language(), &m_language, &m_territory);
    *langGuessed = false;
    if (m_language == QLocale::C) {
        QLocale sys;
        m_language = sys.language();
        m_territory = sys.territory();
        *langGuessed = true;
    }

    QString lang = hand->sourceLanguage();
    if (lang.isEmpty()) {
        m_sourceLanguage = QLocale::C;
        m_sourceTerritory = QLocale::AnyTerritory;
    } else {
        Translator::languageAndTerritory(lang, &m_sourceLanguage, &m_sourceTerritory);
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

    t << "<!DOCTYPE QPH>\n<QPH";
    if (sourceLanguage() != QLocale::C)
        t << " sourcelanguage=\""
          << Translator::makeLanguageCode(sourceLanguage(), sourceTerritory()) << '"';
    if (language() != QLocale::C)
        t << " language=\"" << Translator::makeLanguageCode(language(), territory()) << '"';
    t << ">\n";
    for (Phrase *p : std::as_const(m_phrases)) {
        t << "<phrase>\n";
        t << "    <source>" << xmlProtect( p->source() ) << "</source>\n";
        t << "    <target>" << xmlProtect( p->target() ) << "</target>\n";
        if (!p->definition().isEmpty())
            t << "    <definition>" << xmlProtect( p->definition() )
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
