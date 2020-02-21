include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(designer))

TEMPLATE = subdirs

SUBDIRS = src
