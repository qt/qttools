option(host_build)
CONFIG += force_bootstrap

TARGET = qtd3dservice

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII WINRT_LIBRARY WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP

QMAKE_CXXFLAGS += -EHsc

LIBS += -lruntimeobject -lsetupapi -lcredui -lsecur32

SOURCES = \
    appxhandler.cpp \
    d3dservice.cpp \
    main.cpp \
    xaphandler.cpp \
    compilation.cpp \
    info.cpp

HEADERS = \
    d3dservice.h

include(../shared/corecon/corecon.pri)

load(qt_tool)
