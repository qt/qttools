option(host_build)
QT = core

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

include(../shared/proparser.pri)

DEFINES += PROEVALUATOR_DEBUG

HEADERS += \
    ../shared/qrcreader.h

SOURCES += \
    ../shared/qrcreader.cpp \
    main.cpp

qmake.name = QMAKE
qmake.value = $$shell_path($$QMAKE_QMAKE)
QT_TOOL_ENV += qmake

QMAKE_TARGET_DESCRIPTION = "Qt Linguist QMake Project Dump Tool"
load(qt_tool)
