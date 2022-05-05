/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include <QtWidgets/QPushButton>

#include "qfilternamedialog_p.h"

QT_BEGIN_NAMESPACE

QFilterNameDialog::QFilterNameDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked,
            this, &QDialog::accept);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked,
            this, &QDialog::reject);
    connect(m_ui.lineEdit, &QLineEdit::textChanged,
            this, &QFilterNameDialog::updateOkButton);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
}

void QFilterNameDialog::setFilterName(const QString &filter)
{
    m_ui.lineEdit->setText(filter);
    m_ui.lineEdit->selectAll();
}

QString QFilterNameDialog::filterName() const
{
    return m_ui.lineEdit->text();
}

void QFilterNameDialog::updateOkButton()
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)
        ->setDisabled(m_ui.lineEdit->text().isEmpty());
}

QT_END_NAMESPACE
