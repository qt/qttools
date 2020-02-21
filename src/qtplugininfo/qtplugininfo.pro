include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qtplugininfo))

SOURCES = qtplugininfo.cpp
QT = core
CONFIG += console

load(qt_app)
