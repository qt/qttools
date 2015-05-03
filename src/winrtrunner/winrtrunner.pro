option(host_build)
CONFIG += force_bootstrap

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII WINRT_LIBRARY

SOURCES += main.cpp runner.cpp
HEADERS += runner.h runnerengine.h

DEFINES += RTRUNNER_NO_APPXLOCAL RTRUNNER_NO_APPXPHONE

win32-msvc2013 {
    SOURCES += appxengine.cpp appxlocalengine.cpp appxphoneengine.cpp
    HEADERS += appxengine.h appxengine_p.h appxlocalengine.h appxphoneengine.h
    LIBS += -lruntimeobject -lwsclient -lShlwapi -lurlmon -lxmllite
    DEFINES -= RTRUNNER_NO_APPXLOCAL RTRUNNER_NO_APPXPHONE

    include(../shared/corecon/corecon.pri)
}

load(qt_tool)
