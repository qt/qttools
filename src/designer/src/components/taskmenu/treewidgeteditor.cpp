// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "treewidgeteditor.h"
#include <formwindowbase_p.h>
#include <iconloader_p.h>
#include <qdesigner_command_p.h>
#include <qdesigner_utils_p.h>
#include <abstractformbuilder.h>
#include <designerpropertymanager.h>
#include <qttreepropertybrowser_p.h>

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtCore/qdir.h>
#include <QtCore/qqueue.h>
#include <QtWidgets/qheaderview.h>
#include <QtWidgets/qtreewidgetitemiterator.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

TreeWidgetEditor::TreeWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog)
    : AbstractItemEditor(form, nullptr), m_updatingBrowser(false)
{
    m_columnEditor = new ItemListEditor(form, this);
    m_columnEditor->setObjectName(u"columnEditor"_s);
    m_columnEditor->setNewItemText(tr("New Column"));
    ui.setupUi(dialog);

    injectPropertyBrowser(ui.itemsTab, ui.widget);
    connect(ui.showPropertiesButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::togglePropertyBrowser);
    setPropertyBrowserVisible(false);

    ui.tabWidget->insertTab(0, m_columnEditor, tr("&Columns"));
    ui.tabWidget->setCurrentIndex(0);

    ui.newItemButton->setIcon(createIconSet("plus.png"_L1));
    ui.newSubItemButton->setIcon(createIconSet("downplus.png"_L1));
    ui.deleteItemButton->setIcon(createIconSet("minus.png"_L1));
    ui.moveItemUpButton->setIcon(createIconSet("up.png"_L1));
    ui.moveItemDownButton->setIcon(createIconSet("down.png"_L1));
    ui.moveItemRightButton->setIcon(createIconSet("leveldown.png"_L1));
    ui.moveItemLeftButton->setIcon(createIconSet("levelup.png"_L1));

    ui.treeWidget->header()->setSectionsMovable(false);

    connect(ui.newItemButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::newItemButtonClicked);
    connect(ui.newSubItemButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::newSubItemButtonClicked);
    connect(ui.moveItemUpButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::moveItemUpButtonClicked);
    connect(ui.moveItemDownButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::moveItemDownButtonClicked);
    connect(ui.moveItemRightButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::moveItemRightButtonClicked);
    connect(ui.moveItemLeftButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::moveItemLeftButtonClicked);
    connect(ui.deleteItemButton, &QAbstractButton::clicked,
            this, &TreeWidgetEditor::deleteItemButtonClicked);
    connect(ui.treeWidget, &QTreeWidget::currentItemChanged,
            this, &TreeWidgetEditor::treeWidgetCurrentItemChanged);
    connect(ui.treeWidget, &QTreeWidget::itemChanged,
            this, &TreeWidgetEditor::treeWidgetItemChanged);

    connect(m_columnEditor, &ItemListEditor::indexChanged,
            this, &TreeWidgetEditor::columnEditorIndexChanged);
    connect(m_columnEditor, &ItemListEditor::itemChanged,
            this, &TreeWidgetEditor::columnEditorItemChanged);
    connect(m_columnEditor, &ItemListEditor::itemInserted,
            this, &TreeWidgetEditor::columnEditorItemInserted);
    connect(m_columnEditor, &ItemListEditor::itemDeleted,
            this, &TreeWidgetEditor::columnEditorItemDeleted);
    connect(m_columnEditor, &ItemListEditor::itemMovedUp,
            this, &TreeWidgetEditor::columnEditorItemMovedUp);
    connect(m_columnEditor, &ItemListEditor::itemMovedDown,
            this, &TreeWidgetEditor::columnEditorItemMovedDown);

    connect(iconCache(), &DesignerIconCache::reloaded, this, &TreeWidgetEditor::cacheReloaded);
}

static AbstractItemEditor::PropertyDefinition treeHeaderPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { Qt::ToolTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "toolTip" },
    { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
    { Qt::WhatsThisPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
    { Qt::FontRole, QMetaType::QFont, nullptr, "font" },
    { Qt::TextAlignmentRole, 0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
    { Qt::BackgroundRole, QMetaType::QColor, nullptr, "background" },
    { Qt::ForegroundRole, QMetaType::QBrush, nullptr, "foreground" },
    { 0, 0, nullptr, nullptr }
};

static AbstractItemEditor::PropertyDefinition treeItemColumnPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { Qt::ToolTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "toolTip" },
    { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
    { Qt::WhatsThisPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
    { Qt::FontRole, QMetaType::QFont, nullptr, "font" },
    { Qt::TextAlignmentRole, 0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
    { Qt::BackgroundRole, QMetaType::QBrush, nullptr, "background" },
    { Qt::ForegroundRole, QMetaType::QBrush, nullptr, "foreground" },
    { Qt::CheckStateRole, 0, QtVariantPropertyManager::enumTypeId, "checkState" },
    { 0, 0, nullptr, nullptr }
};

static AbstractItemEditor::PropertyDefinition treeItemCommonPropList[] = {
    { ItemFlagsShadowRole, 0, QtVariantPropertyManager::flagTypeId, "flags" },
    { 0, 0, nullptr, nullptr }
};

QtVariantProperty *TreeWidgetEditor::setupPropertyGroup(const QString &title, PropertyDefinition *propDefs)
{
    setupProperties(propDefs);
    QtVariantProperty *groupProp = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), title);
    for (QtVariantProperty *prop : std::as_const(m_rootProperties))
        groupProp->addSubProperty(prop);
    m_rootProperties.clear();
    return groupProp;
}

TreeWidgetContents TreeWidgetEditor::fillContentsFromTreeWidget(QTreeWidget *treeWidget)
{
    TreeWidgetContents treeCont;
    treeCont.fromTreeWidget(treeWidget, false);
    treeCont.applyToTreeWidget(ui.treeWidget, iconCache(), true);

    treeCont.m_headerItem.applyToListWidget(m_columnEditor->listWidget(), iconCache(), true);
    m_columnEditor->setupEditor(treeWidget, treeHeaderPropList);

    QList<QtVariantProperty*> rootProperties;
    rootProperties.append(setupPropertyGroup(tr("Per column properties"), treeItemColumnPropList));
    rootProperties.append(setupPropertyGroup(tr("Common properties"), treeItemCommonPropList));
    m_rootProperties = rootProperties;
    m_propertyBrowser->setPropertiesWithoutValueMarked(true);
    m_propertyBrowser->setRootIsDecorated(false);
    setupObject(treeWidget);

    if (ui.treeWidget->topLevelItemCount() > 0)
        ui.treeWidget->setCurrentItem(ui.treeWidget->topLevelItem(0));

    updateEditor();

    return treeCont;
}

TreeWidgetContents TreeWidgetEditor::contents() const
{
    TreeWidgetContents retVal;
    retVal.fromTreeWidget(ui.treeWidget, true);
    return retVal;
}

void TreeWidgetEditor::setItemData(int role, const QVariant &v)
{
    const int col = (role == ItemFlagsShadowRole) ? 0 : ui.treeWidget->currentColumn();
    QVariant newValue = v;
    BoolBlocker block(m_updatingBrowser);
    if (role == Qt::FontRole && newValue.metaType().id() == QMetaType::QFont) {
        QFont oldFont = ui.treeWidget->font();
        QFont newFont = qvariant_cast<QFont>(newValue).resolve(oldFont);
        newValue = QVariant::fromValue(newFont);
        ui.treeWidget->currentItem()->setData(col, role, QVariant()); // force the right font with the current resolve mask is set (item view bug)
    }
    ui.treeWidget->currentItem()->setData(col, role, newValue);
}

QVariant TreeWidgetEditor::getItemData(int role) const
{
    const int col = (role == ItemFlagsShadowRole) ? 0 : ui.treeWidget->currentColumn();
    return ui.treeWidget->currentItem()->data(col, role);
}

int TreeWidgetEditor::defaultItemFlags() const
{
    static const int flags = QTreeWidgetItem().flags();
    return flags;
}

void TreeWidgetEditor::newItemButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    QTreeWidgetItem *newItem = nullptr;
    ui.treeWidget->blockSignals(true);
    if (curItem) {
        if (curItem->parent())
            newItem = new QTreeWidgetItem(curItem->parent(), curItem);
        else
            newItem = new QTreeWidgetItem(ui.treeWidget, curItem);
    } else
        newItem = new QTreeWidgetItem(ui.treeWidget);
    const QString newItemText = tr("New Item");
    newItem->setText(0, newItemText);
    newItem->setData(0, Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(newItemText)));
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
    ui.treeWidget->blockSignals(false);

    ui.treeWidget->setCurrentItem(newItem, qMax(ui.treeWidget->currentColumn(), 0));
    updateEditor();
    ui.treeWidget->editItem(newItem, ui.treeWidget->currentColumn());
}

void TreeWidgetEditor::newSubItemButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    ui.treeWidget->blockSignals(true);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(curItem);
    const QString newItemText = tr("New Subitem");
    newItem->setText(0, newItemText);
    newItem->setData(0, Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(newItemText)));
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
    ui.treeWidget->blockSignals(false);

    ui.treeWidget->setCurrentItem(newItem, ui.treeWidget->currentColumn());
    updateEditor();
    ui.treeWidget->editItem(newItem, ui.treeWidget->currentColumn());
}

void TreeWidgetEditor::deleteItemButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    QTreeWidgetItem *nextCurrent = nullptr;
    if (curItem->parent()) {
        int idx = curItem->parent()->indexOfChild(curItem);
        if (idx == curItem->parent()->childCount() - 1)
            idx--;
        else
            idx++;
        if (idx < 0)
            nextCurrent = curItem->parent();
        else
            nextCurrent = curItem->parent()->child(idx);
    } else {
        int idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        if (idx == ui.treeWidget->topLevelItemCount() - 1)
            idx--;
        else
            idx++;
        if (idx >= 0)
            nextCurrent = ui.treeWidget->topLevelItem(idx);
    }
    closeEditors();
    ui.treeWidget->blockSignals(true);
    delete curItem;
    ui.treeWidget->blockSignals(false);

    if (nextCurrent)
        ui.treeWidget->setCurrentItem(nextCurrent, ui.treeWidget->currentColumn());
    updateEditor();
}

void TreeWidgetEditor::moveItemUpButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx;
    if (curItem->parent())
        idx = curItem->parent()->indexOfChild(curItem);
    else
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
    if (idx == 0)
        return;

    QTreeWidgetItem *takenItem;
    ui.treeWidget->blockSignals(true);
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent();
        takenItem = parentItem->takeChild(idx);
        parentItem->insertChild(idx - 1, takenItem);
    } else {
        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        ui.treeWidget->insertTopLevelItem(idx - 1, takenItem);
    }
    ui.treeWidget->blockSignals(false);

    ui.treeWidget->setCurrentItem(takenItem, ui.treeWidget->currentColumn());
    updateEditor();
}

void TreeWidgetEditor::moveItemDownButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx, idxCount;
    if (curItem->parent()) {
        idx = curItem->parent()->indexOfChild(curItem);
        idxCount = curItem->parent()->childCount();
    } else {
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        idxCount = ui.treeWidget->topLevelItemCount();
    }
    if (idx == idxCount - 1)
        return;

    QTreeWidgetItem *takenItem;
    ui.treeWidget->blockSignals(true);
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent();
        takenItem = parentItem->takeChild(idx);
        parentItem->insertChild(idx + 1, takenItem);
    } else {
        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        ui.treeWidget->insertTopLevelItem(idx + 1, takenItem);
    }
    ui.treeWidget->blockSignals(false);

    ui.treeWidget->setCurrentItem(takenItem, ui.treeWidget->currentColumn());
    updateEditor();
}

void TreeWidgetEditor::moveItemLeftButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    QTreeWidgetItem *parentItem = curItem->parent();
    if (!parentItem)
        return;

    ui.treeWidget->blockSignals(true);
    QTreeWidgetItem *takenItem = parentItem->takeChild(parentItem->indexOfChild(curItem));
    if (parentItem->parent()) {
        int idx = parentItem->parent()->indexOfChild(parentItem);
        parentItem->parent()->insertChild(idx, takenItem);
    } else {
        int idx = ui.treeWidget->indexOfTopLevelItem(parentItem);
        ui.treeWidget->insertTopLevelItem(idx, takenItem);
    }
    ui.treeWidget->blockSignals(false);

    ui.treeWidget->setCurrentItem(takenItem, ui.treeWidget->currentColumn());
    updateEditor();
}

void TreeWidgetEditor::moveItemRightButtonClicked()
{
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx, idxCount;
    if (curItem->parent()) {
        idx = curItem->parent()->indexOfChild(curItem);
        idxCount = curItem->parent()->childCount();
    } else {
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        idxCount = ui.treeWidget->topLevelItemCount();
    }
    if (idx == idxCount - 1)
        return;

    QTreeWidgetItem *takenItem;
    ui.treeWidget->blockSignals(true);
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent()->child(idx + 1);
        takenItem = curItem->parent()->takeChild(idx);
        parentItem->insertChild(0, takenItem);
    } else {
        QTreeWidgetItem *parentItem = ui.treeWidget->topLevelItem(idx + 1);
        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        parentItem->insertChild(0, takenItem);
    }
    ui.treeWidget->blockSignals(false);

    ui.treeWidget->setCurrentItem(takenItem, ui.treeWidget->currentColumn());
    updateEditor();
}

void TreeWidgetEditor::togglePropertyBrowser()
{
    setPropertyBrowserVisible(!m_propertyBrowser->isVisible());
}

void TreeWidgetEditor::setPropertyBrowserVisible(bool v)
{
    ui.showPropertiesButton->setText(v ? tr("Properties &>>") : tr("Properties &<<"));
    m_propertyBrowser->setVisible(v);
}

void TreeWidgetEditor::treeWidgetCurrentItemChanged()
{
    m_columnEditor->setCurrentIndex(ui.treeWidget->currentColumn());
    updateEditor();
}

void TreeWidgetEditor::treeWidgetItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_updatingBrowser)
        return;

    PropertySheetStringValue val = qvariant_cast<PropertySheetStringValue>(item->data(column, Qt::DisplayPropertyRole));
    val.setValue(item->text(column));
    BoolBlocker block(m_updatingBrowser);
    item->setData(column, Qt::DisplayPropertyRole, QVariant::fromValue(val));

    updateBrowser();
}

void TreeWidgetEditor::columnEditorIndexChanged(int idx)
{
    if (QTreeWidgetItem *item = ui.treeWidget->currentItem())
        ui.treeWidget->setCurrentItem(item, idx);
}

void TreeWidgetEditor::columnEditorItemChanged(int idx, int role, const QVariant &v)
{
    if (role == Qt::DisplayPropertyRole)
        ui.treeWidget->headerItem()->setData(idx, Qt::EditRole, qvariant_cast<PropertySheetStringValue>(v).value());
    ui.treeWidget->headerItem()->setData(idx, role, v);
}

void TreeWidgetEditor::updateEditor()
{
    QTreeWidgetItem *current = ui.treeWidget->currentItem();

    bool itemsEnabled = false;
    bool currentItemEnabled = false;
    bool moveItemUpEnabled = false;
    bool moveItemDownEnabled = false;
    bool moveItemRightEnabled = false;
    bool moveItemLeftEnabled = false;

    if (ui.treeWidget->columnCount() > 0) {
        itemsEnabled = true;
        if (current) {
            int idx;
            int idxCount;
            currentItemEnabled = true;
            if (current->parent()) {
                moveItemLeftEnabled = true;
                idx = current->parent()->indexOfChild(current);
                idxCount = current->parent()->childCount();
            } else {
                idx = ui.treeWidget->indexOfTopLevelItem(current);
                idxCount = ui.treeWidget->topLevelItemCount();
            }
            if (idx > 0)
                moveItemUpEnabled = true;
            if (idx < idxCount - 1) {
                moveItemDownEnabled = true;
                moveItemRightEnabled = true;
            }
        }
    }
    ui.tabWidget->setTabEnabled(1, itemsEnabled);
    ui.newSubItemButton->setEnabled(currentItemEnabled);
    ui.deleteItemButton->setEnabled(currentItemEnabled);

    ui.moveItemUpButton->setEnabled(moveItemUpEnabled);
    ui.moveItemDownButton->setEnabled(moveItemDownEnabled);
    ui.moveItemRightButton->setEnabled(moveItemRightEnabled);
    ui.moveItemLeftButton->setEnabled(moveItemLeftEnabled);

    if (current)
        updateBrowser();
    else
        m_propertyBrowser->clear();
}

void TreeWidgetEditor::moveColumnItems(const PropertyDefinition *propList,
        QTreeWidgetItem *item, int fromColumn, int toColumn, int step)
{
    BoolBlocker block(m_updatingBrowser);

    QList<QVariant> saveCol;
    for (int j = 0; propList[j].name; j++)
        saveCol.append(item->data(toColumn, propList[j].role));
    QVariant editVariant = item->data(toColumn, Qt::EditRole);
    QVariant toolTipVariant = item->data(toColumn, Qt::ToolTipRole);
    QVariant statusTipVariant = item->data(toColumn, Qt::StatusTipRole);
    QVariant whatsThisVariant = item->data(toColumn, Qt::WhatsThisRole);
    QVariant decorationVariant = item->data(toColumn, Qt::DecorationRole);
    for (int i = toColumn; i != fromColumn; i += step) {
        for (int j = 0; propList[j].name; j++)
            item->setData(i, propList[j].role, item->data(i + step, propList[j].role));
        item->setData(i, Qt::EditRole, item->data(i + step, Qt::EditRole));
        item->setData(i, Qt::ToolTipRole, item->data(i + step, Qt::ToolTipRole));
        item->setData(i, Qt::StatusTipRole, item->data(i + step, Qt::StatusTipRole));
        item->setData(i, Qt::WhatsThisRole, item->data(i + step, Qt::WhatsThisRole));
        item->setData(i, Qt::DecorationRole, item->data(i + step, Qt::DecorationRole));
    }
    for (int j = 0; propList[j].name; j++)
        item->setData(fromColumn, propList[j].role, saveCol[j]);
    item->setData(fromColumn, Qt::EditRole, editVariant);
    item->setData(fromColumn, Qt::ToolTipRole, toolTipVariant);
    item->setData(fromColumn, Qt::StatusTipRole, statusTipVariant);
    item->setData(fromColumn, Qt::WhatsThisRole, whatsThisVariant);
    item->setData(fromColumn, Qt::DecorationRole, decorationVariant);
}

void TreeWidgetEditor::moveColumns(int fromColumn, int toColumn, int step)
{
    ui.treeWidget->blockSignals(true);

    moveColumnItems(treeHeaderPropList, ui.treeWidget->headerItem(), fromColumn, toColumn, step);

    QQueue<QTreeWidgetItem *> pendingQueue;
    for (int i = 0; i < ui.treeWidget->topLevelItemCount(); i++)
        pendingQueue.enqueue(ui.treeWidget->topLevelItem(i));

    while (!pendingQueue.isEmpty()) {
        QTreeWidgetItem *item = pendingQueue.dequeue();
        for (int i = 0; i < item->childCount(); i++)
            pendingQueue.enqueue(item->child(i));

        moveColumnItems(treeItemColumnPropList, item, fromColumn, toColumn, step);
    }

    ui.treeWidget->blockSignals(false);
}

void TreeWidgetEditor::moveColumnsLeft(int fromColumn, int toColumn)
{
    if (fromColumn >= toColumn)
        return;

    moveColumns(fromColumn, toColumn, -1);
}

void TreeWidgetEditor::moveColumnsRight(int fromColumn, int toColumn)
{
    if (fromColumn >= toColumn)
        return;

    moveColumns(toColumn, fromColumn, 1);
}

void TreeWidgetEditor::columnEditorItemInserted(int idx)
{
    int columnCount = ui.treeWidget->columnCount();
    ui.treeWidget->setColumnCount(columnCount + 1);
    ui.treeWidget->headerItem()->setText(columnCount, m_columnEditor->newItemText());
    moveColumnsLeft(idx, columnCount);

    updateEditor();
}

void TreeWidgetEditor::columnEditorItemDeleted(int idx)
{
    closeEditors();

    int columnCount = ui.treeWidget->columnCount() - 1;
    if (!columnCount)
        ui.treeWidget->clear();
    else
        moveColumnsRight(idx, columnCount);
    ui.treeWidget->setColumnCount(columnCount);

    updateEditor();
}

void TreeWidgetEditor::columnEditorItemMovedUp(int idx)
{
    moveColumnsRight(idx - 1, idx);

    ui.treeWidget->setCurrentItem(ui.treeWidget->currentItem(), idx - 1);
    updateEditor();
}

void TreeWidgetEditor::columnEditorItemMovedDown(int idx)
{
    moveColumnsLeft(idx, idx + 1);

    ui.treeWidget->setCurrentItem(ui.treeWidget->currentItem(), idx + 1);
    updateEditor();
}

void TreeWidgetEditor::closeEditors()
{
    if (QTreeWidgetItem *cur = ui.treeWidget->currentItem() ) {
        const int numCols = cur->columnCount ();
        for (int i = 0; i < numCols; i++)
            ui.treeWidget->closePersistentEditor (cur, i);
    }
}

void TreeWidgetEditor::cacheReloaded()
{
    reloadIconResources(iconCache(), ui.treeWidget);
}

TreeWidgetEditorDialog::TreeWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent) :
    QDialog(parent), m_editor(form, this)
{
}

TreeWidgetContents TreeWidgetEditorDialog::fillContentsFromTreeWidget(QTreeWidget *treeWidget)
{
    return m_editor.fillContentsFromTreeWidget(treeWidget);
}

TreeWidgetContents TreeWidgetEditorDialog::contents() const
{
    return m_editor.contents();
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
