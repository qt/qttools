// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "itemlisteditor.h"
#include <abstractformbuilder.h>
#include <iconloader_p.h>
#include <formwindowbase_p.h>
#include <designerpropertymanager.h>

#include <QtDesigner/abstractformwindow.h>

#include <qttreepropertybrowser_p.h>

#include <QtWidgets/qsplitter.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

class ItemPropertyBrowser : public QtTreePropertyBrowser
{
public:
    ItemPropertyBrowser()
    {
        setResizeMode(Interactive);
        //: Sample string to determinate the width for the first column of the list item property browser
        const QString widthSampleString = QCoreApplication::translate("ItemPropertyBrowser", "XX Icon Selected off");
        m_width = fontMetrics().horizontalAdvance(widthSampleString);
        setSplitterPosition(m_width);
        m_width += fontMetrics().horizontalAdvance(u"/this/is/some/random/path"_s);
    }

    QSize sizeHint() const override
    {
        return QSize(m_width, 1);
    }

private:
    int m_width;
};

////////////////// Item editor ///////////////
AbstractItemEditor::AbstractItemEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QWidget(parent),
      m_iconCache(qobject_cast<FormWindowBase *>(form)->iconCache())
{
    m_propertyManager = new DesignerPropertyManager(form->core(), this);
    m_editorFactory = new DesignerEditorFactory(form->core(), this);
    m_editorFactory->setSpacing(0);
    m_propertyBrowser = new ItemPropertyBrowser;
    m_propertyBrowser->setFactoryForManager(static_cast<QtVariantPropertyManager *>(m_propertyManager),
                                            m_editorFactory);

    connect(m_editorFactory, &DesignerEditorFactory::resetProperty,
            this, &AbstractItemEditor::resetProperty);
    connect(m_propertyManager, &DesignerPropertyManager::valueChanged,
            this, &AbstractItemEditor::propertyChanged);
    connect(iconCache(), &DesignerIconCache::reloaded, this, &AbstractItemEditor::cacheReloaded);
}

AbstractItemEditor::~AbstractItemEditor()
{
    m_propertyBrowser->unsetFactoryForManager(m_propertyManager);
}

static const char * const itemFlagNames[] = {
    QT_TRANSLATE_NOOP("AbstractItemEditor", "Selectable"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "Editable"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "DragEnabled"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "DropEnabled"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "UserCheckable"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "Enabled"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "Tristate"),
    nullptr
};

static const char * const checkStateNames[] = {
    QT_TRANSLATE_NOOP("AbstractItemEditor", "Unchecked"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "PartiallyChecked"),
    QT_TRANSLATE_NOOP("AbstractItemEditor", "Checked"),
    nullptr
};

static QStringList c2qStringList(const char * const in[])
{
    QStringList out;
    for (int i = 0; in[i]; i++)
        out << AbstractItemEditor::tr(in[i]);
    return out;
}

void AbstractItemEditor::setupProperties(const PropertyDefinition *propList,
                                         Qt::Alignment alignDefault)
{
    for (int i = 0; propList[i].name; i++) {
        int type = propList[i].typeFunc ? propList[i].typeFunc() : propList[i].type;
        int role = propList[i].role;
        QtVariantProperty *prop = m_propertyManager->addProperty(type, QLatin1StringView(propList[i].name));
        if (role == Qt::TextAlignmentRole) {
            prop->setAttribute(DesignerPropertyManager::alignDefaultAttribute(),
                               QVariant(uint(alignDefault)));
        }
        Q_ASSERT(prop);
        if (role == Qt::ToolTipPropertyRole || role == Qt::WhatsThisPropertyRole)
            prop->setAttribute(u"validationMode"_s, ValidationRichText);
        else if (role == Qt::DisplayPropertyRole)
            prop->setAttribute(u"validationMode"_s, ValidationMultiLine);
        else if (role == Qt::StatusTipPropertyRole)
            prop->setAttribute(u"validationMode"_s, ValidationSingleLine);
        else if (role == ItemFlagsShadowRole)
            prop->setAttribute(u"flagNames"_s, c2qStringList(itemFlagNames));
        else if (role == Qt::CheckStateRole)
            prop->setAttribute(u"enumNames"_s, c2qStringList(checkStateNames));
        prop->setAttribute(u"resettable"_s, true);
        m_properties.append(prop);
        m_rootProperties.append(prop);
        m_propertyToRole.insert(prop, role);
    }
}

void AbstractItemEditor::setupObject(QWidget *object)
{
    m_propertyManager->setObject(object);
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(object);
    FormWindowBase *fwb = qobject_cast<FormWindowBase *>(formWindow);
    m_editorFactory->setFormWindowBase(fwb);
}

void AbstractItemEditor::setupEditor(QWidget *object,
                                     const PropertyDefinition *propList,
                                     Qt::Alignment alignDefault)
{
    setupProperties(propList, alignDefault);
    setupObject(object);
}

void AbstractItemEditor::propertyChanged(QtProperty *property)
{
    if (m_updatingBrowser)
        return;


    BoolBlocker block(m_updatingBrowser);
    QtVariantProperty *prop = m_propertyManager->variantProperty(property);
    int role;
    if ((role = m_propertyToRole.value(prop, -1)) == -1)
        // Subproperty
        return;

    if ((role == ItemFlagsShadowRole && prop->value().toInt() == defaultItemFlags())
        || (role == Qt::DecorationPropertyRole && !qvariant_cast<PropertySheetIconValue>(prop->value()).mask())
        || (role == Qt::FontRole && !qvariant_cast<QFont>(prop->value()).resolveMask())) {
        prop->setModified(false);
        setItemData(role, QVariant());
    } else {
        prop->setModified(true);
        setItemData(role, prop->value());
    }

    switch (role) {
    case Qt::DecorationPropertyRole:
        setItemData(Qt::DecorationRole, QVariant::fromValue(iconCache()->icon(qvariant_cast<PropertySheetIconValue>(prop->value()))));
        break;
    case Qt::DisplayPropertyRole:
        setItemData(Qt::EditRole, QVariant::fromValue(qvariant_cast<PropertySheetStringValue>(prop->value()).value()));
        break;
    case Qt::ToolTipPropertyRole:
        setItemData(Qt::ToolTipRole, QVariant::fromValue(qvariant_cast<PropertySheetStringValue>(prop->value()).value()));
        break;
    case Qt::StatusTipPropertyRole:
        setItemData(Qt::StatusTipRole, QVariant::fromValue(qvariant_cast<PropertySheetStringValue>(prop->value()).value()));
        break;
    case Qt::WhatsThisPropertyRole:
        setItemData(Qt::WhatsThisRole, QVariant::fromValue(qvariant_cast<PropertySheetStringValue>(prop->value()).value()));
        break;
    default:
        break;
    }

    prop->setValue(getItemData(role));
}

void AbstractItemEditor::resetProperty(QtProperty *property)
{
    if (m_propertyManager->resetFontSubProperty(property))
        return;

    if (m_propertyManager->resetIconSubProperty(property))
        return;

    if (m_propertyManager->resetTextAlignmentProperty(property))
        return;

    BoolBlocker block(m_updatingBrowser);

    QtVariantProperty *prop = m_propertyManager->variantProperty(property);
    int role = m_propertyToRole.value(prop);
    if (role == ItemFlagsShadowRole)
        prop->setValue(QVariant::fromValue(defaultItemFlags()));
    else
        prop->setValue(QVariant(QMetaType(prop->valueType()), nullptr));
    prop->setModified(false);

    setItemData(role, QVariant());
    if (role == Qt::DecorationPropertyRole)
        setItemData(Qt::DecorationRole, QVariant::fromValue(QIcon()));
    if (role == Qt::DisplayPropertyRole)
        setItemData(Qt::EditRole, QVariant::fromValue(QString()));
    if (role == Qt::ToolTipPropertyRole)
        setItemData(Qt::ToolTipRole, QVariant::fromValue(QString()));
    if (role == Qt::StatusTipPropertyRole)
        setItemData(Qt::StatusTipRole, QVariant::fromValue(QString()));
    if (role == Qt::WhatsThisPropertyRole)
        setItemData(Qt::WhatsThisRole, QVariant::fromValue(QString()));
}

void AbstractItemEditor::cacheReloaded()
{
    BoolBlocker block(m_updatingBrowser);
    m_propertyManager->reloadResourceProperties();
}

void AbstractItemEditor::updateBrowser()
{
    BoolBlocker block(m_updatingBrowser);
    for (QtVariantProperty *prop : std::as_const(m_properties)) {
        int role = m_propertyToRole.value(prop);
        QVariant val = getItemData(role);

        bool modified = false;
        if (!val.isValid()) {
            if (role == ItemFlagsShadowRole)
                val = QVariant::fromValue(defaultItemFlags());
            else
                val = QVariant(QMetaType(prop->value().userType()), nullptr);
        } else {
            modified = role != Qt::TextAlignmentRole
                || val.toUInt() != DesignerPropertyManager::alignDefault(prop);
        }
        prop->setModified(modified);
        prop->setValue(val);
    }

    if (m_propertyBrowser->topLevelItems().isEmpty()) {
        for (QtVariantProperty *prop : std::as_const(m_rootProperties))
            m_propertyBrowser->addProperty(prop);
    }
}

void AbstractItemEditor::injectPropertyBrowser(QWidget *parent, QWidget *widget)
{
    // It is impossible to design a splitter with just one widget, so we do it by hand.
    m_propertySplitter = new QSplitter;
    m_propertySplitter->addWidget(widget);
    m_propertySplitter->addWidget(m_propertyBrowser);
    m_propertySplitter->setStretchFactor(0, 1);
    m_propertySplitter->setStretchFactor(1, 0);
    parent->layout()->addWidget(m_propertySplitter);
}

////////////////// List editor ///////////////
ItemListEditor::ItemListEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : AbstractItemEditor(form, parent),
      m_updating(false)
{
    ui.setupUi(this);

    injectPropertyBrowser(this, ui.widget);
    connect(ui.showPropertiesButton, &QAbstractButton::clicked,
            this, &ItemListEditor::togglePropertyBrowser);

    connect(ui.newListItemButton, &QAbstractButton::clicked,
            this, &ItemListEditor::newListItemButtonClicked);
    connect(ui.deleteListItemButton, &QAbstractButton::clicked,
            this, &ItemListEditor::deleteListItemButtonClicked);
    connect(ui.moveListItemUpButton, &QAbstractButton::clicked,
            this, &ItemListEditor::moveListItemUpButtonClicked);
    connect(ui.moveListItemDownButton, &QAbstractButton::clicked,
            this, &ItemListEditor::moveListItemDownButtonClicked);
    connect(ui.listWidget, &QListWidget::currentRowChanged,
            this, &ItemListEditor::listWidgetCurrentRowChanged);
    connect(ui.listWidget, &QListWidget::itemChanged,
            this, &ItemListEditor::listWidgetItemChanged);

    setPropertyBrowserVisible(false);

    QIcon upIcon = createIconSet("up.png"_L1);
    QIcon downIcon = createIconSet("down.png"_L1);
    QIcon minusIcon = createIconSet("minus.png"_L1);
    QIcon plusIcon = createIconSet("plus.png"_L1);
    ui.moveListItemUpButton->setIcon(upIcon);
    ui.moveListItemDownButton->setIcon(downIcon);
    ui.newListItemButton->setIcon(plusIcon);
    ui.deleteListItemButton->setIcon(minusIcon);

    connect(iconCache(), &DesignerIconCache::reloaded, this, &AbstractItemEditor::cacheReloaded);
}

void ItemListEditor::setupEditor(QWidget *object,
                                 const PropertyDefinition *propList,
                                 Qt::Alignment alignDefault)
{
    AbstractItemEditor::setupEditor(object, propList, alignDefault);

    if (ui.listWidget->count() > 0)
        ui.listWidget->setCurrentRow(0);
    else
        updateEditor();
}

void ItemListEditor::setCurrentIndex(int idx)
{
    m_updating = true;
    ui.listWidget->setCurrentRow(idx);
    m_updating = false;
}

void ItemListEditor::newListItemButtonClicked()
{
    int row = ui.listWidget->currentRow() + 1;

    QListWidgetItem *item = new QListWidgetItem(m_newItemText);
    item->setData(Qt::DisplayPropertyRole, QVariant::fromValue(PropertySheetStringValue(m_newItemText)));
    if (m_alignDefault != 0)
        item->setTextAlignment(Qt::Alignment(m_alignDefault));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    if (row < ui.listWidget->count())
        ui.listWidget->insertItem(row, item);
    else
        ui.listWidget->addItem(item);
    emit itemInserted(row);

    ui.listWidget->setCurrentItem(item);
    ui.listWidget->editItem(item);
}

void ItemListEditor::deleteListItemButtonClicked()
{
    int row = ui.listWidget->currentRow();

    if (row != -1) {
        delete ui.listWidget->takeItem(row);
        emit itemDeleted(row);
    }

    if (row == ui.listWidget->count())
        row--;
    if (row < 0)
        updateEditor();
    else
        ui.listWidget->setCurrentRow(row);
}

void ItemListEditor::moveListItemUpButtonClicked()
{
    int row = ui.listWidget->currentRow();
    if (row <= 0)
        return; // nothing to do

    ui.listWidget->insertItem(row - 1, ui.listWidget->takeItem(row));
    ui.listWidget->setCurrentRow(row - 1);
    emit itemMovedUp(row);
}

void ItemListEditor::moveListItemDownButtonClicked()
{
    int row = ui.listWidget->currentRow();
    if (row == -1 || row == ui.listWidget->count() - 1)
        return; // nothing to do

    ui.listWidget->insertItem(row + 1, ui.listWidget->takeItem(row));
    ui.listWidget->setCurrentRow(row + 1);
    emit itemMovedDown(row);
}

void ItemListEditor::listWidgetCurrentRowChanged()
{
    updateEditor();
    if (!m_updating)
        emit indexChanged(ui.listWidget->currentRow());
}

void ItemListEditor::listWidgetItemChanged(QListWidgetItem *item)
{
    if (m_updatingBrowser)
        return;

    PropertySheetStringValue val = qvariant_cast<PropertySheetStringValue>(item->data(Qt::DisplayPropertyRole));
    val.setValue(item->text());
    BoolBlocker block(m_updatingBrowser);
    item->setData(Qt::DisplayPropertyRole, QVariant::fromValue(val));

    // The checkState could change, too, but if this signal is connected,
    // checkState is not in the list anyway, as we are editing a header item.
    emit itemChanged(ui.listWidget->currentRow(), Qt::DisplayPropertyRole,
                     QVariant::fromValue(val));
    updateBrowser();
}

void ItemListEditor::togglePropertyBrowser()
{
    setPropertyBrowserVisible(!m_propertyBrowser->isVisible());
}

void ItemListEditor::setPropertyBrowserVisible(bool v)
{
    ui.showPropertiesButton->setText(v ? tr("Properties &>>") : tr("Properties &<<"));
    m_propertyBrowser->setVisible(v);
}

void ItemListEditor::setItemData(int role, const QVariant &v)
{
    QListWidgetItem *item = ui.listWidget->currentItem();
    bool reLayout = false;
    if ((role == Qt::EditRole
         && (v.toString().count(u'\n') != item->data(role).toString().count(u'\n')))
        || role == Qt::FontRole) {
            reLayout = true;
    }
    QVariant newValue = v;
    if (role == Qt::FontRole && newValue.metaType().id() == QMetaType::QFont) {
        QFont oldFont = ui.listWidget->font();
        QFont newFont = qvariant_cast<QFont>(newValue).resolve(oldFont);
        newValue = QVariant::fromValue(newFont);
        item->setData(role, QVariant()); // force the right font with the current resolve mask is set (item view bug)
    }
    item->setData(role, newValue);
    if (reLayout)
        ui.listWidget->doItemsLayout();
    emit itemChanged(ui.listWidget->currentRow(), role, newValue);
}

QVariant ItemListEditor::getItemData(int role) const
{
    return ui.listWidget->currentItem()->data(role);
}

int ItemListEditor::defaultItemFlags() const
{
    static const int flags = QListWidgetItem().flags();
    return flags;
}

void ItemListEditor::cacheReloaded()
{
    reloadIconResources(iconCache(), ui.listWidget);
}

void ItemListEditor::updateEditor()
{
    bool currentItemEnabled = false;

    bool moveRowUpEnabled = false;
    bool moveRowDownEnabled = false;

    QListWidgetItem *item = ui.listWidget->currentItem();
    if (item) {
        currentItemEnabled = true;
        int currentRow = ui.listWidget->currentRow();
        if (currentRow > 0)
            moveRowUpEnabled = true;
        if (currentRow < ui.listWidget->count() - 1)
            moveRowDownEnabled = true;
    }

    ui.moveListItemUpButton->setEnabled(moveRowUpEnabled);
    ui.moveListItemDownButton->setEnabled(moveRowDownEnabled);
    ui.deleteListItemButton->setEnabled(currentItemEnabled);

    if (item)
        updateBrowser();
    else
        m_propertyBrowser->clear();
}

uint ItemListEditor::alignDefault() const
{
    return m_alignDefault;
}

void ItemListEditor::setAlignDefault(uint newAlignDefault)
{
    m_alignDefault = newAlignDefault;
}
} // namespace qdesigner_internal

QT_END_NAMESPACE
