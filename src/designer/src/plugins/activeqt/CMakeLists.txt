# Generated from activeqt.pro.

#####################################################################
## QAxWidgetPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QAxWidgetPlugin
    OUTPUT_NAME qaxwidget
    PLUGIN_TYPE designer
    SOURCES
        qaxwidgetextrainfo.cpp qaxwidgetextrainfo.h
        qaxwidgetplugin.cpp qaxwidgetplugin.h
        qaxwidgetpropertysheet.cpp qaxwidgetpropertysheet.h
        qaxwidgettaskmenu.cpp qaxwidgettaskmenu.h
        qdesigneraxwidget.cpp qdesigneraxwidget.h
    LIBRARIES
        Qt::AxContainer
        Qt::Core
        Qt::Designer
        Qt::DesignerPrivate
        Qt::Gui
        Qt::Widgets
)

#### Keys ignored in scope 1:.:.:activeqt.pro:<TRUE>:
# OTHER_FILES = "activeqt.json"
