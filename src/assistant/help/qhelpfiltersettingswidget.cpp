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

#include "qhelpfilterdata.h"
#include "qhelpfiltersettings_p.h"
#include "qhelpfiltersettingswidget.h"
#include "ui_qhelpfiltersettingswidget.h"
#include "qfilternamedialog_p.h"

#include <QtWidgets/QMessageBox>
#include <QtCore/QVersionNumber>

QT_BEGIN_NAMESPACE

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
    Q_DECLARE_PUBLIC(QHelpFilterSettingsWidget)
public:
    QHelpFilterSettingsWidgetPrivate() = default;

    QHelpFilterSettings filterSettings() const;
    void setFilterSettings(const QHelpFilterSettings &settings);

    void updateCurrentFilter();
    void componentsChanged(const QStringList &components);
    void versionsChanged(const QStringList &versions);
    void addFilterClicked();
    void renameFilterClicked();
    void removeFilterClicked();
    void addFilter(const QString &filterName,
                   const QHelpFilterData &filterData = QHelpFilterData());
    void removeFilter(const QString &filterName);
    QString getUniqueFilterName(const QString &windowTitle,
                                const QString &initialFilterName);
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

    for (const QString &filterName : m_filterSettings.filterNames()) {
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

QHelpFilterSettings QHelpFilterSettingsWidgetPrivate::filterSettings() const
{
    return m_filterSettings;
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
                              .arg(currentFilter),
                              QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes) {
        return;
    }

    removeFilter(currentFilter);

    if (m_filterSettings.currentFilter() == currentFilter)
        m_filterSettings.setCurrentFilter(QString());
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
    while (1) {
        QFilterNameDialog dialog(q);
        dialog.setWindowTitle(windowTitle);
        dialog.setFilterName(newFilterName);
        if (dialog.exec() == QDialog::Rejected)
            return QString();

        newFilterName = dialog.filterName();
        if (!m_filterToItem.contains(newFilterName))
            break;

        if (QMessageBox::warning(q, QHelpFilterSettingsWidget::tr("Filter Exists"),
                                 QHelpFilterSettingsWidget::tr("The filter \"%1\" already exists.")
                                 .arg(newFilterName),
                                 QMessageBox::Retry | QMessageBox::Cancel)
                == QMessageBox::Cancel) {
            return QString();
        }
    }

    return newFilterName;
}

QString QHelpFilterSettingsWidgetPrivate::suggestedNewFilterName(const QString &initialFilterName) const
{
    QString newFilterName = initialFilterName;

    int counter = 1;
    while (m_filterToItem.contains(newFilterName)) {
        newFilterName = initialFilterName + QLatin1Char(' ')
                + QString::number(++counter);
    }

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
    QString resourcePath = QLatin1String(":/qt-project.org/assistant/images/");
#ifdef Q_OS_MACOS
    resourcePath.append(QLatin1String("mac"));
#else
    resourcePath.append(QLatin1String("win"));
#endif
    d->m_ui.addButton->setIcon(QIcon(resourcePath + QLatin1String("/plus.png")));
    d->m_ui.removeButton->setIcon(QIcon(resourcePath + QLatin1String("/minus.png")));

    connect(d->m_ui.componentWidget, &QOptionsWidget::optionSelectionChanged,
            [this](const QStringList &options) {
        Q_D(QHelpFilterSettingsWidget);
        d->componentsChanged(options);
    });
    connect(d->m_ui.versionWidget, &QOptionsWidget::optionSelectionChanged,
            [this](const QStringList &options) {
        Q_D(QHelpFilterSettingsWidget);
        d->versionsChanged(options);
    });
    connect(d->m_ui.filterWidget, &QListWidget::currentItemChanged,
            this, [this](QListWidgetItem *) {
        Q_D(QHelpFilterSettingsWidget);
        d->updateCurrentFilter();
    });
    connect(d->m_ui.filterWidget, &QListWidget::itemDoubleClicked,
            [this](QListWidgetItem *) {
        Q_D(QHelpFilterSettingsWidget);
        d->renameFilterClicked();
    });

    // TODO: repeat these actions on context menu
    connect(d->m_ui.addButton, &QAbstractButton::clicked,
            [this]() {
        Q_D(QHelpFilterSettingsWidget);
        d->addFilterClicked();
    });
    connect(d->m_ui.renameButton, &QAbstractButton::clicked,
            [this]() {
        Q_D(QHelpFilterSettingsWidget);
        d->renameFilterClicked();
    });
    connect(d->m_ui.removeButton, &QAbstractButton::clicked,
            [this]() {
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
    const QHelpFilterSettings settings = QHelpFilterSettings::readSettings(filterEngine);
    d->setFilterSettings(settings);
}

/*!
    Writes the filter settings, currently presented in this filter settings
    widget, to the \a filterEngine. The old settings stored in the filter
    engine will be overwritten.
*/
bool QHelpFilterSettingsWidget::applySettings(QHelpFilterEngine *filterEngine) const
{
    Q_D(const QHelpFilterSettingsWidget);
    return QHelpFilterSettings::applySettings(filterEngine, d->filterSettings());
}

QT_END_NAMESPACE
