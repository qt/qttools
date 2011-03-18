SOURCES = qdbusxml2cpp.cpp
DESTDIR = $$QT.designer.bins
TARGET = qdbusxml2cpp
QT = core xml
CONFIG += qdbus
CONFIG -= app_bundle
win32:CONFIG += console

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
