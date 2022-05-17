// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "formeditor_optionspage.h"

// shared
#include "formwindowbase_p.h"
#include "gridpanel_p.h"
#include "grid_p.h"
#include "previewconfigurationwidget_p.h"
#include "shared_settings_p.h"
#include "zoomwidget_p.h"
#include <private/actioneditor_p.h>

// SDK
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>

#include <QtCore/qstring.h>
#include <QtCore/qcoreapplication.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qformlayout.h>
#include <QtWidgets/qcombobox.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// Zoom, currently for preview only
class ZoomSettingsWidget : public QGroupBox {
    Q_DISABLE_COPY_MOVE(ZoomSettingsWidget)
public:
    explicit ZoomSettingsWidget(QWidget *parent = nullptr);

    void fromSettings(const QDesignerSharedSettings &s);
    void toSettings(QDesignerSharedSettings &s) const;

private:
    QComboBox *m_zoomCombo;
};

ZoomSettingsWidget::ZoomSettingsWidget(QWidget *parent) :
    QGroupBox(parent),
    m_zoomCombo(new QComboBox)
{
    m_zoomCombo->setEditable(false);
    const QList<int> &zoomValues = ZoomMenu::zoomValues();
    for (int z : zoomValues) {
        //: Zoom percentage
        m_zoomCombo->addItem(QCoreApplication::translate("FormEditorOptionsPage", "%1 %").arg(z), QVariant(z));
    }

    // Layout
    setCheckable(true);
    setTitle(QCoreApplication::translate("FormEditorOptionsPage", "Preview Zoom"));
    QFormLayout *lt = new QFormLayout;
    lt->addRow(QCoreApplication::translate("FormEditorOptionsPage", "Default Zoom"), m_zoomCombo);
    setLayout(lt);
}

void ZoomSettingsWidget::fromSettings(const QDesignerSharedSettings &s)
{
    setChecked(s.zoomEnabled());
    const int idx = m_zoomCombo->findData(QVariant(s.zoom()));
    m_zoomCombo->setCurrentIndex(qMax(0, idx));
}

void ZoomSettingsWidget::toSettings(QDesignerSharedSettings &s) const
{
    s.setZoomEnabled(isChecked());
    const int zoom = m_zoomCombo->itemData(m_zoomCombo->currentIndex()).toInt();
    s.setZoom(zoom);
}



// FormEditorOptionsPage:
FormEditorOptionsPage::FormEditorOptionsPage(QDesignerFormEditorInterface *core)
        : m_core(core)
{
}

QString FormEditorOptionsPage::name() const
{
    //: Tab in preferences dialog
    return QCoreApplication::translate("FormEditorOptionsPage", "Forms");
}

QWidget *FormEditorOptionsPage::createPage(QWidget *parent)
{
    QWidget *optionsWidget = new QWidget(parent);

    const QDesignerSharedSettings settings(m_core);
    m_previewConf = new PreviewConfigurationWidget(m_core);
    m_zoomSettingsWidget = new ZoomSettingsWidget;
    m_zoomSettingsWidget->fromSettings(settings);

    m_defaultGridConf = new GridPanel();
    m_defaultGridConf->setTitle(QCoreApplication::translate("FormEditorOptionsPage", "Default Grid"));
    m_defaultGridConf->setGrid(settings.defaultGrid());

    const QString namingTitle =
        QCoreApplication::translate("FormEditorOptionsPage", "Object Naming Convention");
    QGroupBox *namingGroupBox = new QGroupBox(namingTitle);
    const QString namingToolTip =
        QCoreApplication::translate("FormEditorOptionsPage",
                                    "Naming convention used for generating action object names from their text");
    namingGroupBox->setToolTip(namingToolTip);
    QHBoxLayout *namingHLayout = new QHBoxLayout(namingGroupBox);
    m_namingComboBox = new QComboBox;
    m_namingComboBox->setToolTip(namingToolTip);
    QStringList items; // matching ActionEditor::NamingMode
    items << QCoreApplication::translate("FormEditorOptionsPage", "Camel Case")
        << QCoreApplication::translate("FormEditorOptionsPage", "Underscore");
    m_namingComboBox->addItems(items);
    m_namingComboBox->setCurrentIndex(settings.objectNamingMode());
    namingHLayout->addWidget(m_namingComboBox.data());

    QVBoxLayout *optionsVLayout = new QVBoxLayout();
    optionsVLayout->addWidget(m_defaultGridConf);
    optionsVLayout->addWidget(m_previewConf);
    optionsVLayout->addWidget(m_zoomSettingsWidget);
    optionsVLayout->addWidget(namingGroupBox);
    optionsVLayout->addStretch(1);

    // Outer layout to give it horizontal stretch
    QHBoxLayout *optionsHLayout = new QHBoxLayout();
    optionsHLayout->addLayout(optionsVLayout);
    optionsHLayout->addStretch(1);
    optionsWidget->setLayout(optionsHLayout);

    return optionsWidget;
}

void FormEditorOptionsPage::apply()
{
    QDesignerSharedSettings settings(m_core);
    if (m_defaultGridConf) {
        const Grid defaultGrid = m_defaultGridConf->grid();
        settings.setDefaultGrid(defaultGrid);

        FormWindowBase::setDefaultDesignerGrid(defaultGrid);
        // Update grid settings in all existing form windows
        QDesignerFormWindowManagerInterface *fwm = m_core->formWindowManager();
        if (const int numWindows = fwm->formWindowCount()) {
            for (int i = 0; i < numWindows; i++)
                if (qdesigner_internal::FormWindowBase *fwb
                        = qobject_cast<qdesigner_internal::FormWindowBase *>( fwm->formWindow(i)))
                    if (!fwb->hasFormGrid())
                        fwb->setDesignerGrid(defaultGrid);
        }
    }
    if (m_previewConf) {
        m_previewConf->saveState();
    }

    if (m_zoomSettingsWidget)
        m_zoomSettingsWidget->toSettings(settings);

    if (m_namingComboBox) {
        const ObjectNamingMode namingMode
            = static_cast<ObjectNamingMode>(m_namingComboBox->currentIndex());
        settings.setObjectNamingMode(namingMode);
        ActionEditor::setObjectNamingMode(namingMode);
    }
}

void FormEditorOptionsPage::finish()
{
}

}

QT_END_NAMESPACE
