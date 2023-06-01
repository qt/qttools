// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "tablewidgeteditor.h"
#include <abstractformbuilder.h>
#include <iconloader_p.h>
#include <qdesigner_command_p.h>
#include "formwindowbase_p.h"
#include "qdesigner_utils_p.h"
#include <designerpropertymanager.h>
#include <qttreepropertybrowser.h>

#include <QtDesigner/abstractformwindow.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtCore/qdir.h>
#include <QtCore/qqueue.h>
#include <QtCore/qtextstream.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

TableWidgetEditor::TableWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog)
    : AbstractItemEditor(form, nullptr), m_updatingBrowser(false)
{
    m_columnEditor = new ItemListEditor(form, this);
    m_columnEditor->setObjectName(u"columnEditor"_s);
    m_columnEditor->setAlignDefault(Qt::AlignCenter);
    m_columnEditor->setNewItemText(tr("New Column"));
    m_rowEditor = new ItemListEditor(form, this);
    m_rowEditor->setObjectName(u"rowEditor"_s);
    m_rowEditor->setNewItemText(tr("New Row"));
    ui.setupUi(dialog);

    injectPropertyBrowser(ui.itemsTab, ui.widget);
    connect(ui.showPropertiesButton, &QAbstractButton::clicked,
            this, &TableWidgetEditor::togglePropertyBrowser);
    setPropertyBrowserVisible(false);

    ui.tabWidget->insertTab(0, m_columnEditor, tr("&Columns"));
    ui.tabWidget->insertTab(1, m_rowEditor, tr("&Rows"));
    ui.tabWidget->setCurrentIndex(0);

    ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(iconCache(), &DesignerIconCache::reloaded, this, &TableWidgetEditor::cacheReloaded);

    connect(ui.tableWidget, &QTableWidget::currentCellChanged,
            this, &TableWidgetEditor::tableWidgetCurrentCellChanged);
    connect(ui.tableWidget, &QTableWidget::itemChanged,
            this, &TableWidgetEditor::tableWidgetItemChanged);
    connect(m_columnEditor, &ItemListEditor::indexChanged,
            this, &TableWidgetEditor::columnEditorIndexChanged);
    connect(m_columnEditor, &ItemListEditor::itemChanged,
            this, &TableWidgetEditor::columnEditorItemChanged);
    connect(m_columnEditor, &ItemListEditor::itemInserted,
            this, &TableWidgetEditor::columnEditorItemInserted);
    connect(m_columnEditor, &ItemListEditor::itemDeleted,
            this, &TableWidgetEditor::columnEditorItemDeleted);
    connect(m_columnEditor, &ItemListEditor::itemMovedUp,
            this, &TableWidgetEditor::columnEditorItemMovedUp);
    connect(m_columnEditor, &ItemListEditor::itemMovedDown,
            this, &TableWidgetEditor::columnEditorItemMovedDown);

    connect(m_rowEditor, &ItemListEditor::indexChanged,
            this, &TableWidgetEditor::rowEditorIndexChanged);
    connect(m_rowEditor, &ItemListEditor::itemChanged,
            this, &TableWidgetEditor::rowEditorItemChanged);
    connect(m_rowEditor, &ItemListEditor::itemInserted,
            this, &TableWidgetEditor::rowEditorItemInserted);
    connect(m_rowEditor, &ItemListEditor::itemDeleted,
            this, &TableWidgetEditor::rowEditorItemDeleted);
    connect(m_rowEditor, &ItemListEditor::itemMovedUp,
            this, &TableWidgetEditor::rowEditorItemMovedUp);
    connect(m_rowEditor, &ItemListEditor::itemMovedDown,
            this, &TableWidgetEditor::rowEditorItemMovedDown);
}

static AbstractItemEditor::PropertyDefinition tableHeaderPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { Qt::ToolTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "toolTip" },
//    { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
    { Qt::WhatsThisPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
    { Qt::FontRole, QMetaType::QFont, nullptr, "font" },
    { Qt::TextAlignmentRole, 0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
    { Qt::BackgroundRole, QMetaType::QColor, nullptr, "background" },
    { Qt::ForegroundRole, QMetaType::QBrush, nullptr, "foreground" },
    { 0, 0, nullptr, nullptr }
};

static AbstractItemEditor::PropertyDefinition tableItemPropList[] = {
    { Qt::DisplayPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "text" },
    { Qt::DecorationPropertyRole, 0, DesignerPropertyManager::designerIconTypeId, "icon" },
    { Qt::ToolTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "toolTip" },
//    { Qt::StatusTipPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "statusTip" },
    { Qt::WhatsThisPropertyRole, 0, DesignerPropertyManager::designerStringTypeId, "whatsThis" },
    { Qt::FontRole, QMetaType::QFont, nullptr, "font" },
    { Qt::TextAlignmentRole, 0, DesignerPropertyManager::designerAlignmentTypeId, "textAlignment" },
    { Qt::BackgroundRole, QMetaType::QBrush, nullptr, "background" },
    { Qt::ForegroundRole, QMetaType::QBrush, nullptr, "foreground" },
    { ItemFlagsShadowRole, 0, QtVariantPropertyManager::flagTypeId, "flags" },
    { Qt::CheckStateRole, 0, QtVariantPropertyManager::enumTypeId, "checkState" },
    { 0, 0, nullptr, nullptr }
};

TableWidgetContents TableWidgetEditor::fillContentsFromTableWidget(QTableWidget *tableWidget)
{
    TableWidgetContents tblCont;
    tblCont.fromTableWidget(tableWidget, false);
    tblCont.applyToTableWidget(ui.tableWidget, iconCache(), true);

    auto *header = tableWidget->verticalHeader();
    auto headerAlignment = header != nullptr
            ? header->defaultAlignment() : Qt::Alignment(Qt::AlignLeading | Qt::AlignVCenter);
    tblCont.m_verticalHeader.applyToListWidget(m_rowEditor->listWidget(), iconCache(),
                                               true, headerAlignment);
    m_rowEditor->setupEditor(tableWidget, tableHeaderPropList, headerAlignment);

    header = tableWidget->horizontalHeader();
    headerAlignment = header != nullptr
        ? header->defaultAlignment() : Qt::Alignment(Qt::AlignCenter);
    tblCont.m_horizontalHeader.applyToListWidget(m_columnEditor->listWidget(), iconCache(),
                                                 true, headerAlignment);
    m_columnEditor->setupEditor(tableWidget, tableHeaderPropList, headerAlignment);

    setupEditor(tableWidget, tableItemPropList);
    if (ui.tableWidget->columnCount() > 0 && ui.tableWidget->rowCount() > 0)
        ui.tableWidget->setCurrentCell(0, 0);

    updateEditor();

    return tblCont;
}

TableWidgetContents TableWidgetEditor::contents() const
{
    TableWidgetContents retVal;
    retVal.fromTableWidget(ui.tableWidget, true);
    return retVal;
}

void TableWidgetEditor::setItemData(int role, const QVariant &v)
{
    QTableWidgetItem *item = ui.tableWidget->currentItem();
    BoolBlocker block(m_updatingBrowser);
    if (!item) {
        item = new QTableWidgetItem;
        ui.tableWidget->setItem(ui.tableWidget->currentRow(), ui.tableWidget->currentColumn(), item);
    }
    QVariant newValue = v;
    if (role == Qt::FontRole && newValue.metaType().id() == QMetaType::QFont) {
        QFont oldFont = ui.tableWidget->font();
        QFont newFont = qvariant_cast<QFont>(newValue).resolve(oldFont);
        newValue = QVariant::fromValue(newFont);
        item->setData(role, QVariant()); // force the right font with the current resolve mask is set (item view bug)
    }
    item->setData(role, newValue);
}

QVariant TableWidgetEditor::getItemData(int role) const
{
    QTableWidgetItem *item = ui.tableWidget->currentItem();
    if (!item)
        return QVariant();
    return item->data(role);
}

int TableWidgetEditor::defaultItemFlags() const
{
    static const int flags = QTableWidgetItem().flags();
    return flags;
}

void TableWidgetEditor::tableWidgetCurrentCellChanged(int currentRow, int currentCol)
{
    m_rowEditor->setCurrentIndex(currentRow);
    m_columnEditor->setCurrentIndex(currentCol);
    updateBrowser();
}

void TableWidgetEditor::tableWidgetItemChanged(QTableWidgetItem *item)
{
    if (m_updatingBrowser)
        return;

    PropertySheetStringValue val = qvariant_cast<PropertySheetStringValue>(item->data(Qt::DisplayPropertyRole));
    val.setValue(item->text());
    BoolBlocker block(m_updatingBrowser);
    item->setData(Qt::DisplayPropertyRole, QVariant::fromValue(val));

    updateBrowser();
}

void TableWidgetEditor::columnEditorIndexChanged(int col)
{
    ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), col);
}

void TableWidgetEditor::columnEditorItemChanged(int idx, int role, const QVariant &v)
{
    ui.tableWidget->horizontalHeaderItem(idx)->setData(role, v);
}

void TableWidgetEditor::rowEditorIndexChanged(int col)
{
    ui.tableWidget->setCurrentCell(col, ui.tableWidget->currentColumn());
}

void TableWidgetEditor::rowEditorItemChanged(int idx, int role, const QVariant &v)
{
    ui.tableWidget->verticalHeaderItem(idx)->setData(role, v);
}

void TableWidgetEditor::setPropertyBrowserVisible(bool v)
{
    ui.showPropertiesButton->setText(v ? tr("Properties &>>") : tr("Properties &<<"));
    m_propertyBrowser->setVisible(v);
}

void TableWidgetEditor::togglePropertyBrowser()
{
    setPropertyBrowserVisible(!m_propertyBrowser->isVisible());
}

void TableWidgetEditor::updateEditor()
{
    const bool wasEnabled = ui.tabWidget->isTabEnabled(2);
    const bool isEnabled = ui.tableWidget->columnCount() && ui.tableWidget->rowCount();
    ui.tabWidget->setTabEnabled(2, isEnabled);
    if (!wasEnabled && isEnabled)
        ui.tableWidget->setCurrentCell(0, 0);

    QMetaObject::invokeMethod(ui.tableWidget, "updateGeometries");
    ui.tableWidget->viewport()->update();
}

void TableWidgetEditor::moveColumnsLeft(int fromColumn, int toColumn)
{
    if (fromColumn >= toColumn)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeHorizontalHeaderItem(toColumn);
    for (int i = toColumn; i > fromColumn; i--) {
        ui.tableWidget->setHorizontalHeaderItem(i,
                    ui.tableWidget->takeHorizontalHeaderItem(i - 1));
    }
    ui.tableWidget->setHorizontalHeaderItem(fromColumn, lastItem);

    for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(i, toColumn);
        for (int j = toColumn; j > fromColumn; j--)
            ui.tableWidget->setItem(i, j, ui.tableWidget->takeItem(i, j - 1));
        ui.tableWidget->setItem(i, fromColumn, lastItem);
    }
}

void TableWidgetEditor::moveColumnsRight(int fromColumn, int toColumn)
{
    if (fromColumn >= toColumn)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeHorizontalHeaderItem(fromColumn);
    for (int i = fromColumn; i < toColumn; i++) {
        ui.tableWidget->setHorizontalHeaderItem(i,
                    ui.tableWidget->takeHorizontalHeaderItem(i + 1));
    }
    ui.tableWidget->setHorizontalHeaderItem(toColumn, lastItem);

    for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(i, fromColumn);
        for (int j = fromColumn; j < toColumn; j++)
            ui.tableWidget->setItem(i, j, ui.tableWidget->takeItem(i, j + 1));
        ui.tableWidget->setItem(i, toColumn, lastItem);
    }
}

void TableWidgetEditor::moveRowsDown(int fromRow, int toRow)
{
    if (fromRow >= toRow)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeVerticalHeaderItem(toRow);
    for (int i = toRow; i > fromRow; i--) {
        ui.tableWidget->setVerticalHeaderItem(i,
                    ui.tableWidget->takeVerticalHeaderItem(i - 1));
    }
    ui.tableWidget->setVerticalHeaderItem(fromRow, lastItem);

    for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(toRow, i);
        for (int j = toRow; j > fromRow; j--)
            ui.tableWidget->setItem(j, i, ui.tableWidget->takeItem(j - 1, i));
        ui.tableWidget->setItem(fromRow, i, lastItem);
    }
}

void TableWidgetEditor::moveRowsUp(int fromRow, int toRow)
{
    if (fromRow >= toRow)
        return;

    QTableWidgetItem *lastItem = ui.tableWidget->takeVerticalHeaderItem(fromRow);
    for (int i = fromRow; i < toRow; i++) {
        ui.tableWidget->setVerticalHeaderItem(i,
                    ui.tableWidget->takeVerticalHeaderItem(i + 1));
    }
    ui.tableWidget->setVerticalHeaderItem(toRow, lastItem);

    for (int i = 0; i < ui.tableWidget->columnCount(); i++) {
        QTableWidgetItem *lastItem = ui.tableWidget->takeItem(fromRow, i);
        for (int j = fromRow; j < toRow; j++)
            ui.tableWidget->setItem(j, i, ui.tableWidget->takeItem(j + 1, i));
        ui.tableWidget->setItem(toRow, i, lastItem);
    }
}

void TableWidgetEditor::columnEditorItemInserted(int idx)
{
    const int columnCount = ui.tableWidget->columnCount();
    ui.tableWidget->setColumnCount(columnCount + 1);

    QTableWidgetItem *newItem = new QTableWidgetItem(m_columnEditor->newItemText());
    newItem->setData(Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(m_columnEditor->newItemText())));
    ui.tableWidget->setHorizontalHeaderItem(columnCount, newItem);

    moveColumnsLeft(idx, columnCount);

    int row = ui.tableWidget->currentRow();
    if (row >= 0)
        ui.tableWidget->setCurrentCell(row, idx);

    updateEditor();
}

void TableWidgetEditor::columnEditorItemDeleted(int idx)
{
    const int columnCount = ui.tableWidget->columnCount();

    moveColumnsRight(idx, columnCount - 1);
    ui.tableWidget->setColumnCount(columnCount - 1);

    updateEditor();
}

void TableWidgetEditor::columnEditorItemMovedUp(int idx)
{
    moveColumnsRight(idx - 1, idx);

    ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), idx - 1);
}

void TableWidgetEditor::columnEditorItemMovedDown(int idx)
{
    moveColumnsLeft(idx, idx + 1);

    ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(), idx + 1);
}

void TableWidgetEditor::rowEditorItemInserted(int idx)
{
    const int rowCount = ui.tableWidget->rowCount();
    ui.tableWidget->setRowCount(rowCount + 1);

    QTableWidgetItem *newItem = new QTableWidgetItem(m_rowEditor->newItemText());
    newItem->setData(Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(m_rowEditor->newItemText())));
    ui.tableWidget->setVerticalHeaderItem(rowCount, newItem);

    moveRowsDown(idx, rowCount);

    int col = ui.tableWidget->currentColumn();
    if (col >= 0)
        ui.tableWidget->setCurrentCell(idx, col);

    updateEditor();
}

void TableWidgetEditor::rowEditorItemDeleted(int idx)
{
    const int rowCount = ui.tableWidget->rowCount();

    moveRowsUp(idx, rowCount - 1);
    ui.tableWidget->setRowCount(rowCount - 1);

    updateEditor();
}

void TableWidgetEditor::rowEditorItemMovedUp(int idx)
{
    moveRowsUp(idx - 1, idx);

    ui.tableWidget->setCurrentCell(idx - 1, ui.tableWidget->currentColumn());
}

void TableWidgetEditor::rowEditorItemMovedDown(int idx)
{
    moveRowsDown(idx, idx + 1);

    ui.tableWidget->setCurrentCell(idx + 1, ui.tableWidget->currentColumn());
}

void TableWidgetEditor::cacheReloaded()
{
    reloadIconResources(iconCache(), ui.tableWidget);
}

TableWidgetEditorDialog::TableWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent) :
    QDialog(parent), m_editor(form, this)
{
}

TableWidgetContents TableWidgetEditorDialog::fillContentsFromTableWidget(QTableWidget *tableWidget)
{
    return m_editor.fillContentsFromTableWidget(tableWidget);
}

TableWidgetContents TableWidgetEditorDialog::contents() const
{
    return m_editor.contents();
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
