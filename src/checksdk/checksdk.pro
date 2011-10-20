TEMPLATE = app
DESTDIR = $$QT.designer.bins
TARGET = checksdk
DEPENDPATH += .
INCLUDEPATH += .
QT = core
CONFIG += console

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}


# Input
SOURCES += \
           main.cpp \
           cesdkhandler.cpp

HEADERS += \
           cesdkhandler.h
