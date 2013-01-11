TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    uitools \
    lib \
    components \
    designer

contains(QT_CONFIG, shared): SUBDIRS += plugins
