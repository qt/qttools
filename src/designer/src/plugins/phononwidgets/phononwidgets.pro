TEMPLATE    = lib
TARGET      = phononwidgets
CONFIG     += qt warn_on plugin
QT         += widgets phonon designer-private

include(../plugins.pri)
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

SOURCES += videoplayerplugin.cpp \
    videoplayertaskmenu.cpp \
    seeksliderplugin.cpp \
    volumesliderplugin.cpp \
    phononcollection.cpp

HEADERS += videoplayerplugin.h \
    videoplayertaskmenu.h \
    seeksliderplugin.h \
    volumesliderplugin.h

OTHER_FILES = "phonon.json"

RESOURCES += phononwidgets.qrc
