load(qt_module)

TEMPLATE = app

DESTDIR     = $$QT.designer.bins

DEPENDPATH += .
INCLUDEPATH += . $$QT.gui.sources/../3rdparty/harfbuzz/src
TARGET = qttracereplay
QT += core-private gui-private widgets widgets-private
CONFIG += console

SOURCES += main.cpp

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
