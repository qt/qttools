TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    lib \
    components \
    designer

CONFIG(shared,shared|static):SUBDIRS += plugins
