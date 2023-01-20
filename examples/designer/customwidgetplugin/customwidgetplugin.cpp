// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "analogclock.h"
#include "customwidgetplugin.h"

#include <QtPlugin>

using namespace Qt::StringLiterals;

//! [0]
AnalogClockPlugin::AnalogClockPlugin(QObject *parent)
    : QObject(parent)
{
}
//! [0]

//! [1]
void AnalogClockPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}
//! [1]

//! [2]
bool AnalogClockPlugin::isInitialized() const
{
    return initialized;
}
//! [2]

//! [3]
QWidget *AnalogClockPlugin::createWidget(QWidget *parent)
{
    return new AnalogClock(parent);
}
//! [3]

//! [4]
QString AnalogClockPlugin::name() const
{
    return u"AnalogClock"_s;
}
//! [4]

//! [5]
QString AnalogClockPlugin::group() const
{
    return u"Display Widgets [Examples]"_s;
}
//! [5]

//! [6]
QIcon AnalogClockPlugin::icon() const
{
    return {};
}
//! [6]

//! [7]
QString AnalogClockPlugin::toolTip() const
{
    return {};
}
//! [7]

//! [8]
QString AnalogClockPlugin::whatsThis() const
{
    return {};
}
//! [8]

//! [9]
bool AnalogClockPlugin::isContainer() const
{
    return false;
}
//! [9]

//! [10]
QString AnalogClockPlugin::domXml() const
{
    return uR"(
<ui language="c++">
  <widget class="AnalogClock" name="analogClock">
)"
//! [11]
R"(
    <property name="geometry">
      <rect>
        <x>0</x>
        <y>0</y>
        <width>100</width>
        <height>100</height>
      </rect>
    </property>
")
//! [11]
R"(
    <property name="toolTip">
      <string>The current time</string>
    </property>
    <property name="whatsThis">
      <string>The analog clock widget displays the current time.</string>
    </property>
  </widget>
</ui>
)"_s;
}
//! [10]

//! [12]
QString AnalogClockPlugin::includeFile() const
{
    return u"analogclock.h"_s;
}
//! [12]
