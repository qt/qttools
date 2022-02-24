TEMPLATE = subdirs

include($$OUT_PWD/../../../src/qdoc/qtqdoc-config.pri)
QT_FOR_CONFIG += qdoc-private
requires(qtConfig(qdoc))

SUBDIRS = \
    config \
    generatedoutput \
    utilities
