# Generated from qwebview.pro.

#####################################################################
## qwebview Plugin:
#####################################################################

qt_add_plugin(qwebview
    TYPE designer
    CLASS_NAME QWebViewPlugin
    SOURCES
        qwebview_plugin.cpp qwebview_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Designer
        Qt::Gui
        Qt::Widgets
        webkitwidgets
)

# Resources:
set(qwebview_plugin_resource_files
    "images/qwebview.png"
)

qt_add_resource(qwebview "qwebview_plugin"
    PREFIX
        "/qt-project.org/qwebview"
    FILES
        ${qwebview_plugin_resource_files}
)


#### Keys ignored in scope 1:.:.:qwebview.pro:<TRUE>:
# OTHER_FILES = "qwebview.json"
