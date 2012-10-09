TEMPLATE    = lib
TARGET      = qaxwidget
DESTDIR = $$QT.designer.plugins/designer
CONFIG     += qaxcontainer qt warn_on plugin
QT         += widgets designer-private

include(../plugins.pri)

INCLUDEPATH += $$QT.activeqt.sources/shared/ \
               $$QT.activeqt.sources/container \
               ../../lib/uilib \
               $$QT.designer.includes

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

OTHER_FILES += activeqt.json
