// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "globals.h"
#include "mainwindow.h"
#include "messagemodel.h"
#include "phrase.h"
#include "phraseview.h"
#include "phrasemodel.h"
#include "simtexth.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QSettings>
#include <QShortcut>
#include <QTreeView>
#include <QWidget>
#include <QDebug>


QT_BEGIN_NAMESPACE

static QString phraseViewHeaderKey()
{
    return settingPath("PhraseViewHeader");
}

PhraseView::PhraseView(MultiDataModel *model, QList<QHash<QString, QList<Phrase *> > > *phraseDict, QWidget *parent)
    : QTreeView(parent),
      m_dataModel(model),
      m_phraseDict(phraseDict),
      m_modelIndex(-1),
      m_doGuesses(true)
{
    setObjectName(QLatin1String("phrase list view"));

    m_phraseModel = new PhraseModel(this);

    setModel(m_phraseModel);
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setRootIsDecorated(false);
    setItemsExpandable(false);

    for (int i = 0; i < 9; ++i) {
        const auto key = static_cast<Qt::Key>(int(Qt::Key_1) + i);
        auto shortCut = new QShortcut(Qt::CTRL | key, this);
        connect(shortCut, &QShortcut::activated, this,
                [i, this]() { this->guessShortcut(i); });
    }

    header()->setSectionResizeMode(QHeaderView::Interactive);
    header()->setSectionsClickable(true);
    header()->restoreState(QSettings().value(phraseViewHeaderKey()).toByteArray());

    connect(this, &QAbstractItemView::activated,
            this, &PhraseView::selectPhrase);
}

PhraseView::~PhraseView()
{
    QSettings().setValue(phraseViewHeaderKey(), header()->saveState());
    deleteGuesses();
}

void PhraseView::toggleGuessing()
{
    m_doGuesses = !m_doGuesses;
    update();
}

void PhraseView::update()
{
    setSourceText(m_modelIndex, m_sourceText);
}


void PhraseView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    QMenu *contextMenu = new QMenu(this);

    QAction *insertAction = new QAction(tr("Insert"), contextMenu);
    connect(insertAction, &QAction::triggered,
            this, &PhraseView::selectCurrentPhrase);

    QAction *editAction = new QAction(tr("Edit"), contextMenu);
    connect(editAction, &QAction::triggered,
            this, &PhraseView::editPhrase);
    Qt::ItemFlags isFromPhraseBook = model()->flags(index) & Qt::ItemIsEditable;
    editAction->setEnabled(isFromPhraseBook);

    QAction *gotoAction = new QAction(tr("Go to"), contextMenu);
    connect(gotoAction, &QAction::triggered,
            this, &PhraseView::gotoMessageFromGuess);
    gotoAction->setEnabled(!isFromPhraseBook);

    contextMenu->addAction(insertAction);
    contextMenu->addAction(editAction);
    contextMenu->addAction(gotoAction);

    contextMenu->exec(event->globalPos());
    event->accept();
}

void PhraseView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    emit phraseSelected(m_modelIndex, m_phraseModel->phrase(index)->target());
    event->accept();
}

void PhraseView::guessShortcut(int key)
{
    const auto phrases = m_phraseModel->phraseList();
    for (const Phrase *phrase : phrases)
        if (phrase->shortcut() == key) {
            emit phraseSelected(m_modelIndex, phrase->target());
            return;
        }
}

void PhraseView::selectPhrase(const QModelIndex &index)
{
    emit phraseSelected(m_modelIndex, m_phraseModel->phrase(index)->target());
}

void PhraseView::selectCurrentPhrase()
{
    emit phraseSelected(m_modelIndex, m_phraseModel->phrase(currentIndex())->target());
}

void PhraseView::editPhrase()
{
    edit(currentIndex());
}

void PhraseView::gotoMessageFromGuess()
{
    emit setCurrentMessageFromGuess(m_modelIndex,
                                    m_phraseModel->phrase(currentIndex())->candidate());
}

void PhraseView::setMaxCandidates(const int max)
{
    m_maxCandidates = max;
    emit showFewerGuessesAvailable(m_maxCandidates > DefaultMaxCandidates);
}

void PhraseView::moreGuesses()
{
    setMaxCandidates(m_maxCandidates + DefaultMaxCandidates);
    setSourceText(m_modelIndex, m_sourceText);
}

void PhraseView::fewerGuesses()
{
    setMaxCandidates(m_maxCandidates - DefaultMaxCandidates);
    setSourceText(m_modelIndex, m_sourceText);
}

void PhraseView::resetNumGuesses()
{
    setMaxCandidates(DefaultMaxCandidates);
    setSourceText(m_modelIndex, m_sourceText);
}

static CandidateList similarTextHeuristicCandidates(MultiDataModel *model, int mi,
    const char *text, int maxCandidates)
{
    QList<int> scores;
    CandidateList candidates;

    StringSimilarityMatcher stringmatcher(QString::fromLatin1(text));

    for (MultiDataModelIterator it(model, mi); it.isValid(); ++it) {
        MessageItem *m = it.current();
        if (!m)
            continue;

        TranslatorMessage mtm = m->message();
        if (mtm.type() == TranslatorMessage::Unfinished
            || mtm.translation().isEmpty())
            continue;

        QString s = m->text();

        int score = stringmatcher.getSimilarityScore(s);

        if (candidates.size() == maxCandidates && score > scores[maxCandidates - 1])
            candidates.removeLast();
        if (candidates.size() < maxCandidates && score >= textSimilarityThreshold ) {
            Candidate cand(mtm.context(), s, mtm.comment(), mtm.translation());

            int i;
            for (i = 0; i < candidates.size(); ++i) {
                if (score >= scores.at(i)) {
                    if (score == scores.at(i)) {
                        if (candidates.at(i) == cand)
                            goto continue_outer_loop;
                    } else {
                        break;
                    }
                }
            }
            scores.insert(i, score);
            candidates.insert(i, cand);
        }
        continue_outer_loop:
        ;
    }
    return candidates;
}


void PhraseView::setSourceText(int model, const QString &sourceText)
{
    m_modelIndex = model;
    m_sourceText = sourceText;
    m_phraseModel->removePhrases();
    deleteGuesses();

    if (model < 0)
        return;

    const auto phrases = getPhrases(model, sourceText);
    for (Phrase *p : phrases)
        m_phraseModel->addPhrase(p);

    if (!sourceText.isEmpty() && m_doGuesses) {
        const CandidateList cl = similarTextHeuristicCandidates(m_dataModel, model,
            sourceText.toLatin1(), m_maxCandidates);
        int n = 0;
        for (const Candidate &candidate : cl) {
            QString def;
            if (n < 9)
                def = tr("Guess from '%1' (%2)")
                      .arg(candidate.context, QKeySequence(Qt::CTRL | (Qt::Key_0 + (n + 1)))
                                              .toString(QKeySequence::NativeText));
            else
                def = tr("Guess from '%1'").arg(candidate.context);
            Phrase *guess = new Phrase(candidate.source, candidate.translation, def, candidate, n);
            m_guesses.append(guess);
            m_phraseModel->addPhrase(guess);
            ++n;
        }
    }
}

QList<Phrase *> PhraseView::getPhrases(int model, const QString &source)
{
    QList<Phrase *> phrases;
    const QString f = MainWindow::friendlyString(source);
    const QStringList lookupWords = f.split(QLatin1Char(' '));

    for (const QString &s : lookupWords) {
        if (m_phraseDict->at(model).contains(s)) {
            const auto phraseList = m_phraseDict->at(model).value(s);
            for (Phrase *p : phraseList) {
                if (f.contains(MainWindow::friendlyString(p->source())))
                    phrases.append(p);
            }
        }
    }
    return phrases;
}

void PhraseView::deleteGuesses()
{
    qDeleteAll(m_guesses);
    m_guesses.clear();
}

QT_END_NAMESPACE
