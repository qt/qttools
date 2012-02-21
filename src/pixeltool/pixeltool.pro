CONFIG  += qt warn_on
QT += widgets network

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

DESTDIR     = $$QT.designer.bins

mac {
    QMAKE_INFO_PLIST=Info_mac.plist
}

SOURCES += main.cpp qpixeltool.cpp
HEADERS += qpixeltool.h

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
