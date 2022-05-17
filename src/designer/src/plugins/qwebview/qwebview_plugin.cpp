// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qwebview_plugin.h"

#include <QtCore/qplugin.h>
#include <QWebView>

static const char *toolTipC = "A widget for displaying a web page, from the Qt WebKit Widgets module.";

QT_BEGIN_NAMESPACE

QWebViewPlugin::QWebViewPlugin(QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
}

QString QWebViewPlugin::name() const
{
    return QStringLiteral("QWebView");
}

QString QWebViewPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString QWebViewPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString QWebViewPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString QWebViewPlugin::includeFile() const
{
    return QStringLiteral("<QtWebKitWidgets/QWebView>");
}

QIcon QWebViewPlugin::icon() const
{
    return QIcon(QStringLiteral(":/qt-project.org/qwebview/images/qwebview.png"));
}

bool QWebViewPlugin::isContainer() const
{
    return false;
}

QWidget *QWebViewPlugin::createWidget(QWidget *parent)
{
    return new QWebView(parent);
}

bool QWebViewPlugin::isInitialized() const
{
    return m_initialized;
}

void QWebViewPlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

QString QWebViewPlugin::domXml() const
{
    return QStringLiteral(R"(
<ui language="c++">
    <widget class="QWebView" name="webView">
        <property name="url">
            <url>
                <string>about:blank</string>
            </url>
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

QT_END_NAMESPACE
