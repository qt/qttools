// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "machinetranslationdialog.h"
#include "ui_machinetranslationdialog.h"

#include "messagemodel.h"
#include "auto-translation/machinetranslator.h"

#include <QtWidgets/qmessagebox.h>

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

MachineTranslationDialog::MachineTranslationDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::MachineTranslationDialog>()),
      m_translator(std::make_unique<MachineTranslator>())
{
    m_ui->setupUi(this);
    m_ui->statusLabel->setWordWrap(true);
    m_ui->statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_ui->translateButton, &QPushButton::clicked, this,
            &MachineTranslationDialog::translateSelection);
    connect(m_ui->filesComboBox, &QComboBox::currentIndexChanged, this, [this] {
        m_ui->filterComboBox->setCurrentIndex(0);
        updateStatus();
    });
    connect(m_translator.get(), &MachineTranslator::batchTranslated, this,
            &MachineTranslationDialog::onBatchTranslated);
    connect(m_translator.get(), &MachineTranslator::translationFailed, this,
            &MachineTranslationDialog::onTranslationFailed);
    connect(m_ui->groupComboBox, &QComboBox::currentIndexChanged, this, [this] { updateStatus(); });
    connect(m_ui->doneButton, &QPushButton::clicked, this, [this] {
        if (discardTranslations())
            accept();
    });
    connect(m_ui->cancelButton, &QPushButton::clicked, this, [this] {
        if (discardTranslations())
            reject();
    });
    connect(m_ui->applyButton, &QPushButton::clicked, this,
            &MachineTranslationDialog::applyTranslations);

    connect(m_ui->stopButton, &QToolButton::clicked, this, &MachineTranslationDialog::stop);
    connect(m_ui->connectButton, &QPushButton::clicked, this,
            &MachineTranslationDialog::connectToOllama);
    connect(m_translator.get(), &MachineTranslator::modelsReceived, this,
            [this](const QStringList &models) {
                m_ui->modelComboBox->clear();
                m_ui->modelComboBox->addItems(models);
            });
    connect(this, &QDialog::finished, m_translator.get(), &MachineTranslator::stop);
    connect(m_ui->filterComboBox, &QComboBox::currentIndexChanged, this,
            &MachineTranslationDialog::onFilterChanged);
}

void MachineTranslationDialog::setDataModel(MultiDataModel *dm)
{
    m_dataModel = dm;
    refresh(true);
}

void MachineTranslationDialog::refresh(bool init)
{
    if (init) {
        m_ui->filesComboBox->clear();
        m_ui->filesComboBox->addItems(m_dataModel->srcFileNames());
        m_ui->filesComboBox->setCurrentIndex(0);
        m_ui->translationLog->setText(tr("Translation Log"));
        m_ui->translateButton->setEnabled(true);
        m_ui->stopButton->setEnabled(false);
        connectToOllama();
    }
    m_sentTexts = 0;
    m_failedTranslations = 0;
    m_receivedTranslations.clear();
    m_ongoingTranslations.clear();
    m_ui->applyButton->setEnabled(false);
    m_ui->progressBar->setVisible(false);
    m_translator->start();
}

void MachineTranslationDialog::logProgress(const QList<QStringList> &table)
{
    const qsizetype receivedCount = m_receivedTranslations.size();
    m_ui->statusLabel->setText(
            tr("Translation status: %1/%2 source texts translated, %3/%2 failed.")
                    .arg(receivedCount)
                    .arg(m_sentTexts)
                    .arg(m_failedTranslations));
    m_ui->progressBar->setValue((receivedCount + m_failedTranslations) * 100 / m_sentTexts);
    if (!table.empty()) {
        QString html = "<hr/><table cellpadding=\"4\""
                       "style=\""
                       "width:100%; "
                       "margin-left:10px; "
                       "\">"_L1;
        for (const QStringList &row : table) {
            html += "<tr>"_L1;
            for (const QString &col : row)
                html += "<td>%1</td>"_L1.arg(col);
            html += "</tr>"_L1;
        }
        html += "</table>"_L1;
        m_ui->translationLog->append(html);
    }

    if (receivedCount + m_failedTranslations == m_sentTexts) {
        m_ui->translationLog->append(
                tr("<hr/><b>Translation completed: %1/%2 translated, %3/%2 failed.</b>")
                        .arg(receivedCount)
                        .arg(m_sentTexts)
                        .arg(m_failedTranslations));
        m_ui->translateButton->setEnabled(true);
        m_ui->stopButton->setEnabled(false);
        m_ui->applyButton->setEnabled(true);
        m_ui->progressBar->setVisible(false);
    } else {
        m_ui->translateButton->setEnabled(false);
        m_ui->stopButton->setEnabled(true);
        m_ui->progressBar->setVisible(true);
    }
}

void MachineTranslationDialog::logInfo(const QString &info)
{
    m_ui->translationLog->append("<hr/>"_L1);
    m_ui->translationLog->append(info);
}

void MachineTranslationDialog::logError(const QString &error)
{
    m_ui->translationLog->append("<hr/>"_L1);
    m_ui->translationLog->append(
            "<span style=\"color:red; font-weight: bold; \">%1</span>"_L1.arg(error));
}

bool MachineTranslationDialog::discardTranslations()
{
    return (m_receivedTranslations.empty()
            || QMessageBox::warning(
                       this, tr("Qt Linguist"),
                       tr("The already %n translated item(s) will be discarded. Continue?", 0,
                          m_receivedTranslations.size()),
                       QMessageBox::Yes | QMessageBox::No)
                    == QMessageBox::Yes);
}

void MachineTranslationDialog::stop()
{
    m_translator->stop();
    m_ui->stopButton->setEnabled(false);
    m_ui->translateButton->setEnabled(true);
    refresh(false);
    logError(tr("Translation Stopped."));
}

void MachineTranslationDialog::translateSelection()
{
    const QString model = m_ui->modelComboBox->currentText();
    const int id = m_ui->filesComboBox->currentIndex();
    if (model.isEmpty()) {
        logError(tr("Please verify the service URL is valid, "
                    "then select a translation model."));
        return;
    }
    if (id < 0) {
        logError(tr("Please select a file for translation."));
        return;
    }
    if (!discardTranslations())
        return;
    refresh(false);

    const int filter = m_ui->filterComboBox->currentIndex();
    const int group = m_ui->groupComboBox->currentIndex();
    const DataModel *dm = m_dataModel->model(id);
    Messages messages;
    if (filter == 0) {
        QMutexLocker lock(&m_mutex);
        for (DataModelIterator it(TEXTBASED, dm); it.isValid(); ++it) {
            const TranslatorMessage *tm = &it.current()->message();
            if (tm->translation().isEmpty()) {
                messages.items.append(tm);
                m_ongoingTranslations[tm] =
                        MultiDataIndex{ it.translationType(), id, it.group(), it.message() };
            }
        }
        for (DataModelIterator it(IDBASED, dm); it.isValid(); ++it) {
            const TranslatorMessage *tm = &it.current()->message();
            if (tm->translation().isEmpty()) {
                messages.items.append(tm);
                m_ongoingTranslations[tm] =
                        MultiDataIndex{ it.translationType(), id, it.group(), it.message() };
            }
        }
    } else {
        QMutexLocker lock(&m_mutex);
        const auto type = (filter == 1) ? TEXTBASED : IDBASED;
        GroupItem *g = dm->groupItem(group, type);
        for (int i = 0; i < g->messageCount(); i++) {
            const TranslatorMessage *tm = &g->messageItem(i)->message();
            if (tm->translation().isEmpty()) {
                messages.items.append(tm);
                m_ongoingTranslations[tm] = MultiDataIndex{ type, id, group, i };
            }
        }
    }
    messages.srcLang = QLocale::languageToString(dm->sourceLanguage());
    messages.tgtLang = QLocale::languageToString(dm->language());
    m_sentTexts += messages.items.size();
    m_translator->setTranslationModel(model);
    QString userContext = m_ui->contextEdit->toPlainText().trimmed();
    m_translator->translate(messages, std::move(userContext));
    logInfo(tr("Translation Started"));
    logProgress({});
}

void MachineTranslationDialog::onBatchTranslated(
        QHash<const TranslatorMessage *, QString> translations)
{
    QList<QStringList> log;
    log.reserve(translations.size());
    QMutexLocker lock(&m_mutex);
    for (const auto &[msg, translation] : translations.asKeyValueRange()) {
        log.append({ msg->sourceText().simplified(), translation.simplified() });
        m_receivedTranslations.append(std::make_pair(m_ongoingTranslations.take(msg), translation));
    }
    logInfo(tr("Translation Batch:"));
    logProgress(log);
}

void MachineTranslationDialog::onFilterChanged(int id)
{
    m_ui->groupLabel->setEnabled(id != 0);
    m_ui->groupComboBox->setEnabled(id != 0);
    m_ui->groupComboBox->clear();
    int modelId = m_ui->filesComboBox->currentIndex();
    if (modelId < 0)
        return;

    if (id == 1) {
        for (int i = 0; i < m_dataModel->model(modelId)->contextCount(); i++)
            m_ui->groupComboBox->addItem(
                    m_dataModel->model(modelId)->groupItem(i, TEXTBASED)->group());
    } else if (id == 2) {
        for (int i = 0; i < m_dataModel->model(modelId)->labelCount(); i++)
            m_ui->groupComboBox->addItem(
                    m_dataModel->model(modelId)->groupItem(i, IDBASED)->group());
    }
    m_ui->groupComboBox->setCurrentIndex(0);
}

void MachineTranslationDialog::applyTranslations()
{
    QMutexLocker lock(&m_mutex);
    for (const auto &[item, translation] : std::as_const(m_receivedTranslations))
        m_dataModel->setTranslation(item, translation);
    refresh(false);
    logInfo(tr("Translations Applied."));
}

void MachineTranslationDialog::onTranslationFailed(QList<const TranslatorMessage *> failed)
{
    QList<QStringList> log;
    log.reserve(failed.size() + 1);

    QMutexLocker lock(&m_mutex);
    m_failedTranslations += failed.size();
    for (const TranslatorMessage *m : failed) {
        log << QStringList{ m->sourceText().simplified() };
        m_ongoingTranslations.remove(m);
    }
    logError(tr("Failed Translation(s):"));
    logProgress(log);
}

void MachineTranslationDialog::updateStatus()
{
    const int model = m_ui->filesComboBox->currentIndex();
    const int filter = m_ui->filterComboBox->currentIndex();
    const int group = m_ui->groupComboBox->currentIndex();
    if (model < 0 || filter < 0 || (filter > 0 && group < 0)) {
        m_ui->statusLabel->setText(tr("Translation status: -"));
    } else if (filter == 0) {
        int count = 0;
        for (DataModelIterator it(IDBASED, m_dataModel->model(model)); it.isValid(); ++it)
            if (it.current()->translation().isEmpty())
                count++;
        for (DataModelIterator it(TEXTBASED, m_dataModel->model(model)); it.isValid(); ++it)
            if (it.current()->translation().isEmpty())
                count++;

        m_ui->statusLabel->setText(tr("Translation status: %n item(s). For best results, "
                                      "translate in Context / Label batches.",
                                      0, count));
    } else if (group >= 0) {
        const auto type = (filter == 1) ? TEXTBASED : IDBASED;
        int count = 0;
        GroupItem *g = m_dataModel->model(model)->groupItem(group, type);
        for (int i = 0; i < g->messageCount(); i++)
            if (g->messageItem(i)->message().translation().isEmpty())
                count++;
        m_ui->statusLabel->setText(tr("Translation status: %n item(s).", 0, count));
    }
}

void MachineTranslationDialog::connectToOllama()
{
    if (m_ui->serverText->text().isEmpty())
        return;
    m_translator->setUrl(m_ui->serverText->text());
    m_translator->requestModels();
}

MachineTranslationDialog::~MachineTranslationDialog() = default;

QT_END_NAMESPACE
