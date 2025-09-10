// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef PHRASEBOOKBOX_H
#define PHRASEBOOKBOX_H

#include "ui_phrasebookbox.h"
#include "phrase.h"
#include "phrasemodel.h"
#include <QDialog>

QT_BEGIN_NAMESPACE

class TranslationSettingsDialog;

class QSortFilterProxyModel;

class PhraseBookBox : public QDialog, public Ui::PhraseBookBox
{
    Q_OBJECT
public:
    PhraseBookBox(PhraseBook *phraseBook, QWidget *parent = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void newPhrase();
    void removePhrase();
    void settings();
    void save();
    void sourceChanged(const QString &source);
    void targetChanged(const QString &target);
    void definitionChanged(const QString &definition);
    void selectionChanged();

private:
    void selectItem(const QModelIndex &index);
    void enableDisable();
    QModelIndex currentPhraseIndex() const;

    QString fn;
    PhraseBook *m_phraseBook;
    PhraseModel *phrMdl;
    QSortFilterProxyModel *m_sortedPhraseModel;
    TranslationSettingsDialog *m_translationSettingsDialog;
};

QT_END_NAMESPACE

#endif
