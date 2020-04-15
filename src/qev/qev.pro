include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qev))

QT += widgets

SOURCES += qev.cpp

load(qt_app)
