// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "batchtranslationdialog.h"
#include "phrase.h"
#include "messagemodel.h"

#include <QtCore/QMap>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

QT_BEGIN_NAMESPACE

CheckableListModel::CheckableListModel(QObject *parent)
  : QStandardItemModel(parent)
{
}

Qt::ItemFlags CheckableListModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

BatchTranslationDialog::BatchTranslationDialog(MultiDataModel *dataModel, QWidget *w)
 : QDialog(w), m_model(this), m_dataModel(dataModel)
{
    m_ui.setupUi(this);
    connect(m_ui.runButton, &QAbstractButton::clicked,
            this, &BatchTranslationDialog::startTranslation);
    connect(m_ui.moveUpButton, &QAbstractButton::clicked,
            this, &BatchTranslationDialog::movePhraseBookUp);
    connect(m_ui.moveDownButton, &QAbstractButton::clicked,
            this, &BatchTranslationDialog::movePhraseBookDown);

    m_ui.phrasebookList->setModel(&m_model);
    m_ui.phrasebookList->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_ui.phrasebookList->setSelectionMode(QAbstractItemView::SingleSelection);
}


void BatchTranslationDialog::setPhraseBooks(const QList<PhraseBook *> &phrasebooks, int modelIndex)
{
    QString fn = QFileInfo(m_dataModel->srcFileName(modelIndex)).baseName();
    setWindowTitle(tr("Batch Translation of '%1' - Qt Linguist").arg(fn));
    m_model.clear();
    m_model.insertColumn(0);
    m_phrasebooks = phrasebooks;
    m_modelIndex = modelIndex;
    int count = phrasebooks.size();
    m_model.insertRows(0, count);
    for (int i = 0; i < count; ++i) {
        QModelIndex idx(m_model.index(i, 0));
        m_model.setData(idx, phrasebooks[i]->friendlyPhraseBookName());
        int sortOrder;
        if (phrasebooks[i]->language() != QLocale::C
            && m_dataModel->language(m_modelIndex) != QLocale::C) {
            if (phrasebooks[i]->language() != m_dataModel->language(m_modelIndex))
                sortOrder = 3;
            else
                sortOrder = (phrasebooks[i]->territory()
                             == m_dataModel->model(m_modelIndex)->territory()) ? 0 : 1;
        } else {
            sortOrder = 2;
        }
        m_model.setData(idx, sortOrder == 3 ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
        m_model.setData(idx, sortOrder, Qt::UserRole + 1);
        m_model.setData(idx, i, Qt::UserRole);
    }
    m_model.setSortRole(Qt::UserRole + 1);
    m_model.sort(0);
}

void BatchTranslationDialog::startTranslation()
{
    int translatedcount = 0;
    QCursor oldCursor = cursor();
    setCursor(Qt::BusyCursor);
    int messageCount = m_dataModel->messageCount();

    QProgressDialog *dlgProgress;
    dlgProgress = new QProgressDialog(tr("Searching, please wait..."), tr("&Cancel"), 0, messageCount, this);
    dlgProgress->show();

    int msgidx = 0;
    const bool translateTranslated = m_ui.ckTranslateTranslated->isChecked();
    const bool translateFinished = m_ui.ckTranslateFinished->isChecked();
    for (MultiDataModelIterator it(m_dataModel, m_modelIndex); it.isValid(); ++it) {
        if (MessageItem *m = it.current()) {
            if (!m->isObsolete()
                && (translateTranslated || m->translation().isEmpty())
                && (translateFinished || !m->isFinished())) {

                // Go through them in the order the user specified in the phrasebookList
                for (int b = 0; b < m_model.rowCount(); ++b) {
                    QModelIndex idx(m_model.index(b, 0));
                    QVariant checkState = m_model.data(idx, Qt::CheckStateRole);
                    if (checkState == Qt::Checked) {
                        PhraseBook *pb = m_phrasebooks[m_model.data(idx, Qt::UserRole).toInt()];
                        const auto phrases = pb->phrases();
                        for (const Phrase *ph : phrases) {
                            if (ph->source() == m->text()) {
                                m_dataModel->setTranslation(it, ph->target());
                                m_dataModel->setFinished(it, m_ui.ckMarkFinished->isChecked());
                                ++translatedcount;
                                goto done; // break 2;
                            }
                        }
                    }
                }
            }
        }
      done:
        ++msgidx;
        if (!(msgidx & 15))
            dlgProgress->setValue(msgidx);
        qApp->processEvents();
        if (dlgProgress->wasCanceled())
            break;
    }
    dlgProgress->hide();

    setCursor(oldCursor);
    emit finished();
    QMessageBox::information(this, tr("Linguist batch translator"),
        tr("Batch translated %n entries", "", translatedcount), QMessageBox::Ok);
}

void BatchTranslationDialog::movePhraseBookUp()
{
    QModelIndexList indexes = m_ui.phrasebookList->selectionModel()->selectedIndexes();
    if (indexes.size() <= 0) return;

    QModelIndex sel = indexes[0];
    int row = sel.row();
    if (row > 0) {
        QModelIndex other = m_model.index(row - 1, 0);
        QMap<int, QVariant> seldata = m_model.itemData(sel);
        m_model.setItemData(sel, m_model.itemData(other));
        m_model.setItemData(other, seldata);
        m_ui.phrasebookList->selectionModel()->setCurrentIndex(other, QItemSelectionModel::ClearAndSelect);
    }
}

void BatchTranslationDialog::movePhraseBookDown()
{
    QModelIndexList indexes = m_ui.phrasebookList->selectionModel()->selectedIndexes();
    if (indexes.size() <= 0) return;

    QModelIndex sel = indexes[0];
    int row = sel.row();
    if (row < m_model.rowCount() - 1) {
        QModelIndex other = m_model.index(row + 1, 0);
        QMap<int, QVariant> seldata = m_model.itemData(sel);
        m_model.setItemData(sel, m_model.itemData(other));
        m_model.setItemData(other, seldata);
        m_ui.phrasebookList->selectionModel()->setCurrentIndex(other, QItemSelectionModel::ClearAndSelect);
    }
}

QT_END_NAMESPACE
