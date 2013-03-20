option(host_build)
QT = core-private

qtHaveModule(qmldevtools) {
    QT += qmldevtools-private
} else {
    DEFINES += QT_NO_QML
}

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII

include(../shared/formats.pri)
include(../shared/proparser.pri)

SOURCES += \
    main.cpp \
    merge.cpp \
    ../shared/simtexth.cpp \
    \
    cpp.cpp \
    java.cpp \
    ui.cpp

qtHaveModule(qmldevtools): SOURCES += qdeclarative.cpp

HEADERS += \
    lupdate.h \
    ../shared/simtexth.h

qmake.name = QMAKE
qmake.value = $$shell_path($$QMAKE_QMAKE)
QT_TOOL_ENV += qmake

load(qt_tool)
