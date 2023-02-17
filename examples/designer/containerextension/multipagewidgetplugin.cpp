// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "multipagewidget.h"
#include "multipagewidgetplugin.h"
#include "multipagewidgetextensionfactory.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include <QIcon>
#include <QtPlugin>

using namespace Qt::StringLiterals;

//! [0]
MultiPageWidgetPlugin::MultiPageWidgetPlugin(QObject *parent)
    : QObject(parent)
{
}

QString MultiPageWidgetPlugin::name() const
{
    return u"MultiPageWidget"_s;
}

QString MultiPageWidgetPlugin::group() const
{
    return u"Display Widgets [Examples]"_s;
}

QString MultiPageWidgetPlugin::toolTip() const
{
    return {};
}

QString MultiPageWidgetPlugin::whatsThis() const
{
    return {};
}

QString MultiPageWidgetPlugin::includeFile() const
{
    return u"multipagewidget.h"_s;
}

QIcon MultiPageWidgetPlugin::icon() const
{
    return {};
}

//! [0] //! [1]
bool MultiPageWidgetPlugin::isContainer() const
{
    return true;
}

//! [1] //! [2]
QWidget *MultiPageWidgetPlugin::createWidget(QWidget *parent)
{
    auto *widget = new MultiPageWidget(parent);
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
    auto *manager = formEditor->extensionManager();
//! [5] //! [6]
    auto *factory = new MultiPageWidgetExtensionFactory(manager);

    Q_ASSERT(manager != nullptr);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));

    initialized = true;
}
//! [6]

//! [7]
QString MultiPageWidgetPlugin::domXml() const
{
    return uR"(
<ui language="c++">
    <widget class="MultiPageWidget" name="multipagewidget">
        <widget class="QWidget" name="page"/>
    </widget>
    <customwidgets>
        <customwidget>
            <class>MultiPageWidget</class>
            <extends>QWidget</extends>
            <addpagemethod>addPage</addpagemethod>
        </customwidget>
    </customwidgets>
</ui>)"_s;
}
//! [7]

//! [8]
void MultiPageWidgetPlugin::currentIndexChanged(int index)
{
    Q_UNUSED(index);
    auto *widget = qobject_cast<MultiPageWidget*>(sender());
//! [8] //! [9]
    if (widget) {
        auto *form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}
//! [9]

//! [10]
void MultiPageWidgetPlugin::pageTitleChanged(const QString &title)
{
    Q_UNUSED(title);
    auto *widget = qobject_cast<MultiPageWidget*>(sender());
//! [10] //! [11]
    if (widget) {
        auto *page = widget->widget(widget->currentIndex());
        auto *form = QDesignerFormWindowInterface::findFormWindow(widget);
//! [11]
        if (form) {
//! [12]
            auto *editor = form->core();
            auto *manager = editor->extensionManager();
//! [12] //! [13]
            auto *sheet = qt_extension<QDesignerPropertySheetExtension*>(manager, page);
            const int propertyIndex = sheet->indexOf(QLatin1String("windowTitle"));
            sheet->setChanged(propertyIndex, true);
        }
    }
}

//! [13]
