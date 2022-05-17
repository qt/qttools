// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BATCHTRANSLATIONDIALOG_H
#define BATCHTRANSLATIONDIALOG_H

#include "ui_batchtranslation.h"
#include "phrase.h"

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

QT_BEGIN_NAMESPACE

class MultiDataModel;

class CheckableListModel : public QStandardItemModel
{
public:
    CheckableListModel(QObject *parent = 0);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

class BatchTranslationDialog : public QDialog
{
    Q_OBJECT
public:
    BatchTranslationDialog(MultiDataModel *model, QWidget *w = 0);
    void setPhraseBooks(const QList<PhraseBook *> &phrasebooks, int modelIndex);

signals:
    void finished();

private slots:
    void startTranslation();
    void movePhraseBookUp();
    void movePhraseBookDown();

private:
    Ui::BatchTranslationDialog m_ui;
    CheckableListModel m_model;
    MultiDataModel *m_dataModel;
    QList<PhraseBook *> m_phrasebooks;
    int m_modelIndex;
};

QT_END_NAMESPACE

#endif // BATCHTRANSLATIONDIALOG_H
