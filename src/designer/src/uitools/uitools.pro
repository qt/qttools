load(qt_module)

TARGET = QtUiTools
QPRO_PWD = $$PWD

CONFIG += qt staticlib module
QT = core

unix|win32-g++*:QMAKE_PKGCONFIG_REQUIRES += QtCore

load(qt_module_config)

HEADERS += quiloader.h
SOURCES += quiloader.cpp

DEFINES += \
    QFORMINTERNAL_NAMESPACE \
    QT_DESIGNER_STATIC

include(../lib/uilib/uilib.pri)

include(../sharedcomponents.pri)
