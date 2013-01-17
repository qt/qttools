TARGET      = qaxwidget
QT         += widgets designer-private axcontainer

PLUGIN_CLASS_NAME = QAxWidgetPlugin
include(../plugins.pri)

INCLUDEPATH += ../../lib/uilib \
               $$QT.designer.includes

SOURCES += qaxwidgetextrainfo.cpp \
qaxwidgetplugin.cpp \
qdesigneraxwidget.cpp \
qaxwidgetpropertysheet.cpp \
qaxwidgettaskmenu.cpp

HEADERS += qaxwidgetextrainfo.h \
qaxwidgetplugin.h \
qdesigneraxwidget.h \
qaxwidgetpropertysheet.h \
qaxwidgettaskmenu.h

OTHER_FILES += activeqt.json
