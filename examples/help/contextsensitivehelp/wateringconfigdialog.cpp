// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "wateringconfigdialog.h"

WateringConfigDialog::WateringConfigDialog()
{
    m_ui.setupUi(this);
    m_widgetInfo.insert(m_ui.plantComboBox, tr("plants"));
    m_widgetInfo.insert(m_ui.temperatureCheckBox, tr("temperature"));
    m_widgetInfo.insert(m_ui.temperatureSpinBox, tr("temperature"));
    m_widgetInfo.insert(m_ui.rainCheckBox, tr("rain"));
    m_widgetInfo.insert(m_ui.rainSpinBox, tr("rain"));
    m_widgetInfo.insert(m_ui.startTimeEdit, tr("starting time"));
    m_widgetInfo.insert(m_ui.amountSpinBox, tr("water amount"));
    m_widgetInfo.insert(m_ui.sourceComboBox, tr("water source"));
    m_widgetInfo.insert(m_ui.filterCheckBox, tr("water filtering"));

    connect(qApp, &QApplication::focusChanged,
        this, &WateringConfigDialog::focusChanged);
}

void WateringConfigDialog::focusChanged(QWidget *, QWidget *now)
{
    if (m_widgetInfo.contains(now)) {
        m_ui.helpLabel->setText(tr("Information about %1:").arg(m_widgetInfo.value(now)));
        QStringList lst = m_widgetInfo.value(now).split(QLatin1Char(' '));
        m_ui.helpBrowser->showHelpForKeyword(lst.last());
    }
}

