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
