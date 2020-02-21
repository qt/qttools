include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(qev))

QT += widgets

SOURCES += qev.cpp

load(qt_app)
