// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef ITEMLISTEDITOR_H
#define ITEMLISTEDITOR_H

#include "ui_itemlisteditor.h"

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QtProperty;
class QtVariantProperty;
class QtTreePropertyBrowser;
class QSplitter;
class QVBoxLayout;

namespace qdesigner_internal {

class DesignerIconCache;
class DesignerPropertyManager;
class DesignerEditorFactory;

// Utility class that ensures a bool is true while in scope.
// Courtesy of QBoolBlocker in qobject_p.h
class BoolBlocker
{
public:
    Q_DISABLE_COPY_MOVE(BoolBlocker);

    inline explicit BoolBlocker(bool &b) noexcept : block(b), reset(b) { block = true; }
    inline ~BoolBlocker() noexcept { block = reset; }
private:
    bool &block;
    bool reset;
};

class AbstractItemEditor: public QWidget
{
    Q_OBJECT

public:
    explicit AbstractItemEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    ~AbstractItemEditor();

    DesignerIconCache *iconCache() const { return m_iconCache; }

    struct PropertyDefinition {
        int role;
        int type;
        int (*typeFunc)();
        const char *name;
    };

public slots:
    void cacheReloaded();

private slots:
    void propertyChanged(QtProperty *property);
    void resetProperty(QtProperty *property);

protected:
    virtual int defaultItemFlags() const = 0;
    void setupProperties(const PropertyDefinition *propList,
                         Qt::Alignment alignDefault = Qt::AlignLeading | Qt::AlignVCenter);
    void setupObject(QWidget *object);
    void setupEditor(QWidget *object, const PropertyDefinition *propDefs,
                     Qt::Alignment alignDefault = Qt::AlignLeading | Qt::AlignVCenter);
    void injectPropertyBrowser(QWidget *parent, QWidget *widget);
    void updateBrowser();
    virtual void setItemData(int role, const QVariant &v) = 0;
    virtual QVariant getItemData(int role) const = 0;

    DesignerIconCache *m_iconCache;
    DesignerPropertyManager *m_propertyManager;
    DesignerEditorFactory *m_editorFactory;
    QSplitter *m_propertySplitter =  nullptr;
    QtTreePropertyBrowser *m_propertyBrowser;
    QList<QtVariantProperty*> m_properties;
    QList<QtVariantProperty*> m_rootProperties;
    QHash<QtVariantProperty*, int> m_propertyToRole;
    bool m_updatingBrowser = false;
};

class ItemListEditor: public AbstractItemEditor
{
    Q_OBJECT

public:
    explicit ItemListEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    void setupEditor(QWidget *object, const PropertyDefinition *propDefs,
                     Qt::Alignment alignDefault = Qt::AlignLeading | Qt::AlignVCenter);
    QListWidget *listWidget() const { return ui.listWidget; }
    void setNewItemText(const QString &tpl) { m_newItemText = tpl; }
    QString newItemText() const { return m_newItemText; }
    void setCurrentIndex(int idx);

    uint alignDefault() const;
    void setAlignDefault(uint newAlignDefault);

signals:
    void indexChanged(int idx);
    void itemChanged(int idx, int role, const QVariant &v);
    void itemInserted(int idx);
    void itemDeleted(int idx);
    void itemMovedUp(int idx);
    void itemMovedDown(int idx);

private slots:
    void newListItemButtonClicked();
    void deleteListItemButtonClicked();
    void moveListItemUpButtonClicked();
    void moveListItemDownButtonClicked();
    void listWidgetCurrentRowChanged();
    void listWidgetItemChanged(QListWidgetItem * item);
    void togglePropertyBrowser();
    void cacheReloaded();

protected:
    void setItemData(int role, const QVariant &v) override;
    QVariant getItemData(int role) const override;
    int defaultItemFlags() const override;

private:
    void setPropertyBrowserVisible(bool v);
    void updateEditor();
    Ui::ItemListEditor ui;
    uint m_alignDefault = 0;
    bool m_updating;
    QString m_newItemText;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ITEMLISTEDITOR_H
