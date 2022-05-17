// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TRANSLATIONSETTINGSDIALOG_H
#define TRANSLATIONSETTINGSDIALOG_H

#include "ui_translationsettings.h"

#include <QtCore/QLocale>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class DataModel;
class PhraseBook;

class TranslationSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    TranslationSettingsDialog(QWidget *parent = 0);
    void setDataModel(DataModel *model);
    void setPhraseBook(PhraseBook *phraseBook);

private:
    void showEvent(QShowEvent *e) override;

private slots:
    void on_buttonBox_accepted();
    void on_srcCbLanguageList_currentIndexChanged(int idx);
    void on_tgtCbLanguageList_currentIndexChanged(int idx);

private:
    Ui::TranslationSettingsDialog m_ui;
    DataModel *m_dataModel;
    PhraseBook *m_phraseBook;

};

QT_END_NAMESPACE

#endif // TRANSLATIONSETTINGSDIALOG_H
