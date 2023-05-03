// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef WIDGETBOXCATEGORYLISTVIEW_H
#define WIDGETBOXCATEGORYLISTVIEW_H

#include <QtDesigner/abstractwidgetbox.h>

#include <QtWidgets/qlistview.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerDnDItemInterface;

class QSortFilterProxyModel;

namespace qdesigner_internal {

class WidgetBoxCategoryModel;

// List view of a category, switchable between icon and list mode.
// Provides a filtered view.
class WidgetBoxCategoryListView : public QListView
{
    Q_OBJECT
public:
    // Whether to access the filtered or unfiltered view
    enum AccessMode { FilteredAccess, UnfilteredAccess };

    explicit WidgetBoxCategoryListView(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
    void setViewMode(ViewMode vm);

    void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list);

    using QListView::contentsSize;

    // These methods operate on the filtered/unfiltered model according to accessmode
    int count(AccessMode am) const;
    QDesignerWidgetBoxInterface::Widget widgetAt(AccessMode am, const QModelIndex &index) const;
    QDesignerWidgetBoxInterface::Widget widgetAt(AccessMode am, int row) const;
    void removeRow(AccessMode am, int row);
    void setCurrentItem(AccessMode am, int row);

    // These methods operate on the unfiltered model and are used for serialization
    void addWidget(const QDesignerWidgetBoxInterface::Widget &widget, const QIcon &icon, bool editable);
    bool containsWidget(const QString &name);
    QDesignerWidgetBoxInterface::Category category() const;
    bool removeCustomWidgets();

    // Helper: Ensure a <ui> tag in the case of empty XML
    static QString widgetDomXml(const QDesignerWidgetBoxInterface::Widget &widget);

signals:
    void scratchPadChanged();
    void widgetBoxPressed(const QString &name, const QString &xml, const QPoint &globalPos);
    void itemRemoved();
    void lastItemRemoved();

public slots:
    void filter(const QString &needle, Qt::CaseSensitivity caseSensitivity);
    void removeCurrentItem();
    void editCurrentItem();

private slots:
    void slotPressed(const QModelIndex &index);

private:
    int mapRowToSource(int filterRow) const;
    QSortFilterProxyModel *m_proxyModel;
    WidgetBoxCategoryModel *m_model;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETBOXCATEGORYLISTVIEW_H
