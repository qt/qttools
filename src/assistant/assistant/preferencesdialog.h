// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
