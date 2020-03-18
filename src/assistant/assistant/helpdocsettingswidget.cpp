/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "helpdocsettings.h"
#include "helpdocsettingswidget.h"
#include "ui_helpdocsettingswidget.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class HelpDocSettingsWidgetPrivate
{
    HelpDocSettingsWidget *q_ptr;
    Q_DECLARE_PUBLIC(HelpDocSettingsWidget)
public:
    HelpDocSettingsWidgetPrivate() = default;

    void addDocumentation();
    void removeDocumentation();
    void applyDocListFilter(QListWidgetItem *item);

    QMap<QString, QListWidgetItem *> m_namespaceToItem;
    QHash<QListWidgetItem *, QString> m_itemToNamespace;

    Ui::HelpDocSettingsWidget m_ui;
    HelpDocSettings m_settings;
};

void HelpDocSettingsWidgetPrivate::addDocumentation()
{
    Q_Q(HelpDocSettingsWidget);

    const QStringList &fileNames = QFileDialog::getOpenFileNames(q,
        q->tr("Add Documentation"), QString(), q->tr("Qt Compressed Help Files (*.qch)"));
    if (fileNames.isEmpty())
        return;

    bool added = false;

    for (const QString &fileName : fileNames) {
        if (!m_settings.addDocumentation(fileName))
            continue;

        if (!added) {
            added = true;
            m_ui.registeredDocsListWidget->clearSelection();
        }

        const QString namespaceName = m_settings.namespaceName(fileName);
        QListWidgetItem *item = new QListWidgetItem(namespaceName);
        m_namespaceToItem.insert(namespaceName, item);
        m_itemToNamespace.insert(item, namespaceName);
        m_ui.registeredDocsListWidget->insertItem(m_namespaceToItem.keys().indexOf(namespaceName), item);

        item->setSelected(true);
        applyDocListFilter(item);
    }

    if (added)
        emit q->docSettingsChanged(m_settings);
}

void HelpDocSettingsWidgetPrivate::removeDocumentation()
{
    Q_Q(HelpDocSettingsWidget);

    const QList<QListWidgetItem *> selectedItems = m_ui.registeredDocsListWidget->selectedItems();
    if (selectedItems.isEmpty())
        return;

    for (QListWidgetItem *item : selectedItems) {
        const QString namespaceName = m_itemToNamespace.value(item);
        m_itemToNamespace.remove(item);
        m_namespaceToItem.remove(namespaceName);
        delete item;

        m_settings.removeDocumentation(namespaceName);
    }

    emit q->docSettingsChanged(m_settings);
}

void HelpDocSettingsWidgetPrivate::applyDocListFilter(QListWidgetItem *item)
{
    const QString namespaceName = m_itemToNamespace.value(item);
    const QString nameFilter = m_ui.registeredDocsFilterLineEdit->text();

    const bool matches = nameFilter.isEmpty() || namespaceName.contains(nameFilter);

    if (!matches)
        item->setSelected(false);
    item->setHidden(!matches);
}

HelpDocSettingsWidget::HelpDocSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new HelpDocSettingsWidgetPrivate())
{
    Q_D(HelpDocSettingsWidget);
    d->q_ptr = this;
    d->m_ui.setupUi(this);

    connect(d->m_ui.docAddButton, &QAbstractButton::clicked,
            [this]() {
        Q_D(HelpDocSettingsWidget);
        d->addDocumentation();
    });
    connect(d->m_ui.docRemoveButton, &QAbstractButton::clicked,
            [this]() {
        Q_D(HelpDocSettingsWidget);
        d->removeDocumentation();
    });
    connect(d->m_ui.registeredDocsFilterLineEdit, &QLineEdit::textChanged,
            [this](const QString &) {
        Q_D(HelpDocSettingsWidget);
        for (const auto item : d->m_namespaceToItem)
            d->applyDocListFilter(item);
    });
    connect(d->m_ui.registeredDocsListWidget, &QListWidget::itemSelectionChanged,
            [this]() {
        Q_D(HelpDocSettingsWidget);
        d->m_ui.docRemoveButton->setEnabled(
                    !d->m_ui.registeredDocsListWidget->selectedItems().isEmpty());
    });
}

HelpDocSettingsWidget::~HelpDocSettingsWidget() = default;

void HelpDocSettingsWidget::setDocSettings(const HelpDocSettings &settings)
{
    Q_D(HelpDocSettingsWidget);
    d->m_settings = settings;

    d->m_ui.registeredDocsListWidget->clear();
    d->m_namespaceToItem.clear();
    d->m_itemToNamespace.clear();

    for (const QString &namespaceName : d->m_settings.namespaces()) {
        QListWidgetItem *item = new QListWidgetItem(namespaceName);
        d->m_namespaceToItem.insert(namespaceName, item);
        d->m_itemToNamespace.insert(item, namespaceName);
        d->m_ui.registeredDocsListWidget->addItem(item);
        d->applyDocListFilter(item);
    }

    d->m_ui.docRemoveButton->setEnabled(
                !d->m_ui.registeredDocsListWidget->selectedItems().isEmpty());
}

HelpDocSettings HelpDocSettingsWidget::docSettings() const
{
    Q_D(const HelpDocSettingsWidget);
    return d->m_settings;
}

QT_END_NAMESPACE
