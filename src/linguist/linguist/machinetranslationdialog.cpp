// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "machinetranslationdialog.h"
#include "ui_machinetranslationdialog.h"

#include "messagemodel.h"
#include "auto-translation/machinetranslator.h"
#include "globals.h"

#include <QtCore/qsettings.h>
#include <QtWidgets/qmessagebox.h>

#include <array>

using namespace Qt::Literals::StringLiterals;

namespace {

struct ApiTypeInfo
{
    TranslationApiType type;
    const QLatin1String displayName;
    const QLatin1String defaultUrl;
};

// Central registry of all supported API types and their metadata
constexpr ApiTypeInfo s_apiTypes[] = {
    { TranslationApiType::Ollama, "Ollama"_L1, "http://localhost:11434"_L1 },
    { TranslationApiType::OpenAICompatible, "OpenAI Compatible"_L1, "http://localhost:8080"_L1 },
};

const ApiTypeInfo *findApiTypeInfo(TranslationApiType type)
{
    for (const auto &info : s_apiTypes) {
        if (info.type == type)
            return &info;
    }
    return nullptr;
}

} // namespace

QT_BEGIN_NAMESPACE

static constexpr std::array<const char *, 3> toolBoxTexts {
    QT_TRANSLATE_NOOP("MachineTranslationDialog", "Configuration"),
    QT_TRANSLATE_NOOP("MachineTranslationDialog", "Selection"),
    QT_TRANSLATE_NOOP("MachineTranslationDialog", "Progress")
};

MachineTranslationDialog::MachineTranslationDialog(QWidget *parent)
    : QDialog(parent),
      m_ui(std::make_unique<Ui::MachineTranslationDialog>()),
      m_translator(std::make_unique<MachineTranslator>())
{
    m_ui->setupUi(this);

    updateToolBoxTexts();
    connect(m_ui->toolBox, &QToolBox::currentChanged, this, &MachineTranslationDialog::updateToolBoxTexts);

    m_ui->statusLabel->setWordWrap(true);
    m_ui->statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_ui->translateButton, &QPushButton::clicked, this,
            &MachineTranslationDialog::translateSelection);
    connect(m_ui->filesComboBox, &QComboBox::currentIndexChanged, this, [this] {
        m_ui->filterComboBox->setCurrentIndex(0);
        updateStatus();
    });
    connect(m_ui->groupListWidget, &QListWidget::itemSelectionChanged, this,
            [this] { updateStatus(); });
    connect(m_translator.get(), &MachineTranslator::batchTranslated, this,
            &MachineTranslationDialog::onBatchTranslated);
    connect(m_translator.get(), &MachineTranslator::translationFailed, this,
            &MachineTranslationDialog::onTranslationFailed);
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
            &MachineTranslationDialog::connectToServer);
    connect(m_translator.get(), &MachineTranslator::modelsReceived, this,
            &MachineTranslationDialog::onModelsReceived);
    connect(m_ui->modelComboBox, &QComboBox::currentTextChanged, this, [](const QString &text) {
        if (!text.isEmpty())
            QSettings().setValue(settingPath(selectedModelSettingsKey), text);
    });
    connect(this, &QDialog::finished, m_translator.get(), &MachineTranslator::stop);
    connect(m_ui->filterComboBox, &QComboBox::currentIndexChanged, this,
            &MachineTranslationDialog::onFilterChanged);
    connect(m_ui->serverText, &QLineEdit::textChanged, this,
            &MachineTranslationDialog::onServerUrlChanged);
    connect(m_ui->apiTypeComboBox, &QComboBox::currentIndexChanged, this,
            &MachineTranslationDialog::onApiTypeChanged);

    setConnectionState(ConnectionState::NotConnected);

    // Populate API type combo box from registry
    m_ui->apiTypeComboBox->clear();
    for (const auto &info : s_apiTypes) {
        m_ui->apiTypeComboBox->addItem(QString::fromLatin1(info.displayName),
                                       QVariant::fromValue(info.type));
    }

    // Restore saved API type selection
    QSettings config;
    const int savedApiType = config.value(settingPath(selectedApiTypeSettingsKey),
                                          static_cast<int>(TranslationApiType::Ollama))
                                     .toInt();
    for (int i = 0; i < m_ui->apiTypeComboBox->count(); ++i) {
        if (m_ui->apiTypeComboBox->itemData(i).toInt() == savedApiType) {
            m_ui->apiTypeComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void MachineTranslationDialog::updateToolBoxTexts()
{
    const int count = m_ui->toolBox->count();
    Q_ASSERT(unsigned(count) == toolBoxTexts.size());
    const int index = m_ui->toolBox->currentIndex();

    for (int i = 0; i < count; ++i) {
        const QString baseText = MachineTranslationDialog::tr(toolBoxTexts[i]);
        m_ui->toolBox->setItemText(i, (i == index ? "- "_L1 : "+ "_L1) + baseText);
    }
}

void MachineTranslationDialog::setDataModel(MultiDataModel *dm)
{
    m_dataModel = dm;
    refresh(true);
}

void MachineTranslationDialog::refresh(bool init)
{
    if (init) {
        m_ui->toolBox->setCurrentIndex(0);
        m_ui->filesComboBox->clear();
        m_ui->filesComboBox->addItems(m_dataModel->srcFileNames());
        m_ui->filesComboBox->setCurrentIndex(0);
        m_ui->translationLog->setText(tr("Translation Log"));
        m_ui->translateButton->setEnabled(true);
        m_ui->stopButton->setEnabled(false);
        connectToServer();
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
                       tr("%n translated item(s) will be discarded. Continue?", 0,
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
    m_ui->toolBox->setCurrentIndex(2);
    const QString model = m_ui->modelComboBox->currentText();
    const int id = m_ui->filesComboBox->currentIndex();
    if (model.isEmpty()) {
        logError(tr("Please verify the service URL is valid "
                    "and a translation model is selected."));
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
        const QList<QListWidgetItem *> selectedItems = m_ui->groupListWidget->selectedItems();

        if (selectedItems.isEmpty()) {
            logError(tr("Please select at least one context/label to translate."));
            return;
        }

        QMutexLocker lock(&m_mutex);
        const auto type = (filter == 1) ? TEXTBASED : IDBASED;
        for (QListWidgetItem *item : selectedItems) {
            const int groupIdx = item->data(Qt::UserRole).toInt();
            const GroupItem *g = dm->groupItem(groupIdx, type);
            for (int i = 0; i < g->messageCount(); i++) {
                const TranslatorMessage *tm = &g->messageItem(i)->message();
                if (tm->translation().isEmpty()) {
                    messages.items.append(tm);
                    m_ongoingTranslations[tm] = MultiDataIndex{ type, id, groupIdx, i };
                }
            }
        }
    }
    messages.srcLang = QLocale::languageToString(dm->sourceLanguage());
    messages.tgtLang = QLocale::languageToString(dm->language());
    m_sentTexts += messages.items.size();
    m_translator->activateTranslationModel(model);
    m_translator->translate(messages, m_ui->contextEdit->toPlainText().trimmed());
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
    m_ui->groupListWidget->setEnabled(id != 0);
    m_ui->groupListWidget->clear();
    int modelId = m_ui->filesComboBox->currentIndex();
    if (modelId < 0)
        return;

    QList<QPair<QString, int>> groupsWithIndices;
    if (id == 1) {
        for (int i = 0; i < m_dataModel->model(modelId)->contextCount(); i++)
            groupsWithIndices.append(
                    { m_dataModel->model(modelId)->groupItem(i, TEXTBASED)->group(), i });
    } else if (id == 2) {
        for (int i = 0; i < m_dataModel->model(modelId)->labelCount(); i++)
            groupsWithIndices.append(
                    { m_dataModel->model(modelId)->groupItem(i, IDBASED)->group(), i });
    }

    std::sort(groupsWithIndices.begin(), groupsWithIndices.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.first.compare(b.first, Qt::CaseInsensitive) < 0;
              });

    for (const auto &group : groupsWithIndices) {
        QListWidgetItem *item = new QListWidgetItem(group.first);
        item->setData(Qt::UserRole, group.second);
        m_ui->groupListWidget->addItem(item);
    }
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

    QList<QListWidgetItem *> selectedItems;
    if (filter > 0)
        selectedItems = m_ui->groupListWidget->selectedItems();

    if (model < 0 || filter < 0 || (filter > 0 && selectedItems.isEmpty())) {
        //: No selected items
        m_ui->selectionLabel->setText(tr("Selection status: -"));
    } else if (filter == 0) {
        int count = 0;
        for (DataModelIterator it(IDBASED, m_dataModel->model(model)); it.isValid(); ++it)
            if (it.current()->translation().isEmpty())
                count++;
        for (DataModelIterator it(TEXTBASED, m_dataModel->model(model)); it.isValid(); ++it)
            if (it.current()->translation().isEmpty())
                count++;

        m_ui->selectionLabel->setText(tr("Selected %n item(s).", 0, count));
    } else if (!selectedItems.isEmpty()) {
        const auto type = (filter == 1) ? TEXTBASED : IDBASED;
        int count = 0;
        for (QListWidgetItem *item : std::as_const(selectedItems)) {
            const int groupIdx = item->data(Qt::UserRole).toInt();
            const GroupItem *g = m_dataModel->model(model)->groupItem(groupIdx, type);
            for (int i = 0; i < g->messageCount(); i++)
                if (g->messageItem(i)->message().translation().isEmpty())
                    count++;
        }
        m_ui->selectionLabel->setText(
                tr("Selected %n item(s) in %1 group(s).", 0, count).arg(selectedItems.size()));
    }
}

void MachineTranslationDialog::connectToServer()
{
    if (m_ui->serverText->text().isEmpty()) {
        setConnectionState(ConnectionState::NotConnected);
        return;
    }
    setConnectionState(ConnectionState::Connecting);
    m_translator->setUrl(m_ui->serverText->text());
    m_translator->requestModels();
}

void MachineTranslationDialog::onApiTypeChanged(int index)
{
    const QVariant data = m_ui->apiTypeComboBox->itemData(index);
    if (!data.isValid())
        return;

    const auto apiType = static_cast<TranslationApiType>(data.toInt());
    const ApiTypeInfo *info = findApiTypeInfo(apiType);
    if (!info)
        return;

    QSettings config;
    config.setValue(settingPath(selectedApiTypeSettingsKey), static_cast<int>(apiType));

    m_translator->setApiType(apiType);
    m_ui->serverText->setText(QString::fromLatin1(info->defaultUrl));
    m_ui->modelComboBox->clear();
    connectToServer();
}

void MachineTranslationDialog::onServerUrlChanged()
{
    if (m_connectionState == ConnectionState::Connected
        && m_ui->serverText->text() != m_lastConnectedUrl) {
        setConnectionState(ConnectionState::Modified);
    }
}

void MachineTranslationDialog::onModelsReceived(const QStringList &models)
{
    if (models.isEmpty()) {
        setConnectionState(ConnectionState::Failed);
    } else {
        m_lastConnectedUrl = m_ui->serverText->text();
        setConnectionState(ConnectionState::Connected);

        QSettings config;
        QString savedModel = config.value(settingPath(selectedModelSettingsKey)).toString();
        m_ui->modelComboBox->clear();
        m_ui->modelComboBox->addItems(models);

        // Restore saved selection if found
        if (!savedModel.isEmpty()) {
            int index = m_ui->modelComboBox->findText(savedModel);
            if (index >= 0)
                m_ui->modelComboBox->setCurrentIndex(index);
        }
    }
}

void MachineTranslationDialog::setConnectionState(ConnectionState state)
{
    m_connectionState = state;
    updateConnectionIndicator();
}

void MachineTranslationDialog::updateConnectionIndicator()
{
    QString statusText;
    QString styleSheet;

    switch (m_connectionState) {
    case ConnectionState::NotConnected:
        statusText = tr("Not connected - click Connect to fetch models");
        styleSheet = "QLabel { color: gray; }"_L1;
        break;
    case ConnectionState::Connecting:
        statusText = tr("Connecting...");
        styleSheet = "QLabel { color: orange; }"_L1;
        break;
    case ConnectionState::Connected:
        statusText = tr("Connected");
        styleSheet = "QLabel { color: green;}"_L1;
        break;
    case ConnectionState::Failed:
        statusText = tr("Connection failed - verify server URL and click Connect");
        styleSheet = "QLabel { color: red; }"_L1;
        break;
    case ConnectionState::Modified:
        statusText = tr("URL modified - click Connect to apply");
        styleSheet = "QLabel { color: orange; }"_L1;
        break;
    }

    m_ui->connectionStatusLabel->setText(statusText);
    m_ui->connectionStatusLabel->setStyleSheet(styleSheet);
}

MachineTranslationDialog::~MachineTranslationDialog() = default;

QT_END_NAMESPACE
