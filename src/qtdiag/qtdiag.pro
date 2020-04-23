include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qtdiag))

CONFIG += console
QT += core-private gui-private
qtConfig(opengl): QT += opengl

qtHaveModule(widgets): QT += widgets

qtHaveModule(network) {
    QT += network
    DEFINES += NETWORK_DIAG
}

SOURCES += main.cpp qtdiag.cpp
HEADERS += qtdiag.h

load(qt_app)
