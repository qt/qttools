
TEMPLATE = app
DESTDIR = $$QT.designer.bins
QT = core
CONFIG += console
CONFIG -= app_bundle

DEPENDPATH += $$QT_SOURCE_TREE/src/gui/embedded
INCLUDEPATH += $$QT_SOURCE_TREE/src/gui/embedded

SOURCES += main.cpp
