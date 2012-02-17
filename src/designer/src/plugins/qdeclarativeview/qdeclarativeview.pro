TEMPLATE    = lib
TARGET      = qdeclarativeview
CONFIG     += qt warn_on plugin designer
QT         += quick1 widgets

include(../plugins.pri)
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

SOURCES += qdeclarativeview_plugin.cpp
HEADERS += qdeclarativeview_plugin.h
OTHER_FILES += qdeclarativeview.json
