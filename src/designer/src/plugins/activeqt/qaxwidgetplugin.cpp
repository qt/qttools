// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qaxwidgetplugin.h"
#include "qaxwidgetextrainfo.h"
#include "qdesigneraxwidget.h"
#include "qaxwidgetpropertysheet.h"
#include "qaxwidgettaskmenu.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractformwindow.h>

#include <QtCore/qplugin.h>
#include <QtGui/qicon.h>
#include <QtAxContainer/QAxWidget>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QAxWidgetPlugin::QAxWidgetPlugin(QObject *parent) :
    QObject(parent)
{
}

QString QAxWidgetPlugin::name() const
{
    return u"QAxWidget"_s;
}

QString QAxWidgetPlugin::group() const
{
    return u"Containers"_s;
}

QString QAxWidgetPlugin::toolTip() const
{
    return tr("ActiveX control");
}

QString QAxWidgetPlugin::whatsThis() const
{
    return tr("ActiveX control widget");
}

QString QAxWidgetPlugin::includeFile() const
{
    return u"qaxwidget.h"_s;
}

QIcon QAxWidgetPlugin::icon() const
{
    return QIcon(QDesignerAxWidget::widgetIcon());
}

bool QAxWidgetPlugin::isContainer() const
{
    return false;
}

QWidget *QAxWidgetPlugin::createWidget(QWidget *parent)
{
    // Construction from Widget box or on a form?
    const bool isFormEditor = parent != nullptr
        && QDesignerFormWindowInterface::findFormWindow(parent) != nullptr;
    auto rc = new QDesignerAxPluginWidget(parent);
    if (!isFormEditor)
        rc->setDrawFlags(QDesignerAxWidget::DrawFrame|QDesignerAxWidget::DrawControl);
    return rc;
}

bool QAxWidgetPlugin::isInitialized() const
{
    return m_core != nullptr;
}

void QAxWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
    if (m_core != nullptr)
        return;

    m_core = core;

    QExtensionManager *mgr = core->extensionManager();
    ActiveXPropertySheetFactory::registerExtension(mgr);
    ActiveXTaskMenuFactory::registerExtension(mgr, Q_TYPEID(QDesignerTaskMenuExtension));
    QAxWidgetExtraInfoFactory *extraInfoFactory = new QAxWidgetExtraInfoFactory(core, mgr);
    mgr->registerExtensions(extraInfoFactory, Q_TYPEID(QDesignerExtraInfoExtension));
}

QString QAxWidgetPlugin::domXml() const
{
    return QStringLiteral(R"(<ui language="c++">
    <widget class="QAxWidget" name="axWidget">
        <property name="geometry">
            <rect>
                <x>0</x>
                <y>0</y>
                <width>80</width>
                <height>70</height>
            </rect>
        </property>
    </widget>
</ui>)");
}

QT_END_NAMESPACE
