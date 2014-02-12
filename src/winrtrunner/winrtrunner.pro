option(host_build)
CONFIG += force_bootstrap

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII WINRT_LIBRARY

SOURCES += main.cpp runner.cpp
HEADERS += runner.h runnerengine.h

DEFINES += RTRUNNER_NO_APPX

win32-msvc2012|win32-msvc2013 {
    SOURCES += appxengine.cpp
    HEADERS += appxengine.h
    LIBS += -lruntimeobject
    DEFINES -= RTRUNNER_NO_APPX
}

load(qt_tool)
