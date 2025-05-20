// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "dialog.h"
#include <QTimeZone>

Dialog::Dialog(QWidget *parent) : QDialog(parent), m_ui(std::make_unique<Ui::Dialog>())
{
    m_ui->setupUi(this);
    const QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray &timeZone : timeZoneIds)
        m_ui->comboBox->addItem(QLatin1String(timeZone));
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this,
            [&] { emit timeZoneSelected(m_ui->comboBox->currentText()); });
}

Dialog::~Dialog() { }
