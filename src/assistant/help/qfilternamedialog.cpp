// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qfilternamedialog_p.h"

#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

QFilterNameDialog::QFilterNameDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked,
            this, &QDialog::accept);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked,
            this, &QDialog::reject);
    connect(m_ui.lineEdit, &QLineEdit::textChanged, this, &QFilterNameDialog::updateOkButton);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}

void QFilterNameDialog::setFilterName(const QString &filter)
{
    m_ui.lineEdit->setText(filter);
    m_ui.lineEdit->selectAll();
}

void QFilterNameDialog::updateOkButton()
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(m_ui.lineEdit->text().isEmpty());
}

QT_END_NAMESPACE
