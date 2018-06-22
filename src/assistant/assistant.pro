TEMPLATE = subdirs

SUBDIRS += \
           help \
           assistant \
           qhelpgenerator \
           qcollectiongenerator

assistant.depends = help
qhelpgenerator.depends = help
qcollectiongenerator.depends = help

qtNomakeTools( \
    assistant \
    qhelpgenerator \
    qcollectiongenerator \
)
