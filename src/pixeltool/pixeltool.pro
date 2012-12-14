!build_pass:contains(QT_CONFIG, build_all): CONFIG += release
QT += widgets network

DESTDIR     = $$QT.designer.bins

mac {
    QMAKE_INFO_PLIST=Info_mac.plist
}

SOURCES += main.cpp qpixeltool.cpp
HEADERS += qpixeltool.h

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
