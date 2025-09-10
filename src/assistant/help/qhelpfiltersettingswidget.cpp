// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qhelpfilterdata.h"
#include "qfilternamedialog_p.h"
#include "qhelpfiltersettingswidget.h"
#include "ui_qhelpfiltersettingswidget.h"

#include <QtCore/qversionnumber.h>
#include <QtHelp/qhelpfilterdata.h>
#include <QtHelp/qhelpfilterengine.h>
#include <QtWidgets/qmessagebox.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QHelpFilterSettings final
{
public:
    void setFilter(const QString &filterName, const QHelpFilterData &filterData)
    {
        m_filterToData.insert(filterName, filterData);
    }
    void removeFilter(const QString &filterName) { m_filterToData.remove(filterName); }
    QHelpFilterData filterData(const QString &filterName) const
    {
        return m_filterToData.value(filterName);
    }
    QMap<QString, QHelpFilterData> filters() const { return m_filterToData; }

    void setCurrentFilter(const QString &filterName) { m_currentFilter = filterName; }
    QString currentFilter() const { return m_currentFilter; }

private:
    QMap<QString, QHelpFilterData> m_filterToData;
    QString m_currentFilter;
};

static QHelpFilterSettings readSettingsHelper(const QHelpFilterEngine *filterEngine)
{
    QHelpFilterSettings filterSettings;

    const QStringList allFilters = filterEngine->filters();
    for (const QString &filter : allFilters)
        filterSettings.setFilter(filter, filterEngine->filterData(filter));

    filterSettings.setCurrentFilter(filterEngine->activeFilter());
    return filterSettings;
}

static QMap<QString, QHelpFilterData> subtract(const QMap<QString, QHelpFilterData> &minuend,
                                               const QMap<QString, QHelpFilterData> &subtrahend)
{
    QMap<QString, QHelpFilterData> result = minuend;

    for (auto itSubtrahend = subtrahend.cbegin(); itSubtrahend != subtrahend.cend(); ++itSubtrahend) {
        auto itResult = result.find(itSubtrahend.key());
        if (itResult != result.end() && itSubtrahend.value() == itResult.value())
            result.erase(itResult);
    }
    return result;
}

static bool applySettingsHelper(QHelpFilterEngine *filterEngine, const QHelpFilterSettings &settings)
{
    bool changed = false;
    const QHelpFilterSettings oldSettings = readSettingsHelper(filterEngine);

    const auto filtersToRemove = subtract(oldSettings.filters(), settings.filters());
    const auto filtersToAdd = subtract(settings.filters(), oldSettings.filters());

    const QString &currentFilter = filterEngine->activeFilter();

    for (auto it = filtersToRemove.cbegin(); it != filtersToRemove.cend(); ++it) {
        filterEngine->removeFilter(it.key());
        if (currentFilter == it.key() && !filtersToAdd.contains(it.key()))
            filterEngine->setActiveFilter({});
        changed = true;
    }

    for (auto it = filtersToAdd.cbegin(); it != filtersToAdd.cend(); ++it) {
        filterEngine->setFilterData(it.key(), it.value());
        changed = true;
    }

    if (changed)
        filterEngine->setActiveFilter(settings.currentFilter());
    return changed;
}

static QStringList versionsToStringList(const QList<QVersionNumber> &versions)
{
    QStringList versionList;
    for (const QVersionNumber &version : versions)
        versionList.append(version.isNull() ? QString() : version.toString());
    return versionList;
}

static QList<QVersionNumber> stringListToVersions(const QStringList &versionList)
{
    QList<QVersionNumber> versions;
    for (const QString &versionString : versionList)
        versions.append(QVersionNumber::fromString(versionString));
    return versions;
}

class QHelpFilterSettingsWidgetPrivate
{
    QHelpFilterSettingsWidget *q_ptr;
    Q_DECLARE_PUBLIC(QHelpFilterSettingsWidget) // TODO: remove Q_DECLARE_PUBLIC
public:
    QHelpFilterSettingsWidgetPrivate() = default;

    QHelpFilterSettings filterSettings() const { return m_filterSettings; }
    void setFilterSettings(const QHelpFilterSettings &settings);

    void updateCurrentFilter();
    void componentsChanged(const QStringList &components);
    void versionsChanged(const QStringList &versions);
    void addFilterClicked();
    void renameFilterClicked();
    void removeFilterClicked();
    void addFilter(const QString &filterName, const QHelpFilterData &filterData = {});
    void removeFilter(const QString &filterName);
    QString getUniqueFilterName(const QString &windowTitle, const QString &initialFilterName);
    QString suggestedNewFilterName(const QString &initialFilterName) const;

    QMap<QString, QListWidgetItem *> m_filterToItem;
    QHash<QListWidgetItem *, QString> m_itemToFilter;

    Ui::QHelpFilterSettingsWidget m_ui;
    QStringList m_components;
    QList<QVersionNumber> m_versions;
    QHelpFilterSettings m_filterSettings;
};

void QHelpFilterSettingsWidgetPrivate::setFilterSettings(const QHelpFilterSettings &settings)
{
    QString currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty()) {
        if (!m_filterSettings.currentFilter().isEmpty())
            currentFilter = m_filterSettings.currentFilter();
        else
            currentFilter = settings.currentFilter();
    }

    m_filterSettings = settings;

    m_ui.filterWidget->clear();
    m_ui.componentWidget->clear();
    m_ui.versionWidget->clear();
    m_itemToFilter.clear();
    m_filterToItem.clear();

    const auto filters = m_filterSettings.filters();
    for (auto it = filters.cbegin(); it != filters.cend(); ++it) {
        const QString &filterName = it.key();
        QListWidgetItem *item = new QListWidgetItem(filterName);
        m_ui.filterWidget->addItem(item);
        m_itemToFilter.insert(item, filterName);
        m_filterToItem.insert(filterName, item);
        if (filterName == currentFilter)
            m_ui.filterWidget->setCurrentItem(item);
    }

    if (!m_ui.filterWidget->currentItem() && !m_filterToItem.isEmpty())
        m_ui.filterWidget->setCurrentItem(m_filterToItem.first());

    updateCurrentFilter();
}

void QHelpFilterSettingsWidgetPrivate::updateCurrentFilter()
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());

    const bool filterSelected = !currentFilter.isEmpty();
    m_ui.componentWidget->setEnabled(filterSelected);
    m_ui.versionWidget->setEnabled(filterSelected);
    m_ui.renameButton->setEnabled(filterSelected);
    m_ui.removeButton->setEnabled(filterSelected);

    m_ui.componentWidget->setOptions(m_components,
            m_filterSettings.filterData(currentFilter).components());
    m_ui.versionWidget->setOptions(versionsToStringList(m_versions),
            versionsToStringList(m_filterSettings.filterData(currentFilter).versions()));
}

void QHelpFilterSettingsWidgetPrivate::componentsChanged(const QStringList &components)
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    QHelpFilterData filterData = m_filterSettings.filterData(currentFilter);
    filterData.setComponents(components);
    m_filterSettings.setFilter(currentFilter, filterData);
}

void QHelpFilterSettingsWidgetPrivate::versionsChanged(const QStringList &versions)
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    QHelpFilterData filterData = m_filterSettings.filterData(currentFilter);
    filterData.setVersions(stringListToVersions(versions));
    m_filterSettings.setFilter(currentFilter, filterData);
}

void QHelpFilterSettingsWidgetPrivate::addFilterClicked()
{
    const QString newFilterName = getUniqueFilterName(QHelpFilterSettingsWidget::tr("Add Filter"),
                                  suggestedNewFilterName(QHelpFilterSettingsWidget::tr("New Filter")));
    if (newFilterName.isEmpty())
        return;

    addFilter(newFilterName);
}

void QHelpFilterSettingsWidgetPrivate::renameFilterClicked()
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    const QString newFilterName = getUniqueFilterName(QHelpFilterSettingsWidget::tr("Rename Filter"), currentFilter);
    if (newFilterName.isEmpty())
        return;

    const QHelpFilterData oldFilterData = m_filterSettings.filterData(currentFilter);
    removeFilter(currentFilter);
    addFilter(newFilterName, oldFilterData);

    if (m_filterSettings.currentFilter() == currentFilter)
        m_filterSettings.setCurrentFilter(newFilterName);
}

void QHelpFilterSettingsWidgetPrivate::removeFilterClicked()
{
    Q_Q(QHelpFilterSettingsWidget);

    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    if (QMessageBox::question(q, QHelpFilterSettingsWidget::tr("Remove Filter"),
            QHelpFilterSettingsWidget::tr("Are you sure you want to remove the \"%1\" filter?")
            .arg(currentFilter), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    removeFilter(currentFilter);

    if (m_filterSettings.currentFilter() == currentFilter)
        m_filterSettings.setCurrentFilter({});
}

void QHelpFilterSettingsWidgetPrivate::addFilter(const QString &filterName,
                                                 const QHelpFilterData &filterData)
{
    QListWidgetItem *item = new QListWidgetItem(filterName);
    m_filterSettings.setFilter(filterName, filterData);
    m_filterToItem.insert(filterName, item);
    m_itemToFilter.insert(item, filterName);
    m_ui.filterWidget->insertItem(m_filterToItem.keys().indexOf(filterName), item);

    m_ui.filterWidget->setCurrentItem(item);
    updateCurrentFilter();
}

void QHelpFilterSettingsWidgetPrivate::removeFilter(const QString &filterName)
{
    QListWidgetItem *item = m_filterToItem.value(filterName);
    m_itemToFilter.remove(item);
    m_filterToItem.remove(filterName);
    delete item;
    m_filterSettings.removeFilter(filterName);
}

QString QHelpFilterSettingsWidgetPrivate::getUniqueFilterName(const QString &windowTitle,
                                                              const QString &initialFilterName)
{
    Q_Q(QHelpFilterSettingsWidget);

    QString newFilterName = initialFilterName;
    while (true) {
        QFilterNameDialog dialog(q);
        dialog.setWindowTitle(windowTitle);
        dialog.setFilterName(newFilterName);
        if (dialog.exec() == QDialog::Rejected)
            return {};

        newFilterName = dialog.filterName();
        if (!m_filterToItem.contains(newFilterName))
            break;

        if (QMessageBox::warning(q, QHelpFilterSettingsWidget::tr("Filter Exists"),
                QHelpFilterSettingsWidget::tr("The filter \"%1\" already exists.").arg(newFilterName),
                QMessageBox::Retry | QMessageBox::Cancel) == QMessageBox::Cancel) {
            return {};
        }
    }
    return newFilterName;
}

QString QHelpFilterSettingsWidgetPrivate::suggestedNewFilterName(const QString &initialFilterName) const
{
    QString newFilterName = initialFilterName;
    int counter = 1;
    while (m_filterToItem.contains(newFilterName))
        newFilterName = initialFilterName + u' ' + QString::number(++counter);
    return newFilterName;
}

/*!
    \class QHelpFilterSettingsWidget
    \inmodule QtHelp
    \since 5.15
    \brief The QHelpFilterSettingsWidget class provides a widget that allows
    for creating, editing and removing filters.

    The instance of QHelpFilterSettingsWidget may be a part of
    a preferences dialog. Before showing the dialog, \l setAvailableComponents()
    and \l setAvailableVersions() should be called, otherwise the filter
    settings widget will only offer a creation of empty filters,
    which wouldn't be useful. In addition, \l readSettings should also
    be called to fill up the filter settings widget with the list of filters
    already stored in the filter engine. The creation of new filters,
    modifications to existing filters and removal of unneeded filters are
    handled by the widget automatically. If you want to store the current
    state of the widget and apply it to the filter engine e.g. after
    the user clicked the apply button - call \l applySettings().
*/

/*!
    Constructs a filter settings widget with \a parent as parent widget.
*/
QHelpFilterSettingsWidget::QHelpFilterSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new QHelpFilterSettingsWidgetPrivate())
{
    Q_D(QHelpFilterSettingsWidget);
    d->q_ptr = this;
    d->m_ui.setupUi(this);

    // TODO: make resources configurable
    QString resourcePath = ":/qt-project.org/assistant/images/"_L1;
#ifdef Q_OS_MACOS
    resourcePath.append("mac"_L1);
#else
    resourcePath.append("win"_L1);
#endif
    d->m_ui.addButton->setIcon(QIcon(resourcePath + "/plus.png"_L1));
    d->m_ui.removeButton->setIcon(QIcon(resourcePath + "/minus.png"_L1));

    connect(d->m_ui.componentWidget, &QOptionsWidget::optionSelectionChanged,
            this, [this](const QStringList &options) {
        Q_D(QHelpFilterSettingsWidget);
        d->componentsChanged(options);
    });
    connect(d->m_ui.versionWidget, &QOptionsWidget::optionSelectionChanged,
            this, [this](const QStringList &options) {
        Q_D(QHelpFilterSettingsWidget);
        d->versionsChanged(options);
    });
    connect(d->m_ui.filterWidget, &QListWidget::currentItemChanged,
            this, [this](QListWidgetItem *) {
        Q_D(QHelpFilterSettingsWidget);
        d->updateCurrentFilter();
    });
    connect(d->m_ui.filterWidget, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem *) {
        Q_D(QHelpFilterSettingsWidget);
        d->renameFilterClicked();
    });

    // TODO: repeat these actions on context menu
    connect(d->m_ui.addButton, &QAbstractButton::clicked, this, [this] {
        Q_D(QHelpFilterSettingsWidget);
        d->addFilterClicked();
    });
    connect(d->m_ui.renameButton, &QAbstractButton::clicked, this, [this] {
        Q_D(QHelpFilterSettingsWidget);
        d->renameFilterClicked();
    });
    connect(d->m_ui.removeButton, &QAbstractButton::clicked, this, [this] {
        Q_D(QHelpFilterSettingsWidget);
        d->removeFilterClicked();
    });

    d->m_ui.componentWidget->setNoOptionText(tr("No Component"));
    d->m_ui.componentWidget->setInvalidOptionText(tr("Invalid Component"));
    d->m_ui.versionWidget->setNoOptionText(tr("No Version"));
    d->m_ui.versionWidget->setInvalidOptionText(tr("Invalid Version"));
}

/*!
    Destroys the filter settings widget.
*/
QHelpFilterSettingsWidget::~QHelpFilterSettingsWidget() = default;

/*!
    Sets the list of all available components to \a components.
    \sa QHelpFilterEngine::availableComponents()
*/
void QHelpFilterSettingsWidget::setAvailableComponents(const QStringList &components)
{
    Q_D(QHelpFilterSettingsWidget);
    d->m_components = components;
    d->updateCurrentFilter();
}

/*!
    Sets the list of all available version numbers to \a versions.
    \sa QHelpFilterEngine::availableVersions()
*/
void QHelpFilterSettingsWidget::setAvailableVersions(const QList<QVersionNumber> &versions)
{
    Q_D(QHelpFilterSettingsWidget);
    d->m_versions = versions;
    d->updateCurrentFilter();
}

/*!
    Reads the filter settings stored inside \a filterEngine and sets up
    this filter settings widget accordingly.
*/
void QHelpFilterSettingsWidget::readSettings(const QHelpFilterEngine *filterEngine)
{
    Q_D(QHelpFilterSettingsWidget);
    const QHelpFilterSettings settings = readSettingsHelper(filterEngine);
    d->setFilterSettings(settings);
}

/*!
    Writes the filter settings, currently presented in this filter settings
    widget, to the \a filterEngine. The old settings stored in the filter
    engine will be overwritten. Returns \c true on success.
*/
bool QHelpFilterSettingsWidget::applySettings(QHelpFilterEngine *filterEngine) const
{
    Q_D(const QHelpFilterSettingsWidget);
    return applySettingsHelper(filterEngine, d->filterSettings());
}

QT_END_NAMESPACE
