// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TABLEWIDGETEDITOR_H
#define TABLEWIDGETEDITOR_H

#include "ui_tablewidgeteditor.h"

#include "listwidgeteditor.h"

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class QTableWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TableWidgetEditor: public AbstractItemEditor
{
    Q_OBJECT
public:
    explicit TableWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

    TableWidgetContents fillContentsFromTableWidget(QTableWidget *tableWidget);
    TableWidgetContents contents() const;

private slots:

    void tableWidgetCurrentCellChanged(int currentRow, int currentCol);
    void tableWidgetItemChanged(QTableWidgetItem *item);

    void columnEditorIndexChanged(int idx);
    void columnEditorItemChanged(int idx, int role, const QVariant &v);

    void columnEditorItemInserted(int idx);
    void columnEditorItemDeleted(int idx);
    void columnEditorItemMovedUp(int idx);
    void columnEditorItemMovedDown(int idx);

    void rowEditorIndexChanged(int idx);
    void rowEditorItemChanged(int idx, int role, const QVariant &v);

    void rowEditorItemInserted(int idx);
    void rowEditorItemDeleted(int idx);
    void rowEditorItemMovedUp(int idx);
    void rowEditorItemMovedDown(int idx);

    void togglePropertyBrowser();

    void cacheReloaded();

protected:
    void setItemData(int role, const QVariant &v) override;
    QVariant getItemData(int role) const override;
    int defaultItemFlags() const override;

private:
    void setPropertyBrowserVisible(bool v);
    void updateEditor();
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);
    void moveRowsUp(int fromRow, int toRow);
    void moveRowsDown(int fromRow, int toRow);

    Ui::TableWidgetEditor ui;
    ItemListEditor *m_rowEditor;
    ItemListEditor *m_columnEditor;
    bool m_updatingBrowser;
};

class TableWidgetEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TableWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent);

    TableWidgetContents fillContentsFromTableWidget(QTableWidget *tableWidget);
    TableWidgetContents contents() const;

private:
    TableWidgetEditor m_editor;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TABLEWIDGETEDITOR_H
