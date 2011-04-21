SOURCES = qdbus.cpp
DESTDIR = $$QT.designer.bins
TARGET = qdbus
QT = core dbus-private xml
CONFIG -= app_bundle
win32:CONFIG += console

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
