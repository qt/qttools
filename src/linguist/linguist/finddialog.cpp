/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

/*  TRANSLATOR FindDialog

    Choose Edit|Find from the menu bar or press Ctrl+F to pop up the
    Find dialog
*/

#include "finddialog.h"

QT_BEGIN_NAMESPACE

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    findNxt->setEnabled(false);

    connect(findNxt, &QAbstractButton::clicked,
            this, &FindDialog::emitFindNext);
    connect(useRegExp, &QCheckBox::stateChanged,
            this, &FindDialog::verify);
    connect(led, &QLineEdit::textChanged,
            this, &FindDialog::verify);

    led->setFocus();
}

void FindDialog::verify()
{
    bool validRegExp = true;
    if (useRegExp->isChecked() && !led->text().isEmpty()) {
        m_regExp.setPattern(led->text());
        validRegExp = m_regExp.isValid();
    }
    if (validRegExp && m_redText)
        led->setStyleSheet(QStringLiteral("color: auto;"));
    else if (!validRegExp && !m_redText)
        led->setStyleSheet(QStringLiteral("color: red;"));
    m_redText = !validRegExp;
    findNxt->setEnabled(!led->text().isEmpty() && validRegExp);
}

void FindDialog::emitFindNext()
{
    DataModel::FindLocation where;
    if (sourceText != 0)
        where =
            DataModel::FindLocation(
                (sourceText->isChecked() ? DataModel::SourceText : 0) |
                (translations->isChecked() ? DataModel::Translations : 0) |
                (comments->isChecked() ? DataModel::Comments : 0));
    else
        where = DataModel::Translations;
    emit findNext(led->text(), where, matchCase->isChecked(), ignoreAccelerators->isChecked(),
                  skipObsolete->isChecked(), useRegExp->isChecked());
    led->selectAll();
}

void FindDialog::find()
{
    led->setFocus();

    show();
    activateWindow();
    raise();
}

QT_END_NAMESPACE
