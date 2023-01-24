#! [1]
QT          += widgets uiplugin
#! [1]

QTDIR_build {
# This is only for the Qt build. Do not use externally. We mean it.
PLUGIN_TYPE = designer
PLUGIN_CLASS_NAME = WorldTimeClockPlugin
load(qt_plugin)
CONFIG += install_ok
} else {
# Public example:

TARGET      = $$qtLibraryTarget($$TARGET)
#! [0]
TEMPLATE    = lib
CONFIG     += plugin
#! [0]

#! [3]
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
#! [3]
}

#! [2]
HEADERS     = worldtimeclock.h \
              worldtimeclockplugin.h
SOURCES     = worldtimeclock.cpp \
              worldtimeclockplugin.cpp
#! [2]
