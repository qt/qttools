// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PHRASE_H
#define PHRASE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QtCore/QLocale>

#include "simtexth.h"

QT_BEGIN_NAMESPACE

class PhraseBook;

class Phrase
{
public:
    Phrase();
    Phrase(const QString &source, const QString &target, const QString &definition,
            const Candidate &candidate, int sc = -1);
    Phrase(const QString &source, const QString &target,
            const QString &definition, PhraseBook *phraseBook);

    QString source() const { return s; }
    void setSource(const QString &ns);
    QString target() const {return t;}
    void setTarget(const QString &nt);
    QString definition() const {return d;}
    void setDefinition (const QString &nd);
    int shortcut() const { return shrtc; }
    Candidate candidate() const { return cand; }
    PhraseBook *phraseBook() const { return m_phraseBook; }
    void setPhraseBook(PhraseBook *book) { m_phraseBook = book; }

private:
    int shrtc;
    QString s;
    QString t;
    QString d;
    Candidate cand;
    PhraseBook *m_phraseBook;
};

bool operator==(const Phrase &p, const Phrase &q);
inline bool operator!=(const Phrase &p, const Phrase &q) {
    return !(p == q);
}

class QphHandler;

class PhraseBook : public QObject
{
    Q_OBJECT

public:
    PhraseBook();
    ~PhraseBook();
    bool load(const QString &fileName, bool *langGuessed);
    bool save(const QString &fileName);
    QList<Phrase *> phrases() const { return m_phrases; }
    void append(Phrase *phrase);
    void remove(Phrase *phrase);
    QString fileName() const { return m_fileName; }
    QString friendlyPhraseBookName() const;
    bool isModified() const { return m_changed; }

    void setLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory);
    QLocale::Language language() const { return m_language; }
    QLocale::Territory territory() const { return m_territory; }
    void setSourceLanguageAndTerritory(QLocale::Language lang, QLocale::Territory territory);
    QLocale::Language sourceLanguage() const { return m_sourceLanguage; }
    QLocale::Territory sourceTerritory() const { return m_sourceTerritory; }

signals:
    void modifiedChanged(bool changed);
    void listChanged();

private:
    // Prevent copying
    PhraseBook(const PhraseBook &);
    PhraseBook& operator=(const PhraseBook &);

    void setModified(bool modified);
    void phraseChanged(Phrase *phrase);

    QList<Phrase *> m_phrases;
    QString m_fileName;
    bool m_changed;

    QLocale::Language m_language;
    QLocale::Language m_sourceLanguage;
    QLocale::Territory m_territory;
    QLocale::Territory m_sourceTerritory;

    friend class QphHandler;
    friend class Phrase;
};

QT_END_NAMESPACE

#endif
