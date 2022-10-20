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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtCore/QMap>
#include <QtWidgets/QDialog>
#include <QtHelp/QHelpFilterData>
#include "ui_preferencesdialog.h"
#include "helpdocsettings.h"

QT_BEGIN_NAMESPACE

class FontPanel;
class HelpEngineWrapper;
class QFileSystemWatcher;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent = nullptr);

private slots:
    void okClicked();
    void applyClicked();
    void applyChanges();
    void appFontSettingToggled(bool on);
    void appFontSettingChanged(int index);
    void browserFontSettingToggled(bool on);
    void browserFontSettingChanged(int index);

    void setBlankPage();
    void setCurrentPage();
    void setDefaultPage();

signals:
    void updateBrowserFont();
    void updateApplicationFont();
    void updateUserInterface();

private:
    void updateFontSettingsPage();
    void updateOptionsPage();

    Ui::PreferencesDialogClass m_ui;

    HelpDocSettings m_docSettings;

    FontPanel *m_appFontPanel;
    FontPanel *m_browserFontPanel;
    bool m_appFontChanged;
    bool m_browserFontChanged;
    HelpEngineWrapper &helpEngine;
    const bool m_hideFiltersTab;
    const bool m_hideDocsTab;
    bool m_showTabs;
};

QT_END_NAMESPACE

#endif // SETTINGSDIALOG_H
