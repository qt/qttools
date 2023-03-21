// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "qdesigner_appearanceoptions.h"

#include <QtDesigner/abstractoptionspage.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qpushbutton.h>

QT_BEGIN_NAMESPACE

PreferencesDialog::PreferencesDialog(QDesignerFormEditorInterface *core, QWidget *parentWidget) :
    QDialog(parentWidget),
    m_ui(new Ui::PreferencesDialog()),
    m_core(core)
{
    m_ui->setupUi(this);

    m_optionsPages = core->optionsPages();

    m_ui->m_optionTabWidget->clear();
    for (QDesignerOptionsPageInterface *optionsPage : std::as_const(m_optionsPages)) {
        QWidget *page = optionsPage->createPage(this);
        m_ui->m_optionTabWidget->addTab(page, optionsPage->name());
        if (QDesignerAppearanceOptionsWidget *appearanceWidget = qobject_cast<QDesignerAppearanceOptionsWidget *>(page))
            connect(appearanceWidget, &QDesignerAppearanceOptionsWidget::uiModeChanged,
                    this, &PreferencesDialog::slotUiModeChanged);
    }

    connect(m_ui->m_dialogButtonBox, &QDialogButtonBox::rejected, this, &PreferencesDialog::slotRejected);
    connect(m_ui->m_dialogButtonBox, &QDialogButtonBox::accepted, this, &PreferencesDialog::slotAccepted);
    connect(applyButton(), &QAbstractButton::clicked, this, &PreferencesDialog::slotApply);
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

QPushButton *PreferencesDialog::applyButton() const
{
    return m_ui->m_dialogButtonBox->button(QDialogButtonBox::Apply);
}

void PreferencesDialog::slotApply()
{
    for (QDesignerOptionsPageInterface *optionsPage : std::as_const(m_optionsPages))
        optionsPage->apply();
}

void PreferencesDialog::slotAccepted()
{
    slotApply();
    closeOptionPages();
    accept();
}

void PreferencesDialog::slotRejected()
{
    closeOptionPages();
    reject();
}

void PreferencesDialog::slotUiModeChanged(bool modified)
{
    // Cannot "apply" a ui mode change (destroy the dialogs parent)
    applyButton()->setEnabled(!modified);
}

void PreferencesDialog::closeOptionPages()
{
    for (QDesignerOptionsPageInterface *optionsPage : std::as_const(m_optionsPages))
        optionsPage->finish();
}

QT_END_NAMESPACE
