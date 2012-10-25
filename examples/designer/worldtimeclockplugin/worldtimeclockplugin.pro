#! [0]
QT          += widgets designer
#! [0]

QTDIR_build {
# This is only for the Qt build. Do not use externally. We mean it.
PLUGIN_TYPE = designer
load(qt_plugin)
} else {
# Public example:

TARGET      = $$qtLibraryTarget($$TARGET)
#! [1]
CONFIG     += plugin
TEMPLATE    = lib
#! [1]

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

}

#! [2]
HEADERS     = worldtimeclock.h \
              worldtimeclockplugin.h
SOURCES     = worldtimeclock.cpp \
              worldtimeclockplugin.cpp
#! [2]

# install
sources.files = $$SOURCES $$HEADERS *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qttools/designer/worldtimeclockplugin
INSTALLS += sources
