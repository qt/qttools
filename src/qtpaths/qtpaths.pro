include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qtpaths))

SOURCES = qtpaths.cpp
QT = core
win32:CONFIG += console

load(qt_app)
