load(qt_module)

TEMPLATE = app

DESTDIR     = $$QT.designer.bins

DEPENDPATH += .
INCLUDEPATH += . $$QT_SOURCE_TREE/src/3rdparty/harfbuzz/src
TARGET = qttracereplay
QT += core-private gui-private widgets widgets-private

# Input
SOURCES += main.cpp

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target

CONFIG += console
