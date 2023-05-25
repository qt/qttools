// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "widgetdatabase_p.h"
#include "widgetfactory_p.h"
#include "spacer_widget_p.h"
#include "abstractlanguage.h"
#include "pluginmanager_p.h"
#include "qdesigner_widgetbox_p.h"
#include "qdesigner_utils_p.h"
#include <QtDesigner/private/ui4_p.h>

#include <QtDesigner/propertysheet.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractformeditor.h>

#include <QtUiPlugin/customwidget.h>
#include <QtWidgets/QtWidgets>
#ifdef QT_OPENGLWIDGETS_LIB
#include <QtOpenGLWidgets/qopenglwidget.h>
#endif

#include <QtCore/qxmlstream.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qcoreapplication.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
    enum { debugWidgetDataBase = 0 };
}

namespace qdesigner_internal {

using namespace Qt::StringLiterals;

// ----------------------------------------------------------
WidgetDataBaseItem::WidgetDataBaseItem(const QString &name, const QString &group)
    : m_name(name),
      m_group(group),
      m_compat(0),
      m_container(0),
      m_custom(0),
      m_promoted(0)
{
}

QString WidgetDataBaseItem::name() const
{
    return m_name;
}

void WidgetDataBaseItem::setName(const QString &name)
{
    m_name = name;
}

QString WidgetDataBaseItem::group() const
{
    return m_group;
}

void WidgetDataBaseItem::setGroup(const QString &group)
{
    m_group = group;
}

QString WidgetDataBaseItem::toolTip() const
{
    return m_toolTip;
}

void WidgetDataBaseItem::setToolTip(const QString &toolTip)
{
    m_toolTip = toolTip;
}

QString WidgetDataBaseItem::whatsThis() const
{
    return m_whatsThis;
}

void WidgetDataBaseItem::setWhatsThis(const QString &whatsThis)
{
    m_whatsThis = whatsThis;
}

QString WidgetDataBaseItem::includeFile() const
{
    return m_includeFile;
}

void WidgetDataBaseItem::setIncludeFile(const QString &includeFile)
{
    m_includeFile = includeFile;
}

QIcon WidgetDataBaseItem::icon() const
{
    return m_icon;
}

void WidgetDataBaseItem::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

bool WidgetDataBaseItem::isCompat() const
{
    return m_compat;
}

void WidgetDataBaseItem::setCompat(bool b)
{
    m_compat = b;
}

bool WidgetDataBaseItem::isContainer() const
{
    return m_container;
}

void WidgetDataBaseItem::setContainer(bool b)
{
    m_container = b;
}

bool WidgetDataBaseItem::isCustom() const
{
    return m_custom;
}

void WidgetDataBaseItem::setCustom(bool b)
{
    m_custom = b;
}

QString WidgetDataBaseItem::pluginPath() const
{
    return m_pluginPath;
}

void WidgetDataBaseItem::setPluginPath(const QString &path)
{
    m_pluginPath = path;
}

bool WidgetDataBaseItem::isPromoted() const
{
    return m_promoted;
}

void WidgetDataBaseItem::setPromoted(bool b)
{
    m_promoted = b;
}

QString WidgetDataBaseItem::extends() const
{
    return m_extends;
}

void WidgetDataBaseItem::setExtends(const QString &s)
{
    m_extends = s;
}

void WidgetDataBaseItem::setDefaultPropertyValues(const QList<QVariant> &list)
{
    m_defaultPropertyValues = list;
}

QList<QVariant> WidgetDataBaseItem::defaultPropertyValues() const
{
    return m_defaultPropertyValues;
}

QStringList WidgetDataBaseItem::fakeSlots() const
{
    return m_fakeSlots;
}

void WidgetDataBaseItem::setFakeSlots(const QStringList &fs)
{
    m_fakeSlots = fs;
}

QStringList WidgetDataBaseItem::fakeSignals() const
{
     return m_fakeSignals;
}

void WidgetDataBaseItem::setFakeSignals(const QStringList &fs)
{
    m_fakeSignals = fs;
}

QString WidgetDataBaseItem::addPageMethod() const
{
    return m_addPageMethod;
}

void WidgetDataBaseItem::setAddPageMethod(const QString &m)
{
    m_addPageMethod = m;
}

WidgetDataBaseItem *WidgetDataBaseItem::clone(const QDesignerWidgetDataBaseItemInterface *item)
{
    WidgetDataBaseItem *rc = new WidgetDataBaseItem(item->name(), item->group());

    rc->setToolTip(item->toolTip());
    rc->setWhatsThis(item->whatsThis());
    rc->setIncludeFile(item->includeFile());
    rc->setIcon(item->icon());
    rc->setCompat(item->isCompat());
    rc->setContainer(item->isContainer());
    rc->setCustom(item->isCustom() );
    rc->setPluginPath(item->pluginPath());
    rc->setPromoted(item->isPromoted());
    rc->setExtends(item->extends());
    rc->setDefaultPropertyValues(item->defaultPropertyValues());
    // container page method, fake slots and signals ignored here.y
    return rc;
}

QString WidgetDataBaseItem::baseClassName() const
{
    return m_extends.isEmpty() ? m_baseClassName : m_extends;
}

void WidgetDataBaseItem::setBaseClassName(const QString &b)
{
    m_baseClassName = b;
}

static void addWidgetItem(WidgetDataBase *wdb, const char *name, const QMetaObject &mo,
                          const char *comment)
{
    auto *item = new WidgetDataBaseItem(QString::fromUtf8(name));
    if (auto *base = mo.superClass())
        item->setBaseClassName(QString::fromUtf8(base->className()));
    if (comment[0])
        item->setToolTip(QString::fromUtf8(comment));
    wdb->append(item);
}

// ----------------------------------------------------------
WidgetDataBase::WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent)
    : QDesignerWidgetDataBaseInterface(parent),
      m_core(core)
{
#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) DECLARE_WIDGET(W, C)
#define DECLARE_WIDGET(W, C) addWidgetItem(this, #W, W::staticMetaObject, C);

#include <widgets.table>

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

    const QString msgAbstractClass =
        QCoreApplication::translate("WidgetDataBase",
                                    "Abstract base class that cannot be instantiated. For promotion/custom widget usage only.");

#if QT_CONFIG(abstractbutton)
    auto *abItem = new WidgetDataBaseItem(u"QAbstractButton"_s);
    abItem->setToolTip(msgAbstractClass);
    abItem->setBaseClassName(u"QWidget"_s);
    append(abItem);
#endif // QT_CONFIG(abstractbutton)

#if QT_CONFIG(itemviews)
    auto *aivItem = new WidgetDataBaseItem(u"QAbstractItemView"_s);
    aivItem->setBaseClassName(u"QAbstractScrollArea"_s);
    aivItem->setToolTip(msgAbstractClass);
    append(aivItem);
#endif // QT_CONFIG(itemviews)

    append(new WidgetDataBaseItem(u"Line"_s));
    append(new WidgetDataBaseItem(u"Spacer"_s));
    append(new WidgetDataBaseItem(u"QSplitter"_s));
    append(new WidgetDataBaseItem(u"QLayoutWidget"_s));
    // QDesignerWidget is used as central widget and as container for tab widgets, etc.
    WidgetDataBaseItem *designerWidgetItem = new WidgetDataBaseItem(u"QDesignerWidget"_s);
    designerWidgetItem->setContainer(true);
    append(designerWidgetItem);
    append(new WidgetDataBaseItem(u"QDesignerDialog"_s));
    append(new WidgetDataBaseItem(u"QDesignerMenu"_s));
    append(new WidgetDataBaseItem(u"QDesignerMenuBar"_s));
    append(new WidgetDataBaseItem(u"QDesignerDockWidget"_s));
    append(new WidgetDataBaseItem(u"QAction"_s));
    append(new WidgetDataBaseItem(u"QButtonGroup"_s));

    // ### remove me
    // ### check the casts

#if 0 // ### enable me after 4.1
    item(indexOfClassName(u"QToolBar"_s))->setContainer(true);
#endif

    item(indexOfClassName(u"QTabWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QGroupBox"_s))->setContainer(true);
    item(indexOfClassName(u"QScrollArea"_s))->setContainer(true);
    item(indexOfClassName(u"QStackedWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QToolBox"_s))->setContainer(true);
    item(indexOfClassName(u"QFrame"_s))->setContainer(true);
    item(indexOfClassName(u"QLayoutWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QDesignerWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QDesignerDialog"_s))->setContainer(true);
    item(indexOfClassName(u"QSplitter"_s))->setContainer(true);
    item(indexOfClassName(u"QMainWindow"_s))->setContainer(true);
    item(indexOfClassName(u"QDockWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QDesignerDockWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QMdiArea"_s))->setContainer(true);
    item(indexOfClassName(u"QWizard"_s))->setContainer(true);
    item(indexOfClassName(u"QWizardPage"_s))->setContainer(true);

    item(indexOfClassName(u"QWidget"_s))->setContainer(true);
    item(indexOfClassName(u"QDialog"_s))->setContainer(true);
}

WidgetDataBase::~WidgetDataBase() = default;

QDesignerFormEditorInterface *WidgetDataBase::core() const
{
    return m_core;
}

int WidgetDataBase::indexOfObject(QObject *object, bool /*resolveName*/) const
{
    QExtensionManager *mgr = m_core->extensionManager();
    QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*> (mgr, m_core);

    QString id;

    if (lang)
        id = lang->classNameOf(object);

    if (id.isEmpty())
        id = WidgetFactory::classNameOf(m_core,object);

    return QDesignerWidgetDataBaseInterface::indexOfClassName(id);
}

static WidgetDataBaseItem *createCustomWidgetItem(const QDesignerCustomWidgetInterface *c,
                                                  const QDesignerCustomWidgetData &data)
{
    WidgetDataBaseItem *item = new WidgetDataBaseItem(c->name(), c->group());
    item->setContainer(c->isContainer());
    item->setCustom(true);
    item->setIcon(c->icon());
    item->setIncludeFile(c->includeFile());
    item->setToolTip(c->toolTip());
    item->setWhatsThis(c->whatsThis());
    item->setPluginPath(data.pluginPath());
    item->setAddPageMethod(data.xmlAddPageMethod());
    item->setExtends(data.xmlExtends());
    return item;
}

void WidgetDataBase::loadPlugins()
{
    // 1) create a map of existing custom classes
    QMap<QString, qsizetype> existingCustomClasses;
    QSet<QString> nonCustomClasses;
    for (qsizetype i = 0, count = m_items.size(); i < count; ++i)    {
        const QDesignerWidgetDataBaseItemInterface* item =  m_items.at(i);
        if (item->isCustom() && !item->isPromoted())
            existingCustomClasses.insert(item->name(), i);
        else
            nonCustomClasses.insert(item->name());
    }
    // 2) create a list plugins
    QList<QDesignerWidgetDataBaseItemInterface *> pluginList;
    const QDesignerPluginManager *pm = m_core->pluginManager();
    const auto &customWidgets = pm->registeredCustomWidgets();
    for (QDesignerCustomWidgetInterface* c : customWidgets)
        pluginList += createCustomWidgetItem(c, pm->customWidgetData(c));

    // 3) replace custom classes or add new ones, remove them from existingCustomClasses,
    // leaving behind deleted items
    unsigned replacedPlugins = 0;
    unsigned addedPlugins = 0;
    unsigned removedPlugins = 0;
    if (!pluginList.isEmpty()) {
        for (QDesignerWidgetDataBaseItemInterface *pluginItem : std::as_const(pluginList)) {
            const QString pluginName = pluginItem->name();
            const auto existingIt = existingCustomClasses.constFind(pluginName);
            if (existingIt == existingCustomClasses.cend()) {
                // Add new class.
                if (nonCustomClasses.contains(pluginName)) {
                    designerWarning(tr("A custom widget plugin whose class name (%1) matches that of an existing class has been found.").arg(pluginName));
                } else {
                    append(pluginItem);
                    addedPlugins++;
                }
            } else {
                // replace existing info
                const auto existingIndex = existingIt.value();
                delete m_items[existingIndex];
                m_items[existingIndex] = pluginItem;
                existingCustomClasses.erase(existingIt);
                replacedPlugins++;

            }
        }
    }
    // 4) remove classes that have not been matched. The stored indexes become invalid while deleting.
    for (auto it = existingCustomClasses.cbegin(), cend = existingCustomClasses.cend(); it != cend; ++it )  {
        const int index = indexOfClassName(it.key());
        if (index != -1) {
            remove(index);
            removedPlugins++;
        }
    }

    if (debugWidgetDataBase)
        qDebug() << "WidgetDataBase::loadPlugins(): " << addedPlugins << " added, " << replacedPlugins << " replaced, " << removedPlugins << "deleted.";
}

void WidgetDataBase::remove(int index)
{
    Q_ASSERT(index < m_items.size());
    delete m_items.takeAt(index);
}

QList<QVariant> WidgetDataBase::defaultPropertyValues(const QString &name)
{
    WidgetFactory *factory = qobject_cast<WidgetFactory *>(m_core->widgetFactory());
    Q_ASSERT(factory);
    // Create non-widgets, widgets in order
    QObject* object = factory->createObject(name, nullptr);
    if (!object)
        object = factory->createWidget(name, nullptr);
    if (!object) {
        qDebug() << "** WARNING Factory failed to create " << name;
        return {};
    }
    // Get properties from sheet.
    QVariantList result;
    if (const QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_core->extensionManager(), object)) {
        const int propertyCount = sheet->count();
        for (int i = 0; i < propertyCount; ++i) {
            result.append(sheet->property(i));
        }
    }
    delete object;
    return result;
}

void WidgetDataBase::grabDefaultPropertyValues()
{
    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i) {
        QDesignerWidgetDataBaseItemInterface *dbItem = item(i);
        const auto default_prop_values = defaultPropertyValues(dbItem->name());
        dbItem->setDefaultPropertyValues(default_prop_values);
    }
}

void WidgetDataBase::grabStandardWidgetBoxIcons()
{
    // At this point, grab the default icons for the non-custom widgets from
    // the widget box. They will show up in the object inspector.
    if (const QDesignerWidgetBox *wb = qobject_cast<const QDesignerWidgetBox *>(m_core->widgetBox())) {
        const int itemCount = count();
        for (int i = 0; i < itemCount; ++i) {
            QDesignerWidgetDataBaseItemInterface *dbItem = item(i);
            if (!dbItem->isCustom() && dbItem->icon().isNull()) {
                // Careful not to catch the layout icons when looking for
                // QWidget
                const QString name = dbItem->name();
                if (name == "QWidget"_L1) {
                    dbItem->setIcon(wb->iconForWidget(name, u"Containers"_s));
                } else {
                    dbItem->setIcon(wb->iconForWidget(name));
                }
            }
        }
    }
}

// --------------------- Functions relevant generation of new forms based on widgets (apart from the standard templates)

enum { NewFormWidth = 400, NewFormHeight = 300 };

// Check if class is suitable to generate a form from
static inline bool isExistingTemplate(const QString &className)
{
    return className == "QWidget"_L1 || className == "QDialog"_L1 || className == "QMainWindow"_L1;
}

// Check if class is suitable to generate a form from
static inline bool suitableForNewForm(const QString &className)
{
    if (className.isEmpty()) // Missing custom widget information
        return false;
    if (className == "QSplitter"_L1)
         return false;
    if (className.startsWith("QDesigner"_L1) ||  className.startsWith("QLayout"_L1))
        return false;
    return true;
}

// Return a list of widget classes from which new forms can be generated.
// Suitable for 'New form' wizards in integrations.
QStringList WidgetDataBase::formWidgetClasses(const QDesignerFormEditorInterface *core)
{
    static QStringList rc;
    if (rc.isEmpty()) {
        const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
        const int widgetCount = wdb->count();
        for (int i = 0; i < widgetCount; i++) {
            const QDesignerWidgetDataBaseItemInterface *item = wdb->item(i);
            if (item->isContainer() && !item->isCustom() && !item->isPromoted()) {
                const QString name = item->name(); // Standard Widgets: no existing templates
                if (!isExistingTemplate(name) && suitableForNewForm(name))
                    rc += name;
            }
        }
    }
    return rc;
}

// Return a list of custom widget classes from which new forms can be generated.
// Suitable for 'New form' wizards in integrations.
QStringList WidgetDataBase::customFormWidgetClasses(const QDesignerFormEditorInterface *core)
{
    QStringList rc;
    const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
    const int widgetCount = wdb->count();
    for (int i = 0; i < widgetCount; i++) { // Custom widgets: check name and base class.
        const QDesignerWidgetDataBaseItemInterface *item = wdb->item(i);
        if (item->isContainer() && item->isCustom() && !item->isPromoted()) {
            if (suitableForNewForm(item->name()) && suitableForNewForm(item->extends()))
                rc += item->name();
        }
    }
    return rc;
}

// Get XML for a new form from the widget box. Change objectName/geometry
// properties to be suitable for new forms
static QString xmlFromWidgetBox(const QDesignerFormEditorInterface *core, const QString &className, const QString &objectName)
{
    QDesignerWidgetBoxInterface::Widget widget;
    const bool found = QDesignerWidgetBox::findWidget(core->widgetBox(), className, QString(), &widget);
    if (!found)
        return QString();
    QScopedPointer<DomUI> domUI(QDesignerWidgetBox::xmlToUi(className, widget.domXml(), false));
    if (domUI.isNull())
        return QString();
    domUI->setAttributeVersion(u"4.0"_s);
    DomWidget *domWidget = domUI->elementWidget();
    if (!domWidget)
        return QString();
    // Properties: Remove the "objectName" property in favour of the name attribute and check geometry.
    domWidget->setAttributeName(objectName);
    QList<DomProperty *> properties = domWidget->elementProperty();
    for (auto it = properties.begin(); it != properties.end(); ) {
        DomProperty *property = *it;
        if (property->attributeName() == "objectName"_L1) { // remove  "objectName"
            it = properties.erase(it);
            delete property;
        } else {
            if (property->attributeName() == "geometry"_L1) { // Make sure form is at least 400, 300
                if (DomRect *geom = property->elementRect()) {
                    if (geom->elementWidth() < NewFormWidth)
                        geom->setElementWidth(NewFormWidth);
                    if (geom->elementHeight() < NewFormHeight)
                        geom->setElementHeight(NewFormHeight);
                }
            }
            ++it;
        }
    }
    // Add a window title property
    DomString *windowTitleString = new DomString;
    windowTitleString->setText(objectName);
    DomProperty *windowTitleProperty = new DomProperty;
    windowTitleProperty->setAttributeName(u"windowTitle"_s);
    windowTitleProperty->setElementString(windowTitleString);
    properties.push_back(windowTitleProperty);
    // ------
    domWidget->setElementProperty(properties);
    // Embed in in DomUI and get string. Omit the version number.
    domUI->setElementClass(objectName);

    QString rc;
    { // Serialize domUI
        QXmlStreamWriter writer(&rc);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(1);
        writer.writeStartDocument();
        domUI->write(writer);
        writer.writeEndDocument();
    }
    return rc;
}

// Generate default standard ui new form xml based on the class passed on as similarClassName.
static QString generateNewFormXML(const QString &className, const QString &similarClassName, const QString &name)
{
    QString rc;
    QTextStream str(&rc);
    str << R"(<ui version="4.0"><class>)" << name << "</class>"
        << R"(<widget class=")" << className << R"(" name=")" << name << R"(">)"
        << R"(<property name="geometry" ><rect><x>0</x><y>0</y><width>)"
        << NewFormWidth << "</width><height>" << NewFormHeight << "</height></rect></property>"
        << R"(<property name="windowTitle"><string>)" << name << "</string></property>\n";

    if (similarClassName == "QMainWindow"_L1) {
        str << R"(<widget class="QWidget" name="centralwidget"/>)";
    } else if (similarClassName == "QWizard"_L1) {
        str << R"(<widget class="QWizardPage" name="wizardPage1"/><widget class="QWizardPage" name="wizardPage2"/>)";
    } else if (similarClassName == "QDockWidget"_L1) {
        str << R"(<widget class="QWidget" name="dockWidgetContents"/>)";
    }
    str << "</widget></ui>\n";
    return rc;
}

// Generate a form template using a class name obtained from formWidgetClasses(), customFormWidgetClasses().
QString WidgetDataBase::formTemplate(const QDesignerFormEditorInterface *core, const QString &className, const QString &objectName)
{
    // How to find suitable XML for a class:
    // 1) Look in widget box (as all the required centralwidgets, tab widget pages, etc. should be there).
    const QString widgetBoxXml = xmlFromWidgetBox(core, className, objectName);
    if (!widgetBoxXml.isEmpty())
        return widgetBoxXml;
    // 2) If that fails, only custom main windows, custom dialogs and unsupported Qt Widgets should
    //    be left over. Generate something that is similar to the default templates. Find a similar class.
    const QDesignerWidgetDataBaseInterface *wdb = core->widgetDataBase();
    QString similarClass = u"QWidget"_s;
    const int index = wdb->indexOfClassName(className);
    if (index != -1) {
        const QDesignerWidgetDataBaseItemInterface *item = wdb->item(index);
        similarClass = item->isCustom() ? item->extends() : item->name();
    }
    // Generate standard ui based on the class passed on as baseClassName.
    const QString rc = generateNewFormXML(className, similarClass, objectName);
    return rc;
}

// Set a fixed size on a XML template
QString WidgetDataBase::scaleFormTemplate(const QString &xml, const QSize &size, bool fixed)
{
    QScopedPointer<DomUI> domUI(QDesignerWidgetBox::xmlToUi(u"Form"_s, xml, false));
    if (!domUI)
        return QString();
    DomWidget *domWidget = domUI->elementWidget();
    if (!domWidget)
        return QString();
    // Properties: Find/Ensure the geometry, minimum and maximum sizes properties
    const QString geometryPropertyName = u"geometry"_s;
    const QString minimumSizePropertyName = u"minimumSize"_s;
    const QString maximumSizePropertyName = u"maximumSize"_s;
    DomProperty *geomProperty = nullptr;
    DomProperty *minimumSizeProperty = nullptr;
    DomProperty *maximumSizeProperty = nullptr;

    auto properties = domWidget->elementProperty();
    for (DomProperty *p : properties) {
        const QString name = p->attributeName();
        if (name == geometryPropertyName) {
            geomProperty = p;
        } else if (name == minimumSizePropertyName) {
            minimumSizeProperty = p;
        } else if (name == maximumSizePropertyName) {
            maximumSizeProperty = p;
        }
    }
    if (!geomProperty) {
        geomProperty = new DomProperty;
        geomProperty->setAttributeName(geometryPropertyName);
        geomProperty->setElementRect(new DomRect);
        properties.push_front(geomProperty);
    }
    if (fixed) {
        if (!minimumSizeProperty) {
            minimumSizeProperty = new DomProperty;
            minimumSizeProperty->setAttributeName(minimumSizePropertyName);
            minimumSizeProperty->setElementSize(new DomSize);
            properties.push_back(minimumSizeProperty);
        }
        if (!maximumSizeProperty) {
            maximumSizeProperty = new DomProperty;
            maximumSizeProperty->setAttributeName(maximumSizePropertyName);
            maximumSizeProperty->setElementSize(new DomSize);
            properties.push_back(maximumSizeProperty);
        }
    }
    // Set values of geometry, minimum and maximum sizes properties
    const int width = size.width();
    const int height = size.height();
    if (DomRect *geom = geomProperty->elementRect()) {
        geom->setElementWidth(width);
        geom->setElementHeight(height);
    }
    if (fixed) {
        if (DomSize *s = minimumSizeProperty->elementSize()) {
            s->setElementWidth(width);
            s->setElementHeight(height);
        }
        if (DomSize *s = maximumSizeProperty->elementSize()) {
            s->setElementWidth(width);
            s->setElementHeight(height);
        }
    }
    // write back
    domWidget->setElementProperty(properties);

    QString rc;
    { // serialize domUI
        QXmlStreamWriter writer(&rc);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(1);
        writer.writeStartDocument();
        domUI->write(writer);
        writer.writeEndDocument();
    }

    return rc;
}

// ---- free functions
QDESIGNER_SHARED_EXPORT IncludeSpecification  includeSpecification(QString includeFile)
{
    const bool global = includeFile.startsWith(u'<') && includeFile.endsWith(u'>');
    if (global) {
        includeFile.chop(1);
        includeFile.remove(0, 1);
    }
    return IncludeSpecification(includeFile, global ? IncludeGlobal : IncludeLocal);
}

QDESIGNER_SHARED_EXPORT QString buildIncludeFile(QString includeFile, IncludeType includeType)
{
    if (includeType == IncludeGlobal && !includeFile.isEmpty()) {
        includeFile.append(u'>');
        includeFile.prepend(u'<');
    }
    return includeFile;
}


/* Appends a derived class to the database inheriting the data of the base class. Used
   for custom and promoted widgets.

   Depending on whether an entry exists, the existing or a newly created entry is
   returned. A return value of 0 indicates that the base class could not be found. */

QDESIGNER_SHARED_EXPORT QDesignerWidgetDataBaseItemInterface *
        appendDerived(QDesignerWidgetDataBaseInterface *db,
                      const QString &className, const QString &group,
                      const QString &baseClassName,
                      const QString &includeFile,
                      bool promoted, bool custom)
{
    if (debugWidgetDataBase)
        qDebug() << "appendDerived " << className << " derived from " << baseClassName;
    // Check.
    if (className.isEmpty() || baseClassName.isEmpty()) {
        qWarning("** WARNING %s called with an empty class names: '%s' extends '%s'.",
                 Q_FUNC_INFO, className.toUtf8().constData(), baseClassName.toUtf8().constData());
        return nullptr;
    }
    // Check whether item already exists.
    QDesignerWidgetDataBaseItemInterface *derivedItem = nullptr;
    const int existingIndex = db->indexOfClassName(className);
    if ( existingIndex != -1)
        derivedItem =  db->item(existingIndex);
    if (derivedItem) {
        // Check the existing item for base class mismatch. This will likely
        // happen when loading a file written by an instance with missing plugins.
        // In that case, just warn and ignore the file properties.
        //
        // An empty base class indicates that it is not known (for example, for custom plugins).
        // In this case, the widget DB is later updated once the widget is created
        // by DOM (by querying the metaobject). Suppress the warning.
        const QString existingBaseClass = derivedItem->extends();
        if (existingBaseClass.isEmpty() || baseClassName ==  existingBaseClass)
            return derivedItem;

        // Warn about mismatches
        designerWarning(QCoreApplication::translate("WidgetDataBase",
          "The file contains a custom widget '%1' whose base class (%2)"
          " differs from the current entry in the widget database (%3)."
          " The widget database is left unchanged.").
                        arg(className, baseClassName, existingBaseClass));
        return derivedItem;
    }
    // Create this item, inheriting its base properties
    const int baseIndex = db->indexOfClassName(baseClassName);
    if (baseIndex == -1) {
        if (debugWidgetDataBase)
            qDebug() << "appendDerived failed due to missing base class";
        return nullptr;
    }
    const QDesignerWidgetDataBaseItemInterface *baseItem = db->item(baseIndex);
    derivedItem = WidgetDataBaseItem::clone(baseItem);
    // Sort of hack: If base class is QWidget, we most likely
    // do not want to inherit the container attribute.
    if (baseItem->name() == "QWidget"_L1)
        derivedItem->setContainer(false);
    // set new props
    derivedItem->setName(className);
    derivedItem->setGroup(group);
    derivedItem->setCustom(custom);
    derivedItem->setPromoted(promoted);
    derivedItem->setExtends(baseClassName);
    derivedItem->setIncludeFile(includeFile);
    db->append(derivedItem);
    return derivedItem;
}

/* Return a list of database items to which a class can be promoted to. */

QDESIGNER_SHARED_EXPORT WidgetDataBaseItemList
        promotionCandidates(const QDesignerWidgetDataBaseInterface *db,
                            const QString &baseClassName)
{
    WidgetDataBaseItemList rc;
    // find existing promoted widgets deriving from base.
    const int count = db->count();
    for (int i = 0; i < count; ++i) {
        QDesignerWidgetDataBaseItemInterface *item = db->item(i);
        if (item->isPromoted() && item->extends() == baseClassName) {
            rc.push_back(item);
        }
    }
    return rc;
}
} // namespace qdesigner_internal

QT_END_NAMESPACE
