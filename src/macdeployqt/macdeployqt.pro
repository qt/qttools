include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(macdeployqt))

TEMPLATE = subdirs
SUBDIRS = macdeployqt macchangeqt
