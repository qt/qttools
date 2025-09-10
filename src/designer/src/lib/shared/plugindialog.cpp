// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


#include "plugindialog_p.h"
#include "pluginmanager_p.h"
#include "iconloader_p.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/abstractwidgetdatabase.h>

#include <QtUiPlugin/customwidget.h>

#include <QtWidgets/qaction.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qpushbutton.h>

#if QT_CONFIG(clipboard)
#  include <QtGui/QClipboard>
#endif

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpluginloader.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

enum { ErrorItemRole = Qt::UserRole + 1 };

namespace qdesigner_internal {

PluginDialog::PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent)
    : QDialog(parent
#ifdef Q_OS_MACOS
            , Qt::Tool
#endif
            ), m_core(core)
{
    ui.setupUi(this);

    ui.message->hide();

    const QStringList headerLabels(tr("Components"));

    ui.treeWidget->setAlternatingRowColors(false);
    ui.treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui.treeWidget->setHeaderLabels(headerLabels);
    ui.treeWidget->header()->hide();
    ui.treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.treeWidget, &QWidget::customContextMenuRequested,
            this, &PluginDialog::treeWidgetContextMenu);

    interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirOpenIcon),
                            QIcon::Normal, QIcon::On);
    interfaceIcon.addPixmap(style()->standardPixmap(QStyle::SP_DirClosedIcon),
                            QIcon::Normal, QIcon::Off);
    featureIcon.addPixmap(style()->standardPixmap(QStyle::SP_FileIcon));

    setWindowTitle(tr("Plugin Information"));
    populateTreeWidget();

    QPushButton *updateButton = new QPushButton(tr("Refresh"));
    const QString tooltip = tr("Scan for newly installed custom widget plugins.");
    updateButton->setToolTip(tooltip);
    updateButton->setWhatsThis(tooltip);
    connect(updateButton, &QAbstractButton::clicked, this, &PluginDialog::updateCustomWidgetPlugins);
    ui.buttonBox->addButton(updateButton, QDialogButtonBox::ActionRole);

}

void PluginDialog::populateTreeWidget()
{
    ui.treeWidget->clear();
    QDesignerPluginManager *pluginManager = m_core->pluginManager();
    const QStringList fileNames = pluginManager->registeredPlugins();

    if (!fileNames.isEmpty()) {
        QTreeWidgetItem *topLevelItem = setTopLevelItem(tr("Loaded Plugins"));
        QFont boldFont = topLevelItem->font(0);

        for (const QString &fileName : fileNames) {
            QPluginLoader loader(fileName);
            const QFileInfo fileInfo(fileName);

            QTreeWidgetItem *pluginItem = setPluginItem(topLevelItem, fileInfo, boldFont);

            if (QObject *plugin = loader.instance()) {
                if (const QDesignerCustomWidgetCollectionInterface *c = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(plugin)) {
                    const auto &collCustomWidgets = c->customWidgets();
                    for (const QDesignerCustomWidgetInterface *p : collCustomWidgets)
                        setItem(pluginItem, p->name(), p->toolTip(), p->whatsThis(), p->icon());
                } else {
                    if (const QDesignerCustomWidgetInterface *p = qobject_cast<QDesignerCustomWidgetInterface*>(plugin))
                        setItem(pluginItem, p->name(), p->toolTip(), p->whatsThis(), p->icon());
                }
            }
        }
    }

    const QStringList notLoadedPlugins = pluginManager->failedPlugins();
    if (!notLoadedPlugins.isEmpty()) {
        QTreeWidgetItem *topLevelItem = setTopLevelItem(tr("Failed Plugins"));
        const QFont boldFont = topLevelItem->font(0);
        for (const QString &plugin : notLoadedPlugins) {
            const QString failureReason = pluginManager->failureReason(plugin);
            const QString htmlFailureReason = "<html><head/><body><p>"_L1
                + failureReason.toHtmlEscaped()
                + "</p></body></html>"_L1;
            QTreeWidgetItem *pluginItem = setPluginItem(topLevelItem, QFileInfo(plugin), boldFont);
            auto errorItem = setItem(pluginItem, failureReason,
                                     htmlFailureReason, QString(), QIcon());
            errorItem->setData(0, ErrorItemRole, QVariant(true));
        }
    }

    if (ui.treeWidget->topLevelItemCount() == 0) {
        ui.label->setText(tr("Qt Widgets Designer couldn't find any plugins"));
        ui.treeWidget->hide();
    } else {
        ui.label->setText(tr("Qt Widgets Designer found the following plugins"));
    }
}

QTreeWidgetItem* PluginDialog::setTopLevelItem(const QString &itemName)
{
    QTreeWidgetItem *topLevelItem = new QTreeWidgetItem(ui.treeWidget);
    topLevelItem->setText(0, itemName);
    topLevelItem->setExpanded(true);
    topLevelItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));

    QFont boldFont = topLevelItem->font(0);
    boldFont.setBold(true);
    topLevelItem->setFont(0, boldFont);

    return topLevelItem;
}

QTreeWidgetItem* PluginDialog::setPluginItem(QTreeWidgetItem *topLevelItem,
                                             const QFileInfo &file, const QFont &font)
{
    QTreeWidgetItem *pluginItem = new QTreeWidgetItem(topLevelItem);
    QString toolTip = QDir::toNativeSeparators(file.absoluteFilePath());
    if (file.exists())
        toolTip += u'\n' + file.lastModified().toString();
    pluginItem->setFont(0, font);
    pluginItem->setText(0, file.fileName());
    pluginItem->setToolTip(0, toolTip);
    pluginItem->setExpanded(true);
    pluginItem->setIcon(0, style()->standardPixmap(QStyle::SP_DirOpenIcon));

    return pluginItem;
}

QTreeWidgetItem *PluginDialog::setItem(QTreeWidgetItem *pluginItem, const QString &name,
                                       const QString &toolTip, const QString &whatsThis,
                                       const QIcon &icon)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(pluginItem);
    item->setText(0, name);
    item->setToolTip(0, toolTip);
    item->setWhatsThis(0, whatsThis);
    item->setIcon(0, icon.isNull() ? qtLogoIcon() : icon);
    return item;
}

void  PluginDialog::updateCustomWidgetPlugins()
{
    const int before = m_core->widgetDataBase()->count();
    m_core->integration()->updateCustomWidgetPlugins();
    const int after = m_core->widgetDataBase()->count();
    if (after >  before) {
        ui.message->setText(tr("New custom widget plugins have been found."));
        ui.message->show();
    } else {
        ui.message->setText(QString());
    }
    populateTreeWidget();
}

void PluginDialog::treeWidgetContextMenu(const QPoint &pos)
{
#if QT_CONFIG(clipboard)
    const QTreeWidgetItem *item = ui.treeWidget->itemAt(pos);
    if (item == nullptr || !item->data(0, ErrorItemRole).toBool())
        return;
    QMenu menu;
    //: Copy error text
    auto copyAction = menu.addAction(tr("Copy"));
    auto chosenAction = menu.exec(ui.treeWidget->mapToGlobal(pos));
    if (chosenAction == nullptr)
        return;
    if (chosenAction == copyAction)
        QGuiApplication::clipboard()->setText(item->text(0));
#else
    Q_UNUSED(pos);
#endif
}

} // namespace qdesigner_internal

QT_END_NAMESPACE

#include "moc_plugindialog_p.cpp"
