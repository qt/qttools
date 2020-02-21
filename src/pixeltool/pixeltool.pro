include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(pixeltool))

QT += core-private gui-private widgets

mac {
    QMAKE_INFO_PLIST=Info_mac.plist
}

SOURCES += main.cpp qpixeltool.cpp
HEADERS += qpixeltool.h

load(qt_app)
