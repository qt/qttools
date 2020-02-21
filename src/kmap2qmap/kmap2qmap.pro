include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(kmap2qmap))

QT = core input_support-private
CONFIG += console

SOURCES += main.cpp

load(qt_app)
