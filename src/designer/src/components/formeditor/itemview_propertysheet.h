// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ITEMVIEW_PROPERTYSHEET_H
#define ITEMVIEW_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>

#include <QtWidgets/qtreeview.h>
#include <QtWidgets/qtableview.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

struct ItemViewPropertySheetPrivate;

class ItemViewPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit ItemViewPropertySheet(QTreeView *treeViewObject, QObject *parent = nullptr);
    explicit ItemViewPropertySheet(QTableView *tableViewObject, QObject *parent = nullptr);
    ~ItemViewPropertySheet();

    QHash<QString,QString> propertyNameMap() const;

    // QDesignerPropertySheet
    QVariant property(int index) const override;
    void setProperty(int index, const QVariant &value) override;

    void setChanged(int index, bool changed) override;
    bool isChanged(int index) const override;

    bool hasReset(int index) const override;
    bool reset(int index) override;

private:
    void initHeaderProperties(QHeaderView *hv, const QString &prefix);

    ItemViewPropertySheetPrivate *d;
};

using QTreeViewPropertySheetFactory = QDesignerPropertySheetFactory<QTreeView, ItemViewPropertySheet>;
using QTableViewPropertySheetFactory = QDesignerPropertySheetFactory<QTableView, ItemViewPropertySheet>;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ITEMVIEW_PROPERTYSHEET_H
