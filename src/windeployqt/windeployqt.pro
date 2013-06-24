option(host_build)
QT = core-private
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += main.cpp utils.cpp
HEADERS += utils.h

CONFIG += force_bootstrap console

INCLUDEPATH += $$QT_BUILD_TREE/src/corelib/global

win32: LIBS += -lShlwapi

load(qt_tool)
