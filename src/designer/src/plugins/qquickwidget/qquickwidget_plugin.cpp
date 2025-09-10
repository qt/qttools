// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquickwidget_plugin.h"

#include <QtDesigner/default_extensionfactory.h>
#include <QtDesigner/qextensionmanager.h>

#include <QtCore/qplugin.h>
#include <QtCore/qdebug.h>
#include <QtQuickWidgets/qquickwidget.h>

#include <QtQuick/QQuickWindow>


QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QQuickWidgetPlugin::QQuickWidgetPlugin(QObject *parent)
    : QObject(parent)
{
}

QString QQuickWidgetPlugin::name() const
{
    return u"QQuickWidget"_s;
}

QString QQuickWidgetPlugin::group() const
{
    return u"Display Widgets"_s;
}

QString QQuickWidgetPlugin::toolTip() const
{
    return u"A widget for displaying a Qt Quick 2 user interface."_s;
}

QString QQuickWidgetPlugin::whatsThis() const
{
    return toolTip();
}

QString QQuickWidgetPlugin::includeFile() const
{
    return u"<QtQuickWidgets/QQuickWidget>"_s;
}

QIcon QQuickWidgetPlugin::icon() const
{
    return QIcon(u":/qt-project.org/qquickwidget/images/qquickwidget.png"_s);
}

bool QQuickWidgetPlugin::isContainer() const
{
    return false;
}

QWidget *QQuickWidgetPlugin::createWidget(QWidget *parent)
{
    QQuickWidget *result = new QQuickWidget(parent);
    connect(result, &QQuickWidget::sceneGraphError,
            this, &QQuickWidgetPlugin::sceneGraphError);
    return result;
}

bool QQuickWidgetPlugin::isInitialized() const
{
    return m_initialized;
}

void QQuickWidgetPlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

QString QQuickWidgetPlugin::domXml() const
{
    return QStringLiteral(R"(
<ui language="c++">
    <widget class="QQuickWidget" name="quickWidget">
        <property name="resizeMode">
            <enum>QQuickWidget::SizeRootObjectToView</enum>
        </property>
        <property name="geometry">
            <rect>
                <x>0</x>
                <y>0</y>
                <width>300</width>
                <height>200</height>
            </rect>
        </property>
    </widget>
</ui>
)");
}

void QQuickWidgetPlugin::sceneGraphError(QQuickWindow::SceneGraphError, const QString &message)
{
    qWarning() << Q_FUNC_INFO << ':' << message;
}

QT_END_NAMESPACE
