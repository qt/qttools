// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#include "preferencesdialog.h"

#include "centralwidget.h"
#include "fontpanel_p.h"
#include "helpenginewrapper.h"
#include "openpagesmanager.h"
#include "helpdocsettingswidget.h"

#include <QtCore/QVersionNumber>

#include <QtGui/QFontDatabase>

#include <QtHelp/QHelpEngineCore>
#include <QtHelp/QHelpFilterData>
#include <QtHelp/QHelpFilterEngine>
#include <QtHelp/QHelpFilterSettingsWidget>

#include <QtDebug>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , m_appFontChanged(false)
    , m_browserFontChanged(false)
    , helpEngine(HelpEngineWrapper::instance())
    , m_hideFiltersTab(!helpEngine.filterFunctionalityEnabled())
    , m_hideDocsTab(!helpEngine.documentationManagerEnabled())
{
    m_ui.setupUi(this);

    connect(m_ui.buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked,
            this, &PreferencesDialog::okClicked);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked,
            this, &PreferencesDialog::applyClicked);
    connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked,
            this, &QDialog::reject);

    m_docSettings = HelpDocSettings::readSettings(helpEngine.helpEngine());

    if (m_hideDocsTab) {
        m_ui.tabWidget->removeTab(m_ui.tabWidget->indexOf(m_ui.docsTab));
    } else {
        connect(m_ui.docSettingsWidget, &HelpDocSettingsWidget::docSettingsChanged, this,
                [this](const HelpDocSettings &settings) {
            m_docSettings = settings;
            if (m_hideFiltersTab)
                return;

            m_ui.filterSettingsWidget->setAvailableComponents(m_docSettings.components());
            m_ui.filterSettingsWidget->setAvailableVersions(m_docSettings.versions());
        });

        m_ui.docSettingsWidget->setDocSettings(m_docSettings);
    }

    if (m_hideFiltersTab) {
        m_ui.tabWidget->removeTab(m_ui.tabWidget->indexOf(m_ui.filtersTab));
    } else {
        m_ui.filterSettingsWidget->setAvailableComponents(m_docSettings.components());
        m_ui.filterSettingsWidget->setAvailableVersions(m_docSettings.versions());
        m_ui.filterSettingsWidget->readSettings(helpEngine.filterEngine());
    }

    updateFontSettingsPage();
    updateOptionsPage();

    if (helpEngine.usesAppFont())
        setFont(helpEngine.appFont());
}

void PreferencesDialog::okClicked()
{
    applyChanges();
    accept();
}

void PreferencesDialog::applyClicked()
{
    applyChanges();

    m_docSettings = HelpDocSettings::readSettings(helpEngine.helpEngine());

    if (!m_hideDocsTab)
        m_ui.docSettingsWidget->setDocSettings(m_docSettings);
    if (!m_hideFiltersTab) {
        m_ui.filterSettingsWidget->setAvailableComponents(m_docSettings.components());
        m_ui.filterSettingsWidget->setAvailableVersions(m_docSettings.versions());
        m_ui.filterSettingsWidget->readSettings(helpEngine.filterEngine());
    }
}

void PreferencesDialog::applyChanges()
{
    bool changed = false;
    if (!m_hideDocsTab)
        changed = HelpDocSettings::applySettings(helpEngine.helpEngine(), m_docSettings);
    if (!m_hideFiltersTab)
        changed = changed || m_ui.filterSettingsWidget->applySettings(helpEngine.filterEngine());

    if (changed) {
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
        homePage = "help"_L1;
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
        connect(box, &QComboBox::currentIndexChanged,
                this, &PreferencesDialog::appFontSettingChanged);
    }

    const QList<QComboBox*> &browserCombos = m_browserFontPanel->findChildren<QComboBox*>();
    for (QComboBox* box : browserCombos) {
        connect(box, &QComboBox::currentIndexChanged,
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
    m_ui.homePageLineEdit->setText("about:blank"_L1);
}

void PreferencesDialog::setCurrentPage()
{
    QString homepage = CentralWidget::instance()->currentSource().toString();
    if (homepage.isEmpty())
        homepage = "help"_L1;

    m_ui.homePageLineEdit->setText(homepage);
}

void PreferencesDialog::setDefaultPage()
{
    m_ui.homePageLineEdit->setText(helpEngine.defaultHomePage());
}

QT_END_NAMESPACE
