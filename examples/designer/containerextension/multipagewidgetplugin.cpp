// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include <QIcon>
#include <QtPlugin>

#include "multipagewidget.h"
#include "multipagewidgetplugin.h"
#include "multipagewidgetextensionfactory.h"

//! [0]
MultiPageWidgetPlugin::MultiPageWidgetPlugin(QObject *parent)
    : QObject(parent)
{
}

QString MultiPageWidgetPlugin::name() const
{
    return QLatin1String("MultiPageWidget");
}

QString MultiPageWidgetPlugin::group() const
{
    return QLatin1String("Display Widgets [Examples]");
}

QString MultiPageWidgetPlugin::toolTip() const
{
    return QString();
}

QString MultiPageWidgetPlugin::whatsThis() const
{
    return QString();
}

QString MultiPageWidgetPlugin::includeFile() const
{
    return QLatin1String("multipagewidget.h");
}

QIcon MultiPageWidgetPlugin::icon() const
{
    return QIcon();
}

//! [0] //! [1]
bool MultiPageWidgetPlugin::isContainer() const
{
    return true;
}

//! [1] //! [2]
QWidget *MultiPageWidgetPlugin::createWidget(QWidget *parent)
{
    MultiPageWidget *widget = new MultiPageWidget(parent);
    connect(widget, &MultiPageWidget::currentIndexChanged,
            this, &MultiPageWidgetPlugin::currentIndexChanged);
    connect(widget, &MultiPageWidget::pageTitleChanged,
            this, &MultiPageWidgetPlugin::pageTitleChanged);
    return widget;
}

//! [2] //! [3]
bool MultiPageWidgetPlugin::isInitialized() const
{
    return initialized;
}
//! [3]

//! [4]
void MultiPageWidgetPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
    if (initialized)
        return;
//! [4]

//! [5]
    QExtensionManager *manager = formEditor->extensionManager();
//! [5] //! [6]
    QExtensionFactory *factory = new MultiPageWidgetExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));

    initialized = true;
}
//! [6]

//! [7]
QString MultiPageWidgetPlugin::domXml() const
{
    return QLatin1String(R"(
<ui language="c++">
    <widget class="MultiPageWidget" name="multipagewidget">
        <widget class="QWidget" name="page" />
    </widget>
    <customwidgets>
        <customwidget>
            <class>MultiPageWidget</class>
            <extends>QWidget</extends>
            <addpagemethod>addPage</addpagemethod>
        </customwidget>
    </customwidgets>
</ui>)");
}
//! [7]

//! [8]
void MultiPageWidgetPlugin::currentIndexChanged(int index)
{
    Q_UNUSED(index);
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(sender());
//! [8] //! [9]
    if (widget) {
        QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}
//! [9]

//! [10]
void MultiPageWidgetPlugin::pageTitleChanged(const QString &title)
{
    Q_UNUSED(title);
    MultiPageWidget *widget = qobject_cast<MultiPageWidget*>(sender());
//! [10] //! [11]
    if (widget) {
        QWidget *page = widget->widget(widget->currentIndex());
        QDesignerFormWindowInterface *form;
        form = QDesignerFormWindowInterface::findFormWindow(widget);
//! [11]
        if (form) {
//! [12]
            QDesignerFormEditorInterface *editor = form->core();
            QExtensionManager *manager = editor->extensionManager();
//! [12] //! [13]
            QDesignerPropertySheetExtension *sheet;
            sheet = qt_extension<QDesignerPropertySheetExtension*>(manager, page);
            const int propertyIndex = sheet->indexOf(QLatin1String("windowTitle"));
            sheet->setChanged(propertyIndex, true);
        }
    }
}

//! [13]
