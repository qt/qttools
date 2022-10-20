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

#ifndef PHRASEVIEW_H
#define PHRASEVIEW_H

#include <QList>
#include <QTreeView>
#include "phrase.h"

QT_BEGIN_NAMESPACE

static const int DefaultMaxCandidates = 5;

class MultiDataModel;
class PhraseModel;

class PhraseView : public QTreeView
{
    Q_OBJECT

public:
    PhraseView(MultiDataModel *model, QList<QHash<QString, QList<Phrase *> > > *phraseDict, QWidget *parent = 0);
    ~PhraseView();
    void setSourceText(int model, const QString &sourceText);

public slots:
    void toggleGuessing();
    void update();
    int getMaxCandidates() const { return m_maxCandidates; }
    void setMaxCandidates(const int max);
    static int getDefaultMaxCandidates() { return DefaultMaxCandidates; }
    void moreGuesses();
    void fewerGuesses();
    void resetNumGuesses();

signals:
    void phraseSelected(int latestModel, const QString &phrase);
    void showFewerGuessesAvailable(bool canShow);
    void setCurrentMessageFromGuess(int modelIndex, const Candidate &cand);

protected:
    // QObject
    void contextMenuEvent(QContextMenuEvent *event) override;
    // QAbstractItemView
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void guessShortcut(int nkey);
    void selectPhrase(const QModelIndex &index);
    void selectCurrentPhrase();
    void editPhrase();
    void gotoMessageFromGuess();

private:
    QList<Phrase *> getPhrases(int model, const QString &sourceText);
    void deleteGuesses();

    MultiDataModel *m_dataModel;
    QList<QHash<QString, QList<Phrase *> > > *m_phraseDict;
    QList<Phrase *> m_guesses;
    PhraseModel *m_phraseModel;
    QString m_sourceText;
    int m_modelIndex;
    bool m_doGuesses;
    int m_maxCandidates = DefaultMaxCandidates;
};

QT_END_NAMESPACE

#endif // PHRASEVIEW_H
