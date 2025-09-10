// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "helpdocsettings.h"
#include "helpdocsettingswidget.h"
#include "ui_helpdocsettingswidget.h"

#include <QtCore/QMap>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPushButton>

#include <QtCore/QMap>

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

    const QStringList &fileNames =
        QFileDialog::getOpenFileNames(q, HelpDocSettingsWidget::tr("Add Documentation"), {},
                                      HelpDocSettingsWidget::tr("Qt Compressed Help Files (*.qch)"));
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

    connect(d->m_ui.docAddButton, &QAbstractButton::clicked, this,
            [this]() {
        Q_D(HelpDocSettingsWidget);
        d->addDocumentation();
    });
    connect(d->m_ui.docRemoveButton, &QAbstractButton::clicked, this,
            [this]() {
        Q_D(HelpDocSettingsWidget);
        d->removeDocumentation();
    });
    connect(d->m_ui.registeredDocsFilterLineEdit, &QLineEdit::textChanged, this,
            [this](const QString &) {
        Q_D(HelpDocSettingsWidget);
        for (const auto item : d->m_namespaceToItem)
            d->applyDocListFilter(item);
    });
    connect(d->m_ui.registeredDocsListWidget, &QListWidget::itemSelectionChanged, this,
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
