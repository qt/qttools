option(host_build)
QT = core
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

HEADERS += \
    ../shared/runqttool.h

SOURCES += \
    ../shared/runqttool.cpp \
    main.cpp

INCLUDEPATH += ../shared

qmake.name = QMAKE
qmake.value = $$shell_path($$QMAKE_QMAKE)
QT_TOOL_ENV += qmake

QMAKE_TARGET_DESCRIPTION = "Qt Translation File Compiler for QMake Projects"
load(qt_tool)
