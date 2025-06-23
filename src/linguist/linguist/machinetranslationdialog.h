// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef MACHINETRANSLATIONDIALOG_H
#define MACHINETRANSLATIONDIALOG_H

#include <QDialog>
#include <QMutex>

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

private:
    void refresh(bool init);
    void logProgress(const QList<QStringList> &table);
    void logInfo(const QString &info);
    void logError(const QString &error);
    bool discardTranslations();
    void updateStatus();

    MultiDataModel *m_dataModel;
    QHash<const TranslatorMessage *, MultiDataIndex> m_ongoingTranslations;
    QList<std::pair<MultiDataIndex, QString>> m_receivedTranslations;
    QMutex m_mutex;
    std::unique_ptr<Ui::MachineTranslationDialog> m_ui;
    std::unique_ptr<MachineTranslator> m_translator;
    int m_failedTranslations = 0;
    int m_sentTexts = 0;

private slots:
    void stop();
    void translateSelection();
    void onBatchTranslated(QHash<const TranslatorMessage *, QString> translations);
    void onFilterChanged(int id);
    void applyTranslations();
    void onTranslationFailed(QList<const TranslatorMessage *>);
    void connectToOllama();
};

QT_END_NAMESPACE

#endif // MACHINETRANSLATIONDIALOG_H
