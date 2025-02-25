// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "propertyeditor.h"

#include "qttreepropertybrowser_p.h"
#include "qtbuttonpropertybrowser_p.h"
#include "qtvariantproperty_p.h"
#include "designerpropertymanager.h"
#include "qdesigner_propertysheet_p.h"
#include "formwindowbase_p.h"

#include "newdynamicpropertydialog.h"
#include "dynamicpropertysheet.h"
#include "shared_enums_p.h"

// sdk
#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/abstractsettings.h>
// shared
#include <qdesigner_utils_p.h>
#include <qdesigner_propertycommand_p.h>
#include <metadatabase_p.h>
#include <iconloader_p.h>
#include <widgetfactory_p.h>

#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qtoolbutton.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qpainter.h>

#include <QtCore/qdebug.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimezone.h>

enum SettingsView { TreeView, ButtonView };

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto SettingsGroupC = "PropertyEditor"_L1;
static constexpr auto ViewKeyC = "View"_L1;
static constexpr auto ColorKeyC = "Colored"_L1;
static constexpr auto SortedKeyC = "Sorted"_L1;
static constexpr auto ExpansionKeyC = "ExpandedItems"_L1;
static constexpr auto SplitterPositionKeyC = "SplitterPosition"_L1;

// ---------------------------------------------------------------------------------

namespace qdesigner_internal {

// ----------- ElidingLabel
// QLabel does not support text eliding so we need a helper class

class ElidingLabel : public QWidget
{
public:
    explicit ElidingLabel(const QString &text = QString(),
                          QWidget *parent = nullptr) : QWidget(parent), m_text(text)
        { setContentsMargins(3, 2, 3, 2); }

    void setText(const QString &text) {
        m_text = text;
        updateGeometry();
    }
    void setElidemode(Qt::TextElideMode mode) {
        m_mode = mode;
        updateGeometry();
    }

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *e) override;

private:
    QString m_text;
    Qt::TextElideMode m_mode = Qt::ElideRight;
};

QSize ElidingLabel::sizeHint() const
{
    QSize size = fontMetrics().boundingRect(m_text).size();
    size += QSize(contentsMargins().left() + contentsMargins().right(),
                  contentsMargins().top() + contentsMargins().bottom());
    return size;
}

void ElidingLabel::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setPen(QColor(0, 0, 0, 60));
    painter.setBrush(QColor(255, 255, 255, 40));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.setPen(palette().windowText().color());
    painter.drawText(contentsRect(), Qt::AlignLeft,
                     fontMetrics().elidedText(m_text, Qt::ElideRight, width(), 0));
}


// ----------- PropertyEditor::Strings

PropertyEditor::Strings::Strings() :
    m_alignmentProperties{u"alignment"_s,
                          u"layoutLabelAlignment"_s, // QFormLayout
                          u"layoutFormAlignment"_s},
    m_fontProperty(u"font"_s),
    m_qLayoutWidget(u"QLayoutWidget"_s),
    m_designerPrefix(u"QDesigner"_s),
    m_layout(u"Layout"_s),
    m_validationModeAttribute(u"validationMode"_s),
    m_fontAttribute(u"font"_s),
    m_superPaletteAttribute(u"superPalette"_s),
    m_enumNamesAttribute(u"enumNames"_s),
    m_resettableAttribute(u"resettable"_s),
    m_flagsAttribute(u"flags"_s)
{
}

// ----------- PropertyEditor

QDesignerMetaDataBaseItemInterface* PropertyEditor::metaDataBaseItem() const
{
    QObject *o = object();
    if (!o)
        return nullptr;
    QDesignerMetaDataBaseInterface *db = core()->metaDataBase();
    if (!db)
        return nullptr;
    return db->item(o);
}

void PropertyEditor::setupStringProperty(QtVariantProperty *property, bool isMainContainer)
{
    const StringPropertyParameters params = textPropertyValidationMode(core(), m_object, property->propertyName(), isMainContainer);
    // Does a meta DB entry exist - add comment
    const bool hasComment = params.second;
    property->setAttribute(m_strings.m_validationModeAttribute, params.first);
    // assuming comment cannot appear or disappear for the same property in different object instance
    if (!hasComment)
        qDeleteAll(property->subProperties());
}

void PropertyEditor::setupPaletteProperty(QtVariantProperty *property)
{
    QPalette superPalette = QPalette();
    QWidget *currentWidget = qobject_cast<QWidget *>(m_object);
    if (currentWidget) {
        if (currentWidget->isWindow())
            superPalette = QApplication::palette(currentWidget);
        else {
            if (currentWidget->parentWidget())
                superPalette = currentWidget->parentWidget()->palette();
        }
    }
    m_updatingBrowser = true;
    property->setAttribute(m_strings.m_superPaletteAttribute, superPalette);
    m_updatingBrowser = false;
}

static inline QToolButton *createDropDownButton(QAction *defaultAction, QWidget *parent = nullptr)
{
    QToolButton *rc = new QToolButton(parent);
    rc->setDefaultAction(defaultAction);
    rc->setPopupMode(QToolButton::InstantPopup);
    return rc;
}

PropertyEditor::PropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)  :
    QDesignerPropertyEditor(parent, flags),
    m_core(core),
    m_propertyManager(new DesignerPropertyManager(m_core, this)),
    m_stackedWidget(new QStackedWidget),
    m_filterWidget(new QLineEdit),
    m_addDynamicAction(new QAction(createIconSet("plus.png"_L1), tr("Add Dynamic Property..."), this)),
    m_removeDynamicAction(new QAction(createIconSet("minus.png"_L1), tr("Remove Dynamic Property"), this)),
    m_sortingAction(new QAction(createIconSet("sort.png"_L1), tr("Sorting"), this)),
    m_coloringAction(new QAction(createIconSet("color.png"_L1), tr("Color Groups"), this)),
    m_treeAction(new QAction(tr("Tree View"), this)),
    m_buttonAction(new QAction(tr("Drop Down Button View"), this)),
    m_classLabel(new ElidingLabel)
{
    const QColor colors[] = {{255, 230, 191}, {255, 255, 191}, {191, 255, 191},
                             {199, 255, 255}, {234, 191, 255}, {255, 191, 239}};
    const int darknessFactor = 250;
    m_colors.reserve(std::size(colors));
    for (const QColor &c : colors)
        m_colors.append({c, c.darker(darknessFactor)});
    QColor dynamicColor(191, 207, 255);
    QColor layoutColor(255, 191, 191);
    m_dynamicColor = {dynamicColor, dynamicColor.darker(darknessFactor)};
    m_layoutColor = {layoutColor, layoutColor.darker(darknessFactor)};

    updateForegroundBrightness();

    QActionGroup *actionGroup = new QActionGroup(this);

    m_treeAction->setCheckable(true);
    m_treeAction->setIcon(createIconSet("widgets/listview.png"_L1));
    m_buttonAction->setCheckable(true);
    m_buttonAction->setIcon(createIconSet("dropdownbutton.png"_L1));

    actionGroup->addAction(m_treeAction);
    actionGroup->addAction(m_buttonAction);
    connect(actionGroup, &QActionGroup::triggered,
            this, &PropertyEditor::slotViewTriggered);

    // Add actions
    QActionGroup *addDynamicActionGroup = new QActionGroup(this);
    connect(addDynamicActionGroup, &QActionGroup::triggered,
            this, &PropertyEditor::slotAddDynamicProperty);

    QMenu *addDynamicActionMenu = new QMenu(this);
    m_addDynamicAction->setMenu(addDynamicActionMenu);
    m_addDynamicAction->setEnabled(false);
    QAction *addDynamicAction = addDynamicActionGroup->addAction(tr("String..."));
    addDynamicAction->setData(static_cast<int>(QMetaType::QString));
    addDynamicActionMenu->addAction(addDynamicAction);
    addDynamicAction = addDynamicActionGroup->addAction(tr("Bool..."));
    addDynamicAction->setData(static_cast<int>(QMetaType::Bool));
    addDynamicActionMenu->addAction(addDynamicAction);
    addDynamicActionMenu->addSeparator();
    addDynamicAction = addDynamicActionGroup->addAction(tr("Other..."));
    addDynamicAction->setData(static_cast<int>(QMetaType::UnknownType));
    addDynamicActionMenu->addAction(addDynamicAction);
    // remove
    m_removeDynamicAction->setEnabled(false);
    connect(m_removeDynamicAction, &QAction::triggered, this, &PropertyEditor::slotRemoveDynamicProperty);
    // Configure
    QAction *configureAction = new QAction(tr("Configure Property Editor"), this);
    configureAction->setIcon(createIconSet("configure.png"_L1));
    QMenu *configureMenu = new QMenu(this);
    configureAction->setMenu(configureMenu);

    m_sortingAction->setCheckable(true);
    connect(m_sortingAction, &QAction::toggled, this, &PropertyEditor::slotSorting);

    m_coloringAction->setCheckable(true);
    connect(m_coloringAction, &QAction::toggled, this, &PropertyEditor::slotColoring);

    configureMenu->addAction(m_sortingAction);
    configureMenu->addAction(m_coloringAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_treeAction);
    configureMenu->addAction(m_buttonAction);
    // Assemble toolbar
    QToolBar *toolBar = new QToolBar;
    toolBar->addWidget(m_filterWidget);
    toolBar->addWidget(createDropDownButton(m_addDynamicAction));
    toolBar->addAction(m_removeDynamicAction);
    toolBar->addWidget(createDropDownButton(configureAction));
    // Views
    QScrollArea *buttonScroll = new QScrollArea(m_stackedWidget);
    m_buttonBrowser = new QtButtonPropertyBrowser(buttonScroll);
    buttonScroll->setWidgetResizable(true);
    buttonScroll->setWidget(m_buttonBrowser);
    m_buttonIndex = m_stackedWidget->addWidget(buttonScroll);
    connect(m_buttonBrowser, &QtAbstractPropertyBrowser::currentItemChanged,
            this, &PropertyEditor::slotCurrentItemChanged);

    m_treeBrowser = new QtTreePropertyBrowser(m_stackedWidget);
    m_treeBrowser->setRootIsDecorated(false);
    m_treeBrowser->setPropertiesWithoutValueMarked(true);
    m_treeBrowser->setResizeMode(QtTreePropertyBrowser::Interactive);
    m_treeIndex = m_stackedWidget->addWidget(m_treeBrowser);
    connect(m_treeBrowser, &QtAbstractPropertyBrowser::currentItemChanged,
            this, &PropertyEditor::slotCurrentItemChanged);
    m_filterWidget->setPlaceholderText(tr("Filter"));
    m_filterWidget->setClearButtonEnabled(true);
    connect(m_filterWidget, &QLineEdit::textChanged, this, &PropertyEditor::setFilter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(toolBar);
    layout->addWidget(m_classLabel);
    layout->addSpacerItem(new QSpacerItem(0,1));
    layout->addWidget(m_stackedWidget);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    m_treeFactory = new DesignerEditorFactory(m_core, this);
    m_treeFactory->setSpacing(0);
    m_groupFactory = new DesignerEditorFactory(m_core, this);
    QtVariantPropertyManager *variantManager = m_propertyManager;
    m_buttonBrowser->setFactoryForManager(variantManager, m_groupFactory);
    m_treeBrowser->setFactoryForManager(variantManager, m_treeFactory);

    m_stackedWidget->setCurrentIndex(m_treeIndex);
    m_currentBrowser = m_treeBrowser;
    m_treeAction->setChecked(true);

    connect(m_groupFactory, &DesignerEditorFactory::resetProperty,
            this, &PropertyEditor::slotResetProperty);
    connect(m_treeFactory, &DesignerEditorFactory::resetProperty,
            this, &PropertyEditor::slotResetProperty);
    connect(m_propertyManager, &DesignerPropertyManager::valueChanged,
            this, &PropertyEditor::slotValueChanged);

    // retrieve initial settings
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(SettingsGroupC);
    const SettingsView view = settings->value(ViewKeyC, TreeView).toInt() == TreeView ? TreeView :  ButtonView;
    // Coloring not available unless treeview and not sorted
    m_sorting = settings->value(SortedKeyC, false).toBool();
    m_coloring = settings->value(ColorKeyC, true).toBool();
    const QVariantMap expansionState = settings->value(ExpansionKeyC, QVariantMap()).toMap();
    const int splitterPosition = settings->value(SplitterPositionKeyC, 150).toInt();
    settings->endGroup();
    // Apply settings
    m_sortingAction->setChecked(m_sorting);
    m_coloringAction->setChecked(m_coloring);
    m_treeBrowser->setSplitterPosition(splitterPosition);
    switch (view) {
    case TreeView:
        m_currentBrowser = m_treeBrowser;
        m_stackedWidget->setCurrentIndex(m_treeIndex);
        m_treeAction->setChecked(true);
        break;
    case ButtonView:
        m_currentBrowser = m_buttonBrowser;
        m_stackedWidget->setCurrentIndex(m_buttonIndex);
        m_buttonAction->setChecked(true);
        break;
    }
    // Restore expansionState from QVariant map
    for (auto it = expansionState.cbegin(), cend = expansionState.cend(); it != cend; ++it)
        m_expansionState.insert(it.key(), it.value().toBool());

    updateActionsState();
}

PropertyEditor::~PropertyEditor()
{
    // Prevent emission of QtTreePropertyBrowser::itemChanged() when deleting
    // the current item, causing asserts.
    m_treeBrowser->setCurrentItem(nullptr);
    storeExpansionState();
    saveSettings();
}

void PropertyEditor::saveSettings() const
{
    QDesignerSettingsInterface *settings = m_core->settingsManager();
    settings->beginGroup(SettingsGroupC);
    settings->setValue(ViewKeyC, QVariant(m_treeAction->isChecked() ? TreeView : ButtonView));
    settings->setValue(ColorKeyC, QVariant(m_coloring));
    settings->setValue(SortedKeyC, QVariant(m_sorting));
    // Save last expansionState as QVariant map
    QVariantMap expansionState;
    for (auto it = m_expansionState.cbegin(), cend = m_expansionState.cend(); it != cend; ++it)
        expansionState.insert(it.key(), QVariant(it.value()));
    settings->setValue(ExpansionKeyC, expansionState);
    settings->setValue(SplitterPositionKeyC, m_treeBrowser->splitterPosition());
    settings->endGroup();
}

void PropertyEditor::setExpanded(QtBrowserItem *item, bool expanded)
{
    if (m_buttonBrowser == m_currentBrowser)
        m_buttonBrowser->setExpanded(item, expanded);
    else if (m_treeBrowser == m_currentBrowser)
        m_treeBrowser->setExpanded(item, expanded);
}

bool PropertyEditor::isExpanded(QtBrowserItem *item) const
{
    if (m_buttonBrowser == m_currentBrowser)
        return m_buttonBrowser->isExpanded(item);
    if (m_treeBrowser == m_currentBrowser)
        return m_treeBrowser->isExpanded(item);
    return false;
}

void PropertyEditor::setItemVisible(QtBrowserItem *item, bool visible)
{
    if (m_currentBrowser == m_treeBrowser) {
        m_treeBrowser->setItemVisible(item, visible);
    } else {
        qWarning("** WARNING %s is not implemented for this browser.", Q_FUNC_INFO);
    }
}

bool PropertyEditor::isItemVisible(QtBrowserItem *item) const
{
    return m_currentBrowser == m_treeBrowser ? m_treeBrowser->isItemVisible(item) : true;
}

/* Default handling of items not found in the map:
 * - Top-level items (classes) are assumed to be expanded
 * - Anything below (properties) is assumed to be collapsed
 * That is, the map is required, the state cannot be stored in a set */

void PropertyEditor::storePropertiesExpansionState(const QList<QtBrowserItem *> &items)
{
    for (QtBrowserItem *propertyItem : items) {
        if (!propertyItem->children().isEmpty()) {
            QtProperty *property = propertyItem->property();
            const QString propertyName = property->propertyName();
            const auto itGroup = m_propertyToGroup.constFind(property);
            if (itGroup != m_propertyToGroup.constEnd()) {
                const QString key = itGroup.value() + u'|' + propertyName;
                m_expansionState[key] = isExpanded(propertyItem);
            }
        }
    }
}

void PropertyEditor::storeExpansionState()
{
    const auto items = m_currentBrowser->topLevelItems();
    if (m_sorting) {
        storePropertiesExpansionState(items);
    } else {
        for (QtBrowserItem *item : items) {
            const QString groupName = item->property()->propertyName();
            auto propertyItems = item->children();
            if (!propertyItems.isEmpty())
                m_expansionState[groupName] = isExpanded(item);

            // properties stuff here
            storePropertiesExpansionState(propertyItems);
        }
    }
}

void PropertyEditor::collapseAll()
{
    const auto items = m_currentBrowser->topLevelItems();
    for (QtBrowserItem *group : items)
        setExpanded(group, false);
}

void PropertyEditor::applyPropertiesExpansionState(const QList<QtBrowserItem *> &items)
{
    for (QtBrowserItem *propertyItem : items) {
        const auto excend = m_expansionState.cend();
        QtProperty *property = propertyItem->property();
        const QString propertyName = property->propertyName();
        const auto itGroup = m_propertyToGroup.constFind(property);
        if (itGroup != m_propertyToGroup.constEnd()) {
            const QString key = itGroup.value() + u'|' + propertyName;
            const auto pit = m_expansionState.constFind(key);
            if (pit != excend)
                setExpanded(propertyItem, pit.value());
            else
                setExpanded(propertyItem, false);
        }
    }
}

void PropertyEditor::applyExpansionState()
{
    const auto items = m_currentBrowser->topLevelItems();
    if (m_sorting) {
        applyPropertiesExpansionState(items);
    } else {
        const auto excend = m_expansionState.cend();
        for (QtBrowserItem *item : items) {
            const QString groupName = item->property()->propertyName();
            const auto git = m_expansionState.constFind(groupName);
            if (git != excend)
                setExpanded(item, git.value());
            else
                setExpanded(item, true);
            // properties stuff here
            applyPropertiesExpansionState(item->children());
        }
    }
}

int PropertyEditor::applyPropertiesFilter(const QList<QtBrowserItem *> &items)
{
    int showCount = 0;
    const bool matchAll = m_filterPattern.isEmpty();
    for (QtBrowserItem *propertyItem : items) {
        QtProperty *property = propertyItem->property();
        const QString propertyName = property->propertyName();
        const bool showProperty = matchAll || propertyName.contains(m_filterPattern, Qt::CaseInsensitive);
        setItemVisible(propertyItem, showProperty);
        if (showProperty)
            showCount++;
    }
    return showCount;
}

void PropertyEditor::applyFilter()
{
    const auto items = m_currentBrowser->topLevelItems();
    if (m_sorting) {
        applyPropertiesFilter(items);
    } else {
        for (QtBrowserItem *item : items)
            setItemVisible(item, applyPropertiesFilter(item->children()));
    }
}

void PropertyEditor::clearView()
{
    m_currentBrowser->clear();
}

bool PropertyEditor::event(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange)
        updateForegroundBrightness();

    return QDesignerPropertyEditor::event(event);
}

void PropertyEditor::updateForegroundBrightness()
{
    QColor c = palette().color(QPalette::Text);
    bool newBrightness = qRound(0.3 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF());

    if (m_brightness == newBrightness)
        return;

    m_brightness = newBrightness;

    updateColors();
}

QColor PropertyEditor::propertyColor(QtProperty *property) const
{
    if (!m_coloring)
        return QColor();

    QtProperty *groupProperty = property;

    const auto itProp = m_propertyToGroup.constFind(property);
    if (itProp != m_propertyToGroup.constEnd())
        groupProperty = m_nameToGroup.value(itProp.value());

    const int groupIdx = m_groups.indexOf(groupProperty);
    std::pair<QColor, QColor> pair;
    if (groupIdx != -1) {
        if (groupProperty == m_dynamicGroup)
            pair = m_dynamicColor;
        else if (isLayoutGroup(groupProperty))
            pair = m_layoutColor;
        else
            pair = m_colors[groupIdx % m_colors.size()];
    }
    if (!m_brightness)
        return pair.first;
    return pair.second;
}

void PropertyEditor::fillView()
{
    if (m_sorting) {
        for (auto itProperty = m_nameToProperty.cbegin(), end = m_nameToProperty.cend(); itProperty != end; ++itProperty)
            m_currentBrowser->addProperty(itProperty.value());
    } else {
        for (QtProperty *group : std::as_const(m_groups)) {
            QtBrowserItem *item = m_currentBrowser->addProperty(group);
            if (m_currentBrowser == m_treeBrowser)
                m_treeBrowser->setBackgroundColor(item, propertyColor(group));
            group->setModified(m_currentBrowser == m_treeBrowser);
        }
    }
}

bool PropertyEditor::isLayoutGroup(QtProperty *group) const
{
   return group->propertyName() == m_strings.m_layout;
}

void PropertyEditor::updateActionsState()
{
    m_coloringAction->setEnabled(m_treeAction->isChecked() && !m_sortingAction->isChecked());
}

void PropertyEditor::slotViewTriggered(QAction *action)
{
    storeExpansionState();
    collapseAll();
    {
        UpdateBlocker ub(this);
        clearView();
        int idx = 0;
        if (action == m_treeAction) {
            m_currentBrowser = m_treeBrowser;
            idx = m_treeIndex;
        } else if (action == m_buttonAction) {
            m_currentBrowser = m_buttonBrowser;
            idx = m_buttonIndex;
        }
        fillView();
        m_stackedWidget->setCurrentIndex(idx);
        applyExpansionState();
        applyFilter();
    }
    updateActionsState();
}

void PropertyEditor::slotSorting(bool sort)
{
    if (sort == m_sorting)
        return;

    storeExpansionState();
    m_sorting = sort;
    collapseAll();
    {
        UpdateBlocker ub(this);
        clearView();
        m_treeBrowser->setRootIsDecorated(sort);
        fillView();
        applyExpansionState();
        applyFilter();
    }
    updateActionsState();
}

void PropertyEditor::updateColors()
{
    if (m_treeBrowser && m_currentBrowser == m_treeBrowser) {
        const auto items = m_treeBrowser->topLevelItems();
        for (QtBrowserItem *item : items)
            m_treeBrowser->setBackgroundColor(item, propertyColor(item->property()));
    }
}

void PropertyEditor::slotColoring(bool coloring)
{
    if (coloring == m_coloring)
        return;

    m_coloring = coloring;

    updateColors();
}

void PropertyEditor::slotAddDynamicProperty(QAction *action)
{
    if (!m_propertySheet)
        return;

    const QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension*>(m_core->extensionManager(), m_object);

    if (!dynamicSheet)
        return;

    QString newName;
    QVariant newValue;
    { // Make sure the dialog is closed before the signal is emitted.
        const int  type = action->data().toInt();
        NewDynamicPropertyDialog dlg(core()->dialogGui(), m_currentBrowser);
        if (type != QMetaType::UnknownType)
            dlg.setPropertyType(type);

        QStringList reservedNames;
        const int propertyCount = m_propertySheet->count();
        for (int i = 0; i < propertyCount; i++) {
            if (!dynamicSheet->isDynamicProperty(i) || m_propertySheet->isVisible(i))
                reservedNames.append(m_propertySheet->propertyName(i));
        }
        dlg.setReservedNames(reservedNames);
        if (dlg.exec() == QDialog::Rejected)
            return;
        newName = dlg.propertyName();
        newValue = dlg.propertyValue();
    }
    m_recentlyAddedDynamicProperty = newName;
    emit addDynamicProperty(newName, newValue);
}

QDesignerFormEditorInterface *PropertyEditor::core() const
{
    return m_core;
}

bool PropertyEditor::isReadOnly() const
{
    return false;
}

void PropertyEditor::setReadOnly(bool /*readOnly*/)
{
    qDebug() << "PropertyEditor::setReadOnly() request";
}

void PropertyEditor::setPropertyValue(const QString &name, const QVariant &value, bool changed)
{
    const auto it = m_nameToProperty.constFind(name);
    if (it == m_nameToProperty.constEnd())
        return;
    QtVariantProperty *property = it.value();
    updateBrowserValue(property, value);
    property->setModified(changed);
}

/* Quick update that assumes the actual count of properties has not changed
 * N/A when for example executing a layout command and margin properties appear. */
void PropertyEditor::updatePropertySheet()
{
    if (!m_propertySheet)
        return;

    updateToolBarLabel();

    const int propertyCount = m_propertySheet->count();
    const auto npcend = m_nameToProperty.cend();
    for (int i = 0; i < propertyCount; ++i) {
        const QString propertyName = m_propertySheet->propertyName(i);
        const auto it = m_nameToProperty.constFind(propertyName);
        if (it != npcend)
            updateBrowserValue(it.value(), m_propertySheet->property(i));
    }
}

static inline QLayout *layoutOfQLayoutWidget(QObject *o)
{
    if (o->isWidgetType() && !qstrcmp(o->metaObject()->className(), "QLayoutWidget"))
        return static_cast<QWidget*>(o)->layout();
    return nullptr;
}

void PropertyEditor::updateToolBarLabel()
{
    QString objectName;
    QString className;
    if (m_object) {
        if (QLayout *l = layoutOfQLayoutWidget(m_object))
            objectName = l->objectName();
        else
            objectName = m_object->objectName();
        className = realClassName(m_object);
    }

    m_classLabel->setVisible(!objectName.isEmpty() || !className.isEmpty());
    m_classLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QString classLabelText;
    if (!objectName.isEmpty())
        classLabelText += objectName + " : "_L1;
    classLabelText += className;

    m_classLabel->setText(classLabelText);
    m_classLabel->setToolTip(tr("Object: %1\nClass: %2")
                             .arg(objectName, className));
}

void PropertyEditor::updateBrowserValue(QtVariantProperty *property, const QVariant &value)
{
    QVariant v = value;
    const int type = property->propertyType();
    if (type == QtVariantPropertyManager::enumTypeId()) {
        const PropertySheetEnumValue e = qvariant_cast<PropertySheetEnumValue>(v);
        v = e.metaEnum.keys().indexOf(e.metaEnum.valueToKey(e.value));
    } else if (type == DesignerPropertyManager::designerFlagTypeId()) {
        const PropertySheetFlagValue f = qvariant_cast<PropertySheetFlagValue>(v);
        v = QVariant(f.value);
    } else if (type == DesignerPropertyManager::designerAlignmentTypeId()) {
        const PropertySheetFlagValue f = qvariant_cast<PropertySheetFlagValue>(v);
        v = QVariant(f.value);
    }
    QDesignerPropertySheet *sheet = qobject_cast<QDesignerPropertySheet*>(m_core->extensionManager()->extension(m_object, Q_TYPEID(QDesignerPropertySheetExtension)));
    int index = -1;
    if (sheet)
        index = sheet->indexOf(property->propertyName());
    if (sheet && m_propertyToGroup.contains(property)) { // don't do it for comments since property sheet doesn't keep them
        property->setEnabled(sheet->isEnabled(index));
    }

    // Rich text string property with comment: Store/Update the font the rich text editor dialog starts out with
    if (type == QMetaType::QString && !property->subProperties().isEmpty()) {
        const int fontIndex = m_propertySheet->indexOf(m_strings.m_fontProperty);
        if (fontIndex != -1)
            property->setAttribute(m_strings.m_fontAttribute, m_propertySheet->property(fontIndex));
    }

    m_updatingBrowser = true;
    property->setValue(v);
    if (sheet && sheet->isResourceProperty(index))
        property->setAttribute(u"defaultResource"_s, sheet->defaultResourceProperty(index));
    m_updatingBrowser = false;
}

int PropertyEditor::toBrowserType(const QVariant &value, const QString &propertyName) const
{
    if (value.canConvert<PropertySheetFlagValue>()) {
        if (m_strings.m_alignmentProperties.contains(propertyName))
            return DesignerPropertyManager::designerAlignmentTypeId();
        return DesignerPropertyManager::designerFlagTypeId();
    }
    if (value.canConvert<PropertySheetEnumValue>())
        return DesignerPropertyManager::enumTypeId();

    return value.userType();
}

QString PropertyEditor::realClassName(QObject *object) const
{
    if (!object)
        return QString();

    QString className = QLatin1StringView(object->metaObject()->className());
    const QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
    if (QDesignerWidgetDataBaseItemInterface *widgetItem = db->item(db->indexOfObject(object, true))) {
        className = widgetItem->name();

        if (object->isWidgetType() && className == m_strings.m_qLayoutWidget
                && static_cast<QWidget*>(object)->layout()) {
            className = QLatin1StringView(static_cast<QWidget*>(object)->layout()->metaObject()->className());
        }
    }

    if (className.startsWith(m_strings.m_designerPrefix))
        className.remove(1, m_strings.m_designerPrefix.size() - 1);

    return className;
}

static const char *typeName(int type)
{
    if (type == qMetaTypeId<PropertySheetStringValue>())
        type = QMetaType::QString;
    if (type < int(QMetaType::User))
        return QMetaType(type).name();
    if (type == qMetaTypeId<PropertySheetIconValue>())
        return "QIcon";
    if (type == qMetaTypeId<PropertySheetPixmapValue>())
        return "QPixmap";
    if (type == qMetaTypeId<PropertySheetKeySequenceValue>())
        return "QKeySequence";
    if (type == qMetaTypeId<PropertySheetFlagValue>())
        return "QFlags";
    if (type == qMetaTypeId<PropertySheetEnumValue>())
        return "enum";
    if (type == QMetaType::UnknownType)
        return "invalid";
    if (type == QMetaType::User)
        return "user type";
    const auto metaType = QMetaType(type);
    if (metaType.isValid())
        return metaType.name();
    return nullptr;
}

static QString msgUnsupportedType(const QString &propertyName, int type)
{
    QString rc;
    QTextStream str(&rc);
    const char *typeS = typeName(type);
    str << "The property \"" << propertyName << "\" of type ("
        << (typeS ? typeS : "unknown") << ") is not supported yet.";
    return rc;
}

static QString msgDeprecatedProperty(const QLatin1StringView version,
                                     const QString &baseTip)
{
    return PropertyEditor::tr("Deprecated since Qt %1: %2").arg(version, baseTip);
}

static QString basePropertyToolTip(const QString &propertyName, int type)
{
    QString result;
    if (const char *typeS = typeName(type))
        result = propertyName + " ("_L1 + QLatin1StringView(typeS) + u')';
    return result;
}

static QString propertyToolTip(const QDesignerFormEditorInterface *core,
                               const QString &className,
                               const QString &propertyName, int type)
{
    const QDesignerCustomWidgetData customData = core->pluginManager()->customWidgetData(className);
    if (!customData.isNull()) {
        if (QString customToolTip = customData.propertyToolTip(propertyName); !customToolTip.isEmpty())
            return customToolTip;
    }
    const QString base = basePropertyToolTip(propertyName, type);
    // QTBUG-108199, timeSpec deprecation
    if (type == QtVariantPropertyManager::enumTypeId() && propertyName == "timeSpec"_L1)
        return msgDeprecatedProperty("6.9"_L1, base);
    return base;
}

void PropertyEditor::setObject(QObject *object)
{
    QDesignerFormWindowInterface *oldFormWindow = QDesignerFormWindowInterface::findFormWindow(m_object);
    // In the first setObject() call following the addition of a dynamic property, focus and edit it.
    const bool editNewDynamicProperty = object != nullptr && m_object == object && !m_recentlyAddedDynamicProperty.isEmpty();
    m_object = object;
    m_propertyManager->setObject(object);
    QDesignerFormWindowInterface *formWindow = QDesignerFormWindowInterface::findFormWindow(m_object);
    // QTBUG-68507: Form window can be null for objects in Morph Undo macros with buddies
    if (object != nullptr && formWindow == nullptr) {
        formWindow = m_core->formWindowManager()->activeFormWindow();
        if (formWindow == nullptr) {
            qWarning("PropertyEditor::setObject(): Unable to find form window for \"%s\".",
                     qPrintable(object->objectName()));
            return;
        }
    }
    FormWindowBase *fwb = qobject_cast<FormWindowBase *>(formWindow);
    const bool idIdBasedTranslation = fwb && fwb->useIdBasedTranslations();
    const bool idIdBasedTranslationUnchanged = (idIdBasedTranslation == DesignerPropertyManager::useIdBasedTranslations());
    DesignerPropertyManager::setUseIdBasedTranslations(idIdBasedTranslation);
    m_treeFactory->setFormWindowBase(fwb);
    m_groupFactory->setFormWindowBase(fwb);

    storeExpansionState();

    UpdateBlocker ub(this);

    updateToolBarLabel();

    QMap<QString, QtVariantProperty *> toRemove = m_nameToProperty;

    const QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension*>(m_core->extensionManager(), m_object);
    const QDesignerPropertySheet *sheet = qobject_cast<QDesignerPropertySheet*>(m_core->extensionManager()->extension(m_object, Q_TYPEID(QDesignerPropertySheetExtension)));

    // Optimizization: Instead of rebuilding the complete list every time, compile a list of properties to remove,
    // remove them, traverse the sheet, in case property exists just set a value, otherwise - create it.
    QExtensionManager *m = m_core->extensionManager();

    m_propertySheet = qobject_cast<QDesignerPropertySheetExtension*>(m->extension(object, Q_TYPEID(QDesignerPropertySheetExtension)));
    if (m_propertySheet) {
        const int stringTypeId = qMetaTypeId<PropertySheetStringValue>();
        const int propertyCount = m_propertySheet->count();
        for (int i = 0; i < propertyCount; ++i) {
            if (!m_propertySheet->isVisible(i))
                continue;

            const QString propertyName = m_propertySheet->propertyName(i);
            if (m_propertySheet->indexOf(propertyName) != i)
                continue;
            const QString groupName = m_propertySheet->propertyGroup(i);
            const auto rit = toRemove.constFind(propertyName);
            if (rit != toRemove.constEnd()) {
                QtVariantProperty *property = rit.value();
                const int propertyType = property->propertyType();
                // Also remove string properties in case a change in translation mode
                // occurred since different sub-properties are used (disambiguation/id).
                if (m_propertyToGroup.value(property) == groupName
                    && (idIdBasedTranslationUnchanged || propertyType != stringTypeId)
                    && toBrowserType(m_propertySheet->property(i), propertyName) == propertyType) {
                    toRemove.remove(propertyName);
                }
            }
        }
    }

    for (auto itRemove = toRemove.cbegin(), end = toRemove.cend(); itRemove != end; ++itRemove) {
        QtVariantProperty *property = itRemove.value();
        m_nameToProperty.remove(itRemove.key());
        m_propertyToGroup.remove(property);
        delete property;
    }

    if (oldFormWindow != formWindow)
        reloadResourceProperties();

    bool isMainContainer = false;
    if (QWidget *widget = qobject_cast<QWidget*>(object)) {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(widget)) {
            isMainContainer = (fw->mainContainer() == widget);
        }
    }
    m_groups.clear();

    if (m_propertySheet) {
        const QString className = WidgetFactory::classNameOf(formWindow->core(), m_object);

        QtProperty *lastProperty = nullptr;
        QtProperty *lastGroup = nullptr;
        const int propertyCount = m_propertySheet->count();
        for (int i = 0; i < propertyCount; ++i) {
            if (!m_propertySheet->isVisible(i))
                continue;

            const QString propertyName = m_propertySheet->propertyName(i);
            if (m_propertySheet->indexOf(propertyName) != i)
                continue;
            const QVariant value = m_propertySheet->property(i);

            const int type = toBrowserType(value, propertyName);

            QtVariantProperty *property = m_nameToProperty.value(propertyName, 0);
            bool newProperty = property == nullptr;
            if (newProperty) {
                property = m_propertyManager->addProperty(type, propertyName);
                if (property) {
                    newProperty = true;
                    if (type == DesignerPropertyManager::enumTypeId()) {
                        const PropertySheetEnumValue e = qvariant_cast<PropertySheetEnumValue>(value);
                        m_updatingBrowser = true;
                        property->setAttribute(m_strings.m_enumNamesAttribute, e.metaEnum.keys());
                        m_updatingBrowser = false;
                    } else if (type == DesignerPropertyManager::designerFlagTypeId()) {
                        const PropertySheetFlagValue f = qvariant_cast<PropertySheetFlagValue>(value);
                        QList<std::pair<QString, uint>> flags;
                        for (const QString &name : f.metaFlags.keys()) {
                            const uint val = f.metaFlags.keyToValue(name);
                            flags.append({name, val});
                        }
                        m_updatingBrowser = true;
                        QVariant v;
                        v.setValue(flags);
                        property->setAttribute(m_strings.m_flagsAttribute, v);
                        m_updatingBrowser = false;
                    }
                }
            }

            if (property != nullptr) {
                const bool dynamicProperty = (dynamicSheet && dynamicSheet->isDynamicProperty(i))
                            || (sheet && sheet->isDefaultDynamicProperty(i));
                QString descriptionToolTip = dynamicProperty
                        ? basePropertyToolTip(propertyName, type)
                        : propertyToolTip(formWindow->core(), className, propertyName, type);
                if (!descriptionToolTip.isEmpty())
                    property->setDescriptionToolTip(descriptionToolTip);
                switch (type) {
                case QMetaType::QPalette:
                    setupPaletteProperty(property);
                    break;
                case QMetaType::QKeySequence:
                    //addCommentProperty(property, propertyName);
                    break;
                default:
                    break;
                }
                if (type == QMetaType::QString || type == qMetaTypeId<PropertySheetStringValue>())
                    setupStringProperty(property, isMainContainer);
                property->setAttribute(m_strings.m_resettableAttribute, m_propertySheet->hasReset(i));

                const QString groupName = m_propertySheet->propertyGroup(i);
                QtVariantProperty *groupProperty = nullptr;

                if (newProperty) {
                    auto itPrev = m_nameToProperty.insert(propertyName, property);
                    m_propertyToGroup[property] = groupName;
                    if (m_sorting) {
                        QtProperty *previous = nullptr;
                        if (itPrev != m_nameToProperty.begin())
                            previous = (--itPrev).value();
                        m_currentBrowser->insertProperty(property, previous);
                    }
                }
                const auto gnit = m_nameToGroup.constFind(groupName);
                if (gnit != m_nameToGroup.constEnd()) {
                    groupProperty = gnit.value();
                } else {
                    groupProperty = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), groupName);
                    QtBrowserItem *item = nullptr;
                    if (!m_sorting)
                         item = m_currentBrowser->insertProperty(groupProperty, lastGroup);
                    m_nameToGroup[groupName] = groupProperty;
                    m_groups.append(groupProperty);
                    if (dynamicProperty)
                        m_dynamicGroup = groupProperty;
                    if (m_currentBrowser == m_treeBrowser && item) {
                        m_treeBrowser->setBackgroundColor(item, propertyColor(groupProperty));
                        groupProperty->setModified(true);
                    }
                }
                /*  Group changed or new group. Append to last subproperty of
                 * that group. Note that there are cases in which a derived
                 * property sheet appends fake properties for the class
                 * which will appear after the layout group properties
                 * (QWizardPage). To make them appear at the end of the
                 * actual class group, goto last element. */
                if (lastGroup != groupProperty) {
                    lastGroup = groupProperty;
                    lastProperty = nullptr;  // Append at end
                    const auto subProperties = lastGroup->subProperties();
                    if (!subProperties.isEmpty())
                        lastProperty = subProperties.constLast();
                    lastGroup = groupProperty;
                }
                if (!m_groups.contains(groupProperty))
                    m_groups.append(groupProperty);
                if (newProperty)
                    groupProperty->insertSubProperty(property, lastProperty);

                lastProperty = property;

                updateBrowserValue(property, value);

                property->setModified(m_propertySheet->isChanged(i));
                if (propertyName == "geometry"_L1 && type == QMetaType::QRect) {
                    const auto &subProperties = property->subProperties();
                    for (QtProperty *subProperty : subProperties) {
                        const QString subPropertyName = subProperty->propertyName();
                        if (subPropertyName == "X"_L1 || subPropertyName == "Y"_L1)
                            subProperty->setEnabled(!isMainContainer);
                    }
                }
            } else {
                // QTBUG-80417, suppress warning for QDateEdit::timeZone
                const int typeId = value.typeId();
                if (typeId != qMetaTypeId<QTimeZone>())
                    qWarning("%s", qPrintable(msgUnsupportedType(propertyName, type)));
            }
        }
    }
    QMap<QString, QtVariantProperty *> groups = m_nameToGroup;
    for (auto itGroup = groups.cbegin(), end = groups.cend(); itGroup != end; ++itGroup) {
        QtVariantProperty *groupProperty = itGroup.value();
        if (groupProperty->subProperties().isEmpty()) {
            if (groupProperty == m_dynamicGroup)
                m_dynamicGroup = nullptr;
            delete groupProperty;
            m_nameToGroup.remove(itGroup.key());
        }
    }
    const bool addEnabled = dynamicSheet ? dynamicSheet->dynamicPropertiesAllowed() : false;
    m_addDynamicAction->setEnabled(addEnabled);
    m_removeDynamicAction->setEnabled(false);
    applyExpansionState();
    applyFilter();
    // In the first setObject() call following the addition of a dynamic property, focus and edit it.
    if (editNewDynamicProperty) {
        // Have QApplication process the events related to completely closing the modal 'add' dialog,
        // otherwise, we cannot focus the property editor in docked mode.
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        editProperty(m_recentlyAddedDynamicProperty);
    }
    m_recentlyAddedDynamicProperty.clear();
    m_filterWidget->setEnabled(object);
}

void PropertyEditor::reloadResourceProperties()
{
    m_updatingBrowser = true;
    m_propertyManager->reloadResourceProperties();
    m_updatingBrowser = false;
}

QtBrowserItem *PropertyEditor::nonFakePropertyBrowserItem(QtBrowserItem *item) const
{
    // Top-level properties are QObject/QWidget groups, etc. Find first item property below
    // which should be nonfake
    const auto topLevelItems = m_currentBrowser->topLevelItems();
    do {
        if (topLevelItems.contains(item->parent()))
            return item;
        item = item->parent();
    } while (item);
    return nullptr;
}

QString PropertyEditor::currentPropertyName() const
{
    if (QtBrowserItem *browserItem = m_currentBrowser->currentItem())
        if (QtBrowserItem *topLevelItem = nonFakePropertyBrowserItem(browserItem)) {
            return topLevelItem->property()->propertyName();
        }
    return QString();
}

void PropertyEditor::slotResetProperty(QtProperty *property)
{
    QDesignerFormWindowInterface *form = m_core->formWindowManager()->activeFormWindow();
    if (!form)
        return;

    if (m_propertyManager->resetFontSubProperty(property))
        return;

    if (m_propertyManager->resetIconSubProperty(property))
        return;

    if (m_propertyManager->resetTextAlignmentProperty(property))
        return;

    if (!m_propertyToGroup.contains(property))
        return;

    emit resetProperty(property->propertyName());
}

void PropertyEditor::slotValueChanged(QtProperty *property, const QVariant &value, bool enableSubPropertyHandling)
{
    if (m_updatingBrowser)
        return;

    if (!m_propertySheet)
        return;

    QtVariantProperty *varProp = m_propertyManager->variantProperty(property);

    if (!varProp)
        return;

    if (!m_propertyToGroup.contains(property))
        return;

    if (varProp->propertyType() == QtVariantPropertyManager::enumTypeId()) {
        PropertySheetEnumValue e = qvariant_cast<PropertySheetEnumValue>(m_propertySheet->property(m_propertySheet->indexOf(property->propertyName())));
        const int val = value.toInt();
        const QString valName = varProp->attributeValue(m_strings.m_enumNamesAttribute).toStringList().at(val);
        bool ok = false;
        e.value = e.metaEnum.parseEnum(valName, &ok);
        Q_ASSERT(ok);
        QVariant v;
        v.setValue(e);
        emitPropertyValueChanged(property->propertyName(), v, true);
        return;
    }

    emitPropertyValueChanged(property->propertyName(), value, enableSubPropertyHandling);
}

bool PropertyEditor::isDynamicProperty(const QtBrowserItem* item) const
{
    if (!item)
        return false;

    const QDesignerDynamicPropertySheetExtension *dynamicSheet =
            qt_extension<QDesignerDynamicPropertySheetExtension*>(m_core->extensionManager(), m_object);

    if (!dynamicSheet)
        return false;

    return m_propertyToGroup.contains(item->property())
        && dynamicSheet->isDynamicProperty(m_propertySheet->indexOf(item->property()->propertyName()));
}

void PropertyEditor::editProperty(const QString &name)
{
    // find the browser item belonging to the property, make it current and edit it
    QtBrowserItem *browserItem = nullptr;
    if (QtVariantProperty *property = m_nameToProperty.value(name, 0)) {
        const auto items = m_currentBrowser->items(property);
        if (items.size() == 1)
            browserItem = items.constFirst();
    }
    if (browserItem == nullptr)
        return;
    m_currentBrowser->setFocus(Qt::OtherFocusReason);
    if (m_currentBrowser == m_treeBrowser) { // edit is currently only supported in tree view
        m_treeBrowser->editItem(browserItem);
    } else {
        m_currentBrowser->setCurrentItem(browserItem);
    }
}

void PropertyEditor::slotCurrentItemChanged(QtBrowserItem *item)
{
    m_removeDynamicAction->setEnabled(isDynamicProperty(item));

}

void PropertyEditor::slotRemoveDynamicProperty()
{
    if (QtBrowserItem* item = m_currentBrowser->currentItem())
        if (isDynamicProperty(item))
            emit removeDynamicProperty(item->property()->propertyName());
}

void PropertyEditor::setFilter(const QString &pattern)
{
    m_filterPattern = pattern;
    applyFilter();
}
}

QT_END_NAMESPACE
