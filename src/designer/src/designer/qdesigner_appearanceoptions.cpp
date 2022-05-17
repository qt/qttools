// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qdesigner_appearanceoptions.h"
#include "ui_qdesigner_appearanceoptions.h"

#include "qdesigner_settings.h"
#include "qdesigner_toolwindow.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtCore/qtimer.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

// ---------------- AppearanceOptions
bool AppearanceOptions::equals(const AppearanceOptions &rhs) const
{
    return uiMode == rhs.uiMode && toolWindowFontSettings == rhs.toolWindowFontSettings;
}

void AppearanceOptions::toSettings(QDesignerSettings &settings) const
{
    settings.setUiMode(uiMode);
    settings.setToolWindowFont(toolWindowFontSettings);
}

void AppearanceOptions::fromSettings(const QDesignerSettings &settings)
{
    uiMode = settings.uiMode();
    toolWindowFontSettings = settings.toolWindowFont();
}

// ---------------- QDesignerAppearanceOptionsWidget
QDesignerAppearanceOptionsWidget::QDesignerAppearanceOptionsWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::AppearanceOptionsWidget)
{
    m_ui->setupUi(this);

    m_ui->m_uiModeCombo->addItem(tr("Docked Window"), QVariant(DockedMode));
    m_ui->m_uiModeCombo->addItem(tr("Multiple Top-Level Windows"), QVariant(TopLevelMode));
    connect(m_ui->m_uiModeCombo, &QComboBox::currentIndexChanged,
            this, &QDesignerAppearanceOptionsWidget::slotUiModeComboChanged);

    m_ui->m_fontPanel->setCheckable(true);
    m_ui->m_fontPanel->setTitle(tr("Toolwindow Font"));

}

QDesignerAppearanceOptionsWidget::~QDesignerAppearanceOptionsWidget()
{
    delete m_ui;
}

UIMode QDesignerAppearanceOptionsWidget::uiMode() const
{
    return static_cast<UIMode>(m_ui->m_uiModeCombo->itemData(m_ui->m_uiModeCombo->currentIndex()).toInt());
}

AppearanceOptions QDesignerAppearanceOptionsWidget::appearanceOptions() const
{
    AppearanceOptions rc;
    rc.uiMode = uiMode();
    rc.toolWindowFontSettings.m_font = m_ui->m_fontPanel->selectedFont();
    rc.toolWindowFontSettings.m_useFont = m_ui->m_fontPanel->isChecked();
    rc.toolWindowFontSettings.m_writingSystem = m_ui->m_fontPanel->writingSystem();
    return rc;
}

void QDesignerAppearanceOptionsWidget::setAppearanceOptions(const AppearanceOptions &ao)
{
    m_initialUIMode = ao.uiMode;
    m_ui->m_uiModeCombo->setCurrentIndex(m_ui->m_uiModeCombo->findData(QVariant(ao.uiMode)));
    m_ui->m_fontPanel->setWritingSystem(ao.toolWindowFontSettings.m_writingSystem);
    m_ui->m_fontPanel->setSelectedFont(ao.toolWindowFontSettings.m_font);
    m_ui->m_fontPanel->setChecked(ao.toolWindowFontSettings.m_useFont);
}

void QDesignerAppearanceOptionsWidget::slotUiModeComboChanged()
{
    emit uiModeChanged(m_initialUIMode != uiMode());
}

// ----------- QDesignerAppearanceOptionsPage
QDesignerAppearanceOptionsPage::QDesignerAppearanceOptionsPage(QDesignerFormEditorInterface *core) :
    m_core(core)
{
}

QString QDesignerAppearanceOptionsPage::name() const
{
    //: Tab in preferences dialog
    return QCoreApplication::translate("QDesignerAppearanceOptionsPage", "Appearance");
}

QWidget *QDesignerAppearanceOptionsPage::createPage(QWidget *parent)
{
    m_widget = new QDesignerAppearanceOptionsWidget(parent);
    m_initialOptions.fromSettings(QDesignerSettings(m_core));
    m_widget->setAppearanceOptions(m_initialOptions);
    return m_widget;
}

void QDesignerAppearanceOptionsPage::apply()
{
    if (m_widget) {
        const AppearanceOptions newOptions = m_widget->appearanceOptions();
        if (newOptions != m_initialOptions) {
            QDesignerSettings settings(m_core);
            newOptions.toSettings(settings);
            m_initialOptions = newOptions;
            emit settingsChanged();
        }
    }
}

void QDesignerAppearanceOptionsPage::finish()
{
}

QT_END_NAMESPACE

