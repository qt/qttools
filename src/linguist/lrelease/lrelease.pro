load(qt_build_config)

TEMPLATE        = app
TARGET          = lrelease
DESTDIR         = $$QT.designer.bins

QT              -= gui
QT              += core-private

CONFIG          += qt warn_on console
CONFIG          -= app_bundle

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
SOURCES += main.cpp

include(../shared/formats.pri)
include(../shared/proparser.pri)

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
