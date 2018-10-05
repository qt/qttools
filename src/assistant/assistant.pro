requires(qtHaveModule(sql))
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
