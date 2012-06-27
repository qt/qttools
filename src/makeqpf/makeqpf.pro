INCLUDEPATH += ../../src/3rdparty/harfbuzz/src
QT += widgets gui-private core-private
CONFIG += console
DESTDIR = $$QT.designer.bins

HEADERS += qpf2.h mainwindow.h
SOURCES += main.cpp qpf2.cpp mainwindow.cpp
DEFINES += QT_NO_FREETYPE
FORMS += mainwindow.ui
RESOURCES += makeqpf.qrc

target.path = $$[QT_INSTALL_BINS]
INSTALLS += target
