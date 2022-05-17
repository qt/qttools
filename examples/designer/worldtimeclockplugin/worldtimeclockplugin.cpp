// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "worldtimeclock.h"
#include "worldtimeclockplugin.h"

#include <QtPlugin>

WorldTimeClockPlugin::WorldTimeClockPlugin(QObject *parent)
    : QObject(parent)
{
}

void WorldTimeClockPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool WorldTimeClockPlugin::isInitialized() const
{
    return initialized;
}

QWidget *WorldTimeClockPlugin::createWidget(QWidget *parent)
{
    return new WorldTimeClock(parent);
}

QString WorldTimeClockPlugin::name() const
{
    return QStringLiteral("WorldTimeClock");
}

QString WorldTimeClockPlugin::group() const
{
    return QStringLiteral("Display Widgets [Examples]");
}

QIcon WorldTimeClockPlugin::icon() const
{
    return QIcon();
}

QString WorldTimeClockPlugin::toolTip() const
{
    return QString();
}

QString WorldTimeClockPlugin::whatsThis() const
{
    return QString();
}

bool WorldTimeClockPlugin::isContainer() const
{
    return false;
}

QString WorldTimeClockPlugin::domXml() const
{
    return QLatin1String(R"(
<ui language="c++">
  <widget class="WorldTimeClock" name="worldTimeClock">
    <property name="geometry">
      <rect>
        <x>0</x>
        <y>0</y>
        <width>100</width>
        <height>100</height>
      </rect>
    </property>
  </widget>
</ui>
)");
}

QString WorldTimeClockPlugin::includeFile() const
{
    return QStringLiteral("worldtimeclock.h");
}
