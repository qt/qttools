// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "formeditor.h"
#include "formeditor_optionspage.h"
#include "embeddedoptionspage.h"
#include "templateoptionspage.h"
#include "metadatabase_p.h"
#include "widgetdatabase_p.h"
#include "widgetfactory_p.h"
#include "formwindowmanager.h"
#include "qmainwindow_container.h"
#include "qmdiarea_container.h"
#include "qwizard_container.h"
#include "default_container.h"
#include "default_layoutdecoration.h"
#include "default_actionprovider.h"
#include "qlayoutwidget_propertysheet.h"
#include "spacer_propertysheet.h"
#include "line_propertysheet.h"
#include "layout_propertysheet.h"
#include "qdesigner_dockwidget_p.h"
#include "qdesigner_stackedbox_p.h"
#include "qdesigner_toolbox_p.h"
#include "qdesigner_tabwidget_p.h"
#include "qtresourcemodel_p.h"
#include "itemview_propertysheet.h"

// sdk
#include <QtDesigner/qextensionmanager.h>
#include <QtDesigner/abstractintegration.h>
// shared
#include <pluginmanager_p.h>
#include <qdesigner_taskmenu_p.h>
#include <qdesigner_membersheet_p.h>
#include <qdesigner_promotion_p.h>
#include <dialoggui_p.h>
#include <qdesigner_introspection_p.h>
#include <qdesigner_qsettings_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace qdesigner_internal {

FormEditor::FormEditor(QObject *parent) : FormEditor(QStringList{}, parent)
{
}

FormEditor::FormEditor(const QStringList &pluginPaths,
                       QObject *parent)
    : QDesignerFormEditorInterface(parent)
{
    setIntrospection(new QDesignerIntrospection);
    setDialogGui(new DialogGui);
    auto *pluginManager = new QDesignerPluginManager(pluginPaths, this);
    setPluginManager(pluginManager);

    WidgetDataBase *widgetDatabase = new WidgetDataBase(this, this);
    setWidgetDataBase(widgetDatabase);

    MetaDataBase *metaDataBase = new MetaDataBase(this, this);
    setMetaDataBase(metaDataBase);

    WidgetFactory *widgetFactory = new WidgetFactory(this, this);
    setWidgetFactory(widgetFactory);

    FormWindowManager *formWindowManager = new FormWindowManager(this, this);
    setFormManager(formWindowManager);
    connect(formWindowManager, &QDesignerFormWindowManagerInterface::formWindowAdded,
            widgetFactory, &WidgetFactory::formWindowAdded);
    connect(formWindowManager, &QDesignerFormWindowManagerInterface::activeFormWindowChanged,
            widgetFactory, &WidgetFactory::activeFormWindowChanged);

    QExtensionManager *mgr = new QExtensionManager(this);
    const QString containerExtensionId = Q_TYPEID(QDesignerContainerExtension);

    QDesignerStackedWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
    QDesignerTabWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
    QDesignerToolBoxContainerFactory::registerExtension(mgr, containerExtensionId);
    QMainWindowContainerFactory::registerExtension(mgr, containerExtensionId);
    QDockWidgetContainerFactory::registerExtension(mgr, containerExtensionId);
    QScrollAreaContainerFactory::registerExtension(mgr, containerExtensionId);
    QMdiAreaContainerFactory::registerExtension(mgr, containerExtensionId);
    QWizardContainerFactory::registerExtension(mgr, containerExtensionId);

    mgr->registerExtensions(new QDesignerLayoutDecorationFactory(mgr),
                            Q_TYPEID(QDesignerLayoutDecorationExtension));

    const QString actionProviderExtensionId = Q_TYPEID(QDesignerActionProviderExtension);
    QToolBarActionProviderFactory::registerExtension(mgr, actionProviderExtensionId);
    QMenuBarActionProviderFactory::registerExtension(mgr, actionProviderExtensionId);
    QMenuActionProviderFactory::registerExtension(mgr, actionProviderExtensionId);

    QDesignerDefaultPropertySheetFactory::registerExtension(mgr);
    QDockWidgetPropertySheetFactory::registerExtension(mgr);
    QLayoutWidgetPropertySheetFactory::registerExtension(mgr);
    SpacerPropertySheetFactory::registerExtension(mgr);
    LinePropertySheetFactory::registerExtension(mgr);
    LayoutPropertySheetFactory::registerExtension(mgr);
    QStackedWidgetPropertySheetFactory::registerExtension(mgr);
    QToolBoxWidgetPropertySheetFactory::registerExtension(mgr);
    QTabWidgetPropertySheetFactory::registerExtension(mgr);
    QMdiAreaPropertySheetFactory::registerExtension(mgr);
    QWizardPagePropertySheetFactory::registerExtension(mgr);
    QWizardPropertySheetFactory::registerExtension(mgr);

    QTreeViewPropertySheetFactory::registerExtension(mgr);
    QTableViewPropertySheetFactory::registerExtension(mgr);

    QDesignerTaskMenuFactory::registerExtension(mgr, u"QDesignerInternalTaskMenuExtension"_s);

    mgr->registerExtensions(new QDesignerMemberSheetFactory(mgr),
                            Q_TYPEID(QDesignerMemberSheetExtension));

    setExtensionManager(mgr);

    setPromotion(new QDesignerPromotion(this));

    QtResourceModel *resourceModel = new QtResourceModel(this);
    setResourceModel(resourceModel);
    connect(resourceModel, &QtResourceModel::qrcFileModifiedExternally,
            this, &FormEditor::slotQrcFileChangedExternally);

    QList<QDesignerOptionsPageInterface*> optionsPages;
    optionsPages << new TemplateOptionsPage(this) << new FormEditorOptionsPage(this) << new EmbeddedOptionsPage(this);
    setOptionsPages(optionsPages);

    setSettingsManager(new QDesignerQSettings());
}

FormEditor::~FormEditor() = default;

void FormEditor::slotQrcFileChangedExternally(const QString &path)
{
    if (!integration())
        return;

    QDesignerIntegration::ResourceFileWatcherBehaviour behaviour = integration()->resourceFileWatcherBehaviour();
    if (behaviour == QDesignerIntegration::NoResourceFileWatcher)
        return;
    if (behaviour == QDesignerIntegration::PromptToReloadResourceFile) {
        QMessageBox::StandardButton button = dialogGui()->message(topLevel(), QDesignerDialogGuiInterface::FileChangedMessage, QMessageBox::Warning,
                tr("Resource File Changed"),
                tr("The file \"%1\" has changed outside Designer. Do you want to reload it?").arg(path),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (button != QMessageBox::Yes)
            return;
    }

    resourceModel()->reload(path);
}

}

QT_END_NAMESPACE
