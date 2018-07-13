option(host_build)
QT = core
DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

INCLUDEPATH += ../shared

HEADERS += \
    ../shared/runqttool.h

SOURCES += \
    ../shared/runqttool.cpp \
    main.cpp

mingw {
    RC_FILE = lupdate-pro.rc
}

qmake.name = QMAKE
qmake.value = $$shell_path($$QMAKE_QMAKE)
QT_TOOL_ENV += qmake

QMAKE_TARGET_DESCRIPTION = "Qt Translation File Update Tool for QMake Projects"
load(qt_tool)
