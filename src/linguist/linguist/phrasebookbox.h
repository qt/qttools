/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
******************************************************************************/

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
