// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "abstractformeditor.h"
#include "abstractdialoggui_p.h"
#include "abstractintrospection_p.h"

#include <QtDesigner/abstractoptionspage.h>
#include <QtDesigner/abstractsettings.h>
#include <QtDesigner/abstractpropertyeditor.h>
#include <QtDesigner/abstractformwindowmanager.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractmetadatabase.h>
#include <QtDesigner/abstractwidgetdatabase.h>
#include <QtDesigner/abstractwidgetfactory.h>
#include <QtDesigner/abstractobjectinspector.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/abstractactioneditor.h>
#include <QtDesigner/abstractwidgetbox.h>

#include <actioneditor_p.h>
#include <pluginmanager_p.h>
#include <qtresourcemodel_p.h>
#include <qtgradientmanager_p.h>
#include <widgetfactory_p.h>
#include <shared_settings_p.h>
#include <formwindowbase_p.h>
#include <grid_p.h>
#include <iconloader_p.h>
#include <QtDesigner/abstractpromotioninterface.h>

#include <QtGui/qicon.h>

// Must be done outside of the Qt namespace
static void initResources()
{
    Q_INIT_RESOURCE(shared);
    Q_INIT_RESOURCE(ClamshellPhone);
    Q_INIT_RESOURCE(PortableMedia);
    Q_INIT_RESOURCE(S60_nHD_Touchscreen);
    Q_INIT_RESOURCE(S60_QVGA_Candybar);
    Q_INIT_RESOURCE(SmartPhone2);
    Q_INIT_RESOURCE(SmartPhone);
    Q_INIT_RESOURCE(SmartPhoneWithButtons);
    Q_INIT_RESOURCE(TouchscreenPhone);
}

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QDesignerFormEditorInterfacePrivate {
public:
    QDesignerFormEditorInterfacePrivate();
    ~QDesignerFormEditorInterfacePrivate();


    QPointer<QWidget> m_topLevel;
    QPointer<QDesignerWidgetBoxInterface> m_widgetBox;
    QPointer<QDesignerPropertyEditorInterface> m_propertyEditor;
    QPointer<QDesignerFormWindowManagerInterface> m_formWindowManager;
    QPointer<QExtensionManager> m_extensionManager;
    QPointer<QDesignerMetaDataBaseInterface> m_metaDataBase;
    QPointer<QDesignerWidgetDataBaseInterface> m_widgetDataBase;
    QPointer<QDesignerWidgetFactoryInterface> m_widgetFactory;
    QPointer<QDesignerObjectInspectorInterface> m_objectInspector;
    QPointer<QDesignerIntegrationInterface> m_integration;
    QPointer<QDesignerActionEditorInterface> m_actionEditor;
    QDesignerSettingsInterface *m_settingsManager = nullptr;
    QDesignerPluginManager *m_pluginManager = nullptr;
    QDesignerPromotionInterface *m_promotion = nullptr;
    QDesignerIntrospectionInterface *m_introspection = nullptr;
    QDesignerDialogGuiInterface *m_dialogGui = nullptr;
    QPointer<QtResourceModel> m_resourceModel;
    QPointer<QtGradientManager> m_gradientManager; // instantiated and deleted by designer_integration
    QList<QDesignerOptionsPageInterface*> m_optionsPages;
};

QDesignerFormEditorInterfacePrivate::QDesignerFormEditorInterfacePrivate() = default;

QDesignerFormEditorInterfacePrivate::~QDesignerFormEditorInterfacePrivate()
{
    delete m_settingsManager;
    delete m_formWindowManager;
    delete m_promotion;
    delete m_introspection;
    delete m_dialogGui;
    delete m_resourceModel;
    qDeleteAll(m_optionsPages);
}

/*!
    \class QDesignerFormEditorInterface

    \brief The QDesignerFormEditorInterface class allows you to access
    Qt Widgets Designer's various components.

    \inmodule QtDesigner

    \QD's current QDesignerFormEditorInterface object holds
    information about all \QD's components: The action editor, the
    object inspector, the property editor, the widget box, and the
    extension and form window managers. QDesignerFormEditorInterface
    contains a collection of functions that provides interfaces to all
    these components. They are typically used to query (and
    manipulate) the respective component. For example:

    \snippet lib/tools_designer_src_lib_sdk_abstractobjectinspector.cpp 0

    QDesignerFormEditorInterface is not intended to be instantiated
    directly. A pointer to \QD's current QDesignerFormEditorInterface
    object (\c formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    QDesignerFormEditorInterface also provides functions that can set
    the action editor, property editor, object inspector and widget
    box. These are only useful if you want to provide your own custom
    components.

    If designer is embedded in another program, one could to provide its
    own settings manager. The manager is used by the components of \QD
    to store/retrieve persistent configuration settings. The default
    manager uses QSettings as the backend.

    Finally, QDesignerFormEditorInterface provides the topLevel()
    function that returns \QD's top-level widget.

    \sa QDesignerCustomWidgetInterface
*/

/*!
    Constructs a QDesignerFormEditorInterface object with the given \a
    parent.
*/

QDesignerFormEditorInterface::QDesignerFormEditorInterface(QObject *parent)
    : QObject(parent),
      d(new QDesignerFormEditorInterfacePrivate())
{
    initResources();
}

/*!
    Destroys the QDesignerFormEditorInterface object.
*/
QDesignerFormEditorInterface::~QDesignerFormEditorInterface() = default;

/*!
    Returns an interface to \QD's widget box.

    \sa setWidgetBox()
*/
QDesignerWidgetBoxInterface *QDesignerFormEditorInterface::widgetBox() const
{
    return d->m_widgetBox;
}

/*!
    Sets \QD's widget box to be the specified \a widgetBox.

    \sa widgetBox()
*/
void QDesignerFormEditorInterface::setWidgetBox(QDesignerWidgetBoxInterface *widgetBox)
{
    d->m_widgetBox = widgetBox;
}

/*!
    Returns an interface to \QD's property editor.

    \sa setPropertyEditor()
*/
QDesignerPropertyEditorInterface *QDesignerFormEditorInterface::propertyEditor() const
{
    return d->m_propertyEditor;
}

/*!
    Sets \QD's property editor to be the specified \a propertyEditor.

    \sa propertyEditor()
*/
void QDesignerFormEditorInterface::setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor)
{
    d->m_propertyEditor = propertyEditor;
}

/*!
    Returns an interface to \QD's action editor.

    \sa setActionEditor()
*/
QDesignerActionEditorInterface *QDesignerFormEditorInterface::actionEditor() const
{
    return d->m_actionEditor;
}

/*!
    Sets \QD's action editor to be the specified \a actionEditor.

    \sa actionEditor()
*/
void QDesignerFormEditorInterface::setActionEditor(QDesignerActionEditorInterface *actionEditor)
{
    d->m_actionEditor = actionEditor;
}

/*!
    Returns \QD's top-level widget.
*/
QWidget *QDesignerFormEditorInterface::topLevel() const
{
    return d->m_topLevel;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setTopLevel(QWidget *topLevel)
{
    d->m_topLevel = topLevel;
}

/*!
    Returns an interface to \QD's form window manager.
*/
QDesignerFormWindowManagerInterface *QDesignerFormEditorInterface::formWindowManager() const
{
    return d->m_formWindowManager;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setFormManager(QDesignerFormWindowManagerInterface *formWindowManager)
{
    d->m_formWindowManager = formWindowManager;
}

/*!
    Returns an interface to \QD's extension manager.
*/
QExtensionManager *QDesignerFormEditorInterface::extensionManager() const
{
    return d->m_extensionManager;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setExtensionManager(QExtensionManager *extensionManager)
{
    d->m_extensionManager = extensionManager;
}

/*!
    \internal

    Returns an interface to the meta database used by the form editor.
*/
QDesignerMetaDataBaseInterface *QDesignerFormEditorInterface::metaDataBase() const
{
    return d->m_metaDataBase;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase)
{
    d->m_metaDataBase = metaDataBase;
}

/*!
    \internal

    Returns an interface to the widget database used by the form editor.
*/
QDesignerWidgetDataBaseInterface *QDesignerFormEditorInterface::widgetDataBase() const
{
    return d->m_widgetDataBase;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase)
{
    d->m_widgetDataBase = widgetDataBase;
}

/*!
    \internal

    Returns an interface to the designer promotion handler.
*/

QDesignerPromotionInterface *QDesignerFormEditorInterface::promotion() const
{
    return d->m_promotion;
}

/*!
    \internal

    Sets the designer promotion handler.
*/

void QDesignerFormEditorInterface::setPromotion(QDesignerPromotionInterface *promotion)
{
    delete d->m_promotion;
    d->m_promotion = promotion;
}

/*!
    \internal

    Returns an interface to the widget factory used by the form editor
    to create widgets for the form.
*/
QDesignerWidgetFactoryInterface *QDesignerFormEditorInterface::widgetFactory() const
{
    return d->m_widgetFactory;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory)
{
    d->m_widgetFactory = widgetFactory;
}

/*!
    Returns an interface to \QD's object inspector.
*/
QDesignerObjectInspectorInterface *QDesignerFormEditorInterface::objectInspector() const
{
    return d->m_objectInspector;
}

/*!
    Sets \QD's object inspector to be the specified \a
    objectInspector.

    \sa objectInspector()
*/
void QDesignerFormEditorInterface::setObjectInspector(QDesignerObjectInspectorInterface *objectInspector)
{
    d->m_objectInspector = objectInspector;
}

/*!
    \internal

    Returns an interface to the integration.
*/
QDesignerIntegrationInterface *QDesignerFormEditorInterface::integration() const
{
    return d->m_integration;
}

/*!
    \internal
*/
void QDesignerFormEditorInterface::setIntegration(QDesignerIntegrationInterface *integration)
{
    d->m_integration = integration;
}

/*!
    \internal
    \since 4.5
    Returns the list of options pages that allow the user to configure \QD components.
*/
QList<QDesignerOptionsPageInterface*> QDesignerFormEditorInterface::optionsPages() const
{
    return d->m_optionsPages;
}

/*!
    \internal
    \since 4.5
    Sets the list of options pages that allow the user to configure \QD components.
*/
void QDesignerFormEditorInterface::setOptionsPages(const QList<QDesignerOptionsPageInterface*> &optionsPages)
{
    d->m_optionsPages = optionsPages;
}


/*!
    \internal

    Returns the plugin manager used by the form editor.
*/
QDesignerPluginManager *QDesignerFormEditorInterface::pluginManager() const
{
    return d->m_pluginManager;
}

/*!
    \internal

    Sets the plugin manager used by the form editor to the specified
    \a pluginManager.
*/
void QDesignerFormEditorInterface::setPluginManager(QDesignerPluginManager *pluginManager)
{
    d->m_pluginManager = pluginManager;
}

/*!
    \internal
    \since 4.4
    Returns the resource model used by the form editor.
*/
QtResourceModel *QDesignerFormEditorInterface::resourceModel() const
{
    return d->m_resourceModel;
}

/*!
    \internal

    Sets the resource model used by the form editor to the specified
    \a resourceModel.
*/
void QDesignerFormEditorInterface::setResourceModel(QtResourceModel *resourceModel)
{
    d->m_resourceModel = resourceModel;
}

/*!
    \internal
    \since 4.4
    Returns the gradient manager used by the style sheet editor.
*/
QtGradientManager *QDesignerFormEditorInterface::gradientManager() const
{
    return d->m_gradientManager;
}

/*!
    \internal

    Sets the gradient manager used by the style sheet editor to the specified
    \a gradientManager.
*/
void QDesignerFormEditorInterface::setGradientManager(QtGradientManager *gradientManager)
{
    d->m_gradientManager = gradientManager;
}

/*!
    \internal
    \since 4.5
    Returns the settings manager used by the components to store persistent settings.
*/
QDesignerSettingsInterface *QDesignerFormEditorInterface::settingsManager() const
{
    return d->m_settingsManager;
}

/*!
    \internal
    \since 4.5
    Sets the settings manager used to store/retrieve the persistent settings of the components.
*/
void QDesignerFormEditorInterface::setSettingsManager(QDesignerSettingsInterface *settingsManager)
{
    if (d->m_settingsManager)
        delete d->m_settingsManager;
    d->m_settingsManager = settingsManager;

    // This is a (hopefully) safe place to perform settings-dependent
    // initializations.
    const qdesigner_internal::QDesignerSharedSettings settings(this);
    qdesigner_internal::FormWindowBase::setDefaultDesignerGrid(settings.defaultGrid());
    qdesigner_internal::ActionEditor::setObjectNamingMode(settings.objectNamingMode());
}

/*!
    \internal
    \since 4.4
    Returns the introspection used by the form editor.
*/
QDesignerIntrospectionInterface *QDesignerFormEditorInterface::introspection() const
{
    return d->m_introspection;
}

/*!
    \internal
    \since 4.4

    Sets the introspection used by the form editor to the specified \a introspection.
*/
void QDesignerFormEditorInterface::setIntrospection(QDesignerIntrospectionInterface *introspection)
{
     delete d->m_introspection;
     d->m_introspection = introspection;
}

/*!
    \internal

    Returns the path to the resources used by the form editor.
*/
QString QDesignerFormEditorInterface::resourceLocation() const
{
#ifdef Q_OS_MACOS
    return u":/qt-project.org/formeditor/images/mac"_s;
#else
    return u":/qt-project.org/formeditor/images/win"_s;
#endif
}

/*!
    \internal

    Returns the dialog GUI used by the form editor.
*/

QDesignerDialogGuiInterface *QDesignerFormEditorInterface::dialogGui() const
{
    return d->m_dialogGui;
}

/*!
    \internal

    Sets the dialog GUI used by the form editor to the specified \a dialogGui.
*/

void QDesignerFormEditorInterface::setDialogGui(QDesignerDialogGuiInterface *dialogGui)
{
    delete  d->m_dialogGui;
    d->m_dialogGui = dialogGui;
}

/*!
    \internal

    \since 5.0

    Returns the plugin instances of QDesignerPluginManager.
*/

QObjectList QDesignerFormEditorInterface::pluginInstances() const
{
    return d->m_pluginManager->instances();
}

/*!
    \internal

    \since 5.0

    Return icons for actions of \QD.
*/

QIcon QDesignerFormEditorInterface::createIcon(const QString &name)
{
    return qdesigner_internal::createIconSet(name);
}

QT_END_NAMESPACE
