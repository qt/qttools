// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "templateoptionspage.h"
#include "ui_templateoptionspage.h"

#include <shared_settings_p.h>
#include <iconloader_p.h>

#include <QtDesigner/abstractformeditor.h>
#include <abstractdialoggui_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

// ----------------- TemplateOptionsWidget
TemplateOptionsWidget::TemplateOptionsWidget(QDesignerFormEditorInterface *core, QWidget *parent) :
    QWidget(parent),
    m_core(core),
    m_ui(new QT_PREPEND_NAMESPACE(qdesigner_internal)::Ui::TemplateOptionsWidget)
{
    m_ui->setupUi(this);

    m_ui->m_addTemplatePathButton->setIcon(
            qdesigner_internal::createIconSet("plus.png"_L1));
    m_ui->m_removeTemplatePathButton->setIcon(
            qdesigner_internal::createIconSet("minus.png"_L1));

    connect(m_ui->m_templatePathListWidget, &QListWidget::itemSelectionChanged,
            this, &TemplateOptionsWidget::templatePathSelectionChanged);
    connect(m_ui->m_addTemplatePathButton, &QAbstractButton::clicked,
            this, &TemplateOptionsWidget::addTemplatePath);
    connect(m_ui->m_removeTemplatePathButton, &QAbstractButton::clicked,
            this, &TemplateOptionsWidget::removeTemplatePath);
}

TemplateOptionsWidget::~TemplateOptionsWidget()
{
    delete m_ui;
}

QStringList TemplateOptionsWidget::templatePaths() const
{
    QStringList rc;
    const int count = m_ui->m_templatePathListWidget->count();
    for (int i = 0; i < count; i++) {
        rc += m_ui->m_templatePathListWidget->item(i)->text();
    }
    return rc;
}

void TemplateOptionsWidget::setTemplatePaths(const QStringList &l)
{
    // add paths and select 0
    m_ui->m_templatePathListWidget->clear();
    if (l.isEmpty()) {
        // disable button
        templatePathSelectionChanged();
    } else {
        for (const auto &s : l)
            m_ui->m_templatePathListWidget->addItem(s);
        m_ui->m_templatePathListWidget->setCurrentItem(m_ui->m_templatePathListWidget->item(0));
    }
}

void TemplateOptionsWidget::addTemplatePath()
{
    const QString templatePath = chooseTemplatePath(m_core, this);
    if (templatePath.isEmpty())
        return;

    const auto existing
            = m_ui->m_templatePathListWidget->findItems(templatePath, Qt::MatchExactly);
    if (!existing.isEmpty())
        return;

    QListWidgetItem *newItem = new QListWidgetItem(templatePath);
    m_ui->m_templatePathListWidget->addItem(newItem);
    m_ui->m_templatePathListWidget->setCurrentItem(newItem);
}

void TemplateOptionsWidget::removeTemplatePath()
{
    const auto selectedPaths = m_ui->m_templatePathListWidget->selectedItems();
    if (selectedPaths.isEmpty())
        return;
    delete selectedPaths.constFirst();
}

void TemplateOptionsWidget::templatePathSelectionChanged()
{
    const auto selectedPaths = m_ui->m_templatePathListWidget->selectedItems();
    m_ui->m_removeTemplatePathButton->setEnabled(!selectedPaths.isEmpty());
}

QString TemplateOptionsWidget::chooseTemplatePath(QDesignerFormEditorInterface *core, QWidget *parent)
{
    QString rc = core->dialogGui()->getExistingDirectory(parent,
                                                   tr("Pick a directory to save templates in"));
    if (rc.isEmpty())
        return rc;

    if (rc.endsWith(QDir::separator()))
        rc.remove(rc.size() - 1, 1);
    return rc;
}

// ----------------- TemplateOptionsPage
TemplateOptionsPage::TemplateOptionsPage(QDesignerFormEditorInterface *core) :
    m_core(core)
{
}

QString TemplateOptionsPage::name() const
{
    //: Tab in preferences dialog
    return QCoreApplication::translate("TemplateOptionsPage", "Template Paths");
}

QWidget *TemplateOptionsPage::createPage(QWidget *parent)
{
    m_widget = new TemplateOptionsWidget(m_core, parent);
    m_initialTemplatePaths = QDesignerSharedSettings(m_core).additionalFormTemplatePaths();
    m_widget->setTemplatePaths(m_initialTemplatePaths);
    return m_widget;
}

void TemplateOptionsPage::apply()
{
    if (m_widget) {
        const QStringList newTemplatePaths = m_widget->templatePaths();
        if (newTemplatePaths != m_initialTemplatePaths) {
            QDesignerSharedSettings settings(m_core);
            settings.setAdditionalFormTemplatePaths(newTemplatePaths);
            m_initialTemplatePaths = newTemplatePaths;
        }
    }
}

void TemplateOptionsPage::finish()
{
}
}
QT_END_NAMESPACE
