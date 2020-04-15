include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(designer))

TEMPLATE = subdirs

SUBDIRS = src
