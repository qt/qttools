QT += network help-private
TARGET = qcollectiongenerator
SOURCES += ../shared/helpgenerator.cpp \
    main.cpp \
    ../shared/collectionconfiguration.cpp
HEADERS += ../shared/helpgenerator.h \
    ../shared/collectionconfiguration.h
QTPLUGIN.platforms = qminimal

QMAKE_TARGET_DESCRIPTION = "Qt Help Collection File Generator"
load(qt_tool)
