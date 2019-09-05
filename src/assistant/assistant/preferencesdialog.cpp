/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "preferencesdialog.h"

#include "centralwidget.h"
#include "filternamedialog.h"
#include "fontpanel.h"
#include "helpenginewrapper.h"
#include "openpagesmanager.h"

#include <QtCore/QVersionNumber>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QMessageBox>

#include <QtHelp/QCompressedHelpInfo>
#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpFilterData>
#include <QtHelp/QHelpFilterEngine>

#include <QtWidgets/QFileDialog>

#include <QtDebug>

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

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , m_appFontChanged(false)
    , m_browserFontChanged(false)
    , helpEngine(HelpEngineWrapper::instance())
    , m_hideFiltersTab(!helpEngine.filterFunctionalityEnabled())
    , m_hideDocsTab(!helpEngine.documentationManagerEnabled())
{
    m_ui.setupUi(this);

    QString resourcePath = QLatin1String(":/qt-project.org/assistant/images/");
#ifdef Q_OS_MACOS
    resourcePath.append(QLatin1String("mac"));
#else
    resourcePath.append(QLatin1String("win"));
#endif

    m_ui.filterAddButton->setIcon(QIcon(resourcePath + QLatin1String("/plus.png")));
    m_ui.filterRemoveButton->setIcon(QIcon(resourcePath + QLatin1String("/minus.png")));

    // TODO: filter docs via lineedit

    connect(m_ui.buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked,
            this, &PreferencesDialog::okClicked);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked,
            this, &PreferencesDialog::applyClicked);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked,
            this, &QDialog::reject);

    m_originalSetup = readOriginalSetup();
    m_currentSetup = m_originalSetup;

    if (m_hideDocsTab) {
        m_ui.tabWidget->removeTab(m_ui.tabWidget->indexOf(m_ui.docsTab));
    } else {
        connect(m_ui.docAddButton, &QAbstractButton::clicked,
                this, &PreferencesDialog::addDocumentation);
        connect(m_ui.docRemoveButton, &QAbstractButton::clicked,
                this, &PreferencesDialog::removeDocumentation);
        connect(m_ui.registeredDocsFilterLineEdit, &QLineEdit::textChanged,
                this, [this](const QString &) {
            for (const auto item : m_namespaceToItem)
                applyDocListFilter(item);
        });
        connect(m_ui.registeredDocsListWidget, &QListWidget::itemSelectionChanged,
                this, [this](){
            m_ui.docRemoveButton->setEnabled(
                        !m_ui.registeredDocsListWidget->selectedItems().isEmpty());
        });

        updateDocumentationPage();
    }

    if (m_hideFiltersTab) {
        m_ui.tabWidget->removeTab(m_ui.tabWidget->indexOf(m_ui.filtersTab));
    } else {
        connect(m_ui.componentWidget, &OptionsWidget::optionSelectionChanged,
                this, &PreferencesDialog::componentsChanged);
        connect(m_ui.versionWidget, &OptionsWidget::optionSelectionChanged,
                this, &PreferencesDialog::versionsChanged);
        connect(m_ui.filterWidget, &QListWidget::currentItemChanged,
                this, &PreferencesDialog::filterSelected);
        connect(m_ui.filterWidget, &QListWidget::itemDoubleClicked,
                this, &PreferencesDialog::renameFilterClicked);

        // TODO: repeat these actions on context menu
        connect(m_ui.filterAddButton, &QAbstractButton::clicked,
                this, &PreferencesDialog::addFilterClicked);
        connect(m_ui.filterRenameButton, &QAbstractButton::clicked,
                this, &PreferencesDialog::renameFilterClicked);
        connect(m_ui.filterRemoveButton, &QAbstractButton::clicked,
                this, &PreferencesDialog::removeFilterClicked);

        m_ui.componentWidget->setNoOptionText(tr("No Component"));
        m_ui.componentWidget->setInvalidOptionText(tr("Invalid Component"));
        m_ui.versionWidget->setNoOptionText(tr("No Version"));
        m_ui.versionWidget->setInvalidOptionText(tr("Invalid Version"));

        updateFilterPage();
    }

    updateFontSettingsPage();
    updateOptionsPage();

    if (helpEngine.usesAppFont())
        setFont(helpEngine.appFont());
}

FilterSetup PreferencesDialog::readOriginalSetup() const
{
    FilterSetup filterSetup;

    filterSetup.m_namespaceToComponent = helpEngine.filterEngine()->namespaceToComponent();
    filterSetup.m_namespaceToVersion = helpEngine.filterEngine()->namespaceToVersion();
    for (auto it = filterSetup.m_namespaceToComponent.constBegin();
         it != filterSetup.m_namespaceToComponent.constEnd(); ++it) {
        const QString namespaceName = it.key();
        const QString namespaceFileName = helpEngine.documentationFileName(namespaceName);
        filterSetup.m_namespaceToFileName.insert(namespaceName, namespaceFileName);
        filterSetup.m_fileNameToNamespace.insert(namespaceFileName, namespaceName);
        filterSetup.m_componentToNamespace[it.value()].append(namespaceName);
    }
    for (auto it = filterSetup.m_namespaceToVersion.constBegin();
         it != filterSetup.m_namespaceToVersion.constEnd(); ++it) {
        filterSetup.m_versionToNamespace[it.value()].append(it.key());
    }

    const QStringList allFilters = helpEngine.filterEngine()->filters();
    for (const QString &filter : allFilters)
        filterSetup.m_filterToData.insert(filter, helpEngine.filterEngine()->filterData(filter));

    filterSetup.m_currentFilter = helpEngine.filterEngine()->activeFilter();

    return filterSetup;
}

void PreferencesDialog::updateFilterPage()
{
    if (m_hideFiltersTab)
        return;

    QString currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        currentFilter = m_currentSetup.m_currentFilter;

    m_currentSetup = m_originalSetup;

    m_ui.filterWidget->clear();
    m_ui.componentWidget->clear();
    m_ui.versionWidget->clear();
    m_itemToFilter.clear();
    m_filterToItem.clear();

    for (const QString &filterName : m_currentSetup.m_filterToData.keys()) {
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

void PreferencesDialog::updateCurrentFilter()
{
    if (m_hideFiltersTab)
        return;

    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());

    const bool filterSelected = !currentFilter.isEmpty();
    m_ui.componentWidget->setEnabled(filterSelected);
    m_ui.versionWidget->setEnabled(filterSelected);
    m_ui.filterRenameButton->setEnabled(filterSelected);
    m_ui.filterRemoveButton->setEnabled(filterSelected);

    m_ui.componentWidget->setOptions(m_currentSetup.m_componentToNamespace.keys(),
            m_currentSetup.m_filterToData.value(currentFilter).components());
    m_ui.versionWidget->setOptions(versionsToStringList(m_currentSetup.m_versionToNamespace.keys()),
            versionsToStringList(m_currentSetup.m_filterToData.value(currentFilter).versions()));
}

void PreferencesDialog::updateDocumentationPage()
{
    if (m_hideDocsTab)
        return;

    m_ui.registeredDocsListWidget->clear();
    m_namespaceToItem.clear();
    m_itemToNamespace.clear();

    for (const QString &namespaceName : m_currentSetup.m_namespaceToFileName.keys()) {
        QListWidgetItem *item = new QListWidgetItem(namespaceName);
        m_namespaceToItem.insert(namespaceName, item);
        m_itemToNamespace.insert(item, namespaceName);
        applyDocListFilter(item);
        m_ui.registeredDocsListWidget->addItem(item);
    }
    m_ui.docRemoveButton->setEnabled(
                !m_ui.registeredDocsListWidget->selectedItems().isEmpty());
}

void PreferencesDialog::applyDocListFilter(QListWidgetItem *item)
{
    const QString namespaceName = m_itemToNamespace.value(item);
    const QString nameFilter = m_ui.registeredDocsFilterLineEdit->text();

    const bool matches = nameFilter.isEmpty() || namespaceName.contains(nameFilter);

    if (!matches)
        item->setSelected(false);
    item->setHidden(!matches);
}

void PreferencesDialog::filterSelected(QListWidgetItem *item)
{
    Q_UNUSED(item)

    updateCurrentFilter();
}

void PreferencesDialog::componentsChanged(const QStringList &components)
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    m_currentSetup.m_filterToData[currentFilter].setComponents(components);
}

void PreferencesDialog::versionsChanged(const QStringList &versions)
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    m_currentSetup.m_filterToData[currentFilter].setVersions(stringListToVersions(versions));
}

QString PreferencesDialog::suggestedNewFilterName(const QString &initialFilterName) const
{
    QString newFilterName = initialFilterName;

    int counter = 1;
    while (m_filterToItem.contains(newFilterName)) {
        newFilterName = initialFilterName + QLatin1Char(' ')
                + QString::number(++counter);
    }

    return newFilterName;
}

QString PreferencesDialog::getUniqueFilterName(const QString &windowTitle,
                                               const QString &initialFilterName)
{
    QString newFilterName = initialFilterName;
    while (1) {
        FilterNameDialog dialog(this);
        dialog.setWindowTitle(windowTitle);
        dialog.setFilterName(newFilterName);
        if (dialog.exec() == QDialog::Rejected)
            return QString();

        newFilterName = dialog.filterName();
        if (!m_filterToItem.contains(newFilterName))
            break;

        if (QMessageBox::warning(this, tr("Filter Exists"),
                                 tr("The filter \"%1\" already exists.")
                                 .arg(newFilterName),
                                 QMessageBox::Retry | QMessageBox::Cancel)
                == QMessageBox::Cancel) {
            return QString();
        }
    }

    return newFilterName;
}

void PreferencesDialog::addFilterClicked()
{
    const QString newFilterName = getUniqueFilterName(tr("Add Filter"),
                                  suggestedNewFilterName(tr("New Filter")));
    if (newFilterName.isEmpty())
        return;

    addFilter(newFilterName);
}

void PreferencesDialog::renameFilterClicked()
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    const QString newFilterName = getUniqueFilterName(tr("Rename Filter"), currentFilter);
    if (newFilterName.isEmpty())
        return;

    const QHelpFilterData oldFilterData = m_currentSetup.m_filterToData.value(currentFilter);
    removeFilter(currentFilter);
    addFilter(newFilterName, oldFilterData);

    if (m_currentSetup.m_currentFilter == currentFilter)
        m_currentSetup.m_currentFilter = newFilterName;
}

void PreferencesDialog::removeFilterClicked()
{
    const QString &currentFilter = m_itemToFilter.value(m_ui.filterWidget->currentItem());
    if (currentFilter.isEmpty())
        return;

    if (QMessageBox::question(this, tr("Remove Filter"),
                              tr("Are you sure you want to remove the \"%1\" filter?")
                              .arg(currentFilter),
                              QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes) {
        return;
    }

    removeFilter(currentFilter);

    if (m_currentSetup.m_currentFilter == currentFilter)
        m_currentSetup.m_currentFilter.clear();
}

void PreferencesDialog::addFilter(const QString &filterName,
                                  const QHelpFilterData &filterData)
{
    QListWidgetItem *item = new QListWidgetItem(filterName);
    m_currentSetup.m_filterToData.insert(filterName, filterData);
    m_filterToItem.insert(filterName, item);
    m_itemToFilter.insert(item, filterName);
    m_ui.filterWidget->insertItem(m_filterToItem.keys().indexOf(filterName), item);

    m_ui.filterWidget->setCurrentItem(item);
    updateCurrentFilter();
}

void PreferencesDialog::removeFilter(const QString &filterName)
{
    QListWidgetItem *item = m_filterToItem.value(filterName);
    m_itemToFilter.remove(item);
    m_filterToItem.remove(filterName);
    delete item;

    m_currentSetup.m_filterToData.remove(filterName);
}

void PreferencesDialog::addDocumentation()
{
    const QStringList &fileNames = QFileDialog::getOpenFileNames(this,
        tr("Add Documentation"), QString(), tr("Qt Compressed Help Files (*.qch)"));
    if (fileNames.isEmpty())
        return;

    bool added = false;

    for (const QString &fileName : fileNames) {
        const QCompressedHelpInfo info = QCompressedHelpInfo::fromCompressedHelpFile(fileName);
        const QString namespaceName = info.namespaceName();

        if (m_currentSetup.m_namespaceToFileName.contains(namespaceName))
            continue;

        if (m_currentSetup.m_fileNameToNamespace.contains(fileName))
            continue;

        const QString component = info.component();
        const QVersionNumber version = info.version();

        m_currentSetup.m_namespaceToFileName.insert(namespaceName, fileName);
        m_currentSetup.m_fileNameToNamespace.insert(fileName, namespaceName);

        m_currentSetup.m_namespaceToComponent.insert(namespaceName, component);
        m_currentSetup.m_componentToNamespace[component].append(namespaceName);

        m_currentSetup.m_namespaceToVersion.insert(namespaceName, version);
        m_currentSetup.m_versionToNamespace[version].append(namespaceName);

        if (!added) {
            added = true;
            m_ui.registeredDocsListWidget->clearSelection();
        }

        QListWidgetItem *item = new QListWidgetItem(namespaceName);
        m_namespaceToItem.insert(namespaceName, item);
        m_itemToNamespace.insert(item, namespaceName);
        m_ui.registeredDocsListWidget->insertItem(m_namespaceToItem.keys().indexOf(namespaceName), item);
        item->setSelected(true);
        applyDocListFilter(item);
    }

    if (added)
        updateCurrentFilter();
}

void PreferencesDialog::removeDocumentation()
{
    const QList<QListWidgetItem *> selectedItems = m_ui.registeredDocsListWidget->selectedItems();
    if (selectedItems.isEmpty())
        return;

    for (QListWidgetItem *item : selectedItems) {
        const QString namespaceName = m_itemToNamespace.value(item);
        m_itemToNamespace.remove(item);
        m_namespaceToItem.remove(namespaceName);
        delete item;

        const QString fileName = m_currentSetup.m_namespaceToFileName.value(namespaceName);
        const QString component = m_currentSetup.m_namespaceToComponent.value(namespaceName);
        const QVersionNumber version = m_currentSetup.m_namespaceToVersion.value(namespaceName);
        m_currentSetup.m_namespaceToComponent.remove(namespaceName);
        m_currentSetup.m_namespaceToVersion.remove(namespaceName);
        m_currentSetup.m_namespaceToFileName.remove(namespaceName);
        m_currentSetup.m_fileNameToNamespace.remove(fileName);
        m_currentSetup.m_componentToNamespace[component].removeOne(namespaceName);
        if (m_currentSetup.m_componentToNamespace[component].isEmpty())
            m_currentSetup.m_componentToNamespace.remove(component);
        m_currentSetup.m_versionToNamespace[version].removeOne(namespaceName);
        if (m_currentSetup.m_versionToNamespace[version].isEmpty())
            m_currentSetup.m_versionToNamespace.remove(version);
    }

    updateCurrentFilter();
}

void PreferencesDialog::okClicked()
{
    applyChanges();
    accept();
}

void PreferencesDialog::applyClicked()
{
    applyChanges();
    m_originalSetup = readOriginalSetup();
    m_currentSetup = m_originalSetup;
    updateDocumentationPage();
    updateFilterPage();
}

template <class T>
static QMap<QString, T> subtract(const QMap<QString, T> &minuend,
                                 const QMap<QString, T> &subtrahend)
{
    QMap<QString, T> result = minuend;

    for (auto itSubtrahend = subtrahend.cbegin(); itSubtrahend != subtrahend.cend(); ++itSubtrahend) {
        auto itResult = result.find(itSubtrahend.key());
        if (itResult != result.end() && itSubtrahend.value() == itResult.value())
            result.erase(itResult);
    }

    return result;
}

void PreferencesDialog::applyChanges()
{
    bool changed = false;

    const QMap<QString, QString> docsToRemove = subtract(
                m_originalSetup.m_namespaceToFileName,
                m_currentSetup.m_namespaceToFileName);
    const QMap<QString, QString> docsToAdd = subtract(
                m_currentSetup.m_namespaceToFileName,
                m_originalSetup.m_namespaceToFileName);

    for (const QString &namespaceName : docsToRemove.keys()) {
        if (!helpEngine.unregisterDocumentation(namespaceName))
            qWarning() << "Cannot unregister documentation:" << namespaceName;
        changed = true;
    }

    for (const QString &fileName : docsToAdd.values()) {
        if (!helpEngine.registerDocumentation(fileName))
            qWarning() << "Cannot register documentation file:" << fileName;
        changed = true;
    }

    const QMap<QString, QHelpFilterData> filtersToRemove = subtract(
                m_originalSetup.m_filterToData,
                m_currentSetup.m_filterToData);
    const QMap<QString, QHelpFilterData> filtersToAdd = subtract(
                m_currentSetup.m_filterToData,
                m_originalSetup.m_filterToData);

    const QString &currentFilter = helpEngine.filterEngine()->activeFilter();

    for (const QString &filter : filtersToRemove.keys()) {
        helpEngine.filterEngine()->removeFilter(filter);
        if (currentFilter == filter && !filtersToAdd.contains(filter))
            helpEngine.filterEngine()->setActiveFilter(QString());
        changed = true;
    }

    for (auto it = filtersToAdd.cbegin(); it != filtersToAdd.cend(); ++it) {
        helpEngine.filterEngine()->setFilterData(it.key(), it.value());
        changed = true;
    }

    if (changed) {
        helpEngine.filterEngine()->setActiveFilter(m_currentSetup.m_currentFilter);

        // In order to update the filtercombobox and indexwidget
        // according to the new filter configuration.
        helpEngine.setupData();
    }

    helpEngine.setShowTabs(m_ui.showTabs->isChecked());
    if (m_showTabs != m_ui.showTabs->isChecked())
        emit updateUserInterface();

    if (m_appFontChanged) {
        helpEngine.setAppFont(m_appFontPanel->selectedFont());
        helpEngine.setUseAppFont(m_appFontPanel->isChecked());
        helpEngine.setAppWritingSystem(m_appFontPanel->writingSystem());
        emit updateApplicationFont();
        m_appFontChanged = false;
    }

    if (m_browserFontChanged) {
        helpEngine.setBrowserFont(m_browserFontPanel->selectedFont());
        helpEngine.setUseBrowserFont(m_browserFontPanel->isChecked());
        helpEngine.setBrowserWritingSystem(m_browserFontPanel->writingSystem());
        emit updateBrowserFont();
        m_browserFontChanged = false;
    }

    QString homePage = m_ui.homePageLineEdit->text();
    if (homePage.isEmpty())
        homePage = QLatin1String("help");
    helpEngine.setHomePage(homePage);

    const int option = m_ui.helpStartComboBox->currentIndex();
    helpEngine.setStartOption(option);
}

void PreferencesDialog::updateFontSettingsPage()
{
    m_browserFontPanel = new FontPanel(this);
    m_browserFontPanel->setCheckable(true);
    m_ui.stackedWidget_2->insertWidget(0, m_browserFontPanel);

    m_appFontPanel = new FontPanel(this);
    m_appFontPanel->setCheckable(true);
    m_ui.stackedWidget_2->insertWidget(1, m_appFontPanel);

    m_ui.stackedWidget_2->setCurrentIndex(0);

    const QString customSettings(tr("Use custom settings"));
    m_appFontPanel->setTitle(customSettings);

    QFont font = helpEngine.appFont();
    m_appFontPanel->setSelectedFont(font);

    QFontDatabase::WritingSystem system = helpEngine.appWritingSystem();
    m_appFontPanel->setWritingSystem(system);

    m_appFontPanel->setChecked(helpEngine.usesAppFont());

    m_browserFontPanel->setTitle(customSettings);

    font = helpEngine.browserFont();
    m_browserFontPanel->setSelectedFont(font);

    system = helpEngine.browserWritingSystem();
    m_browserFontPanel->setWritingSystem(system);

    m_browserFontPanel->setChecked(helpEngine.usesBrowserFont());

    connect(m_appFontPanel, &QGroupBox::toggled,
            this, &PreferencesDialog::appFontSettingToggled);
    connect(m_browserFontPanel, &QGroupBox::toggled,
            this, &PreferencesDialog::browserFontSettingToggled);

    const QList<QComboBox*> &appCombos = m_appFontPanel->findChildren<QComboBox*>();
    for (QComboBox* box : appCombos) {
        connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &PreferencesDialog::appFontSettingChanged);
    }

    const QList<QComboBox*> &browserCombos = m_browserFontPanel->findChildren<QComboBox*>();
    for (QComboBox* box : browserCombos) {
        connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &PreferencesDialog::browserFontSettingChanged);
    }
}

void PreferencesDialog::appFontSettingToggled(bool on)
{
    Q_UNUSED(on);
    m_appFontChanged = true;
}

void PreferencesDialog::appFontSettingChanged(int index)
{
    Q_UNUSED(index);
    m_appFontChanged = true;
}

void PreferencesDialog::browserFontSettingToggled(bool on)
{
    Q_UNUSED(on);
    m_browserFontChanged = true;
}

void PreferencesDialog::browserFontSettingChanged(int index)
{
    Q_UNUSED(index);
    m_browserFontChanged = true;
}

void PreferencesDialog::updateOptionsPage()
{
    m_ui.homePageLineEdit->setText(helpEngine.homePage());

    int option = helpEngine.startOption();
    m_ui.helpStartComboBox->setCurrentIndex(option);

    m_showTabs = helpEngine.showTabs();
    m_ui.showTabs->setChecked(m_showTabs);

    connect(m_ui.blankPageButton, &QAbstractButton::clicked,
            this, &PreferencesDialog::setBlankPage);
    connect(m_ui.currentPageButton, &QAbstractButton::clicked,
            this, &PreferencesDialog::setCurrentPage);
    connect(m_ui.defaultPageButton, &QAbstractButton::clicked,
            this, &PreferencesDialog::setDefaultPage);
}

void PreferencesDialog::setBlankPage()
{
    m_ui.homePageLineEdit->setText(QLatin1String("about:blank"));
}

void PreferencesDialog::setCurrentPage()
{
    QString homepage = CentralWidget::instance()->currentSource().toString();
    if (homepage.isEmpty())
        homepage = QLatin1String("help");

    m_ui.homePageLineEdit->setText(homepage);
}

void PreferencesDialog::setDefaultPage()
{
    m_ui.homePageLineEdit->setText(helpEngine.defaultHomePage());
}

QT_END_NAMESPACE
