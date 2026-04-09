// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "optiondialog.h"
#include "ui_optiondialog.h"

OptionDialog::OptionDialog(const QList<QCoapOption> &options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionDialog),
    m_options(options)
{
    ui->setupUi(this);
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, [this]() {
        const auto selection = ui->tableWidget->selectedItems();
        ui->removeButton->setEnabled(!selection.isEmpty());
    });

    fillOptions();
    applyOptionValues();

    auto header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

QList<QCoapOption> OptionDialog::options() const
{
    return m_options;
}

void OptionDialog::on_addButton_clicked()
{
    const auto option =
            ui->optionComboBox->currentData(Qt::UserRole).value<QCoapOption::OptionName>();
    m_options.push_back(QCoapOption(option, ui->optionValueEdit->text()));

    addTableRow(ui->optionComboBox->currentText(), ui->optionValueEdit->text());
}

void OptionDialog::on_removeButton_clicked()
{
    const auto idx = ui->tableWidget->currentRow();
    if (idx >= 0 && idx < ui->tableWidget->rowCount()) {
        ui->tableWidget->removeRow(idx);
        m_options.removeAt(idx);
    }
}

void OptionDialog::on_clearButton_clicked()
{
    m_options.clear();
    ui->tableWidget->setRowCount(0);
}

void OptionDialog::fillOptions()
{
    ui->tableWidget->setHorizontalHeaderLabels({tr("Name"), tr("Value")});
    ui->optionComboBox->addItem(tr("None"), QCoapOption::Invalid);
    ui->optionComboBox->addItem(tr("Block1"), QCoapOption::Block1);
    ui->optionComboBox->addItem(tr("Block2"), QCoapOption::Block2);
    ui->optionComboBox->addItem(tr("Content-Format"), QCoapOption::ContentFormat);
    ui->optionComboBox->addItem(tr("If-Match"), QCoapOption::IfMatch);
    ui->optionComboBox->addItem(tr("If-None-Match"), QCoapOption::IfNoneMatch);
    ui->optionComboBox->addItem(tr("Location-Path"), QCoapOption::LocationPath);
    ui->optionComboBox->addItem(tr("Location-Query"), QCoapOption::LocationQuery);
    ui->optionComboBox->addItem(tr("Max-Age"), QCoapOption::MaxAge);
    ui->optionComboBox->addItem(tr("Observe"), QCoapOption::Observe);
    ui->optionComboBox->addItem(tr("Proxy-Scheme"), QCoapOption::ProxyScheme);
    ui->optionComboBox->addItem(tr("Proxy-Uri"), QCoapOption::ProxyUri);
    ui->optionComboBox->addItem(tr("Size1"), QCoapOption::Size1);
    ui->optionComboBox->addItem(tr("Size2"), QCoapOption::Size2);
    ui->optionComboBox->addItem(tr("Uri-Host"), QCoapOption::UriHost);
    ui->optionComboBox->addItem(tr("Uri-Path"), QCoapOption::UriPath);
    ui->optionComboBox->addItem(tr("Uri-Port"), QCoapOption::UriPort);
    ui->optionComboBox->addItem(tr("Uri-Query"), QCoapOption::UriQuery);
}

void OptionDialog::applyOptionValues()
{
    for (const auto &option : std::as_const(m_options)) {
        const int optionIndex = ui->optionComboBox->findData(option.name());
        if (optionIndex != -1)
            addTableRow(ui->optionComboBox->itemText(optionIndex), option.stringValue());
    }
}

void OptionDialog::addTableRow(const QString &name, const QString &value)
{
    const auto rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);

    QTableWidgetItem *optionItem = new QTableWidgetItem(name);
    optionItem->setFlags(optionItem->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(rowCount, 0, optionItem);

    QTableWidgetItem *valueItem = new QTableWidgetItem(value);
    valueItem->setFlags(valueItem->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(rowCount, 1, valueItem);
}
