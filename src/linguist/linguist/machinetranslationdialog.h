// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MACHINETRANSLATIONDIALOG_H
#define MACHINETRANSLATIONDIALOG_H

#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtWidgets/qdialog.h>

#include <memory>
#include <utility>

QT_BEGIN_NAMESPACE

namespace Ui {
class MachineTranslationDialog;
}

class MultiDataModel;
class MachineTranslator;
class TranslatorMessage;
class MultiDataIndex;

class MachineTranslationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MachineTranslationDialog(QWidget *parent = nullptr);
    ~MachineTranslationDialog();

    void setDataModel(MultiDataModel *dm);

    static constexpr const char *selectedModelSettingsKey = "MachineTranslation/SelectedModel";
    static constexpr const char *selectedApiTypeSettingsKey = "MachineTranslation/ApiType";

private:
    enum class ConnectionState { NotConnected, Connecting, Connected, Failed, Modified };

    void refresh(bool init);
    void logProgress(const QList<QStringList> &table);
    void logInfo(const QString &info);
    void logWarning(const QString &warning);
    void logError(const QString &error);
    bool discardTranslations();
    void updateStatus();
    void setConnectionState(ConnectionState state);
    void updateConnectionIndicator();

    MultiDataModel *m_dataModel;
    QHash<const TranslatorMessage *, MultiDataIndex> m_ongoingTranslations;
    QList<std::pair<MultiDataIndex, QStringList>> m_receivedTranslations;
    QMutex m_mutex;
    std::unique_ptr<Ui::MachineTranslationDialog> m_ui;
    std::unique_ptr<MachineTranslator> m_translator;
    int m_failedTranslations = 0;
    int m_sentTexts = 0;
    ConnectionState m_connectionState = ConnectionState::NotConnected;
    QString m_lastConnectedUrl;

private slots:
    void stop();
    void translateSelection();
    void onBatchTranslated(QHash<const TranslatorMessage *, QStringList> translations);
    void onNewDebugMessage(const QByteArray &message, bool fromLlm);
    void onFilterChanged(int id);
    void applyTranslations();
    void onTranslationFailed(QList<const TranslatorMessage *>);
    void connectToServer();
    void onApiTypeChanged(int index);
    void onServerUrlChanged();
    void onModelsReceived(const QStringList &models);
    void updateToolBoxTexts();
    void toggleAdvancedSettings(bool checked);
    void loadAdvancedSettings();
    void saveAdvancedSettings();
    void resetAdvancedSettings();
    void validateAdvancedSettings();
};

QT_END_NAMESPACE

#endif // MACHINETRANSLATIONDIALOG_H
