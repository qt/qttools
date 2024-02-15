// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtDesigner/QDesignerComponents>

#include <actioneditor_p.h>
#include <pluginmanager_p.h>
#include <widgetdatabase_p.h>
#include <widgetfactory_p.h>

#include <formeditor/formeditor.h>
#include <widgetbox/widgetbox.h>
#include <propertyeditor/propertyeditor.h>
#include <objectinspector/objectinspector.h>
#include <taskmenu/taskmenu_component.h>
#include "qtresourceview_p.h"
#include <signalsloteditor/signalsloteditorwindow.h>

#include <buddyeditor/buddyeditor_plugin.h>
#include <signalsloteditor/signalsloteditor_plugin.h>
#include <tabordereditor/tabordereditor_plugin.h>

#include <QtDesigner/abstractlanguage.h>
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractintegration.h>
#include <QtDesigner/abstractresourcebrowser.h>

#include <QtCore/qplugin.h>
#include <QtCore/qdir.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>

#define INIT_PLUGIN_INSTANCE(PLUGIN) \
    do { \
        Static##PLUGIN##PluginInstance instance; \
        Q_UNUSED(instance); \
    } while (0)

Q_IMPORT_PLUGIN(SignalSlotEditorPlugin)
Q_IMPORT_PLUGIN(BuddyEditorPlugin)
Q_IMPORT_PLUGIN(TabOrderEditorPlugin)

static void initResources()
{
    // Q_INIT_RESOURCE only usable in functions in global namespace
    Q_INIT_RESOURCE(formeditor);
    Q_INIT_RESOURCE(widgetbox);
    Q_INIT_RESOURCE(propertyeditor);
}


static void initInstances()
{
    static bool plugins_initialized = false;

    if (!plugins_initialized) {
        INIT_PLUGIN_INSTANCE(SignalSlotEditorPlugin);
        INIT_PLUGIN_INSTANCE(BuddyEditorPlugin);
        INIT_PLUGIN_INSTANCE(TabOrderEditorPlugin);
        plugins_initialized = true;
    }
}

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

/*!
    \class QDesignerComponents
    \brief The QDesignerComponents class provides a central resource for the various components
    used in the \QD user interface.
    \inmodule QtDesigner
    \internal

    The QDesignerComponents class is a factory for each of the standard components present
    in the \QD user interface. It is mostly useful for developers who want to implement
    a standalone form editing environment using \QD's components, or who need to integrate
    \QD's components into an existing integrated development environment (IDE).

    \sa QDesignerFormEditorInterface, QDesignerObjectInspectorInterface,
        QDesignerPropertyEditorInterface, QDesignerWidgetBoxInterface
*/

/*!
    Initializes the resources used by the components.*/
void QDesignerComponents::initializeResources()
{
    initResources();
}

/*!
    Initializes the plugins used by the components.*/
void QDesignerComponents::initializePlugins(QDesignerFormEditorInterface *core)
{
    QDesignerIntegration::initializePlugins(core);
}

// ### fixme Qt 7 createFormEditorWithPluginPaths->createFormEditor

/*!
    Constructs a form editor interface with the given \a parent.*/
QDesignerFormEditorInterface *QDesignerComponents::createFormEditor(QObject *parent)
{
    return createFormEditorWithPluginPaths({}, parent);
}

/*!
    Constructs a form editor interface with the given \a pluginPaths and the \a parent.
    \since 6.7
*/
QDesignerFormEditorInterface *
    QDesignerComponents::createFormEditorWithPluginPaths(const QStringList &pluginPaths,
                                                         QObject *parent)
{
    initInstances();
    return new qdesigner_internal::FormEditor(pluginPaths, parent);
}

/*!
    Returns a new task menu with the given \a parent for the \a core interface.*/
QObject *QDesignerComponents::createTaskMenu(QDesignerFormEditorInterface *core, QObject *parent)
{
    return new qdesigner_internal::TaskMenuComponent(core, parent);
}

static inline int qtMajorVersion(int qtVersion) { return qtVersion >> 16; }
static inline int qtMinorVersion(int qtVersion) { return (qtVersion >> 8) & 0xFF; }
static inline void setMinorVersion(int minorVersion, int *qtVersion)
{
    *qtVersion &= ~0xFF00;
    *qtVersion |= minorVersion << 8;
}

// Build the version-dependent name of the user widget box file, '$HOME.designer/widgetbox4.4.xml'
static inline QString widgetBoxFileName(int qtVersion, const QDesignerLanguageExtension *lang = nullptr)
{
    QString rc; {
        QTextStream str(&rc);
        str << QDir::homePath() << QDir::separator() << ".designer" << QDir::separator()
            << "widgetbox";
        // The naming convention using the version was introduced with 4.4
        const int major = qtMajorVersion(qtVersion);
        const int minor = qtMinorVersion(qtVersion);
        if (major >= 4 &&  minor >= 4)
            str << major << '.' << minor;
        if (lang)
            str << '.' << lang->uiExtension();
        str << ".xml";
    }
    return rc;
}

/*!
    Returns a new widget box interface with the given \a parent for the \a core interface.*/
QDesignerWidgetBoxInterface *QDesignerComponents::createWidgetBox(QDesignerFormEditorInterface *core, QWidget *parent)
{
    qdesigner_internal::WidgetBox *widgetBox = new qdesigner_internal::WidgetBox(core, parent);

    const QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core);

    do {
        if (lang) {
            const QString languageWidgetBox = lang->widgetBoxContents();
            if (!languageWidgetBox.isEmpty()) {
                widgetBox->loadContents(lang->widgetBoxContents());
                break;
            }
        }

        widgetBox->setFileName(u":/qt-project.org/widgetbox/widgetbox.xml"_s);
        widgetBox->load();
    } while (false);

    const QString userWidgetBoxFile = widgetBoxFileName(QT_VERSION, lang);

    widgetBox->setFileName(userWidgetBoxFile);
    if (!QFileInfo::exists(userWidgetBoxFile)) {
        // check previous version, that is, are we running the new version for the first time
        // If so, try to copy the old widget box file
        if (const int minv = qtMinorVersion(QT_VERSION)) {
            int oldVersion = QT_VERSION;
            setMinorVersion(minv - 1, &oldVersion);
            const QString oldWidgetBoxFile = widgetBoxFileName(oldVersion, lang);
            if (QFileInfo::exists(oldWidgetBoxFile))
                QFile::copy(oldWidgetBoxFile, userWidgetBoxFile);
        }
    }
    widgetBox->load();

    return widgetBox;
}

/*!
    Returns a new property editor interface with the given \a parent for the \a core interface.*/
QDesignerPropertyEditorInterface *QDesignerComponents::createPropertyEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::PropertyEditor(core, parent);
}

/*!
    Returns a new object inspector interface with the given \a parent for the \a core interface.*/
QDesignerObjectInspectorInterface *QDesignerComponents::createObjectInspector(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::ObjectInspector(core, parent);
}

/*!
    Returns a new action editor interface with the given \a parent for the \a core interface.*/
QDesignerActionEditorInterface *QDesignerComponents::createActionEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::ActionEditor(core, parent);
}

/*!
    Returns a new resource editor with the given \a parent for the \a core interface.*/
QWidget *QDesignerComponents::createResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    if (QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core)) {
        QWidget *w = lang->createResourceBrowser(parent);
        if (w)
            return w;
    }
    QtResourceView *resourceView = new QtResourceView(core, parent);
    resourceView->setResourceModel(core->resourceModel());
    resourceView->setSettingsKey(u"ResourceBrowser"_s);
    // Note for integrators: make sure you call createResourceEditor() after you instantiated your subclass of designer integration
    // (designer doesn't do that since by default editing resources is enabled)
    const QDesignerIntegrationInterface *integration = core->integration();
    if (integration && !integration->hasFeature(QDesignerIntegrationInterface::ResourceEditorFeature))
        resourceView->setResourceEditingEnabled(false);
    return resourceView;
}

/*!
    Returns a new signal-slot editor with the given \a parent for the \a core interface.*/
QWidget *QDesignerComponents::createSignalSlotEditor(QDesignerFormEditorInterface *core, QWidget *parent)
{
    return new qdesigner_internal::SignalSlotEditorWindow(core, parent);
}

/*!
    Returns the default plugin paths of Qt Widgets Designer's plugin manager.

    \return Plugin paths
    \since 6.7
*/
QStringList QDesignerComponents::defaultPluginPaths()
{
    return QDesignerPluginManager::defaultPluginPaths();
}

QT_END_NAMESPACE

