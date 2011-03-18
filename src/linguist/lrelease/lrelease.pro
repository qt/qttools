load(qt_module)

TEMPLATE        = app
TARGET          = lrelease
DESTDIR         = $$QT.designer.bins

QT              -= gui

CONFIG          += qt warn_on console
CONFIG          -= app_bundle

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
SOURCES += main.cpp

include(../shared/formats.pri)
include(../shared/proparser.pri)
include($$QT_SOURCE_TREE/tools/shared/symbian/epocroot.pri)

win32:LIBS += -ladvapi32   # for qsettings_win.cpp

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
