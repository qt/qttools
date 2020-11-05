requires(qtHaveModule(sql))

include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(help))

TEMPLATE = subdirs

SUBDIRS += \
           help \
           qhelpgenerator \
           qcollectiongenerator

qhelpgenerator.depends = help

qtNomakeTools( \
    qhelpgenerator \
    qcollectiongenerator \
)
