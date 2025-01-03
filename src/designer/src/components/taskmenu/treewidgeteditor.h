// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef TREEWIDGETEDITOR_H
#define TREEWIDGETEDITOR_H

#include "ui_treewidgeteditor.h"

#include "listwidgeteditor.h"

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class QTreeWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TreeWidgetEditor: public AbstractItemEditor
{
    Q_OBJECT
public:
    explicit TreeWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

    TreeWidgetContents fillContentsFromTreeWidget(QTreeWidget *treeWidget);
    TreeWidgetContents contents() const;

private slots:
    void newItemButtonClicked();
    void newSubItemButtonClicked();
    void deleteItemButtonClicked();
    void moveItemUpButtonClicked();
    void moveItemDownButtonClicked();
    void moveItemRightButtonClicked();
    void moveItemLeftButtonClicked();

    void treeWidgetCurrentItemChanged();
    void treeWidgetItemChanged(QTreeWidgetItem *item, int column);

    void columnEditorIndexChanged(int idx);
    void columnEditorItemChanged(int idx, int role, const QVariant &v);

    void columnEditorItemInserted(int idx);
    void columnEditorItemDeleted(int idx);
    void columnEditorItemMovedUp(int idx);
    void columnEditorItemMovedDown(int idx);

    void togglePropertyBrowser();
    void cacheReloaded();

protected:
    void setItemData(int role, const QVariant &v) override;
    QVariant getItemData(int role) const override;
    int defaultItemFlags() const override;

private:
    void setPropertyBrowserVisible(bool v);
    QtVariantProperty *setupPropertyGroup(const QString &title, PropertyDefinition *propDefs);
    void updateEditor();
    void moveColumnItems(const PropertyDefinition *propList, QTreeWidgetItem *item, int fromColumn, int toColumn, int step);
    void moveColumns(int fromColumn, int toColumn, int step);
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);
    void closeEditors();

    Ui::TreeWidgetEditor ui;
    ItemListEditor *m_columnEditor;
    bool m_updatingBrowser;
};

class TreeWidgetEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TreeWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent);

    TreeWidgetContents fillContentsFromTreeWidget(QTreeWidget *treeWidget);
    TreeWidgetContents contents() const;

private:
    TreeWidgetEditor m_editor;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TREEWIDGETEDITOR_H
