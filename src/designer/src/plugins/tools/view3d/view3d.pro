
TEMPLATE = lib
QT      += opengl widgets
CONFIG  += qt warn_on plugin
TARGET = view3d

include(../../plugins.pri)

SOURCES += view3d.cpp view3d_tool.cpp view3d_plugin.cpp
HEADERS += view3d.h view3d_tool.h view3d_plugin.h view3d_global.h
