// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwebview_plugin.h"

#include <QtCore/qplugin.h>
#include <QWebView>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto toolTipC = "A widget for displaying a web page, from the Qt WebKit Widgets module."_L1;

QWebViewPlugin::QWebViewPlugin(QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
}

QString QWebViewPlugin::name() const
{
    return u"QWebView"_s;
}

QString QWebViewPlugin::group() const
{
    return u"Display Widgets"_s;
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
    return u"<QtWebKitWidgets/QWebView>"_s;
}

QIcon QWebViewPlugin::icon() const
{
    return QIcon(u":/qt-project.org/qwebview/images/qwebview.png"_s);
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
