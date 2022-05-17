// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "translatedialog.h"

QT_BEGIN_NAMESPACE

TranslateDialog::TranslateDialog(QWidget *parent)
  : QDialog(parent)
{
    m_ui.setupUi(this);
    connect(m_ui.findNxt, &QAbstractButton::clicked,
            this, &TranslateDialog::emitFindNext);
    connect(m_ui.translate, &QAbstractButton::clicked,
            this, &TranslateDialog::emitTranslateAndFindNext);
    connect(m_ui.translateAll, &QAbstractButton::clicked,
            this, &TranslateDialog::emitTranslateAll);
    connect(m_ui.ledFindWhat, &QLineEdit::textChanged,
            this, &TranslateDialog::verifyText);
    connect(m_ui.ckMatchCase, &QAbstractButton::toggled,
            this, &TranslateDialog::verifyText);
}

void TranslateDialog::showEvent(QShowEvent *)
{
    verifyText();
    m_ui.ledFindWhat->setFocus();
}

void TranslateDialog::verifyText()
{
    QString text = m_ui.ledFindWhat->text();
    bool canFind = !text.isEmpty();
    bool hit = false;
    if (canFind)
        emit requestMatchUpdate(hit);
    m_ui.findNxt->setEnabled(canFind);
    m_ui.translate->setEnabled(canFind && hit);
    m_ui.translateAll->setEnabled(canFind);
}

void TranslateDialog::emitFindNext()
{
    emit activated(Skip);
}

void TranslateDialog::emitTranslateAndFindNext()
{
    emit activated(Translate);
}

void TranslateDialog::emitTranslateAll()
{
    emit activated(TranslateAll);
}

QT_END_NAMESPACE
