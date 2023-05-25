// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef WIDGETBOXTREEWIDGET_H
#define WIDGETBOXTREEWIDGET_H

#include <qdesigner_widgetbox_p.h>

#include <QtWidgets/qtreewidget.h>
#include <QtGui/qicon.h>
#include <QtCore/qlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerDnDItemInterface;

class QTimer;

namespace qdesigner_internal {

class WidgetBoxCategoryListView;

// WidgetBoxTreeWidget: A tree of categories

class WidgetBoxTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    using Widget = QDesignerWidgetBoxInterface::Widget;
    using Category = QDesignerWidgetBoxInterface::Category;
    using CategoryList = QDesignerWidgetBoxInterface::CategoryList;

    explicit WidgetBoxTreeWidget(QDesignerFormEditorInterface *core, QWidget *parent = nullptr);
    ~WidgetBoxTreeWidget();

    int categoryCount() const;
    Category category(int cat_idx) const;
    void addCategory(const Category &cat);
    void removeCategory(int cat_idx);

    int widgetCount(int cat_idx) const;
    Widget widget(int cat_idx, int wgt_idx) const;
    void addWidget(int cat_idx, const Widget &wgt);
    void removeWidget(int cat_idx, int wgt_idx);

    void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list);

    void setFileName(const QString &file_name);
    QString fileName() const;
    bool load(QDesignerWidgetBox::LoadMode loadMode);
    bool loadContents(const QString &contents);
    bool save();
    QIcon iconForWidget(const QString &iconName) const;

signals:
    void widgetBoxPressed(const QString &name, const QString &dom_xml,
                          const QPoint &global_mouse_pos);

public slots:
    void filter(const QString &);

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void slotSave();
    void slotScratchPadItemDeleted();
    void slotLastScratchPadItemDeleted();

    void handleMousePress(QTreeWidgetItem *item);
    void deleteScratchpad();
    void slotListMode();
    void slotIconMode();

private:
    WidgetBoxCategoryListView *addCategoryView(QTreeWidgetItem *parent, bool iconMode);
    WidgetBoxCategoryListView *categoryViewAt(int idx) const;
    void adjustSubListSize(QTreeWidgetItem *cat_item);

    static bool readCategories(const QString &fileName, const QString &xml, CategoryList *cats, QString *errorMessage);
    static bool readWidget(Widget *w, const QString &xml, QXmlStreamReader &r);

    CategoryList loadCustomCategoryList() const;
    void writeCategories(QXmlStreamWriter &writer, const CategoryList &cat_list) const;

    int indexOfCategory(const QString &name) const;
    int indexOfScratchpad() const;
    int ensureScratchpad();
    void addCustomCategories(bool replace);

    void saveExpandedState() const;
    void restoreExpandedState();
    void updateViewMode();

    QDesignerFormEditorInterface *m_core;
    QString m_file_name;
    mutable QHash<QString, QIcon> m_pluginIcons;
    bool m_iconMode;
    QTimer *m_scratchPadDeleteTimer;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETBOXTREEWIDGET_H
