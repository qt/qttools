include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(winrtrunner))

option(host_build)
CONFIG += force_bootstrap

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII WINRT_LIBRARY

SOURCES += \
    main.cpp \
    runner.cpp \
    appxengine.cpp \
    appxlocalengine.cpp

HEADERS += \
    runner.h \
    runnerengine.h \
    appxengine.h \
    appxengine_p.h \
    appxlocalengine.h

LIBS += -lruntimeobject -lwsclient -lShlwapi -lurlmon -lxmllite -lcrypt32

include(../shared/winutils/winutils.pri)

QMAKE_TARGET_DESCRIPTION = "Qt WinRT Runner"
load(qt_tool)
