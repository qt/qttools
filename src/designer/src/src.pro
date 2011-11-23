TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    uitools \
    lib \
    components \
    designer

# QtCore sources are directly included by lib.
!exists($$QT.core.sources/../tools/rcc/rcc.pri) {
    warning(Designer cannot be compiled because QtCore sources are not available; \
            QT.core.sources == $$QT.core.sources)
    SUBDIRS -= lib components designer
}

CONFIG(shared,shared|static):SUBDIRS += plugins
