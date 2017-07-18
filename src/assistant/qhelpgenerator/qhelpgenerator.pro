QT += network help-private

QTPLUGIN.platforms = qminimal

SOURCES += ../shared/helpgenerator.cpp \
           main.cpp

HEADERS += ../shared/helpgenerator.h

QMAKE_TARGET_DESCRIPTION = "Qt Compressed Help File Generator"
load(qt_tool)
