requires(qtHaveModule(sql))

include($$OUT_PWD/../../qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(assistant))

TEMPLATE = subdirs

SUBDIRS += \
           help \
           assistant \
           qhelpgenerator \
           qcollectiongenerator

assistant.depends = help
qhelpgenerator.depends = help

qtNomakeTools( \
    assistant \
    qhelpgenerator \
    qcollectiongenerator \
)
