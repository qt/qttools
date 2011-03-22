TARGET      = $$qtLibraryTarget(qaxwidget)
TEMPLATE    = lib
DESTDIR = $$QT.designer.plugins/designer

CONFIG     += qt warn_on qaxcontainer plugin designer debug_and_release
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

INCLUDEPATH += $$QT.activeqt.sources/shared/ \
               $$QT.activeqt.sources/container \
               ../../lib/uilib \
               $$QT.designer.includes

# Input
SOURCES += qaxwidgetextrainfo.cpp \
qaxwidgetplugin.cpp \
qdesigneraxwidget.cpp \
qaxwidgetpropertysheet.cpp \
qaxwidgettaskmenu.cpp \
    $$QT.activeqt.sources/shared/qaxtypes.cpp

HEADERS += qaxwidgetextrainfo.h \
qaxwidgetplugin.h \
qdesigneraxwidget.h \
qaxwidgetpropertysheet.h \
qaxwidgettaskmenu.h \
    $$QT.activeqt.sources/shared/qaxtypes.h

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
