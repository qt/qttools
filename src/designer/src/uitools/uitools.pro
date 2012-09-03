TARGET = QtUiTools
CONFIG += static
QT = core

load(qt_module)

HEADERS += quiloader.h
SOURCES += quiloader.cpp

DEFINES += \
    QFORMINTERNAL_NAMESPACE \
    QT_DESIGNER_STATIC

include(../lib/uilib/uilib.pri)

include(../sharedcomponents.pri)
