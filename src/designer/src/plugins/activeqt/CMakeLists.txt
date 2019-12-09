# Generated from activeqt.pro.

#####################################################################
## qaxwidget Plugin:
#####################################################################

qt_add_plugin(qaxwidget
    TYPE designer
    CLASS_NAME QAxWidgetPlugin
    SOURCES
        qaxwidgetextrainfo.cpp qaxwidgetextrainfo.h
        qaxwidgetplugin.cpp qaxwidgetplugin.h
        qaxwidgetpropertysheet.cpp qaxwidgetpropertysheet.h
        qaxwidgettaskmenu.cpp qaxwidgettaskmenu.h
        qdesigneraxwidget.cpp qdesigneraxwidget.h
    LIBRARIES
        Qt::DesignerPrivate
    PUBLIC_LIBRARIES
        Qt::AxContainer
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::Widgets
)

#### Keys ignored in scope 1:.:.:activeqt.pro:<TRUE>:
# OTHER_FILES = "activeqt.json"
