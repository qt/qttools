// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QCoapOption>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class OptionDialog;
}
QT_END_NAMESPACE

class OptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionDialog(const QList<QCoapOption> &options, QWidget *parent = nullptr);
    ~OptionDialog();

    QList<QCoapOption> options() const;

private slots:
    void on_addButton_clicked();
    void on_clearButton_clicked();
    void on_removeButton_clicked();

private:
    void fillOptions();
    void applyOptionValues();
    void addTableRow(const QString &name, const QString &value);

    Ui::OptionDialog *ui;
    QList<QCoapOption> m_options;
};

#endif // OPTIONDIALOG_H
