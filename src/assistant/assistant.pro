requires(qtHaveModule(sql))

include($$OUT_PWD/../../src/global/qttools-config.pri)
QT_FOR_CONFIG += tools-private
requires(qtConfig(assistant))

TEMPLATE = subdirs

SUBDIRS += \
           help \
           assistant \
           qhelpgenerator

assistant.depends = help
qhelpgenerator.depends = help

qtNomakeTools( \
    assistant \
    qhelpgenerator \
)
