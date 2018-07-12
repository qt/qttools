QT += network help-private

QTPLUGIN.platforms = qminimal

SOURCES += ../shared/collectionconfiguration.cpp \
           ../shared/helpgenerator.cpp \
           collectionconfigreader.cpp \
           main.cpp

HEADERS += ../shared/collectionconfiguration.h \
           ../shared/helpgenerator.h \
           collectionconfigreader.h

QMAKE_TARGET_DESCRIPTION = "Qt Compressed Help File Generator"
load(qt_tool)
