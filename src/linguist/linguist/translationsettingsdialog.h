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
