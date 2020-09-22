include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(macdeployqt))

TEMPLATE = subdirs
SUBDIRS = macdeployqt
