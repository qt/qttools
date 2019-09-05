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

#include <QtCore/QVersionNumber>
#include <QtWidgets/QDialog>
#include <QtHelp/QHelpFilterData>
#include "ui_preferencesdialog.h"

QT_BEGIN_NAMESPACE

class FontPanel;
class HelpEngineWrapper;
class QFileSystemWatcher;
class QVersionNumber;

struct FilterSetup {
    QMap<QString, QString>            m_namespaceToComponent;
    QMap<QString, QStringList>        m_componentToNamespace;

    QMap<QString, QVersionNumber>     m_namespaceToVersion;
    QMap<QVersionNumber, QStringList> m_versionToNamespace;

    QMap<QString, QString>            m_namespaceToFileName;
    QMap<QString, QString>            m_fileNameToNamespace;

    QMap<QString, QHelpFilterData>    m_filterToData;
    QString                           m_currentFilter;
};

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent = nullptr);

private slots:
    void filterSelected(QListWidgetItem *item);
    void componentsChanged(const QStringList &components);
    void versionsChanged(const QStringList &versions);
    void addFilterClicked();
    void renameFilterClicked();
    void removeFilterClicked();
    void addFilter(const QString &filterName,
                   const QHelpFilterData &filterData = QHelpFilterData());
    void removeFilter(const QString &filterName);
    void addDocumentation();
    void removeDocumentation();
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
    QString suggestedNewFilterName(const QString &initialFilterName) const;
    QString getUniqueFilterName(const QString &windowTitle,
                                const QString &initialFilterName = QString());
    void applyDocListFilter(QListWidgetItem *item);

    void updateFilterPage();
    void updateCurrentFilter();
    void updateDocumentationPage();
    void updateFontSettingsPage();
    void updateOptionsPage();
    FilterSetup readOriginalSetup() const;

    Ui::PreferencesDialogClass m_ui;

    FilterSetup m_originalSetup;
    FilterSetup m_currentSetup;

    QMap<QString, QListWidgetItem *> m_namespaceToItem;
    QHash<QListWidgetItem *, QString> m_itemToNamespace;

    QMap<QString, QListWidgetItem *> m_filterToItem;
    QHash<QListWidgetItem *, QString> m_itemToFilter;

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
